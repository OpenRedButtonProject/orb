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

#pragma once

#include <memory>
#include <string>
#include <nlohmann/json.hpp>

#include "Capabilities.h"
#include "Channel.h"
#include "Programme.h"
#include "ParentalRating.h"
#include "LocalSystem.h"
#include "Component.h"
#include "DrmSystemStatus.h"
#include "DisplayInfo.h"

using json = nlohmann::json;

namespace orb
{
class JsonUtil
{
public:

    // Capabilities

    static json CapabilitiesToJsonObject(std::shared_ptr<Capabilities> capabilities);

    static json AudioProfileToJsonObject(AudioProfile audioProfile);

    static json VideoProfileToJsonObject(VideoProfile videoProfile);

    static json VideoDisplayFormatToJsonObject(VideoDisplayFormat videoDisplayFormat);

    // Channel

    static std::shared_ptr<Channel> ChannelFromJsonString(std::string jsonChannelAsString);

    static std::shared_ptr<Channel> ChannelFromJsonObject(json jsonChannel);

    static json ChannelToJsonObject(Channel channel);

    // Programme

    static std::shared_ptr<Programme> ProgrammeFromJsonString(std::string jsonProgrammeAsString);

    static std::shared_ptr<Programme> ProgrammeFromJsonObject(json jsonProgramme);

    static json ProgrammeToJsonObject(Programme programme);

    // ParentalRating

    static json ParentalRatingToJsonObject(ParentalRating parentalRating);

    // LocalSystem

    static json LocalSystemToJsonObject(LocalSystem localSystem);

    // Component

    static json ComponentToJsonObject(Component component);

    // DrmSystemStatus

    static json DrmSystemStatusToJsonObject(DrmSystemStatus drmSystemStatus);

#ifdef BBC_API_ENABLE
    // DisplayInfo

    static json DisplayInfoToJsonObject(DisplayInfo displayInfo);

#endif

    // CommonQuery
}; // class JsonUtil
} // namespace orb
