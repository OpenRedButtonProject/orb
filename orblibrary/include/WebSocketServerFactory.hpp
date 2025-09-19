#ifndef WEBSOCKET_SERVER_FACTORY_HPP
#define WEBSOCKET_SERVER_FACTORY_HPP

#include "IOrbBrowser.h"
#include "OrbConstants.h"
#include <memory>

namespace orb
{

class VideoWindow;

namespace networkServices {
    class JsonRpcService;
}

/**
 * @brief WebSocketServerFactory class
 *
 * This class is responsible for creating a WebSocketServer for the given application type.
 */
class WebSocketServerFactory
{
public:
    /**
     * @brief Create a WebSocketServer for the given application type.
     *
     * @param appType Application type
     * @param browser OrbBrowser instance to send request to ORBClient
     * @param videoWindow VideoWindow instance to call websocket service APIs to manage OpApp video window
     * @return WebSocketServer instance
     */
    static std::unique_ptr<networkServices::JsonRpcService> createWebSocketServer(
        orb::ApplicationType appType, IOrbBrowser* browser, std::weak_ptr<VideoWindow> videoWindow = {});
};

}

#endif