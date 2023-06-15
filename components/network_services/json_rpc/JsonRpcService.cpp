/**
 * ORB Software. Copyright (c) 2023 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

#include "JsonRpcService.h"
#include "log.h"

#include <iostream>

namespace NetworkServices {

JsonRpcService::JsonRpcService(int port, const std::string &endpoint,
    std::unique_ptr<SessionCallback> sessionCallback) :
    WebSocketService("JsonRpcService", port, false, "lo"), 
    m_endpoint(endpoint),
    m_sessionCallback(std::move(sessionCallback))
{
    LOG(LOG_INFO, "RPC Ctor");
}

bool JsonRpcService::OnConnection(WebSocketConnection *connection)
{
    if (connection->Uri() != m_endpoint) {
        LOG(LOG_INFO, "RPC Unknown endpoint received");
        return false;
    }

    LOG(LOG_INFO, "RPC OnConnection");

    return true;
}

void JsonRpcService::OnMessageReceived(WebSocketConnection *connection, const std::string &text)
{
    if (text == "Hello Terminal")
    {
        m_sessionCallback->HelloTerminal(connection->UniqueId(), "Testing 1234");
    }
    else
    {
        LOG(LOG_INFO, "RPC Unknown message received: %s", text.c_str());
    }
}

void JsonRpcService::OnDisconnected(WebSocketConnection *connection)
{

}

void JsonRpcService::OnServiceStopped()
{

}

void JsonRpcService::HelloApp(int id)
{
    connections_mutex_.lock();
    WebSocketConnection *connection = GetConnection(id);
    if (connection != nullptr)
    {
        connection->SendMessage("Hello App");
    }
    connections_mutex_.unlock();
}

} // namespace NetworkServices
