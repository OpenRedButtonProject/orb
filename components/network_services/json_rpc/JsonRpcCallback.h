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
 */

#ifndef OBS_NS_JSON_RPC_CALLBACK_H
#define OBS_NS_JSON_RPC_CALLBACK_H

#include "JsonRpcService.h"
#include "log.h"

namespace orb
{
namespace networkServices 
{    
    class JsonRpcCallback : public JsonRpcService::ISessionCallback 
    {
    public:
        JsonRpcCallback() = default;
        virtual ~JsonRpcCallback() = default;

        // ISessionCallback interface
        void RequestNegotiateMethods() override;

        void RequestSubscribe(const JsonRpcService::SubscribeOptions& options) override;

        void RequestUnsubscribe(const JsonRpcService::SubscribeOptions& options) override;

        void RequestDialogueEnhancementOverride(
            int connectionId,
            std::string id,
            int dialogueEnhancementGain) override;

        void RequestTriggerResponseToUserAction(
            int connectionId,
            std::string id,
            std::string magnitude) override;

        void RequestFeatureSupportInfo(
            int connectionId,
            std::string id,
            int feature) override;

        void RequestFeatureSettingsQuery(
            int connectionId,
            std::string id,
            int feature) override;

        void RequestFeatureSuppress(
            int connectionId,
            std::string id,
            int feature) override;

        void NotifyVoiceReady(
            bool isReady) override;

        void NotifyStateMedia(
            std::string state) override;

        void RespondMessage(
            std::string info) override;

        void ReceiveConfirm(
            int connectionId,
            std::string id,
            std::string method) override;

        void ReceiveConfirmForSelectChannel(
            int connectionId,
            std::string id,
            std::string method,
            int sessionId) override;    

        void ReceiveError(
            int code,
            std::string message) override;

        void ReceiveError(
            int code,
            std::string message,
            std::string method,
            std::string data) override;

        void RequestIPPlaybackStatusUpdate(
            const Json::Value &params) override;

        void RequestIPPlaybackMediaPositionUpdate(
            const Json::Value &params) override;
        
        void RequestIPPlaybackSetComponents(
            const Json::Value &params) override;

        void RequestIPPlaybackSetPresentFollowing(
            const Json::Value &params) override;

        void RequestIPPlaybackSetTimelineMapping(
            const Json::Value &params) override;   
    }; 
} // namespace networkServices
} // namespace orb

#endif // OBS_NS_JSON_RPC_CALLBACK_H