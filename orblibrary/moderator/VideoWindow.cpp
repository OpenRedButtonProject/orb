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

    std::string VideoWindow::executeRequest(std::string method, Json::Value token, Json::Value params)
    {
        LOGD("executeRequest called: " << method << " " << JsonUtil::convertJsonToString(params));
        if (!mWebSocketService)
        {
            LOGE("WebSocket service not available.");
            return "{\"error\": \"WebSocket service not available\"}";
        }

        if (method == SELECT_CHANNEL_METHOD)
        {
            mWebSocketService->SendIPPlayerSelectChannel(
                params["channelType"].asInt(),
                params["idType"].asInt(),
                params["ipBroadcastID"].asString());
        }
        else if (method == VIDEO_WINDOW_PAUSE)
        {
            mWebSocketService->SendIPPlayerPause(mWebSocketService->GetCurrentSessionId());
        }
        else if (method == VIDEO_WINDOW_RESUME)
        {
            mWebSocketService->SendIPPlayerResume(mWebSocketService->GetCurrentSessionId());
        }
        else
        {
            LOGI("Unhandled method: " << method);
            return "{\"error\": \"Unhandled method: " + method + "\"}";
        }
        return "{\"result\": \"Success\"}";
    }

    std::string VideoWindow::DispatchChannelStatusChangedEvent(const Json::Value& params)
    {
       int status = params["status"].asInt();
       int statusCode = CHANNEL_STATUS_CONNECTING;
       bool permanentError = false;
       if (JsonUtil::HasParam(params, "error", Json::intValue))
       {
           // See OPApp Spec section 9.9.4.4.1
           // See OIPF DAE spec section 7.13.1.2 onChannelChangeError table for error codes
           // See OIPF DAE spec section 7.13.1.1
           statusCode = params["error"].asInt();
           if (statusCode == CHANNEL_STATUS_NO_SIGNAL
            || statusCode == CHANNEL_STATUS_INSUFFICIENT_RESOURCES
            || statusCode == CHANNEL_STATUS_UNKNOWN_ERROR)
            {
                permanentError = true;
            }
       }
       else if (status == PLAYBACK_STATUS_CONNECTING)
       {
           statusCode = CHANNEL_STATUS_CONNECTING;
       }
       else if (status == PLAYBACK_STATUS_PRESENTING)
       {
           statusCode = CHANNEL_STATUS_PRESENTING;
       }
       else if (status == PLAYBACK_STATUS_STOPPED)
       {
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