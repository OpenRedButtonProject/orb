/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "UdpSocketService.h"
#include "media_synchroniser.h"

#include <iostream>

#define    LWS_PROTOCOL_LIST_TERM   { NULL, NULL, 0, 0, 0, NULL, 0 }

namespace NetworkServices {
#define VHOST_NAME "localhost"
#define SSL_CERT_FILEPATH "todo.cert"
#define SSL_PRIVATE_KEY_FILEPATH "todo.key"
#define SECS_SINCE_VALID_PING 3
#define SECS_SINCE_VALID_HANGUP 10
#define RX_BUFFER_SIZE 4096

UdpSocketService::UdpSocketService(const std::string &protocol_name, int port,
                                   bool use_ssl) :
    stop_(true),
    protocol_name_(protocol_name),
    use_ssl_(use_ssl),
#if LWS_VERSION_4 == 1
    retry_{.secs_since_valid_ping = SECS_SINCE_VALID_PING, .secs_since_valid_hangup =
               SECS_SINCE_VALID_HANGUP},
#endif
    protocols_{Protocol(protocol_name_.c_str()), LWS_PROTOCOL_LIST_TERM},
    context_(nullptr),
    port_{port},
    vhost_{nullptr}
{
    info_ =
    {
        .protocols = protocols_,
        .port = CONTEXT_PORT_NO_LISTEN_SERVER,
        .options = LWS_SERVER_OPTION_EXPLICIT_VHOSTS
            /* | LWS_SERVER_OPTION_VHOST_UPG_STRICT_HOST_CHECK */,
        .vhost_name = VHOST_NAME,
#if LWS_VERSION_4 == 1
        .retry_and_idle_policy = &retry_,
#endif
    };
    if (use_ssl_)
    {
        info_.options |= LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
        info_.ssl_cert_filepath = SSL_CERT_FILEPATH;
        info_.ssl_private_key_filepath = SSL_PRIVATE_KEY_FILEPATH;
    }
    lws_set_log_level(LLL_ERR | LLL_WARN /*| LLL_NOTICE */, nullptr);
}

UdpSocketService::~UdpSocketService()
{
    mutex_.lock();
    ClearWriteQueueMap();
    mutex_.unlock();
}

bool UdpSocketService::Start()
{
    bool bRet = false;
    mutex_.lock();
    if (context_ == nullptr && (context_ = lws_create_context(&info_)) != nullptr)
    {
        vhost_ = lws_create_vhost(context_, &info_);
        if (vhost_)
        {
#if LWS_LIBRARY_VERSION_NUMBER > 4002000
            if (lws_create_adopt_udp(vhost_, NULL, port_, LWS_CAUDP_BIND,
                protocols_[0].name, NULL, NULL, NULL, NULL,
                "user"))
#elif LWS_LIBRARY_VERSION_NUMBER > 4000000
            if (lws_create_adopt_udp(vhost_, NULL, port_, LWS_CAUDP_BIND,
                protocols_[0].name, NULL, NULL, NULL, NULL))
#else
            if (lws_create_adopt_udp(vhost_, port_, LWS_CAUDP_BIND,
                protocols_[0].name, NULL))
#endif
            {
                stop_ = false;
                pthread_t thread;
                pthread_create(&thread, nullptr, EnterMainLooper, this);
                bRet = true;
            }
            else
            {
                lws_vhost_destroy(vhost_);
                lws_context_destroy(context_);
                context_ = nullptr;
            }
        }
        else
        {
            lws_context_destroy(context_);
            context_ = nullptr;
        }
    }
    mutex_.unlock();
    return bRet;
}

void UdpSocketService::Stop()
{
    mutex_.lock();
    stop_ = true;
    if (context_ != nullptr)
    {
        lws_cancel_service(context_);
    }
    mutex_.unlock();
}

void * UdpSocketService::EnterMainLooper(void *instance)
{
    static_cast<UdpSocketService *>(instance)->MainLooper();
    return nullptr;
}

void UdpSocketService::MainLooper()
{
    mutex_.lock();
    while (!stop_)
    {
        mutex_.unlock();
        if (lws_service(context_, 0) < 0)
        {
            stop_ = true;
            lws_cancel_service(context_);
        }
        mutex_.lock();
    }
    lws_vhost_destroy(vhost_);
    if (context_ != nullptr)
    {
        lws_context_destroy(context_);
        context_ = nullptr;
    }
    mutex_.unlock();
    OnServiceStopped();
}

int UdpSocketService::EnterLwsCallback(struct lws *wsi, enum lws_callback_reasons reason,
    void *user, void *in, size_t len)
{
    const struct lws_protocols *protocol = lws_get_protocol(wsi);
    if (protocol != nullptr)
    {
        UdpSocketService *server = static_cast<UdpSocketService *>(protocol->user);
        return server->LwsCallback(wsi, reason, user, in, len);
    }
    return 0;
}

int UdpSocketService::LwsCallback(struct lws *wsi, enum lws_callback_reasons reason,
    void *user, void *in, size_t len)
{
    int result = 0;
    mutex_.lock();
    lws_sockfd_type fd;
    switch (reason)
    {
        case LWS_CALLBACK_RAW_SKT_BIND_PROTOCOL: {
            std::cout << "LWS_CALLBACK_RAW_SKT_BIND_PROTOCOL" << std::endl;
            break;
        }
        case LWS_CALLBACK_RAW_SKT_DROP_PROTOCOL: {
            std::cout << "LWS_CALLBACK_RAW_SKT_DROP_PROTOCOL" << std::endl;
            break;
        }
        case LWS_CALLBACK_RAW_CONNECTED: {
            std::cout << "LWS_CALLBACK_RAW_CONNECTED" << std::endl;
            break;
        }

        case LWS_CALLBACK_RAW_ADOPT: {
            std::cout << "LWS_CALLBACK_RAW_ADOPT" << std::endl;
            if (!OnConnection())
            {
                result = -1;
                break;
            }
            break;
        }

        case LWS_CALLBACK_RAW_CLOSE: {
            std::cout << "LWS_CALLBACK_RAW_CLOSE" << std::endl;
            ClearWriteQueueMap();
            OnDisconnected();
            break;
        }

        case LWS_CALLBACK_RAW_WRITEABLE: {
            std::cout << "LWS_CALLBACK_RAW_WRITEABLE" << std::endl;
            fd = lws_get_socket_fd(wsi);
            if (fd < 0)       /* keep Coverity happy: actually it cannot be < 0 */
            {
                break;
            }
            auto it = write_queue_map_.find(wsi);
            if (it != write_queue_map_.end())
            {
                std::queue<std::tuple<void *, size_t> > &write_queue = it->second;
                while (!write_queue.empty())
                {
                    auto data = std::move(write_queue.front());
                    write_queue.pop();
                    struct lws_udp udp = *(lws_get_udp(wsi));
                    void *d = std::get<0>(data);
                    int size = std::get<1>(data);
#if LWS_LIBRARY_VERSION_NUMBER > 4002000
                    size_t bytesSent = sendto(fd, d, size, 0,
                        sa46_sockaddr(&udp.sa46),
                        sa46_socklen(&udp.sa46));
#else
                    size_t bytesSent = sendto(fd, d, size, 0, &udp.sa, udp.salen);
#endif
                    std::cout << "Sent " << bytesSent << " bytes." << std::endl;
                    free(d);
                    if (bytesSent < size)
                    {
                        result = -1;
                    }
                }
                write_queue_map_.erase(it);
            }

            break;
        }

        case LWS_CALLBACK_RAW_RX: {
            std::cout << "LWS_CALLBACK_RAW_RX" << std::endl;
            OnMessageReceived(wsi, std::string(static_cast<char *>(in), len));
            break;
        }

        default: {
            break;
        }
    }
    mutex_.unlock();
    return result;
}

struct lws_protocols UdpSocketService::Protocol(const char *protocol_name)
{
    return
        {
            .name = protocol_name,
            .callback = EnterLwsCallback,
            .per_session_data_size = 0,
            .rx_buffer_size = 0,
            .id = 0,
            .user = this,
            .tx_packet_size = 0
        };
}

void UdpSocketService::ClearWriteQueueMap()
{
    auto it = write_queue_map_.begin();
    while (it != write_queue_map_.end())
    {
        std::queue<std::tuple<void *, size_t> > &write_queue = it->second;
        while (!write_queue.empty())
        {
            auto data = std::move(write_queue.front());
            write_queue.pop();
            void *d = std::get<0>(data);
            free(d);
        }
        ++it;
    }
    write_queue_map_.erase(write_queue_map_.begin(), it);
}
} // namespace NetworkServices

