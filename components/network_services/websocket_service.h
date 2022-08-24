/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

#ifndef OBS_NS_WEBSOCKET_SERVICE_
#define OBS_NS_WEBSOCKET_SERVICE_

#include "service_manager.h"

#include <string>
#include <unordered_map>
#include <memory>
#include <queue>
#include <mutex>
#include <libwebsockets.h>

namespace NetworkServices {
class WebSocketService : public ServiceManager::Service {
public:
   class WebSocketConnection
   {
      friend class WebSocketService;

public:
      std::string Uri() const
      {
         return uri_;
      }

      void SendMessage(const std::string &text);
      void SendFragment(std::vector<uint8_t> &&data, bool is_first, bool is_final, bool is_binary);
      void Close();
      WebSocketConnection *paired_connection_;

protected:
      struct FragmentWriteInfo
      {
         lws_write_protocol write_protocol;
         std::vector<uint8_t> data;
         bool close;
      };

      WebSocketConnection(struct lws *wsi, const std::string &uri)
         : wsi_(wsi), uri_(uri), paired_connection_(nullptr)
      {
      }

      struct lws *wsi_;
      std::string uri_;
      std::string text_buffer_;
      std::queue<struct FragmentWriteInfo> write_queue_;

      // Disallow copy and assign
      WebSocketConnection(const WebSocketConnection&) = delete;
      WebSocketConnection& operator=(const WebSocketConnection&) = delete;
   };

   WebSocketService(const std::string &server_name, int port, bool use_ssl, const std::string &interface_name);
   virtual ~WebSocketService() = default;
   virtual bool Start();
   virtual void Stop();
   virtual bool OnConnection(WebSocketConnection *connection) = 0;
   virtual void OnFragmentReceived(WebSocketConnection *connection, std::vector<uint8_t> &&data, bool is_first, bool is_final, bool is_binary);
   virtual void OnMessageReceived(WebSocketConnection *connection, const std::string &text);
   virtual void OnDisconnected(WebSocketConnection *connection) = 0;

protected:
   std::unordered_map<void *, std::unique_ptr<WebSocketConnection> > connections_;

private:
   static void* EnterMainLooper(void *instance);
   void MainLooper();
   static int EnterLwsCallback(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len);
   int LwsCallback(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len);
   struct lws_protocols Protocol(const char *protocol_name);
   std::string Header(struct lws *wsi, enum lws_token_indexes header);

   std::recursive_mutex mutex_;
   bool stop_;
   std::string protocol_name_;
   lws_retry_bo_t retry_;
   struct lws_protocols protocols_[2];
   struct lws_context_creation_info info_;
   bool use_ssl_;
   std::string interface_name_;
   struct lws_context *context_;
};
} // namespace NetworkServices

#endif // OBS_NS_WEBSOCKET_SERVICE_

