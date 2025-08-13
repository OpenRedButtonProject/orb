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

#ifndef PROGRAMME_H
#define PROGRAMME_H

#include <vector>
#include "ParentalRating.h"

namespace orb
{
/**
 * @brief orb::Programme
 *
 * HbbTV Programme representation.
 */
class Programme
{
public:

    /**
     * Enumerates the Programme::programmeIdType values.
     */
    enum ProgrammeIdType
    {
        ID_UNDEFINED      = -1,
        ID_TVA_CRID       =  0,
        ID_DVB_EVENT      =  1,
        ID_TVA_GROUP_CRID =  2
    };

public:

    /**
     * Constructor.
     * (See OIPF DAE spec section 7.16.2 and in particular 7.16.2.2)
     *
     * @param programmeId     The unique identifier of the programme or series
     * @param name            The short name of the programme
     * @param description     The description of the programme
     * @param longDescription The long description of the programme
     * @param channelId       The identifier of the channel from which the broadcasted content is
     *                        to be recorded.
     * @param startTime       The start time of the programme, measured in seconds since midnight
     *                        (GMT) on 1/1/1970
     * @param duration        The duration of the programme (in seconds)
     * @param programmeIdType The type of identification used to reference the programme
     * @param parentalRatings A collection of parental rating values for the programme
     */
    Programme(
        std::string programmeId,
        std::string name,
        std::string description,
        std::string longDescription,
        std::string channelId,
        long startTime,
        long duration,
        ProgrammeIdType programmeIdType,
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

    ~Programme()
    {
    }

    std::string GetProgrammeId() const
    {
        return m_programmeId;
    }

    std::string GetName() const
    {
        return m_name;
    }

    std::string GetDescription() const
    {
        return m_description;
    }

    std::string GetLongDescription() const
    {
        return m_longDescription;
    }

    std::string GetChannelId() const
    {
        return m_channelId;
    }

    long GetStartTime() const
    {
        return m_startTime;
    }

    long GetDuration() const
    {
        return m_duration;
    }

    ProgrammeIdType GetProgrammeIdType() const
    {
        return m_programmeIdType;
    }

    std::vector<ParentalRating> GetParentalRatings() const
    {
        return m_parentalRatings;
    }

private:

    std::string m_programmeId;
    std::string m_name;
    std::string m_description;
    std::string m_longDescription;
    std::string m_channelId;
    long m_startTime;
    long m_duration;
    ProgrammeIdType m_programmeIdType;
    std::vector<ParentalRating> m_parentalRatings;
}; // class Programme
} // namespace orb

#endif // PROGRAMME_H