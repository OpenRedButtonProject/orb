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
 *
 * NOTICE: This file has been created by Ocean Blue Software and is based on
 * the original work (https://github.com/bbc/pydvbcss) of the British
 * Broadcasting Corporation, as part of a translation of that work from a
 * Python library/tool to a native service. The following is the copyright
 * notice of the original work:
 *
 * Copyright 2015 British Broadcasting Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "TimelineSyncService.h"
#include "CSSUtilities.h"
#include "log.h"
#include "ContentIdentificationService.h"
#include "media_synchroniser.h"

namespace NetworkServices {
TimeStamp::TimeStamp(const Nullable<unsigned long long int> &contentTime, std::string wallclockTime)
{
    setTimeStamp(contentTime, std::move(wallclockTime));
}

void TimeStamp::setTimeStamp(Nullable<unsigned long long> contentTime, std::string wallclockTime)
{
    if (CSSUtilities::isWallclockTimeValid(wallclockTime))
    {
        m_contentTime = contentTime;
        m_wallClockTime = std::move(wallclockTime);
    }
    else
    {
        LOG(LOG_ERROR, "Invalid wallclock time value [%s].\n", wallclockTime.c_str());
    }
}

void TimeStamp::getTimeStamp(Nullable<unsigned long long> &contentTime, std::string &wallclockTime)
{
    contentTime = m_contentTime;
    wallclockTime = m_wallClockTime;
}

bool TimeStamp::isNull()
{
    return m_contentTime.isNull();
}

Json::Value TimeStamp::pack()
{
    Json::Value ctstamp;

    if (m_contentTime.isNull())
    {
        ctstamp["contentTime"] = {};
    }
    else
    {
        ctstamp["contentTime"] = std::to_string(m_contentTime.value());
    }

    ctstamp["wallClockTime"] = m_wallClockTime;

    return ctstamp;
}

Json::Value ControlTimestamp::pack()
{
    Nullable<unsigned long long> contentTime;
    Json::Value ctstamp = std::move(m_tstamp.pack());

    if (m_timelineSpeedMultiplier.isNull())
    {
        ctstamp["timelineSpeedMultiplier"] = {};
    }
    else
    {
        ctstamp["timelineSpeedMultiplier"] = m_timelineSpeedMultiplier.value();
    }

    return ctstamp;
}

ControlTimestamp ControlTimestamp::unpack(const std::string &msg)
{
    Json::Value root = {};
    if (CSSUtilities::unpack(msg, root))
    {
        Nullable<unsigned long long> contentTime;
        std::string wallclockTime = root["wallClockTime"].asString();
        Nullable<float> timelineSpeedMultiplier;

        if (!CSSUtilities::isWallclockTimeValid(wallclockTime))
        {
            LOG(LOG_ERROR, "Invalid wallclock time value [%s].\n", wallclockTime.c_str());
            return ControlTimestamp();
        }

        if (!root["contentTime"].isNull())
        {
            contentTime = root["contentTime"].asUInt64();
        }
        if (!root["timelineSpeedMultiplier"].isNull())
        {
            timelineSpeedMultiplier = root["timelineSpeedMultiplier"].asFloat();
        }

        if (contentTime.isNull() != timelineSpeedMultiplier.isNull())
        {
            LOG(LOG_ERROR,
                "Both contentTime and timelineSpeedMutliplier must be null, or neither must be null. Cannot be only one of them.\n");
            return ControlTimestamp();
        }
        return ControlTimestamp(TimeStamp(contentTime, wallclockTime), timelineSpeedMultiplier);
    }
    return ControlTimestamp();
}

TimeStamp &ControlTimestamp::getTimestamp()
{
    return m_tstamp;
}

Json::Value SetupTSData::pack()
{
    Json::Value setupData;
    setupData["contentIdStem"] = m_contentIdStem;
    setupData["timelineSelector"] = m_timelineSelector;
    setupData["private"] = m_private;
    return setupData;
}

SetupTSData SetupTSData::unpack(const std::string &msg)
{
    Json::Value root = {};

    if (CSSUtilities::unpack(msg, root))
    {
        if (!root["timelineSelector"].isNull())
        {
            //TODO convert privateData
            return SetupTSData(root["contentIdStem"].asString(),
                root["timelineSelector"].asString(), {});
        }
    }

    return SetupTSData();
}

bool SetupTSData::isEmpty()
{
    return m_contentIdStem.empty() && m_timelineSelector.empty();
}

const std::string &SetupTSData::getContentIdStem() const
{
    return m_contentIdStem;
}

const std::string &SetupTSData::getTimelineSelector() const
{
    return m_timelineSelector;
}

bool AptEptLpt::isInDefaultState()
{
    Nullable<unsigned long long> contentTime;
    std::string wallclockTime;

    m_earliest.getTimeStamp(contentTime, wallclockTime);
    if (contentTime != 0 && wallclockTime != "-inf")
    {
        return false;
    }

    m_latest.getTimeStamp(contentTime, wallclockTime);
    if (contentTime != 0 && wallclockTime != "+inf")
    {
        return false;
    }

    if (m_actual.value().isNull())
    {
        return false;
    }

    return true;
}

Json::Value AptEptLpt::pack()
{
    Json::Value timestamps;

    timestamps["actual"] = m_actual.value().pack();
    timestamps["earliest"] = m_earliest.pack();
    timestamps["latest"] = m_latest.pack();

    return timestamps;
}

AptEptLpt AptEptLpt::unpack(const std::string &msg)
{
    Json::Value root = {};
    TimeStamp earliestTStamp;
    TimeStamp latestTStamp;
    TimeStamp actualTStamp;
    AptEptLpt timestamps;

    try
    {
        if (CSSUtilities::unpack(msg, root))
        {
            if (root["actual"]["wallClockTime"].isUInt64() &&
                root["actual"]["contentTime"].isUInt64())
            {
                if (!root["actual"]["wallClockTime"].isNull() &&
                    !root["actual"]["contentTime"].isNull())
                {
                    actualTStamp.setTimeStamp(root["actual"]["contentTime"].asUInt64(),
                        root["actual"]["wallClockTime"].asString());
                }

                if (!root["earliest"]["wallClockTime"].isNull() &&
                    !root["earliest"]["contentTime"].isNull())
                {
                    earliestTStamp.setTimeStamp(root["earliest"]["contentTime"].asUInt64(),
                        root["earliest"]["wallClockTime"].asString());
                }

                if (!root["latest"]["wallClockTime"].isNull() &&
                    !root["latest"]["contentTime"].isNull())
                {
                    latestTStamp.setTimeStamp(root["latest"]["contentTime"].asUInt64(),
                        root["latest"]["wallClockTime"].asString());
                }
            }
        }
    }
    catch (const std::exception &e)
    {
        LOG(LOG_ERROR, "Not all fields in SetupData message present as expected");
        return AptEptLpt();
    }

    return timestamps;
}

Nullable<ControlTimestamp> &SimpleTimeLineSource::getControlTimestamp(const
    std::string &timelineSelector)
{
    return m_ctStamp;
}

bool SimpleTimeLineSource::recognisesTimelineSelector(const std::string &timelineSelector)
{
    return TimelineSource::m_timelineSelector == timelineSelector;
}

SimpleClockTimelineSource::~SimpleClockTimelineSource()
{
    m_clock->unbind(this);
    m_wallClock->unbind(this);
    m_speedSource->unbind(this);
}

SimpleClockTimelineSource::SimpleClockTimelineSource(const std::string &timelineSelector,
                                                     ClockBase *wallClock, ClockBase *clock,
                                                     ClockBase *speedSource, bool autoUpdateClients)
{
    TimelineSource::m_timelineSelector = timelineSelector;
    m_wallClock = wallClock;
    m_clock = clock;
    m_changed = true;
    m_latestCt = ControlTimestamp();

    if (speedSource == nullptr)
    {
        m_speedSource = clock;
    }
    else
    {
        m_speedSource = speedSource;
    }

    m_autoUpdateClients = autoUpdateClients;
}

bool SimpleClockTimelineSource::recognisesTimelineSelector(const std::string &timelineSelector)
{
    return TimelineSource::m_timelineSelector == timelineSelector;
}

Nullable<ControlTimestamp> &SimpleClockTimelineSource::getControlTimestamp(const
    std::string &timelineSelector)
{
    if (m_changed)
    {
        m_changed = false;
        if (m_clock->isAvailable())
        {
            m_latestCt = ControlTimestamp(TimeStamp(m_clock->getTicks(), std::to_string(
                m_wallClock->getTicks())),
                m_speedSource->getSpeed());
        }
        else
        {
            m_latestCt = ControlTimestamp(
                TimeStamp(Nullable<unsigned long long>(), std::to_string(m_wallClock->getTicks())));
        }
    }

    return m_latestCt;
}

bool SimpleClockTimelineSource::attachSink(TimelineSyncService *tservice)
{
    bool result = TimelineSource::attachSink(tservice);

    if (result && TimelineSource::m_sinks.size() == 1)
    {
        m_clock->bind(this);
        m_wallClock->bind(this);

        if (m_clock != m_speedSource)
        {
            m_speedSource->bind(this);
        }
    }

    return result;
}

bool SimpleClockTimelineSource::removeSink(TimelineSyncService *tservice)
{
    bool result = TimelineSource::removeSink(tservice);

    if (result && TimelineSource::m_sinks.empty())
    {
        m_clock->unbind(this);
        m_wallClock->unbind(this);

        if (m_clock != m_speedSource)
        {
            m_speedSource->unbind(this);
        }
    }

    return result;
}

void SimpleClockTimelineSource::notify()
{
    m_changed = true;

    if (m_autoUpdateClients)
    {
        for (auto sink : TimelineSource::m_sinks)
        {
            if (sink.second)
            {
                sink.first->updateAllClients();
            }
        }
    }
}

//TS SERVICE
TimelineSyncService::TimelineSyncService(int port, ClockBase *wallClock,
                                         MediaSynchroniser *mediaSync,
                                         ContentIdentificationService *cii, std::string
                                         contentIdOverride) : WebSocketService("lws-ts", port,
                                                                               false, ""),
    m_wallclock{wallClock}, m_mediaSync(mediaSync), m_ciiService{cii}, m_contentIdOverride{
                                                                                           contentIdOverride}
{
}

void TimelineSyncService::setContentId(const std::string &cid, const bool forceUpdate)
{
    if (cid != m_contentId)
    {
        m_contentId = cid;
        std::string contentId = std::move(getContentId());
        if (m_contentId == contentId)
        {
            m_ciiService->setCIIMessageProperty("contentId", getContentId());
            if (forceUpdate)
            {
                updateAllClients();
                m_ciiService->updateClients(true);
            }
        }
    }
}

std::string TimelineSyncService::getContentId() const
{
    if (m_contentIdOverride.empty())
    {
        return m_contentId;
    }
    return m_contentIdOverride;
}

void TimelineSyncService::setContentIdOverride(const std::string &cid, const bool forceUpdate)
{
    if (cid != m_contentIdOverride)
    {
        m_contentIdOverride = cid;
        m_ciiService->setCIIMessageProperty("contentId", getContentId());
        m_ciiService->setCIIMessageProperty("contentIdStatus", "final");
        if (forceUpdate)
        {
            updateAllClients();
            m_ciiService->updateClients(true);
        }
    }
}

bool TimelineSyncService::OnConnection(WebSocketService::WebSocketConnection *connection)
{
    LOG(LOG_INFO, "%s connected to TS service\n", connection->Uri().c_str());
    return true;
}

void TimelineSyncService::OnDisconnected(WebSocketService::WebSocketConnection *connection)
{
    SetupTSData &currentSetupTSData = m_connectionSetupData[connection];

    if (!currentSetupTSData.isEmpty())
    {
        auto &tSel = const_cast<std::string &>(currentSetupTSData.getTimelineSelector());
        m_timelineSelectors[tSel] -= 1;

        if (m_timelineSelectors[tSel] == 0)
        {
            m_timelineSelectors.erase(tSel);
            for (auto &src : m_timelineSources)
            {
                src.first->timelineSelectorNotNeeded(tSel);
            }
            m_mediaSync->stopTimelineMonitoring(tSel);
        }

        m_connectionSetupData.erase(connection);
        m_connectionPreviousControlTimestamp.erase(connection);
    }

    LOG(LOG_INFO, "%s disconnected from TS service\n", connection->Uri().c_str());
}

void TimelineSyncService::OnMessageReceived(WebSocketService::WebSocketConnection *connection,
    const std::string &text)
{
    LOG(LOG_DEBUG, "TimelineSyncService::OnMessageReceived %s \n", text.c_str());

    if (m_connectionSetupData[connection].isEmpty())       //initial setup
    {
        configureConnectionWithSetupData(connection, text);
    }
    else
    {
        AptEptLpt AptEptLptCandidate = AptEptLpt::unpack(text);
        if (!AptEptLptCandidate.isInDefaultState())
        {
            //onClientAptEptLpt(AptEptLpt::unpack(msg))
            LOG(LOG_DEBUG, "TimelineSyncService::onClientAptEptLpt (ignore)\n");
        }
        else
        {
            LOG(LOG_DEBUG, "Received updated setup data from connection");
            configureConnectionWithSetupData(connection, text);
        }
    }
}

void TimelineSyncService::updateAllClients()
{
    for (auto const &connection : connections_)
    {
        updateClient(connection.second.get());
    }
}

void TimelineSyncService::updateClient(WebSocketService::WebSocketConnection *connection)
{
    if (!m_connectionSetupData[connection].isEmpty())
    {
        Nullable<ControlTimestamp> ct = ControlTimestamp(
            TimeStamp(Nullable<unsigned long long>(), std::to_string(m_wallclock->getTicks())));

        if (ciMatchesStem(getContentId(), m_connectionSetupData[connection].getContentIdStem()))
        {
            for (auto &src : m_timelineSources)
            {
                if (src.first->recognisesTimelineSelector(
                    m_connectionSetupData[connection].getTimelineSelector()))
                {
                    ct = src.first->getControlTimestamp(
                        m_connectionSetupData[connection].getTimelineSelector());
                }
            }
        }
        else
        {
            LOG(LOG_DEBUG, "ci stem does not match");
        }

        if (!ct.isNull() && isControlTimestampChanged(
            m_connectionPreviousControlTimestamp[connection], ct))
        {
            m_connectionPreviousControlTimestamp[connection] = ct;
            LOG(LOG_DEBUG, "Current Control timestamp: %s\n",
                ct.value().pack().toStyledString().c_str());
#if JSONCPP_VERSION_HEXA > 0x01080200
            connection->SendMessage(Json::writeString(m_wbuilder, ct.value().pack()));
#else
            connection->SendMessage(m_writer.write(ct.value().pack()));
#endif
        }
        else
        {
            LOG(LOG_DEBUG, "Control Timestamp is Null or not changed");
        }
    }
}

void TimelineSyncService::attachTimelineSource(TimelineSource *tls)
{
    tls->attachSink(this);
    m_timelineSources[tls] = true;
}

void TimelineSyncService::removeTimelineSource(TimelineSource *tls)
{
    tls->removeSink(this);
    m_timelineSources.erase(tls);
}

bool TimelineSyncService::ciMatchesStem(const std::string &contentId, const
    std::string &contentIdStem)
{
    return(!contentId.empty() && ((contentId.rfind(contentIdStem, 0) == 0) ||
                                  contentIdStem.empty()));
}

bool TimelineSyncService::isControlTimestampChanged(const Nullable<ControlTimestamp> &prev,
    const Nullable<ControlTimestamp> &latest) const
{
    // sanity check
    if (latest.isNull())
    {
        //raise error!!
        LOG(LOG_ERROR, "Latest control timestamp cannot be None");
        return false;
    }

    // if we don't have a previous CT yet, then we always take the latest
    if (prev.isNull())
    {
        return true;
    }

    // check if timeline remains unavailable, then irrespective of wallClockTime, it is unchanged
    if (prev.value().getTimestamp().isNull() && latest.value().getTimestamp().isNull())
    {
        return false;
    }

    if (prev != latest)
    {
        return true;
    }

    return false;
}

void TimelineSyncService::configureConnectionWithSetupData(
    WebSocketService::WebSocketConnection *connection, const std::string &text)
{
    m_connectionSetupData[connection] = SetupTSData::unpack(text);
    if (m_connectionSetupData[connection].isEmpty())
    {
        LOG(LOG_ERROR, "Unexpected setup data (%s) from %s\n", text.c_str(),
            connection->Uri().c_str());
    }
    else
    {
        std::string tSel = m_connectionSetupData[connection].getTimelineSelector();
        if (tSel.empty())
        {
            LOG(LOG_ERROR, "Setup Timeline Selector from %s cannot be empty\n",
                connection->Uri().c_str());
        }
        else
        {
            if (m_timelineSelectors.find(tSel) == m_timelineSelectors.end())
            {
                m_timelineSelectors[tSel] = 1;
                m_mediaSync->startTimelineMonitoring(tSel, false);
            }
            else
            {
                m_timelineSelectors[tSel] += 1;
            }

            if (m_timelineSelectors[tSel] == 1)
            {
                for (auto &src : m_timelineSources)
                {
                    src.first->timelineSelectorNeeded(tSel);
                }
            }
            updateClient(connection);
        }
    }
}
}
