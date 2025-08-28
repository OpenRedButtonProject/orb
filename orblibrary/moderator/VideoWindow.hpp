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

namespace orb
{

class VideoWindow
{
public:
    explicit VideoWindow(IOrbBrowser* browser);
    virtual ~VideoWindow();
    bool handleBridgeEvent(const std::string& etype, const std::string& properties);
    void setWebSocketService(std::shared_ptr<networkServices::JsonRpcService> webSocketService);
    std::string DispatchChannelStatusChangedEvent(const Json::Value& params);

private:
    IOrbBrowser *mOrbBrowser;
    std::shared_ptr<networkServices::JsonRpcService> mWebSocketService;

private:
    bool handleSelectChannel(const Json::Value& params);
    bool handlePause(const Json::Value& params);
    bool handleResume(const Json::Value& params);
};

}

#endif // VIDEO_WINDOW_HPP