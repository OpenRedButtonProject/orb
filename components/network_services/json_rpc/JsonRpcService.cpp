/**
 * ORB Software. Copyright (c) 2023 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */
#define LOG_TAG "JsonRpcService"

#include "JsonRpcService.h"
#include "log.h"

#include <iostream>
#include <sstream>

namespace NetworkServices {

JsonRpcService::JsonRpcService(
    int port,
    const std::string &endpoint,
    std::unique_ptr<SessionCallback> sessionCallback) :
    WebSocketService("JsonRpcService", port, false, "lo"), 
    m_endpoint(endpoint),
    m_sessionCallback(std::move(sessionCallback))
{
    LOG(LOG_INFO, "Start");
    Start();
}

bool JsonRpcService::OnConnection(WebSocketConnection *connection)
{
    if (connection->Uri() != m_endpoint) {
        LOG(LOG_INFO, "Unknown endpoint received. Got: %s, expected: %s",
            connection->Uri().c_str(), m_endpoint.c_str());
        return false;
    }
    LOG(LOG_INFO, "Connected: connectionId=%d", connection->Id());
    return true;
}

void JsonRpcService::OnMessageReceived(WebSocketConnection *connection, const std::string &text)
{
    LOG(LOG_INFO, "Message received: connection=%d, text=%s", connection->Id(), text.c_str());
    // TODO Use JSON library to parse JSON request
    if (text == "request=dialogueEnhancementOverride")
    {
        LOG(LOG_INFO, "JSON-RPC-EXAMPLE #2: Service received request. Call sesssion callback...");
        m_sessionCallback->RequestDialogueEnhancementOverride(connection->Id(), 1, 2);
    }
    else
    {
        LOG(LOG_INFO, "Message not handled");
    }
}

void JsonRpcService::OnDisconnected(WebSocketConnection *connection)
{

}

void JsonRpcService::OnServiceStopped()
{

}

void JsonRpcService::RespondDialogueEnhancementOverride(
    int connectionId,
    int id,
    int dialogueEnhancementGain)
{
    LOG(LOG_INFO, "JSON-RPC-EXAMPLE #9: Service called with response. Send response to client...");

    connections_mutex_.lock();
    WebSocketConnection *connection = GetConnection(connectionId);
    if (connection != nullptr)
    {
        // TODO Use JSON library to create JSON response
        std::ostringstream oss;
        oss << "response=dialogueEnhancementOverride|" << id << "|" << dialogueEnhancementGain;
        connection->SendMessage(oss.str());
    }
    connections_mutex_.unlock();
}

} // namespace NetworkServices
