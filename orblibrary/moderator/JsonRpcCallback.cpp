/**
 * ORB Software. Copyright (c) 2025 Ocean Blue Software Limited
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
 *
 * ORB JsonRpcCallback
 *
 */

#include "log.hpp"
#include "JsonRpcCallback.h"
#include "JsonUtil.h"
#include "BroadcastInterface.hpp"

namespace orb
{
    const std::string CHANNEL_STATUS_CHANGED = "ChannelStatusChanged";
    const std::string COMPONENT_CHANGED = "ComponentChanged";

    const int PLAYBACK_STATUS_CONNECTING = 1;
    const int PLAYBACK_STATUS_PRESENTING = 2;
    const int PLAYBACK_STATUS_STOPPED = 3;

    const int CHANNEL_STATUS_PRESENTING = -3;
    const int CHANNEL_STATUS_CONNECTING = -2;
    const int CHANNEL_STATUS_INTERRUPTED = 6;

    JsonRpcCallback::JsonRpcCallback(BroadcastInterface* broadcastInterface)
        : mBroadcastInterface(broadcastInterface)
    {
    }

    void JsonRpcCallback::RequestNegotiateMethods()
    {
        LOGD("JsonRpcCallback::RequestNegotiateMethods");
        // Implementation of method negotiation logic goes here
    }

    void JsonRpcCallback::RequestSubscribe(const networkServices::JsonRpcService::SubscribeOptions& options)
    {
        LOGD("JsonRpcCallback::RequestSubscribe");
        // Implementation of subscription logic goes here
    }

    void JsonRpcCallback::RequestUnsubscribe(const networkServices::JsonRpcService::SubscribeOptions& options)
    {
        LOGD("JsonRpcCallback::RequestUnsubscribe");
        // Implementation of unsubscription logic goes here
    }

    void JsonRpcCallback::RequestDialogueEnhancementOverride(
        int connectionId,
        std::string id,
        int dialogueEnhancementGain)
    {
        LOGD("JsonRpcCallback::RequestDialogueEnhancementOverride");
        // Implementation of dialogue enhancement override logic goes here
    }

    void JsonRpcCallback::RequestTriggerResponseToUserAction(
        int connectionId,
        std::string id,
        std::string magnitude)
    {
        LOGD("JsonRpcCallback::RequestTriggerResponseToUserAction");
        // Implementation of user action response logic goes here
    }

    void JsonRpcCallback::RequestFeatureSupportInfo(
        int connectionId,
        std::string id,
        int feature)
    {
        LOGD("JsonRpcCallback::RequestFeatureSupportInfo");
        // Implementation of feature support info request logic goes here
    }

    void JsonRpcCallback::RequestFeatureSettingsQuery(
        int connectionId,
        std::string id,
        int feature)
    {
        LOGD("JsonRpcCallback::RequestFeatureSettingsQuery");
        // Implementation of feature settings query logic goes here
    }

    void JsonRpcCallback::RequestFeatureSuppress(
        int connectionId,
        std::string id,
        int feature)
    {
        LOGD("JsonRpcCallback::RequestFeatureSuppress");
        // Implementation of feature suppression logic goes here
    }

    void JsonRpcCallback::NotifyVoiceReady(bool isReady)
    {
        LOGD("JsonRpcCallback::NotifyVoiceReady");
        // Implementation of voice ready notification logic goes here
    }

    void JsonRpcCallback::NotifyStateMedia(std::string state)
    {
        LOGD("JsonRpcCallback::NotifyStateMedia");
        // Implementation of media state notification logic goes here
    }

    void JsonRpcCallback::RespondMessage(std::string info)
    {
        LOGD("JsonRpcCallback::RespondMessage");
        // Implementation of response message logic goes here
    }

    void JsonRpcCallback::ReceiveConfirm(
        int connectionId,
        std::string id,
        std::string method)
    {
        LOGD("JsonRpcCallback::ReceiveConfirm");
        // Implementation of confirm reception logic goes here
    }

    void JsonRpcCallback::ReceiveConfirmForSelectChannel(
        int connectionId,
        std::string id,
        std::string method,
        int sessionId)
    {
        LOGD("JsonRpcCallback::ReceiveConfirmForSelectChannel");
        mBroadcastInterface->CreateIPChannelSession(sessionId);
        // Implementation of confirm reception for select channel logic goes here
    }

    void JsonRpcCallback::ReceiveError(
        int code,
        std::string message)
    {
        LOGD("JsonRpcCallback::ReceiveError");
        // Implementation of error reception logic goes here
    }

    void JsonRpcCallback::ReceiveError(
        int code,
        std::string message,
        std::string method,
        std::string data)
    {
        LOGD("JsonRpcCallback::ReceiveError with method and data");
        // Implementation of error reception with method and data logic goes here
    }

    void JsonRpcCallback::RequestIPPlaybackStatusUpdate(
        const Json::Value &params)
    {
        LOG(INFO) << "JSON Params: " + params.toStyledString();
        int sessionId = params["sessionID"].asInt();
        int status = params["status"].asInt();
         // test with mock data, the real data will be got from the service manager according to service list.

        int statusCode = CHANNEL_STATUS_CONNECTING;

        if (JsonUtil::HasParam(params, "error", Json::intValue)) {
            // if the error is not 0, the status code is the error code
            int errorCode = params["error"].asInt();
            statusCode = errorCode;
            mSessionMap[sessionId].errorCode = errorCode;
        } else if (status == PLAYBACK_STATUS_CONNECTING) {
            statusCode = CHANNEL_STATUS_CONNECTING;
        } else if (status == PLAYBACK_STATUS_PRESENTING) {
            statusCode = CHANNEL_STATUS_PRESENTING;
        } else if (status == PLAYBACK_STATUS_STOPPED) {
            statusCode = CHANNEL_STATUS_INTERRUPTED;
        }

        mSessionMap[sessionId].status = status;
        if (mBroadcastInterface) {
            mBroadcastInterface->DispatchChannelStatusChangedEvent(-1, -1, -1, statusCode, false, sessionId);
        }
    }

    void JsonRpcCallback::RequestIPPlaybackMediaPositionUpdate(
        const Json::Value &params)
    {
        LOGD("JSON Params: " + params.toStyledString());
        // Implementation of IP playback media position update request logic goes here
    }

    void JsonRpcCallback::RequestIPPlaybackSetComponents(
        const Json::Value &params)
    {
        LOGD("JSON Params: " + params.toStyledString());
        Json::Value components = params["componentsList"];
        int sessionId = params["sessionID"].asInt();
        for (auto& component : components) {
            component["active"] = component["initiallyActive"].asBool();
            component.removeMember("initiallyActive");
        }
        // save the components info to the session map
        mSessionMap[sessionId].componentsInfo = components;
        LOGD("componentsInfo: " + mSessionMap[sessionId].componentsInfo.toStyledString());
        // send the component changed event
        if (mBroadcastInterface) {
            mBroadcastInterface->DispatchComponentChangedEvent(-1, sessionId, components); // all components
        }
    }


    void JsonRpcCallback::RequestIPPlaybackSetPresentFollowing(
        const Json::Value &params)
    {
        LOGD("JSON Params: " + params.toStyledString());
        // Implementation of IP playback set present following request logic goes here
    }

    void JsonRpcCallback::RequestIPPlaybackSetTimelineMapping(
        const Json::Value &params)
    {
        LOGD("JSON Params: " + params.toStyledString());
        // Implementation of IP playback set timeline mapping request logic goes here
    }
} // namespace orb