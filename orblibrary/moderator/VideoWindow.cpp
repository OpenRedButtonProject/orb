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

#include "VideoWindow.hpp"
#include "JsonUtil.h"
#include "log.hpp"
#include "OrbConstants.h"

using namespace std;

namespace orb
{
    const string SELECT_CHANNEL_METHOD = "VideoWindow.selectChannel";
    const string VIDEO_WINDOW_PAUSE = "VideoWindow.pause";
    const string VIDEO_WINDOW_RESUME = "VideoWindow.resume";

    const string VIDEO_WINDOW_CHANNEL_STATUS_CHANGE = "VideoWindow.ChannelStatusChanged";

    const int PLAYBACK_STATUS_CONNECTING = 1;
    const int PLAYBACK_STATUS_PRESENTING = 2;
    const int PLAYBACK_STATUS_STOPPED = 3;

    VideoWindow::VideoWindow(IOrbBrowser* browser)
    : mOrbBrowser(browser)
    {
    }

    VideoWindow::~VideoWindow()
    {
    }

    void VideoWindow::setWebSocketService(std::shared_ptr<networkServices::JsonRpcService> webSocketService)
    {
        mWebSocketService = webSocketService;
    }

    bool VideoWindow::handleBridgeEvent(const std::string& etype, const std::string& properties)
    {
        LOGD("handleBridgeEvent called: " << etype << " " << properties);
        if (!mWebSocketService) {
            LOGE("WebSocket service not available.");
            return false;
        }

        Json::Value jsonval;
        if (JsonUtil::decodeJson(properties, &jsonval)) {
            if (etype == SELECT_CHANNEL_METHOD) {
                return handleSelectChannel(jsonval);
            }
            else if (etype == VIDEO_WINDOW_PAUSE) {
                return handlePause(jsonval);
            }
            else if (etype == VIDEO_WINDOW_RESUME) {
                return handleResume(jsonval);
            }
        }
        return false;
    }

    bool VideoWindow::handleSelectChannel(const Json::Value& params)
    {
        mWebSocketService->SendIPPlayerSelectChannel(
        params["channelType"].asInt(),
        params["idType"].asInt(),
        params["ipBroadcastID"].asString());
        return true;
    }

    bool VideoWindow::handlePause(const Json::Value& params)
    {
        // web socket service will provide current sessionID
        mWebSocketService->SendIPPlayerPause(mWebSocketService->GetCurrentSessionId());
        return true;
    }

    bool VideoWindow::handleResume(const Json::Value& params)
    {
        // web socket service will provide current sessionID
        mWebSocketService->SendIPPlayerResume(mWebSocketService->GetCurrentSessionId());
        return true;
    }

    std::string VideoWindow::DispatchChannelStatusChangedEvent(const Json::Value& params)
    {
       int status = params["status"].asInt();
       int statusCode = CHANNEL_STATUS_CONNECTING;
       bool permanentError = false;
       if (JsonUtil::HasParam(params, "error", Json::intValue)) {
           // if the error is not 0, the status code is the error code
           statusCode = params["error"].asInt();
           permanentError = true;
       } else if (status == PLAYBACK_STATUS_CONNECTING) {
           statusCode = CHANNEL_STATUS_CONNECTING;
       } else if (status == PLAYBACK_STATUS_PRESENTING) {
           statusCode = CHANNEL_STATUS_PRESENTING;
       } else if (status == PLAYBACK_STATUS_STOPPED) {
           statusCode = CHANNEL_STATUS_INTERRUPTED;
       }
       Json::Value request;
       request["method"] = VIDEO_WINDOW_CHANNEL_STATUS_CHANGE;
       request["params"]["statusCode"] = statusCode;
       request["params"]["permanentError"] = permanentError;
       std::string requestString = JsonUtil::convertJsonToString(request);
       return mOrbBrowser->sendRequestToClient(requestString);
    }
}