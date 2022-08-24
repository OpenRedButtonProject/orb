/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

#ifndef OBS_NS_APP2APP_LOCAL_SERVICE_
#define OBS_NS_APP2APP_LOCAL_SERVICE_

#include "service_manager.h"
#include "websocket_service.h"
#include "app2app_remote_service.h"

#include <string>
#include <unordered_map>
#include <deque>

namespace NetworkServices {
class App2AppLocalService : public WebSocketService {
public:
   App2AppLocalService(ServiceManager *manager, int local_port, int remote_port);
   bool OnConnection(WebSocketConnection *connection) override;
   void OnFragmentReceived(WebSocketConnection *connection, std::vector<uint8_t> &&data,
      bool is_first, bool is_final, bool is_binary) override;
   void OnDisconnected(WebSocketConnection *connection) override;
   bool OnRemoteConnection(WebSocketConnection *connection);
   void OnRemoteFragmentReceived(WebSocketConnection *connection, std::vector<uint8_t> &&data,
      bool is_first, bool is_final, bool is_binary);
   void OnRemoteDisconnected(WebSocketConnection *connection);
   void Stop() override;
   void OnServiceStopped() override;
   void OnRemoteServiceStopped();

private:
   std::string GetAppEndPoint(const std::string &uri);
   void AddWaitingConnection(const std::string &type, const std::string &app_endpoint,
      WebSocketConnection *connection);
   void RemoveWaitingConnection(const std::string &type, const std::string &app_endpoint,
      WebSocketConnection *connection);
   WebSocketConnection* GetNextWaitingConnection(const std::string &type, const std::string &app_endpoint);

   ServiceManager *manager_;
   App2AppRemoteService remote_service_;
   std::recursive_mutex mutex_;
   std::unordered_map<std::string, std::deque<WebSocketConnection *> > waiting_connections_;
   bool service_stopped_;
   bool remote_service_stopped_;
};
} // namespace NetworkServices

#endif // OBS_NS_APP2APP_LOCAL_SERVICE_
