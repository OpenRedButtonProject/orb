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

#ifndef VIDEO_WINDOW_HPP
#define VIDEO_WINDOW_HPP

#include "IOrbBrowser.h"
#include "JsonRpcService.h"
#include "ComponentBase.hpp"

namespace orb
{

/**
 * @brief VideoWindow class
 *
 * This class is responsible for handling the request to OpApp video window
 * by calling websocket service APIs..
 *
 */
class VideoWindow : public ComponentBase
{
public:
    /**
     * @brief VideoWindow constructor
     *
     * @param browser OrbBrowser instance to send request to ORBClient
     */
    explicit VideoWindow(IOrbBrowser* browser);

    /**
     * @brief VideoWindow destructor
     */
    virtual ~VideoWindow();

    /**
     * @brief Execute the request from Orbclient
     *
     * @param method Method name
     * @param token Token
     * @param params Json parameters
     * @return The response string in JSON format
     */
    std::string executeRequest(std::string method, Json::Value token, Json::Value params) override;

    /**
     * @brief Set the WebSocket service
     *
     * @param webSocketService WebSocket service instance
     */
    void setWebSocketService(std::shared_ptr<networkServices::JsonRpcService> webSocketService);

    /**
     * @brief Dispatch the channel status changed event to Orbclient
     *
     * @param params Event parameters for channel status changed event
     * @return The response string in JSON format from Orbclient
     */
    std::string DispatchChannelStatusChangedEvent(const Json::Value& params);


private:
    IOrbBrowser *mOrbBrowser;
    std::shared_ptr<networkServices::JsonRpcService> mWebSocketService;
};

}

#endif // VIDEO_WINDOW_HPP