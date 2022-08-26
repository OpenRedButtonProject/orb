/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

#pragma once

#include <vector>
#include "ParentalRating.h"

namespace orb {
/**
 * @brief orb::Programme
 *
 * HbbTV Programme representation.
 */
class Programme {
public:

   static std::shared_ptr<Programme> FromJsonObject(JsonObject jsonProgramme);
   static std::shared_ptr<Programme> FromJsonString(std::string jsonProgrammeAsString);

   Programme(
      std::string programmeId,
      std::string name,
      std::string description,
      std::string longDescription,
      std::string channelId,
      long startTime,
      long duration,
      int programmeIdType,
      std::vector<ParentalRating> parentalRatings
      );

   ~Programme();

   std::string GetProgrammeId() const;
   std::string GetName() const;
   std::string GetDescription() const;
   std::string GetLongDescription() const;
   std::string GetChannelId() const;
   long GetStartTime() const;
   long GetDuration() const;
   int GetProgrammeIdType() const;
   std::vector<ParentalRating> GetParentalRatings() const;

   JsonObject ToJsonObject() const;

private:

   std::string m_programmeId;
   std::string m_name;
   std::string m_description;
   std::string m_longDescription;
   std::string m_channelId;
   long m_startTime;
   long m_duration;
   int m_programmeIdType;
   std::vector<ParentalRating> m_parentalRatings;
}; // class Programme
} // namespace orb
