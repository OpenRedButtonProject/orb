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

#include "media_synchroniser.h"
#include "log.h"
#include "CSSUtilities.h"
#include "service_manager.h"
#include "CorrelatedClock.h"

#include <algorithm>
#include <memory>
#include <cmath>

#define CSS_SERVICE_COUNT 3;

using namespace NetworkServices;

/*
 * Factory class for SimpleClockTimeline objects to organize memory
 * management, due to the required argument of a CorrelatedClock
 * object.
 */
class SimpleClockTimelineSourceFactory {
public:
    SimpleClockTimelineSourceFactory() = delete;
    static TimelineSource* Make(const std::string &timelineSelector, ClockBase *wallclock, double
        tickRate)
    {
        CorrelatedClock *correlatedClock = new CorrelatedClock(wallclock, Correlation(
            wallclock->getTicks(), 0), tickRate);
        SimpleClockTimelineSource *tls = new SimpleClockTimelineSource(timelineSelector, wallclock,
            correlatedClock);
        correlatedClocks_[tls] = correlatedClock;
        return tls;
    }

    static void Destroy(TimelineSource *tls)
    {
        auto it = correlatedClocks_.find(tls);
        if (it != correlatedClocks_.end())
        {
            delete it->first;
            delete it->second;
            correlatedClocks_.erase(it);
        }
    }

    static CorrelatedClock* GetCorrelatedClock(TimelineSource *tls)
    {
        auto it = correlatedClocks_.find(tls);
        if (it != correlatedClocks_.end())
        {
            return it->second;
        }
        return nullptr;
    }

private:
    static std::unordered_map<TimelineSource *, CorrelatedClock *> correlatedClocks_;
};

void MediaSynchroniser::MediaSyncServiceCallback::OnStopped()
{
    std::lock_guard<std::recursive_mutex> lockGuard(mMediaSync->m_mutex);
    LOG(LOG_DEBUG, "Called MediaSyncServiceCallback::OnStopped().\n");

    // if one of the media synchroniser services is stopped abtruptly,
    // stop the others as well
    mMediaSync->disableInterDeviceSync();
    if (--mMediaSync->m_runningServices <= 0)
    {
        mMediaSync->m_tsService = -1;
        mMediaSync->m_ciiService = -1;
        mMediaSync->m_tsService = -1;
        if (mMediaSync->m_delete)
        {
            delete mMediaSync;
        }
    }
}

std::unordered_map<TimelineSource *,
                   CorrelatedClock *> SimpleClockTimelineSourceFactory::correlatedClocks_ = {};

////////////////////////////////////////////////////////
////////////////// MediaSynchroniser ///////////////////
////////////////////////////////////////////////////////

MediaSynchroniser::MediaSynchroniser(const int &id,
                                     std::shared_ptr<MediaSyncCallback> mediaSyncCallback, const
                                     int &ciiPort, const int &wcPort, const
                                     int &tsPort) :
    m_id(id),
    m_mediaSyncCallback(std::move(mediaSyncCallback)),
    m_delete(false),
    m_initialised(false),
    m_runningServices(0),
    m_wcService(-1),
    m_ciiService(-1),
    m_tsService(-1),
    m_syncing(false),
    m_ciiPort(ciiPort),
    m_wcPort(wcPort),
    m_tsPort(tsPort),
    m_sysClock(1000000000, 45),
    m_currentCSSId(""),
    m_currentCSSresentationStatus(""),
    m_contentCSSIdStatus("")
{
    LOG(LOG_INFO, "MediaSynchroniser ctor. id=%d\n", id);
    m_ciiProps.setProperty("teUrl", Json::Value::null);
    m_ciiProps.setProperty("mrsUrl", Json::Value::null);
}

MediaSynchroniser::~MediaSynchroniser()
{
    std::string selector;
    for (auto &it : m_timelineSources)
    {
        if (it.second != nullptr)
        {
            selector = it.second->getTimelineSelector();
            if (selector == m_masterTimeline)
            {
                //delete master timeline last
                continue;
            }
            SimpleClockTimelineSourceFactory::Destroy(it.second);
        }
    }

    if (m_timelineSources[m_masterTimeline] != nullptr)
    {
        SimpleClockTimelineSourceFactory::Destroy(m_timelineSources[m_masterTimeline]);
    }

    LOG(LOG_INFO, "MediaSynchroniser dtor. id=%d\n", m_id);
}

void MediaSynchroniser::initialise(const bool &isMasterBroadcast, std::string dvbUri, bool
    permanentError, bool presenting)
{
    std::lock_guard<std::recursive_mutex> lockGuard(m_mutex);
    m_isMasterBroadcast = isMasterBroadcast;
    m_initialised = true;
    updateBroadcastContentStatus(dvbUri, permanentError, presenting);
}

void MediaSynchroniser::deleteLater()
{
    LOG(LOG_INFO, "MediaSynchroniser::deleteLater. id=%d\n", m_id);
    std::lock_guard<std::recursive_mutex> lockGuard(m_mutex);
    if (!m_delete)
    {
        m_delete = true;

        for (auto &it : m_timelines)
        {
            if (it.second.temiFilterId != -1)
            {
                m_mediaSyncCallback->stopTEMITimelineMonitoring(it.second.temiFilterId);
            }
        }
        m_timelines.clear();

        if (m_runningServices <= 0)
        {
            delete this;
        }
        else if (m_syncing)
        {
            disableInterDeviceSync();
        }
    }
}

void MediaSynchroniser::updateBroadcastContentStatus(std::string dvbUri, bool permanentError, bool
    presenting)
{
    std::lock_guard<std::recursive_mutex> lockGuard(m_mutex);
    if (presenting)
    {
        for (auto &it : m_timelines)
        {
            if (it.second.temiFilterId == -1 && it.second.temiComponentTag != -1 &&
                it.second.temiTimelineId != -1)
            {
                it.second.temiFilterId = m_mediaSyncCallback->startTEMITimelineMonitoring(
                    it.second.temiComponentTag, it.second.temiTimelineId);
            }
        }
    }
    if (m_isMasterBroadcast)
    {
        std::string contentId = dvbUri;
        std::string mrsUrl = "";
        std::string presentationStatus;
        std::string contentIdStatus;

        if (permanentError)
        {
            presentationStatus = "fault";
        }
        else if (presenting)
        {
            presentationStatus = "okay";
        }
        else
        {
            presentationStatus = "transitioning";
        }

        if (presenting)
        {
            contentIdStatus = "final";
        }
        else
        {
            contentIdStatus = "partial";
        }

        updateCssCiiProperties(contentId, presentationStatus, contentIdStatus, mrsUrl);
    }
}

void MediaSynchroniser::updateCssCiiProperties(const std::string &contentId, const
    std::string &presentationStatus, const std::string &contentIdStatus, const std::string &mrsUrl)
{
    LOG(LOG_INFO, "MediaSynchroniser::updateCssCiiProperties(%s, %s, %s, %s)\n", contentId.c_str(),
        presentationStatus.c_str(), contentIdStatus.c_str(), mrsUrl.c_str());
    std::lock_guard<std::recursive_mutex> lockGuard(m_mutex);
    Json::Value properties;
    properties["contentId"] = contentId;
    properties["presentationStatus"] = presentationStatus;
    properties["contentIdStatus"] = contentIdStatus;
    if (!mrsUrl.empty())
    {
        properties["mrsUrl"] = mrsUrl;
    }

    for (auto it = properties.begin(); it != properties.end(); ++it)
    {
        if (it.key().asString() == "contentId")
        {
            setContentId(it->asString());
        }
        else if (it.key().asString() != "contentIdStatus" || getContentIdOverride().empty())
        {
            m_ciiProps.setProperty(it.key().asString(), *it);
        }
    }

    if (!m_currentCSSId.empty() && m_currentCSSresentationStatus != presentationStatus &&
        m_contentCSSIdStatus != contentIdStatus)
    {
        updateAllCIIClients();
    }

    m_currentCSSId = contentId;
    m_currentCSSresentationStatus = presentationStatus;
    m_contentCSSIdStatus = contentIdStatus;

    updateAllTSClients();
}

bool MediaSynchroniser::enableInterDeviceSync(const std::string &ipAddr)
{
    std::lock_guard<std::recursive_mutex> lockGuard(m_mutex);
    bool ret = false;
    if (m_runningServices <= 0 && m_initialised)
    {
        ServiceManager &mngr = ServiceManager::GetInstance();
        m_ciiProps.setProperty("wcUrl", "udp://" + ipAddr + ":" + std::to_string(m_wcPort));
        m_ciiProps.setProperty("tsUrl", "ws://" + ipAddr + ":" + std::to_string(m_tsPort));
        m_wcService = mngr.StartWallClockService(
            std::make_unique<MediaSyncServiceCallback>(this),
            m_wcPort,
            &m_sysClock);
        m_ciiService = mngr.StartContentIdentificationService(
            std::make_unique<MediaSyncServiceCallback>(this),
            m_ciiPort,
            &m_ciiProps);
        m_tsService = mngr.StartTimelineSyncService(
            std::make_unique<MediaSyncServiceCallback>(this),
            m_tsPort,
            &m_sysClock,
            this,
            m_ciiService);
        if (m_ciiService > -1 && m_wcService > -1 && m_tsService > -1)
        {
            m_syncing = true;
            m_runningServices = CSS_SERVICE_COUNT;
            ret = true;
            TimelineSyncService *ts =
                ServiceManager::GetInstance().FindService<TimelineSyncService>(m_tsService);
            ts->setContentId(m_contentId);
            ts->setContentIdOverride(m_contentIdOverride);
            for (auto &it : m_timelineSources)
            {
                ts->attachTimelineSource(it.second);
            }
            m_mediaSyncCallback->dispatchInterDeviceSyncEnabled(m_id);
            LOG(LOG_INFO, "Started all CS servers successfully.\n");
        }
        else
        {
            LOG(LOG_ERROR, "Failed to start all CSS servers.\n");
            disableInterDeviceSync();
        }
    }
    return ret;
}

void MediaSynchroniser::disableInterDeviceSync()
{
    std::lock_guard<std::recursive_mutex> lockGuard(m_mutex);
    if (m_syncing)
    {
        m_syncing = false;
        ServiceManager &mngr = ServiceManager::GetInstance();
        mngr.StopService(m_tsService);
        mngr.StopService(m_ciiService);
        mngr.StopService(m_wcService);
        m_mediaSyncCallback->dispatchInterDeviceSyncDisabled(m_id);
        LOG(LOG_INFO, "Stopped all CSS servers.\n");

        m_currentCSSId.clear();
        m_currentCSSresentationStatus.clear();
        m_contentCSSIdStatus.clear();
    }
}

int MediaSynchroniser::nrOfSlaves()
{
    std::lock_guard<std::recursive_mutex> lockGuard(m_mutex);
    int ret = -1;
    ContentIdentificationService *cii =
        ServiceManager::GetInstance().FindService<ContentIdentificationService>(m_ciiService);
    if (cii != nullptr)
    {
        ret = cii->nrOfClients();
    }
    return ret;
}

void MediaSynchroniser::updateAllCIIClients()
{
    LOG(LOG_INFO, "MediaSynchroniser::updateAllCIIClients().\n");
    ContentIdentificationService *cii =
        ServiceManager::GetInstance().FindService<ContentIdentificationService>(m_ciiService);
    if (cii != nullptr)
    {
        cii->updateClients(true);
    }
}

void MediaSynchroniser::updateAllTSClients()
{
    TimelineSyncService *ts = ServiceManager::GetInstance().FindService<TimelineSyncService>(
        m_tsService);
    LOG(LOG_DEBUG, "MediaSynchroniser::updateAllTSClients(). tsService: %p\n", ts);
    if (ts != nullptr)
    {
        ts->updateAllClients();
    }
}

void MediaSynchroniser::setContentId(const std::string &cid)
{
    TimelineSyncService *ts = ServiceManager::GetInstance().FindService<TimelineSyncService>(
        m_tsService);
    m_contentId = cid;
    if (ts != nullptr)
    {
        ts->setContentId(cid, false);
    }
}

void MediaSynchroniser::setContentIdOverride(const std::string &cid, const bool forceUpdate)
{
    std::lock_guard<std::recursive_mutex> lockGuard(m_mutex);
    TimelineSyncService *ts = ServiceManager::GetInstance().FindService<TimelineSyncService>(
        m_tsService);
    m_contentIdOverride = cid;
    if (ts != nullptr)
    {
        ts->setContentIdOverride(cid, forceUpdate);
    }
}

u_int64_t MediaSynchroniser::getContentTime(const std::string &timelineSelector, bool &success)
{
    std::lock_guard<std::recursive_mutex> lockGuard(m_mutex);

    // TODO: should compare and also check performance!
    // directlyFromDVB = false -> will return time based on our own clock abstractions
    //                            (should be synced at least once with dvbstack TEMI)
    // directlyFromDVB = true ->  will request time from dvbstack TEMI continuously
    bool directlyFromDVB = false;
    u_int64_t ticks = 0;
    success = false;
    TimelineSource *tls = _getTimelineSource(timelineSelector);

    if (!directlyFromDVB && tls != nullptr)
    {
        CorrelatedClock *clock = SimpleClockTimelineSourceFactory::GetCorrelatedClock(tls);
        if (clock != nullptr)
        {
            ticks = clock->getTicks();
        }
    }
    else
    {
        if (timelineSelector.find(":temi:") != std::string::npos)
        {
            ticks = m_mediaSyncCallback->getCurrentTemiTime(
                m_timelines[timelineSelector].temiFilterId);
        }
        else if (timelineSelector.find(":pts") != std::string::npos)
        {
            ticks = m_mediaSyncCallback->getCurrentPtsTime();
        }
    }

    if (ticks >= 0)
    {
        success = true;
    }

    return ticks;
}

bool MediaSynchroniser::setContentTimeAndSpeed(std::string timelineSelector, const
    uint64_t &contentTime, const double &speed)
{
    std::lock_guard<std::recursive_mutex> lockGuard(m_mutex);
    LOG(LOG_DEBUG, "Update content time %llu and speed %f\n", contentTime, speed);
    TimelineSource *tls = _getTimelineSource(timelineSelector);
    if (tls != nullptr)
    {
        CorrelatedClock *clock = SimpleClockTimelineSourceFactory::GetCorrelatedClock(tls);
        if (clock != nullptr)
        {
            std::unordered_map<std::string, u_int64_t> config;
            config["childTicks"] = contentTime; //contentTime in ticks
            config["parentTicks"] = clock->getParent()->getTicks();
            clock->setCorrelationAndSpeed(clock->getCorrelation().butWith(config), speed);
        }
    }

    updateAllTSClients();
    return true;
}

bool MediaSynchroniser::setTimelineAvailability(const std::string &timelineSelector, const
    bool &isAvailable, const u_int64_t currentTime, const double speed)
{
    LOG(LOG_DEBUG, "MediaSynchroniser::setTimelineAvailability(%s, %d, %llu, %f)",
        timelineSelector.c_str(), isAvailable, currentTime, speed);
    bool result = false;
    TimelineSource *tls = _getTimelineSource(timelineSelector);
    if (tls != nullptr)
    {
        CorrelatedClock *clock = SimpleClockTimelineSourceFactory::GetCorrelatedClock(tls);
        if (clock != nullptr)
        {
            clock->SetAvailability(isAvailable);
            result = true;
        }
    }
    if (isAvailable)
    {
        addTimeline(timelineSelector);
        setContentTimeAndSpeed(timelineSelector, currentTime, speed);
        m_mediaSyncCallback->dispatchTimelineAvailableEvent(timelineSelector);
    }
    else
    {
        m_mediaSyncCallback->dispatchTimelineUnavailableEvent(timelineSelector);
        updateAllTSClients();
    }
    return result;
}

bool MediaSynchroniser::setTEMITimelineAvailability(const int &filterId, const bool &isAvailable,
    const u_int64_t &currentTime, const u_int64_t& timescale, const double &speed)
{
    LOG(LOG_DEBUG, "MediaSynchroniser::setTEMITimelineAvailability(%d, %d, %llu, %llu, %f)",
        filterId, isAvailable, currentTime, timescale, speed);
    std::lock_guard<std::recursive_mutex> lockGuard(m_mutex);
    bool result = false;

    auto it = std::find_if(m_timelines.begin(), m_timelines.end(),
        [&](auto& p) {
        return p.second.temiFilterId == filterId;
    });

    if (it != m_timelines.end())
    {
        TimelineSource *tls = _getTimelineSource(it->first);
        if (tls != nullptr)
        {
            CorrelatedClock *clock = SimpleClockTimelineSourceFactory::GetCorrelatedClock(tls);
            if (clock != nullptr)
            {
                clock->SetAvailability(isAvailable);
                result = true;
            }
        }
        if (isAvailable)
        {
            it->second.timeline["timelineProperties"]["unitsPerTick"] = 1;
            it->second.timeline["timelineProperties"]["unitsPerSecond"] = timescale;
            addTimeline(it->first);
            setContentTimeAndSpeed(it->first, currentTime, speed);
            m_mediaSyncCallback->dispatchTimelineAvailableEvent(it->first, timescale);
        }
        else
        {
            m_mediaSyncCallback->dispatchTimelineUnavailableEvent(it->first);
            updateAllTSClients();
        }
    }
    return result;
}

bool MediaSynchroniser::startTimelineMonitoring(const std::string &timelineSelector, const
    bool &isMaster)
{
    bool result = false;
    std::lock_guard<std::recursive_mutex> lockGuard(m_mutex);
    if (m_initialised)
    {
        TimelineWrapper &timelineWrapper = m_timelines[timelineSelector];
        if (timelineWrapper.numWatchers <= 0)
        {
            if (timelineWrapper.pendingWatchers <= 0)
            {
                if (ParseTimelineSelector(timelineSelector, timelineWrapper))
                {
                    LOG(LOG_DEBUG, "Initiating timeline monitoring for timelineSelector '%s'.\n",
                        timelineSelector.c_str());
                    timelineWrapper.pendingWatchers = 1;
                    result = true;
                    if (isMaster)
                    {
                        m_masterTimeline = timelineSelector;
                    }
                    if (timelineWrapper.temiComponentTag != -1 && timelineWrapper.temiTimelineId !=
                        -1)
                    {
                        int filterId = m_mediaSyncCallback->startTEMITimelineMonitoring(
                            timelineWrapper.temiComponentTag, timelineWrapper.temiTimelineId);
                        if (filterId >= 0)
                        {
                            timelineWrapper.temiFilterId = filterId;
                            LOG(LOG_INFO, "Awaiting TEMI notification event for timeline '%s'.\n",
                                timelineSelector.c_str());
                        }
                        else
                        {
                            LOG(LOG_ERROR,
                                "Failed to start timeline monitoring for temi timeline '%s'.\n",
                                timelineSelector.c_str());
                        }
                    }
                    else
                    {
                        if (timelineSelector.compare(timelineSelector.length() - 4, 4, ":pts") == 0)
                        {
                            addTimeline(timelineSelector);
                        }
                        else
                        {
                            LOG(LOG_INFO,
                                "Awaiting call to setTimelineAvailability for timeline '%s'.\n",
                                timelineSelector.c_str());
                        }
                    }
                }
                else
                {
                    m_timelines.erase(timelineSelector);
                    LOG(LOG_ERROR, "Invalid timeline selector '%s'.\n", timelineSelector.c_str());
                }
            }
            else
            {
                timelineWrapper.pendingWatchers++;
                LOG(LOG_DEBUG,
                    "Incremented pending timeline monitoring counter for timeline selector '%s'. Current count is now %d.\n",
                    timelineSelector.c_str(), timelineWrapper.pendingWatchers);
            }
        }
        else
        {
            timelineWrapper.numWatchers++;
            LOG(LOG_DEBUG,
                "Incremented timeline monitoring counter for timeline selector '%s'. Current count is now %d.\n",
                timelineSelector.c_str(), timelineWrapper.numWatchers);
        }
    }
    else
    {
        LOG(LOG_ERROR,
            "Cannot start timeline monitoring on a media synchroniser that is not initialised.\n");
    }

    return result;
}

bool MediaSynchroniser::stopTimelineMonitoring(std::string timelineSelector, bool forceStop)
{
    bool ret = false;
    std::lock_guard<std::recursive_mutex> lockGuard(m_mutex);

    TimelineWrapper &timelineWrapper = m_timelines[timelineSelector];
    if (forceStop || (timelineWrapper.numWatchers == 1 && timelineWrapper.pendingWatchers <= 0))
    {
        if (timelineWrapper.temiFilterId != -1)
        {
            m_mediaSyncCallback->stopTEMITimelineMonitoring(timelineWrapper.temiFilterId);
        }
        removeTimeline(timelineSelector);
        ret = true;
    }
    else if (timelineWrapper.pendingWatchers > 0)
    {
        timelineWrapper.pendingWatchers--;
        LOG(LOG_DEBUG,
            "Decremented pending timeline monitoring counter for timelineSelector '%s'. Current count is now %d.\n",
            timelineSelector.c_str(), timelineWrapper.pendingWatchers);
    }
    else if (timelineWrapper.numWatchers > 1)
    {
        timelineWrapper.numWatchers--;
        LOG(LOG_DEBUG,
            "id=%d. Decremented reference counter for timelineSelector '%s'. Current count is now %d.\n",
            m_id, timelineSelector.c_str(), timelineWrapper.numWatchers);
    }
    else
    {
        // normally, we should never end up in this block
        m_timelines.erase(timelineSelector);
    }
    return ret;
}

void MediaSynchroniser::addTimeline(const std::string &timelineSelector)
{
    std::lock_guard<std::recursive_mutex> lockGuard(m_mutex);
    TimelineWrapper &timelineWrapper = m_timelines[timelineSelector];
    Json::Value timeline = timelineWrapper.timeline;
    if (!timeline.isNull())
    {
        if (timelineWrapper.numWatchers <= 0)
        {
            timelineWrapper.flushPendingWatchers();
            if (timelineWrapper.numWatchers > 0)
            {
                LOG(LOG_DEBUG, "Adding timeline for timelineSelector '%s'\n",
                    timelineSelector.c_str());

                TimelineSource *tls = _getTimelineSource(timelineSelector);
                if (tls == nullptr)
                {
                    LOG(LOG_DEBUG, "Creating timeline source for timelineSelector '%s'...\n",
                        timelineSelector.c_str());

                    Json::Value temp = std::move(m_ciiProps.getProperty("timelines"));
                    ClockBase *clockBase = &m_sysClock;
                    if (m_masterTimeline != timelineSelector)
                    {
                        tls = _getTimelineSource(m_masterTimeline);
                        if (tls != nullptr)
                        {
                            // TODO: changing parent clock somehow messes wallclock ticks in tsservice messages
                            //clockBase = SimpleClockTimelineSourceFactory::GetCorrelatedClock(tls);
                        }
                    }
                    tls = SimpleClockTimelineSourceFactory::Make(
                        timeline["timelineSelector"].asString(),
                        clockBase,
                        timeline["timelineProperties"]["unitsPerSecond"].asUInt64() /
                        timeline["timelineProperties"]["unitsPerTick"].asDouble());

                    if (m_masterTimeline == timelineSelector)
                    {
                        m_masterTimeline = timelineSelector;
                        clockBase = SimpleClockTimelineSourceFactory::GetCorrelatedClock(tls);
                        if (clockBase != nullptr)
                        {
                            for (auto &it : m_timelineSources)
                            {
                                CorrelatedClock *clock =
                                    SimpleClockTimelineSourceFactory::GetCorrelatedClock(it.second);
                                if (clock != nullptr)
                                {
                                    clock->setParent(clockBase);
                                }
                            }
                        }
                    }
                    m_timelineSources[timelineSelector] = tls;
                    temp.append(timeline);
                    m_ciiProps.setProperty("timelines", std::move(temp));
                    TimelineSyncService *ts =
                        ServiceManager::GetInstance().FindService<TimelineSyncService>(m_tsService);
                    if (ts != nullptr)
                    {
                        ts->attachTimelineSource(tls);
                    }

                    if (timelineSelector.compare(timelineSelector.length() - 4, 4, ":pts") == 0)
                    {
                        setContentTimeAndSpeed(timelineSelector,
                            m_mediaSyncCallback->getCurrentPtsTime(), 1);
                        m_mediaSyncCallback->dispatchTimelineAvailableEvent(timelineSelector);
                    }

                    updateAllCIIClients();
                    updateAllTSClients();
                }
                else
                {
                    LOG(LOG_DEBUG, "A timeline source for timelineSelector '%s' already exists.\n",
                        timelineSelector.c_str());
                }
            }
            else
            {
                m_timelines.erase(timelineSelector);
                LOG(LOG_ERROR, "Pending timelines counter is 0!!!\n");
                return;
            }
        }
        else
        {
            timelineWrapper.flushPendingWatchers();
        }
        LOG(LOG_DEBUG,
            "id=%d. Incremented reference counter for timelineSelector '%s'. Current count is now %d.\n",
            m_id, timelineSelector.c_str(), timelineWrapper.numWatchers);
    }
    else
    {
        LOG(LOG_ERROR, "Timeline is null!!!.\n");
        m_timelines.erase(timelineSelector);
    }
}

void MediaSynchroniser::removeTimeline(const std::string &timelineSelector)
{
    LOG(LOG_DEBUG, "MediaSynchroniser::removeTimeline %s", timelineSelector.c_str());
    std::lock_guard<std::recursive_mutex> lockGuard(m_mutex);
    TimelineWrapper &timelineWrapper = m_timelines[timelineSelector];
    TimelineSource *tls = _getTimelineSource(timelineSelector);
    if (tls != nullptr)
    {
        Json::Value temp = m_ciiProps.getProperty("timelines");
#if JSONCPP_VERSION_HEXA > 0x01080200
#else
        Json::Value newTimeline;
        int count = 0;
#endif
        for (Json::Value::ArrayIndex i = 0; i != temp.size(); i++)
        {
            const Json::Value &jobj = temp[i];
#if JSONCPP_VERSION_HEXA > 0x01080200
            if (jobj["timelineSelector"] == timelineSelector)
            {
                Json::Value removed;
                temp.removeIndex(i, &removed);
                m_ciiProps.setProperty("timelines", std::move(temp));
                updateAllCIIClients();
                break;
            }
#else
            if (jobj["timelineSelector"] != timelineSelector)
            {
                newTimeline[count] = temp[i];
                count++;
            }
#endif
        }
#if JSONCPP_VERSION_HEXA > 0x01080200
#else
        if (newTimeline.size() > 0)
        {
            m_ciiProps.setProperty("timelines", std::move(newTimeline));
            updateAllCIIClients();
        }
#endif
        CorrelatedClock *clock;
        if (m_masterTimeline == timelineSelector)
        {
            m_masterTimeline = "";
            for (auto &it : m_timelineSources)
            {
                clock = SimpleClockTimelineSourceFactory::GetCorrelatedClock(it.second);
                if (clock != nullptr)
                {
                    clock->setParent(&m_sysClock);
                }
            }
        }

        clock = SimpleClockTimelineSourceFactory::GetCorrelatedClock(tls);
        if (clock != nullptr)
        {
            clock->SetAvailability(false);
        }
        TimelineSyncService *ts = ServiceManager::GetInstance().FindService<TimelineSyncService>(
            m_tsService);
        if (ts != nullptr)
        {
            ts->removeTimelineSource(tls);
        }

        updateAllTSClients();

        SimpleClockTimelineSourceFactory::Destroy(tls);
        m_timelineSources.erase(timelineSelector);

        m_mediaSyncCallback->dispatchTimelineUnavailableEvent(timelineSelector);
        LOG(LOG_DEBUG, "Removed timeline source with timelineSelector '%s'.\n",
            timelineSelector.c_str());
    }
    else
    {
        LOG(LOG_DEBUG,
            "A timeline source for timeline selector '%s' was not found. Cleaning up counters...\n",
            timelineSelector.c_str());
    }
    m_timelines.erase(timelineSelector);
}

TimelineSource * MediaSynchroniser::_getTimelineSource(std::string timelineSelector)
{
    TimelineSource *tls = m_timelineSources[timelineSelector];
    if (tls == nullptr)
    {
        // erase the null entry which was created by the previous call to operator[]
        m_timelineSources.erase(timelineSelector);
    }
    return tls;
}

////////////////////////////////////////////////////////
/////////////// MediaSynchroniserManager ///////////////
////////////////////////////////////////////////////////

MediaSynchroniserManager::MediaSynchroniserManager(
    std::shared_ptr<MediaSyncCallback> mediaSyncCallback, const int &ciiPort, const int &wcPort,
    const int &tsPort) :
    m_mediaSyncCallback(mediaSyncCallback),
    m_idCounter(0),
    m_activeMediaSync(-1),
    m_ciiPort(ciiPort),
    m_wcPort(wcPort),
    m_tsPort(tsPort)
{
    LOG(LOG_DEBUG, "MediaSynchroniserManager ctor.\n");
}

MediaSynchroniserManager::~MediaSynchroniserManager()
{
    LOG(LOG_DEBUG, "MediaSynchroniserManager dtor.\n");
    releaseResources();
}

int MediaSynchroniserManager::createMediaSynchroniser()
{
    std::lock_guard<std::recursive_mutex> lockGuard(m_mutex);
    int id = m_idCounter++;
    m_mediaSyncs[id] = new MediaSynchroniser(id, m_mediaSyncCallback, m_ciiPort, m_wcPort,
        m_tsPort);
    return id;
}

void MediaSynchroniserManager::destroyMediaSynchroniser(const int &id)
{
    std::lock_guard<std::recursive_mutex> lockGuard(m_mutex);
    if (id == m_activeMediaSync)
    {
        m_activeMediaSync = -1;
    }
    MediaSynchroniser *mediaSync = m_mediaSyncs[id];
    if (mediaSync != nullptr)
    {
        mediaSync->deleteLater();
    }
    m_mediaSyncs.erase(id);
}

MediaSynchroniser * MediaSynchroniserManager::getMediaSynchroniser(const int &id)
{
    std::lock_guard<std::recursive_mutex> lockGuard(m_mutex);
    MediaSynchroniser *mediaSync = m_mediaSyncs[id];
    if (mediaSync == nullptr)
    {
        // erase the null entry which was created by the previous call to operator[]
        m_mediaSyncs.erase(id);
    }
    return mediaSync;
}

MediaSynchroniser * MediaSynchroniserManager::getActiveMediaSynchroniser()
{
    std::lock_guard<std::recursive_mutex> lockGuard(m_mutex);
    return getMediaSynchroniser(m_activeMediaSync);
}

bool MediaSynchroniserManager::initMediaSynchroniser(const int &id, bool isMasterBroadcast)
{
    std::lock_guard<std::recursive_mutex> lockGuard(m_mutex);
    bool result = false;
    if (id != m_activeMediaSync)
    {
        destroyMediaSynchroniser(m_activeMediaSync);
    }
    MediaSynchroniser *mediaSync = getMediaSynchroniser(id);
    if (mediaSync != nullptr)
    {
        mediaSync->initialise(isMasterBroadcast, m_dvbUri, m_dvbPermanentError, m_dvbPresenting);
        m_activeMediaSync = id;
        result = true;
    }
    return result;
}

void MediaSynchroniserManager::updateDvbInfo(const int &onetId, const int &transId,
    const int &servId, const bool &permanentError,
    const bool &presenting, const std::string &programmeId,
    const std::time_t &startTime, const std::time_t &duration)
{
    char uriBuffer[32];
    std::string ciString;
    std::size_t semicolonPos = programmeId.find(';');
    std::string formattedProgrammeId;

    if (semicolonPos != std::string::npos && semicolonPos + 1 < programmeId.length())
    {
        std::string extractedId = programmeId.substr(semicolonPos + 1);
        char programmeIdBuffer[5];
        sprintf(programmeIdBuffer, "%04x", std::stoi(extractedId, nullptr, 16));
        formattedProgrammeId = programmeIdBuffer;
    }
    else
    {
        formattedProgrammeId = programmeId;
    }

    if (!formattedProgrammeId.empty())
    {
        char ciBuffer[512];
        sprintf(ciBuffer, ";%s~%s--PT%02ldH%02ldM", formattedProgrammeId.c_str(),
            MediaSynchroniser::GetDvbDateFromTimestamp(startTime).c_str(), duration / 3600,
            (duration % 3600) / 60);
        ciString = ciBuffer;
    }
    sprintf(uriBuffer, "dvb://%04x.%04x.%04x", onetId, transId, servId);
    m_dvbUri = uriBuffer + ciString;
    LOG(LOG_DEBUG, "MediaSynchroniserManager::updateDvbInfo(%s,%d,%d).\n", m_dvbUri.c_str(),
        permanentError, presenting);
    m_dvbPermanentError = permanentError;
    m_dvbPresenting = presenting;

    MediaSynchroniser *mediaSync = getActiveMediaSynchroniser();
    if (mediaSync != nullptr)
    {
        mediaSync->updateBroadcastContentStatus(m_dvbUri, m_dvbPermanentError, m_dvbPresenting);
    }
}

void MediaSynchroniserManager::releaseResources()
{
    std::lock_guard<std::recursive_mutex> lockGuard(m_mutex);
    LOG(LOG_DEBUG, "MediaSynchroniserManager::releaseResources().\n");
    for (auto &it : m_mediaSyncs)
    {
        it.second->deleteLater();
    }
    m_mediaSyncs.clear();
    m_activeMediaSync = -1;
}

////////////////////////////////////////////////////////
//////////////////// static methods ////////////////////
////////////////////////////////////////////////////////

bool MediaSynchroniser::ParseTimelineSelector(const std::string &timelineSelector,
    TimelineWrapper &timelineWrapper)
{
    Json::Value &timeline = timelineWrapper.timeline;
    std::size_t pos = timelineSelector.find(":timeline:");
    bool result = false;
    if (pos != std::string::npos)
    {
        std::string str = timelineSelector.substr(pos + 10);
        std::vector<std::string> selectorParts;
        std::istringstream f(str);
        std::string s;
        while (getline(f, s, ':'))
        {
            selectorParts.push_back(s);
        }
        if (selectorParts.size() > 0)
        {
            if (selectorParts[0] == "html-media-timeline")
            {
                result = CSSUtilities::unpack("{\"timelineSelector\":\"" + timelineSelector +
                    "\",\"timelineProperties\":{\"unitsPerTick\":1,\"unitsPerSecond\":1000}}",
                    timeline);
            }
            else if (selectorParts[0] == "pts")
            {
                result = CSSUtilities::unpack("{\"timelineSelector\":\"" + timelineSelector +
                    "\",\"timelineProperties\":{\"unitsPerTick\":1,\"unitsPerSecond\":90000}}",
                    timeline);
            }
            else if (selectorParts[0] == "mpd")
            {
                result = CSSUtilities::unpack("{\"timelineSelector\":\"" + timelineSelector +
                    "\",\"timelineProperties\":{\"unitsPerTick\":1,\"unitsPerSecond\":1000}}",
                    timeline);
                if (result && selectorParts.size() > 3)
                {
                    timeline["timelineProperties"]["unitsPerSecond"] =
                        static_cast<Json::UInt64>(std::stoull(selectorParts[3]));
                }
            }
            else if (selectorParts[0] == "temi" && selectorParts.size() >= 3)
            {
                result = CSSUtilities::unpack("{\"timelineSelector\":\"" + timelineSelector +
                    "\"},\"componentTag\":" + selectorParts[1] + ",\"timelineId\":" +
                    selectorParts[2] +
                    "}", timeline);
                timelineWrapper.temiComponentTag = std::stoi(selectorParts[1]);
                timelineWrapper.temiTimelineId = std::stoi(selectorParts[2]);
            }
        }
    }
    return result;
}

std::string MediaSynchroniser::GetDvbDateFromTimestamp(const std::time_t &timestamp)
{
    std::tm *timeinfo = std::localtime(&timestamp);
    int year = timeinfo->tm_year + 1900;
    int month = timeinfo->tm_mon + 1;
    int day = timeinfo->tm_mday;
    int hour = timeinfo->tm_hour;
    int minute = timeinfo->tm_min;
    char buffer[24];
    sprintf(buffer, "%04d%02d%02dT%02d%02dZ", year, month, day, hour, minute);
    return buffer;
}
