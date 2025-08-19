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

#include "app2app_remote_service.h"
#include "app2app_local_service.h"
#include "service_manager.h"
#include "log.h"

#include <iostream>

namespace NetworkServices {
App2AppRemoteService::App2AppRemoteService(App2AppLocalService *local_service, int port) :
    WebSocketService("", port, false, ""),
    local_service_(local_service)
{
    LOG(LOG_INFO, "App2AppRemoteService ctor.\n");
}

bool App2AppRemoteService::OnConnection(WebSocketConnection *connection)
{
    return local_service_->OnRemoteConnection(connection);
}

void App2AppRemoteService::OnFragmentReceived(WebSocketConnection *connection,
    std::vector<uint8_t> &&data,
    bool is_first, bool is_final, bool is_binary)
{
    local_service_->OnRemoteFragmentReceived(connection, std::move(data), is_first, is_final,
        is_binary);
}

void App2AppRemoteService::OnDisconnected(WebSocketConnection *connection)
{
    local_service_->OnRemoteDisconnected(connection);
}

void App2AppRemoteService::OnServiceStopped()
{
    local_service_->OnRemoteServiceStopped();
}
} // namespace NetworkServices
