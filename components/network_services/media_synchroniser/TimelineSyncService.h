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

#ifndef WIP_DVBCSS_HBBTV_TIMELINESYNCSERVICE_H
#define WIP_DVBCSS_HBBTV_TIMELINESYNCSERVICE_H

#include <string>
#include <list>
#include <iostream>
#include <sstream>
#include <utility>
#include "websocket_service.h"
//#include <json/json.h>
#include <json/json.h>
#include "ClockBase.h"
#include "Nullable.h"


namespace NetworkServices {
class TimelineSyncService;

const std::string PLUSININITY = "+inf";
const std::string MINUSINFINITY = "-inf";

class TimeStamp {
public:
    explicit TimeStamp(const Nullable<unsigned long long> &contentTime = Nullable<unsigned long
                                                                                  long>(),
        std::string wallclockTime = "0");

    ~TimeStamp() = default;

    bool isNull();

    Json::Value pack();

    void getTimeStamp(Nullable<unsigned long long> &contentTime, std::string &wallclockTime);

    void setTimeStamp(Nullable<unsigned long long> contentTime, std::string wallclockTime);

    bool operator==(const TimeStamp &st) const
    {
        return(m_contentTime == st.m_contentTime &&
               m_wallClockTime == st.m_wallClockTime);
    }

    bool operator!=(const TimeStamp &st) const
    {
        return(m_contentTime != st.m_contentTime ||
               m_wallClockTime != st.m_wallClockTime);
    }

private:
    Nullable<unsigned long long> m_contentTime;
    std::string m_wallClockTime;
};

class ControlTimestamp {
public:
    explicit ControlTimestamp(const TimeStamp &tstamp = TimeStamp(), const
                              Nullable<float> &timelineSpeedMultiplier = Nullable<float>())
        : m_tstamp{tstamp}, m_timelineSpeedMultiplier{timelineSpeedMultiplier}
    {
    };

    ~ControlTimestamp() = default;

    Json::Value pack();

    static ControlTimestamp unpack(const std::string &msg);

    bool operator==(const ControlTimestamp &ct) const
    {
        return(m_tstamp == ct.m_tstamp &&
               m_timelineSpeedMultiplier == ct.m_timelineSpeedMultiplier);
    }

    bool operator!=(const ControlTimestamp &ct) const
    {
        return(m_tstamp != ct.m_tstamp ||
               m_timelineSpeedMultiplier != ct.m_timelineSpeedMultiplier);
    }

    TimeStamp &getTimestamp();

private:
    TimeStamp m_tstamp;
    Nullable<float> m_timelineSpeedMultiplier;
};

class SetupTSData {
public:
    SetupTSData() = default;

    SetupTSData(std::string contentIdStem, std::string timelineSelector,
                Json::Value privateData = {}) :
        m_contentIdStem{std::move(contentIdStem)},
        m_timelineSelector{std::move(timelineSelector)},
        m_private{std::move(privateData)}
    {
    };

    Json::Value pack();

    bool isEmpty();

    const std::string &getContentIdStem() const;

    const std::string &getTimelineSelector() const;

    static SetupTSData unpack(const std::string &msg);

private:
    std::string m_contentIdStem;
    std::string m_timelineSelector;
    Json::Value m_private;
};

class AptEptLpt {
public:
    explicit AptEptLpt(const TimeStamp &earliest = TimeStamp(0, "-inf"),
                       const TimeStamp &latest = TimeStamp(0, "+inf"), const
                       Nullable<TimeStamp> &actual = Nullable<TimeStamp>()) :
        m_earliest{earliest}, m_actual{actual}, m_latest{latest}
    {
    };

    ~AptEptLpt() = default;

    Json::Value pack();

    static AptEptLpt unpack(const std::string &msg);

    bool isInDefaultState();

private:
    TimeStamp m_earliest;
    Nullable<TimeStamp> m_actual;
    TimeStamp m_latest;
};

class TimelineSource : public Notifiable {
public:
    TimelineSource() = default;

    TimelineSource(const std::string &timelineSelector) : m_timelineSelector{timelineSelector}
    {
    };

    virtual ~TimelineSource() = default;

    bool operator==(const TimelineSource &tls) const
    {
        return m_timelineSelector == tls.m_timelineSelector;
    }

    virtual void timelineSelectorNeeded(const std::string &timelineSelector) = 0;

    virtual void timelineSelectorNotNeeded(const std::string &timelineSelector) = 0;

    virtual bool recognisesTimelineSelector(const std::string &timelineSelector) = 0;

    virtual Nullable<ControlTimestamp> &getControlTimestamp(const
        std::string &timelineSelector) = 0;

    virtual bool attachSink(TimelineSyncService *tservice)
    {
        if (tservice != nullptr)
        {
            m_sinks[tservice] = true;
            return true;
        }
        return false;
    }

    virtual bool removeSink(TimelineSyncService *tservice)
    {
        if (tservice != nullptr && m_sinks.erase(tservice))
        {
            return true;
        }
        return false;
    }

    const std::string &getTimelineSelector() const
    {
        return m_timelineSelector;
    }

protected:
    std::string m_timelineSelector;
    std::unordered_map<TimelineSyncService *, bool> m_sinks;
};


class SimpleTimeLineSource : public TimelineSource {
public:

    SimpleTimeLineSource(const std::string &timelineSelector,
                         Nullable<ControlTimestamp> &ctStamp) : TimelineSource{
                                                                               timelineSelector},
        m_ctStamp{ctStamp}
    {
    };

    ~SimpleTimeLineSource() override = default;

    void timelineSelectorNeeded(const std::string &timelineSelector) override
    {
    };

    void timelineSelectorNotNeeded(const std::string &timelineSelector) override
    {
    };

    bool attachSink(TimelineSyncService *tservice) override
    {
        return false;
    };

    bool removeSink(TimelineSyncService *tservice) override
    {
        return false;
    };

    bool recognisesTimelineSelector(const std::string &timelineSelector) override;

    Nullable<ControlTimestamp> &getControlTimestamp(const std::string &timelineSelector) override;

    virtual void notify() override
    {
    }

private:
    Nullable<ControlTimestamp> &m_ctStamp;
};


class SimpleClockTimelineSource : public TimelineSource {
public:
    SimpleClockTimelineSource(const std::string &timelineSelector, ClockBase *wallClock,
        ClockBase *clock, ClockBase *speedSource = nullptr,
        bool autoUpdateClients = false);

    virtual ~SimpleClockTimelineSource();

    void timelineSelectorNeeded(const std::string &timelineSelector) override
    {
    };

    void timelineSelectorNotNeeded(const std::string &timelineSelector) override
    {
    };

    bool recognisesTimelineSelector(const std::string &timelineSelector) override;

    Nullable<ControlTimestamp> &getControlTimestamp(const std::string &timelineSelector) override;

    bool attachSink(TimelineSyncService *tservice) override;

    bool removeSink(TimelineSyncService *tservice) override;

    virtual void notify() override;

private:
    ClockBase *m_wallClock;
    ClockBase *m_clock;
    ClockBase *m_speedSource;
    bool m_autoUpdateClients;
    bool m_changed;
    Nullable<ControlTimestamp> m_latestCt;
};


//TS SERVICE

class ContentIdentificationService;
class MediaSynchroniser;

class TimelineSyncService : public WebSocketService {
public:
    TimelineSyncService(int port, ClockBase *wallClock, MediaSynchroniser *mediaSync,
        ContentIdentificationService *cii, std::string contentIdOverride = "");

    ~TimelineSyncService() override = default;

    void setContentId(const std::string &cid, const bool forceUpdate = false);
    void setContentIdOverride(const std::string &cid, const bool forceUpdate = false);
    std::string getContentIdOverride() const
    {
        return m_contentIdOverride;
    }

    bool OnConnection(WebSocketConnection *connection) override;

    void OnMessageReceived(WebSocketConnection *connection, const std::string &text) override;

    void OnDisconnected(WebSocketConnection *connection) override;

    void updateAllClients();

    void updateClient(WebSocketConnection *connection);

    void attachTimelineSource(TimelineSource *);

    void removeTimelineSource(TimelineSource *);

private:

//        struct hash_timelineSource {
//            size_t operator()(const TimelineSource &tls) const {
//                return std::hash<std::string>()(tls.getTimelineSelector());
//            }
//        };

    std::string m_contentId;
    std::string m_contentIdOverride;
    ClockBase *m_wallclock;
    std::unordered_map<TimelineSource *, bool> m_timelineSources;
    std::unordered_map<std::string, int> m_timelineSelectors;
    std::unordered_map<WebSocketConnection *, SetupTSData> m_connectionSetupData;
    std::unordered_map<WebSocketConnection *,
                       Nullable<ControlTimestamp> > m_connectionPreviousControlTimestamp;
#if JSONCPP_VERSION_HEXA > 0x01080200
    Json::StreamWriterBuilder m_wbuilder;
#else
    Json::FastWriter m_writer;
#endif
    ContentIdentificationService *m_ciiService;
    MediaSynchroniser *m_mediaSync;

    bool ciMatchesStem(const std::string &contentId, const std::string &contentIdStem);
    std::string getContentId() const;
    bool isControlTimestampChanged(const Nullable<ControlTimestamp> & prev, const
        Nullable<ControlTimestamp> &latest) const;
    void configureConnectionWithSetupData(WebSocketService::WebSocketConnection *connection, const
        std::string &text);
};
}

#endif
