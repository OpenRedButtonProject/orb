/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

#ifndef WIP_DVBCSS_HBBTV_UDPSOCKETSERVICE_H
#define WIP_DVBCSS_HBBTV_UDPSOCKETSERVICE_H

#include "service_manager.h"

#include <string>
#include <unordered_map>
#include <memory>
#include <queue>
#include <mutex>
#include <libwebsockets.h>
#include <map>

namespace NetworkServices {
class UdpSocketService : public ServiceManager::Service {
public:
   UdpSocketService(const std::string &server_name, int port, bool use_ssl);
   virtual ~UdpSocketService();
   virtual bool Start();
   virtual void Stop();
   virtual bool OnConnection() = 0;
   virtual void OnMessageReceived(struct lws *wsi, const std::string &text) = 0;
   virtual void OnDisconnected() = 0;

   void SendMessage(struct lws *wsi, void *data, size_t len)
   {
      if (data != nullptr && len > 0)
      {
         void *d = malloc(len);
         memcpy(d, data, len);
         write_queue_map_[wsi].emplace(std::make_tuple(d, len));
         lws_callback_on_writable(wsi);
      }
   }

private:
   static void* EnterMainLooper(void *instance);
   void MainLooper();
   static int EnterLwsCallback(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len);
   int LwsCallback(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len);
   struct lws_protocols Protocol(const char *protocol_name);
   void ClearWriteQueueMap();

   std::recursive_mutex mutex_;
   bool stop_;
   std::string protocol_name_;
   lws_retry_bo_t retry_;
   struct lws_protocols protocols_[2];
   struct lws_context_creation_info info_;
   bool use_ssl_;
   struct lws_context *context_;
   int port_;
   struct lws_vhost *vhost_;
   std::unordered_map<struct lws *, std::queue<std::tuple<void *, size_t> > > write_queue_map_;
};
} // namespace NetworkServices


#endif //WIP_DVBCSS_HBBTV_UDPSOCKETSERVICE_H

