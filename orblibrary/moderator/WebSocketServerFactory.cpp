#include "log.h"
#include "JsonUtil.h"
#include "JsonRpcService.h"
#include "JsonRpcCallback.h"
#include "VideoWindow.hpp"
#include "WebSocketServerFactory.hpp"

namespace orb
{
    using namespace networkServices;
    std::unique_ptr<JsonRpcService> WebSocketServerFactory::createWebSocketServer(orb::ApplicationType appType,
        IOrbBrowser* browser, std::weak_ptr<VideoWindow> videoWindow)
    {
        const std::string CONFIGURATION_GETCAPABILITIES = "Configuration.getCapabilities";
        // request capabilities from Live App
        Json::Value request;
        request["method"] = CONFIGURATION_GETCAPABILITIES;
        request["params"]["applicationType"] = appType;
        std::string response = browser->sendRequestToClient(JsonUtil::convertJsonToString(request));

        Json::Value capabilities;
        if (!JsonUtil::decodeJson(response, &capabilities)) {
            LOGE("Failed to decode capabilities");
            return std::unique_ptr<JsonRpcService>(nullptr);
        }

        // Get the endpoint and port from the capabilities
        Json::Value result = capabilities["result"];
        const std::string SERVER_ENDPOINT_KEY = "jsonRpcServerEndpoint";
        const std::string SERVER_PORT_KEY = "jsonRpcServerPort";

        // Check if the capabilities response contains the jsonRpcServerEndpoint and jsonRpcServerPort
        if (!JsonUtil::HasParam(result, SERVER_ENDPOINT_KEY, Json::stringValue) ||
             !JsonUtil::HasParam(result, SERVER_PORT_KEY, Json::intValue))
        {
            LOGE("Websocket Server can not start as Capabilities response does not contain jsonRpcServerEndpoint or jsonRpcServerPort");
            return std::unique_ptr<JsonRpcService>(nullptr);
        }

        std::string endpoint = JsonUtil::getStringValue(result, SERVER_ENDPOINT_KEY);
        int port = JsonUtil::getIntegerValue(result, SERVER_PORT_KEY);


        // OpApp and Video Window will use the same WebSocket Server.
        // For HbbTV App, mVideoWindow is nullptr.
        std::unique_ptr<JsonRpcService::ISessionCallback> callback =
            std::make_unique<JsonRpcCallback>(videoWindow);

        std::unique_ptr<JsonRpcService> webSocketServer = std::make_unique<JsonRpcService>(port,endpoint,std::move(callback));
        webSocketServer->SetOpAppEnabled(appType == orb::APP_TYPE_OPAPP);

        return webSocketServer;
    }
}

