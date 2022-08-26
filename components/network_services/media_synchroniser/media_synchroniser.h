/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
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

#ifndef HBBTV_MEDIA_SYNCHRONISER_H
#define HBBTV_MEDIA_SYNCHRONISER_H

#include "WallClockService.h"
#include "ContentIdentificationService.h"
#include "TimelineSyncService.h"
#include "SysClock.h"

namespace NetworkServices {
class MediaSyncCallback {
public:
   virtual ~MediaSyncCallback() = default;
   virtual void dispatchTimelineAvailableEvent(std::string timelineSelector, u_int64_t unitsPerSecond = 1000) = 0;
   virtual void dispatchTimelineUnavailableEvent(std::string timelineSelector) = 0;
   virtual void dispatchInterDeviceSyncEnabled(int mediaSyncId) = 0;
   virtual void dispatchInterDeviceSyncDisabled(int mediaSyncId) = 0;
   virtual int startTEMITimelineMonitoring(int componentTag, int timelineId) = 0;
   virtual bool stopTEMITimelineMonitoring(int filterId) = 0;
   virtual u_int64_t getCurrentPtsTime() = 0;
   virtual u_int64_t getCurrentTemiTime(int filterId) = 0;
};

/////////////////////////////////////////////////////////

class MediaSynchroniser final {
   friend class MediaSynchroniserManager;

public:
   MediaSynchroniser(const MediaSynchroniser &other) = delete;
   MediaSynchroniser &operator=(const MediaSynchroniser &other) = delete;

   bool enableInterDeviceSync(const std::string &ipAddr);
   void disableInterDeviceSync();
   bool interDeviceSyncEnabled() const
   {
      return m_syncing;
   }

   std::string getContentIdOverride() const
   {
      return m_contentIdOverride;
   }

   void setContentIdOverride(const std::string &cid, const bool forceUpdate = false);
   int nrOfSlaves();
   bool setContentTimeAndSpeed(std::string timelineSelector, const uint64_t &contentTime, const double &speed);
   void updateCssCiiProperties(const Json::Value &properties);
   u_int64_t getContentTime(const std::string &timelineSelector, bool &success);
   bool setTimelineAvailability(const std::string &timelineSelector, const bool &isAvailable, const u_int64_t currentTime = 0, const double speed = 0);
   bool setTEMITimelineAvailability(const int &filterId, const bool &isAvailable, const u_int64_t &currentTime, const u_int64_t &timescale, const double &speed);
   bool startTimelineMonitoring(const std::string &timelineSelector, const bool &isMaster);
   bool stopTimelineMonitoring(std::string timelineSelector, bool forceStop = false);

private:
   class MediaSyncServiceCallback : public ServiceManager::ServiceCallback {
public:
      MediaSyncServiceCallback(MediaSynchroniser *mediaSync) : mMediaSync(mediaSync)
      {
      }

      virtual void OnStopped();

private:
      MediaSynchroniser *mMediaSync;
   };

   struct TimelineWrapper
   {
      int numWatchers;
      int pendingWatchers;
      int temiFilterId;
      int temiComponentTag;
      int temiTimelineId;
      Json::Value timeline;

      TimelineWrapper() :
         numWatchers(0),
         pendingWatchers(0),
         temiFilterId(-1),
         temiComponentTag(-1),
         temiTimelineId(-1),
         timeline(Json::Value::null)
      {
      }

      void flushPendingWatchers()
      {
         numWatchers += pendingWatchers;
         pendingWatchers = 0;
      }
   };

   // MediaSynchroniser objects should be created and destroyed only by MediaSynchroniserManager
   MediaSynchroniser(const int &id, std::shared_ptr<MediaSyncCallback> timelineSyncCallback, const int &ciiPort, const int &wcPort, const int &tsPort);
   ~MediaSynchroniser();
   void deleteLater();
   void addTimeline(const std::string &timelineSelector);
   void removeTimeline(const std::string &timelineSelector);

   void initialise(const bool &isMasterBroadcast, std::string dvbUri = "", bool permanentError = false, bool presenting = true);
   void updateBroadcastContentStatus(std::string dvbUri = "", bool permanentError = false, bool presenting = true);
   void updateAllCIIClients();
   void updateAllTSClients();
   void setContentId(const std::string &cid);
   TimelineSource* _getTimelineSource(std::string timelineSelector);

   static bool ParseTimelineSelector(const std::string &timelineSelector, TimelineWrapper &timelineWrapper);

   std::shared_ptr<MediaSyncCallback> m_mediaSyncCallback;
   std::unordered_map<std::string, TimelineWrapper> m_timelines;
   std::string m_masterTimeline;
   bool m_isMasterBroadcast;
   bool m_syncing;
   bool m_delete;
   bool m_initialised;
   int m_runningServices;
   int m_wcService;
   int m_ciiService;
   int m_tsService;
   const int m_ciiPort;
   const int m_tsPort;
   const int m_wcPort;
   const int m_id;
   ::SysClock m_sysClock;
   ContentIdentificationProperties m_ciiProps;
   std::string m_contentIdOverride;
   std::string m_contentId;
   std::unordered_map<std::string, TimelineSource *> m_timelineSources;
   std::recursive_mutex m_mutex;
};

/////////////////////////////////////////////////////////

class MediaSynchroniserManager final {
public:
   MediaSynchroniserManager(std::shared_ptr<MediaSyncCallback> mediaSyncCallback, const int &ciiPort, const int &wcPort, const int &tsPort);
   ~MediaSynchroniserManager();
   MediaSynchroniserManager(const MediaSynchroniserManager &other) = delete;
   MediaSynchroniserManager &operator=(const MediaSynchroniserManager &other) = delete;

   int createMediaSynchroniser();
   void destroyMediaSynchroniser(const int &id);
   MediaSynchroniser* getMediaSynchroniser(const int &id);
   MediaSynchroniser* getActiveMediaSynchroniser();

   bool initMediaSynchroniser(const int &id, bool isMasterBroadcast);
   void updateDvbInfo(const std::string &dvbUri, bool permanentError, bool presenting);
   void releaseResources();

private:
   std::string m_dvbUri;
   bool m_dvbPermanentError;
   bool m_dvbPresenting;

   std::shared_ptr<MediaSyncCallback> m_mediaSyncCallback;
   int m_idCounter;
   int m_activeMediaSync;
   int m_ciiPort;
   int m_wcPort;
   int m_tsPort;
   std::unordered_map<int, MediaSynchroniser *> m_mediaSyncs;
   std::recursive_mutex m_mutex;
};
}

#endif // HBBTV_MEDIA_SYNCHRONISER_H
