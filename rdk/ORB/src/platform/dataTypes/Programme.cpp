/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

#include "Programme.h"

namespace orb {
std::shared_ptr<Programme> Programme::FromJsonObject(JsonObject jsonProgramme)
{
   std::vector<ParentalRating> parentalRatings;

   JsonArray jsonParentalRatings = jsonProgramme["parentalRatings"].Array();
   for (int i = 0; i < jsonParentalRatings.Length(); i++)
   {
      JsonObject jsonParentalRating = jsonParentalRatings[i].Object();
      parentalRatings.push_back(
         ParentalRating(
            jsonParentalRating["name"].String(),
            jsonParentalRating["scheme"].String(),
            jsonParentalRating["region"].String(),
            jsonParentalRating["value"].Number(),
            jsonParentalRating["labels"].Number()
            )
         );
   }

   return std::make_shared<Programme>(
      jsonProgramme["programmeID"].String(),
      jsonProgramme["name"].String(),
      jsonProgramme["description"].String(),
      jsonProgramme["longDescription"].String(),
      jsonProgramme["channelID"].String(),
      jsonProgramme["startTime"].Number(),
      jsonProgramme["duration"].Number(),
      jsonProgramme["programmeIDType"].Number(),
      parentalRatings
      );
}

std::shared_ptr<Programme> Programme::FromJsonString(std::string jsonProgrammeAsString)
{
   JsonObject jsonProgramme;
   jsonProgramme.FromString(jsonProgrammeAsString);
   return FromJsonObject(jsonProgramme);
}

Programme::Programme(
   std::string programmeId,
   std::string name,
   std::string description,
   std::string longDescription,
   std::string channelId,
   long startTime,
   long duration,
   int programmeIdType,
   std::vector<ParentalRating> parentalRatings
   )
   : m_programmeId(programmeId)
   , m_name(name)
   , m_description(description)
   , m_longDescription(longDescription)
   , m_channelId(channelId)
   , m_startTime(startTime)
   , m_duration(duration)
   , m_programmeIdType(programmeIdType)
   , m_parentalRatings(parentalRatings)
{
}

Programme::~Programme()
{
}

std::string Programme::GetProgrammeId() const
{
   return m_programmeId;
}

std::string Programme::GetName() const
{
   return m_name;
}

std::string Programme::GetDescription() const
{
   return m_description;
}

std::string Programme::GetLongDescription() const
{
   return m_longDescription;
}

std::string Programme::GetChannelId() const
{
   return m_channelId;
}

long Programme::GetStartTime() const
{
   return m_startTime;
}

long Programme::GetDuration() const
{
   return m_duration;
}

int Programme::GetProgrammeIdType() const
{
   return m_programmeIdType;
}

std::vector<ParentalRating> Programme::GetParentalRatings() const
{
   return m_parentalRatings;
}

JsonObject Programme::ToJsonObject() const
{
   JsonObject json_programme;
   json_programme.Set("programmeID", GetProgrammeId());
   json_programme.Set("programmeIDType", GetProgrammeIdType());
   json_programme.Set("name", GetName());
   json_programme.Set("description", GetDescription());
   json_programme.Set("longDescription", GetLongDescription());
   json_programme.Set("startTime", (int64_t) GetStartTime());
   json_programme.Set("duration", (int64_t) GetDuration());
   json_programme.Set("channelID", GetChannelId());
   ArrayType<JsonValue> json_parentalRatings;
   for (unsigned int i = 0; i < m_parentalRatings.size(); i++)
   {
      JsonValue v;
      v.Object(m_parentalRatings[i].ToJsonObject());
      json_parentalRatings.Add(v);
   }
   JsonValue parentalRatings;
   parentalRatings.Array(json_parentalRatings);
   json_programme.Set("parentalRatings", parentalRatings);
   return json_programme;
}
} // namespace orb
