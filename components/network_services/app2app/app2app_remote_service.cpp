/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
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

void App2AppRemoteService::OnFragmentReceived(WebSocketConnection *connection, std::vector<uint8_t> &&data,
   bool is_first, bool is_final, bool is_binary)
{
   local_service_->OnRemoteFragmentReceived(connection, std::move(data), is_first, is_final, is_binary);
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
