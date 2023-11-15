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
