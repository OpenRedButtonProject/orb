/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

#ifndef OBS_NS_JSONRPCSERVICE_
#define OBS_NS_JSONRPCSERVICE_

#include "websocket_service.h"

#include <string>
#include <unordered_map>
#include <mutex>

namespace NetworkServices {
class JsonRpcService : public WebSocketService {
public:
    class SessionCallback {
public:
        virtual void HelloTerminal(int id, const std::string &message) = 0;

        virtual ~SessionCallback() = default;
    };
    
    JsonRpcService(int port, const std::string &endpoint,
        std::unique_ptr<SessionCallback> m_sessionCallback);

    bool OnConnection(WebSocketConnection *connection) override;

    void OnMessageReceived(WebSocketConnection *connection, const std::string &text) override;

    void OnDisconnected(WebSocketConnection *connection) override;

    void OnServiceStopped() override;

    void HelloApp(int id);

private:
    std::string m_endpoint;
    std::unique_ptr<SessionCallback> m_sessionCallback;
};
} // namespace NetworkServices

#endif // OBS_NS_JSONRPCSERVICE_
