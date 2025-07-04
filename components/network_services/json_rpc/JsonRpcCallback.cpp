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
 
#include "JsonRpcCallback.h"

namespace orb
{
namespace networkServices
{    
    void JsonRpcCallback::RequestNegotiateMethods() 
    {
        LOGI("JsonRpcCallback::RequestNegotiateMethods");
        // Implementation of method negotiation logic goes here
    }

    void JsonRpcCallback::RequestSubscribe(const JsonRpcService::SubscribeOptions& options)
    {
        LOGI("JsonRpcCallback::RequestSubscribe");
        // Implementation of subscription logic goes here
    }

    void JsonRpcCallback::RequestUnsubscribe(const JsonRpcService::SubscribeOptions& options)
    {
        LOGI("JsonRpcCallback::RequestUnsubscribe");
        // Implementation of unsubscription logic goes here
    }

    void JsonRpcCallback::RequestDialogueEnhancementOverride(
        int connectionId,
        std::string id,
        int dialogueEnhancementGain)
    {
        LOGI("JsonRpcCallback::RequestDialogueEnhancementOverride");
        // Implementation of dialogue enhancement override logic goes here
    }

    void JsonRpcCallback::RequestTriggerResponseToUserAction(
        int connectionId,
        std::string id,
        std::string magnitude)
    {
        LOGI("JsonRpcCallback::RequestTriggerResponseToUserAction");
        // Implementation of user action response logic goes here
    }

    void JsonRpcCallback::RequestFeatureSupportInfo(
        int connectionId,
        std::string id,
        int feature)
    {
        LOGI("JsonRpcCallback::RequestFeatureSupportInfo");
        // Implementation of feature support info request logic goes here
    }

    void JsonRpcCallback::RequestFeatureSettingsQuery(
        int connectionId,
        std::string id,
        int feature)
    {
        LOGI("JsonRpcCallback::RequestFeatureSettingsQuery");
        // Implementation of feature settings query logic goes here
    }

    void JsonRpcCallback::RequestFeatureSuppress(
        int connectionId,
        std::string id,
        int feature)
    {
        LOGI("JsonRpcCallback::RequestFeatureSuppress");    
        // Implementation of feature suppression logic goes here
    }

    void JsonRpcCallback::NotifyVoiceReady(bool isReady)
    {
        LOGI("JsonRpcCallback::NotifyVoiceReady");
        // Implementation of voice ready notification logic goes here
    }

    void JsonRpcCallback::NotifyStateMedia(std::string state)
    {   
        LOGI("JsonRpcCallback::NotifyStateMedia");
        // Implementation of media state notification logic goes here
    }   

    void JsonRpcCallback::RespondMessage(std::string info)
    {
        LOGI("JsonRpcCallback::RespondMessage");
        // Implementation of response message logic goes here
    }

    void JsonRpcCallback::ReceiveConfirm(
        int connectionId,
        std::string id,
        std::string method)
    {
        LOGI("JsonRpcCallback::ReceiveConfirm");
        // Implementation of confirm reception logic goes here
    }

    void JsonRpcCallback::ReceiveConfirmForSelectChannel(
        int connectionId,
        std::string id,
        std::string method,
        int sessionId)
    {
        LOGI("JsonRpcCallback::ReceiveConfirmForSelectChannel");
        // Implementation of confirm reception for select channel logic goes here
    }

    void JsonRpcCallback::ReceiveError(
        int code,
        std::string message)
    {
        LOGI("JsonRpcCallback::ReceiveError");  
        // Implementation of error reception logic goes here
    }

    void JsonRpcCallback::ReceiveError(
        int code,
        std::string message,
        std::string method,
        std::string data)
    {   
        LOGI("JsonRpcCallback::ReceiveError with method and data");
        // Implementation of error reception with method and data logic goes here
    }   

    void JsonRpcCallback::RequestIPPlaybackStatusUpdate(
        const Json::Value &params)
    {
        LOGI("JSON Params: " + params.toStyledString());
        // Implementation of IP playback status update request logic goes here
    }

    void JsonRpcCallback::RequestIPPlaybackMediaPositionUpdate(
        const Json::Value &params)
    {
        LOGI("JSON Params: " + params.toStyledString());
        // Implementation of IP playback media position update request logic goes here
    }

    void JsonRpcCallback::RequestIPPlaybackSetComponents(
        const Json::Value &params)
    {
        LOGI("JSON Params: " + params.toStyledString());    
        // Implementation of IP playback set components request logic goes here
    }

    void JsonRpcCallback::RequestIPPlaybackSetPresentFollowing(
        const Json::Value &params)
    {       
        LOGI("JSON Params: " + params.toStyledString());
        // Implementation of IP playback set present following request logic goes here
    }

    void JsonRpcCallback::RequestIPPlaybackSetTimelineMapping(
        const Json::Value &params)
    {   
        LOGI("JSON Params: " + params.toStyledString());
        // Implementation of IP playback set timeline mapping request logic goes here
    }
} // namespace networkServices
} // namespace orb