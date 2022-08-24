/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

#ifndef OBS_NS_APP2APP_REMOTE_SERVICE_
#define OBS_NS_APP2APP_REMOTE_SERVICE_

#include "websocket_service.h"

#include <string>

namespace NetworkServices {
class App2AppLocalService;

class App2AppRemoteService : public WebSocketService {
public:
   App2AppRemoteService(App2AppLocalService *local_service, int port);
   bool OnConnection(WebSocketConnection *connection) override;
   void OnFragmentReceived(WebSocketConnection *connection, std::vector<uint8_t> &&data,
      bool is_first, bool is_final, bool is_binary) override;
   void OnDisconnected(WebSocketConnection *connection) override;
   void OnServiceStopped() override;

private:
   App2AppLocalService *local_service_;
};
} // namespace NetworkServices

#endif // OBS_NS_APP2APP_REMOTE_SERVICE_
