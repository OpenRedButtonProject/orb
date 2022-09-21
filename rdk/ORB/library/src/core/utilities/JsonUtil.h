/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

#pragma once

#include <memory>
#include <string>
#include <nlohmann/json.hpp>

#include "Channel.h"
#include "Programme.h"
#include "ParentalRating.h"
#include "LocalSystem.h"
#include "Component.h"

using json = nlohmann::json;

namespace orb
{
class JsonUtil
{
public:

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

   // CommonQuery
}; // class JsonUtil
} // namespace orb