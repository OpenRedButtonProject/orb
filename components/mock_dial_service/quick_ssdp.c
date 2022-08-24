/*
* Copyright (c) 2014 Netflix, Inc.
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* Redistributions of source code must retain the above copyright notice, this
* list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice,
* this list of conditions and the following disclaimer in the documentation
* and/or other materials provided with the distribution.
*
* THIS SOFTWARE IS PROVIDED BY NETFLIX, INC. AND CONTRIBUTORS "AS IS" AND ANY
* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL NETFLIX OR CONTRIBUTORS BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
* THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#define ANDROID_DEBUG 1

#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>
#include "mongoose.h"
#include <stdbool.h>
#include <pthread.h>
#include "log.h"
#include <sys/time.h>
#include <fcntl.h>

// TODO: Partners should define this port
#define SSDP_PORT (56780)
static char gBuf[4096];

// TODO: Partners should get the friendlyName from the system and insert here.
// TODO: Partners should get the UUID from the system and insert here.
static const char ddxml[] = ""
    "<?xml version=\"1.0\"?>"
    "<root"
    " xmlns=\"urn:schemas-upnp-org:device-1-0\""
    " xmlns:r=\"urn:restful-tv-org:schemas:upnp-dd\">"
    " <specVersion>"
    " <major>1</major>"
    " <minor>0</minor>"
    " </specVersion>"
    " <device>"
    " <deviceType>urn:schemas-upnp-org:device:tvdevice:1</deviceType>"
    " <friendlyName>%s</friendlyName>"
    " <manufacturer> </manufacturer>"
    " <modelName>%s</modelName>"
    " <UDN>uuid:%s</UDN>"
    " </device>"
    "</root>";

// TODO: Partners should use appropriate timeout (in seconds) if hardware supports WoL.
static const short wakeup_timeout = 10;

// TODO: Partners should get the UUID from the system and insert here.
static const char ssdp_reply[] = "HTTP/1.1 200 OK\r\n"
    "LOCATION: http://%s:%d/dd.xml\r\n"
    "CACHE-CONTROL: max-age=1800\r\n"
    "EXT:\r\n"
    "BOOTID.UPNP.ORG: 1\r\n"
    "SERVER: Linux/2.6 UPnP/1.1 quick_ssdp/1.1\r\n"
    "ST: urn:dial-multiscreen-org:service:dial:1\r\n"
    "USN: uuid:%s::"
    "urn:dial-multiscreen-org:service:dial:1\r\n"
    "%s"
    "\r\n";

static const char wakeup_header[] = "WAKEUP: MAC=%s;Timeout=%d\r\n";
#define STR_TIMEOUTLEN 6 /* Longest is 32767 */
#define HW_ADDRSTRLEN 18
static char ip_addr[INET6_ADDRSTRLEN] = "127.0.0.1";
static int dial_port = 0;
static int my_port = 0;
static char friendly_name[256];
static char uuid[256];
static char model_name[256];
static struct mg_context *ctx;
static int stop_flag = 0;
static pthread_mutex_t *mutex = NULL;

bool wakeOnWifiLan=true;

static void close_socket(int sock) {
  char buf[BUFSIZ];
  int n;
  int flags;

  // Send FIN to the client
  shutdown(sock, SHUT_WR);

  flags = fcntl(sock, F_GETFL, 0);
  fcntl(sock, F_SETFL, flags | O_NONBLOCK);

  // Read and discard pending data.
  do {
    n = recv(sock, buf, sizeof(buf), 0);
  } while (n > 0);

  // Now we know that our FIN is ACK-ed, safe to close
  close(sock);
}

static void *request_handler(enum mg_event event,
                             struct mg_connection *conn,
                             const struct mg_request_info *request_info) {
    if (event == MG_NEW_REQUEST) {
        if (!strcmp(request_info->uri, "/dd.xml") &&
            !strcmp(request_info->request_method, "GET")) {
            mg_printf(conn, "HTTP/1.1 200 OK\r\n"
                      "Content-Type: text/xml\r\n"
                      "Application-URL: http://%s:%d/apps/\r\n"
                      "\r\n", ip_addr, dial_port);
            mg_printf(conn, ddxml, friendly_name, model_name, uuid);
        } else {
            mg_send_http_error(conn, 404, "Not Found", "Not Found");
        }
        return "done";
    }
    return NULL;
}

/**
 * Returns the local hardware address (e.g. MAC address). On macOS the "en0"
 * interface is used. On other platforms the first non-loopback interface is
 * used.
 *
 * As a side-effect, the local global ip_addr is also populated.
 *
 * (Are these choices of interface really the right ones? Seems risky for
 * multi-homed systems.)
 *
 * @return the local hardware address or NULL if it does not exist, cannot
 *         be retrieved, or out-of-memory. The caller must free the returned
 *         memory.
 */
static char * get_local_address() {
    struct ifconf ifc;
    char buf[4096];
    char * hw_addr = NULL;
    int s, i;

    if (-1 == (s = socket(AF_INET, SOCK_DGRAM, 0))) {
        LOG(LOG_ERROR,"quick_ssdp: socket");
        exit(1);
    }
    ifc.ifc_len = sizeof(buf);
    ifc.ifc_buf = buf;
    if (0 > ioctl(s, SIOCGIFCONF, &ifc)) {
        LOG(LOG_ERROR,"quick_ssdp: SIOCGIFCONF");
        exit(1);
    }
    if (ifc.ifc_len == sizeof(buf)) {
        LOG(LOG_DEBUG,"quick_ssdp: SIOCGIFCONF output too long");
        exit(1);
    }
    for (i = 0; i < ifc.ifc_len/sizeof(ifc.ifc_req[0]); i++) {
        strncpy(ip_addr,
               inet_ntoa(((struct sockaddr_in *)(&ifc.ifc_req[i].ifr_addr))->sin_addr), sizeof(ip_addr) - 1);
        if (0 > ioctl(s, SIOCGIFFLAGS, &ifc.ifc_req[i])) {
            LOG(LOG_ERROR,"quick_ssdp: SIOCGIFFLAGS");
            exit(1);
        }
        if (ifc.ifc_req[i].ifr_flags & IFF_LOOPBACK) {
            // don't use loopback interfaces
            continue;
        }
        if (0 > ioctl(s, SIOCGIFHWADDR, &ifc.ifc_req[i])) {
            LOG(LOG_ERROR,"quick_ssdp: SIOCGIFHWADDR");
            exit(1);
        }
        // FIXME: How do I figure out what type of interface this is in order to
        // cast the struct sockaddr ifr_hraddr to the proper type and extract the
        // hardware address?
        //
        // Make sure this is correct for the target device and platform.
        hw_addr = (char*)malloc(HW_ADDRSTRLEN + 1);
        if (hw_addr == NULL)
            break;
        sprintf(hw_addr, "%02x:%02x:%02x:%02x:%02x:%02x",
                (unsigned char)ifc.ifc_req[i].ifr_hwaddr.sa_data[0],
                (unsigned char)ifc.ifc_req[i].ifr_hwaddr.sa_data[1],
                (unsigned char)ifc.ifc_req[i].ifr_hwaddr.sa_data[2],
                (unsigned char)ifc.ifc_req[i].ifr_hwaddr.sa_data[3],
                (unsigned char)ifc.ifc_req[i].ifr_hwaddr.sa_data[4],
                (unsigned char)ifc.ifc_req[i].ifr_hwaddr.sa_data[5]);
        break;
    }
    close(s);
    return hw_addr;
}

static void handle_mcast(char *hw_addr) {
    int s, one = 1, bytes;
    socklen_t addrlen;
    struct sockaddr_in saddr = {0};
    struct ip_mreq mreq = {0};
    char wakeup_buf[sizeof(wakeup_header) + HW_ADDRSTRLEN + STR_TIMEOUTLEN] = {0, };
    char send_buf[sizeof(ssdp_reply) + INET_ADDRSTRLEN + 256 + 256 + sizeof(wakeup_buf)] = {0,};
    int send_size;
    if (-1 < wakeup_timeout && wakeOnWifiLan) {
        snprintf(wakeup_buf, sizeof(wakeup_buf), wakeup_header, hw_addr, wakeup_timeout);
    }
    send_size = snprintf(send_buf, sizeof(send_buf), ssdp_reply, ip_addr, my_port, uuid, wakeup_buf);
    if (-1 == (s = socket(AF_INET, SOCK_DGRAM, 0))) {
        LOG(LOG_ERROR,"quick_ssdp: socket");
        return;
        //exit(1);
    }
    if (-1 == setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one))) {
        LOG(LOG_ERROR,"quick_ssdp: reuseaddr");
        return;
        //exit(1);
    }
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = inet_addr("239.255.255.250");
    saddr.sin_port = htons(1900);

    if (-1 == bind(s, (struct sockaddr *)&saddr, sizeof(saddr))) {
        LOG(LOG_ERROR,"quick_ssdp: bind");
        return;
        //exit(1);
    }
    mreq.imr_multiaddr.s_addr = inet_addr("239.255.255.250");
    mreq.imr_interface.s_addr = inet_addr(ip_addr);
    if (-1 == setsockopt(s, IPPROTO_IP, IP_ADD_MEMBERSHIP,
                         &mreq, sizeof(mreq))) {
        LOG(LOG_ERROR,"quick_ssdp: add_membership");
        return;
        //exit(1);
    }
    LOG(LOG_DEBUG,"quick_ssdp: Starting Multicast handling on 239.255.255.250\n");
    
    while (1) {
      struct timeval timeout = {1, 0};
      fd_set readSet;
      FD_ZERO(&readSet);
      FD_SET(s, &readSet);

      if (select(s+1, &readSet, NULL, NULL, &timeout) >= 0)
      {
        pthread_mutex_lock(mutex);
        if (stop_flag != 0) {
            LOG(LOG_ERROR,"quick_ssdp: stop_flag");
            pthread_mutex_unlock(mutex);
            break;
        }

        pthread_mutex_unlock(mutex);
        if (FD_ISSET(s, &readSet))
        {
            bytes = recvfrom(s, gBuf, sizeof(gBuf) - 1, 0,
                                    (struct sockaddr *)&saddr, &addrlen);
            gBuf[bytes] = 0;
            addrlen = sizeof(saddr);
            // sophisticated SSDP parsing algorithm
            if (strstr(gBuf, "urn:dial-multiscreen-org:service:dial:1")) {
                LOG(LOG_DEBUG,"quick_ssdp: Sending SSDP reply to %s:%d\n",
                    inet_ntoa(saddr.sin_addr), ntohs(saddr.sin_port));
                if (-1 == sendto(s, send_buf, send_size, 0, (struct sockaddr *)&saddr, addrlen)) {
                    LOG(LOG_ERROR,"quick_ssdp: sendto");
                }
            }
        }
      }
    }

    close_socket(s);
    mg_stop(ctx);
    LOG(LOG_ERROR,"quick_ssdp: mg_stop");
}

void run_ssdp(int port, const char *pFriendlyName, const char * pModelName, const char *pUuid, const char *pIpAddress, const char *pMacAddress) {
    char *hw_addr = (char*)malloc(HW_ADDRSTRLEN + 1);
    strcpy(hw_addr, pMacAddress);
    strncpy(ip_addr, pIpAddress, sizeof(ip_addr) - 1);
    struct sockaddr sa;
    socklen_t len = sizeof(sa);
    if(pFriendlyName) {
        strncpy(friendly_name, pFriendlyName, sizeof(friendly_name));
        friendly_name[sizeof(friendly_name) - 1] = '\0';
    } else {
        strcpy(friendly_name, "DIAL server sample");
    }
    if(pModelName) {
        strncpy(model_name, pModelName, sizeof(model_name));
        model_name[sizeof(model_name) - 1] = '\0';
    } else {
        strcpy(model_name, "deadbeef-dead-beef-dead-beefdeadbeef");
    }
    if(pUuid) {
        strncpy(uuid, pUuid, sizeof(uuid));
        uuid[sizeof(uuid) - 1] = '\0';
    } else {
        strcpy(uuid, "deadbeef-dead-beef-dead-beefdeadbeef");
    }
    dial_port = port;
    //hw_addr = get_local_address();
    if (hw_addr == NULL) {
        LOG(LOG_DEBUG,"quick_ssdp: Unable to retrieve hardware address.");
        return;
    }

    if (mutex == NULL) {
        mutex = malloc(sizeof(pthread_mutex_t));
        if (pthread_mutex_init(mutex, NULL) != 0) {
            LOG(LOG_DEBUG,"quick_ssdp: \n mutex init has failed\n");
            free(mutex);
            mutex = NULL;
            free(hw_addr);
            hw_addr = NULL;
            return;
        }
    }

    //pthread_mutex_lock(mutex);
    stop_flag = 0;
    ctx = mg_start(&request_handler, NULL, rand() % 40000 + 10000);
    pthread_mutex_unlock(mutex);

    if (ctx == NULL) {
        LOG(LOG_DEBUG,"quick_ssdp: Unable to start SSDP master listening thread.");
    } else {
        if (mg_get_listen_addr(ctx, &sa, &len)) {
            my_port = ntohs(((struct sockaddr_in *)&sa)->sin_port);
        }
        LOG(LOG_DEBUG,"quick_ssdp: SSDP listening on %s:%d\n", ip_addr, my_port);

        handle_mcast(hw_addr);
    }
    free(hw_addr); hw_addr = NULL;
}

void stop_ssdp() {
    if (mutex != NULL) {
        pthread_mutex_lock(mutex);
        stop_flag = 1;
        pthread_mutex_unlock(mutex);
    }
}