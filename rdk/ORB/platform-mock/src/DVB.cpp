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
#include "DVB.h"

/**
 * Constructor.
 */
DVB::DVB()
{
}

/**
 * Destructor.
 */
DVB::~DVB()
{
}

/**
 * Initialise the mock DVB stack.
 */
void DVB::Initialise()
{
    // populate channel list
    m_channels.push_back(Channel(
        "ccid:816",                      // std::string ccid
        "HbbTV-Testsuite1",              // std::string name
        "0",                             // std::string dsd
        "0",                             // std::string ipBroadcastId
        Channel::Type::CHANNEL_TYPE_TV,  // int channelType
        Channel::IdType::CHANNEL_ID_DVB_T, // int idType
        0,                               // int majorChannel
        816,                             // int terminalChannel
        1,                               // int nid
        1,                               // int onid
        65283,                           // int tsid
        28186,                           // int sid
        false,                           // bool hidden
        0                                // int sourceId
        ));

    std::vector<ParentalRating> parentalRatings;
    Programme::ProgrammeIdType programmeIdType = Programme::ProgrammeIdType::ID_DVB_EVENT;

    // populate programmes per channel
    m_programmes["ccid:816"].push_back(Programme(
        "1",               // std::string programmeId
        "Event 1, umlaut ä", // std::string name
        "subtitle",        // std::string description
        "",                // std::string longDescription
        "ccid:816",        // std::string channelId
        1627483530,        // long startTime
        300,               // long duration
        programmeIdType,   // int programmeIdType
        parentalRatings    // std::vector<ParentalRating> parentalRatings
        ));

    m_programmes["ccid:816"].push_back(Programme(
        "2",               // std::string programmeId
        "Event 1, umlaut ö", // std::string name
        "subtitle",        // std::string description
        "",                // std::string longDescription
        "ccid:816",        // std::string channelId
        1627483830,        // long startTime
        300,               // long duration
        programmeIdType,   // int programmeIdType
        parentalRatings    // std::vector<ParentalRating> parentalRatings
        ));

    m_programmes["ccid:816"].push_back(Programme(
        "100",             // std::string programmeId
        "Event 3, umlaut ä", // std::string name
        "subtitle",        // std::string description
        "",                // std::string longDescription
        "ccid:816",        // std::string channelId
        1627484430,        // long startTime
        3600,              // long duration
        programmeIdType,   // int programmeIdType
        parentalRatings    // std::vector<ParentalRating> parentalRatings
        ));
}

/**
 * Finalise the mock DVB stack.
 */
void DVB::Finalise()
{
    m_channels.clear();
    m_programmes.clear();
}

/**
 * Get the list of scanned channels.
 *
 * @return a vector containing the scanned channels
 */
std::vector<Channel> DVB::GetChannels()
{
    return m_channels;
}

/**
 * Get the programme list of the specified channel.
 *
 * @param ccid The channel id
 */
std::vector<Programme> DVB::GetProgrammes(std::string ccid)
{
    std::vector<Programme> empty;
    if (m_programmes.find(ccid) != m_programmes.end())
    {
        return m_programmes[ccid];
    }
    return empty;
}