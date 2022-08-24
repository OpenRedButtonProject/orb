/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

#ifndef OBS_NS_SERVICE_MANAGER_
#define OBS_NS_SERVICE_MANAGER_

#include <memory>
#include <unordered_map>
#include <mutex>

class ClockBase;
class SysClock;

namespace NetworkServices {
class ContentIdentificationProperties;
class MediaSynchroniser;

class ServiceManager {
public:
   class Service {
public:
      Service() = default;
      virtual ~Service() = default;
      virtual void Stop() = 0;
      virtual void OnServiceStopped()
      {
         ServiceManager::GetInstance().OnServiceStopped(this);
      }

      // Disallow copy and assign
      Service(const Service&) = delete;
      Service& operator=(const Service&) = delete;
   };

   class ServiceCallback {
public:
      virtual void OnStopped() = 0;
      virtual ~ServiceCallback() = default;
   };

   static ServiceManager &GetInstance();
   void StopService(int id);
   void OnServiceStopped(Service *service);

   template<class type> type* FindService(int id)
   {
      if (services_.find(id) != services_.end())
      {
         return dynamic_cast<type *>(services_[id].get());
      }
      return nullptr;
   }

   int StartWallClockService(std::unique_ptr<ServiceCallback> callback, const int &port, ::SysClock *sysClock);
   int StartContentIdentificationService(std::unique_ptr<ServiceCallback> callback, const int &port, ContentIdentificationProperties *props);
   int StartTimelineSyncService(std::unique_ptr<ServiceCallback> callback, const int &port, ClockBase *wallclock, MediaSynchroniser *ms, int ciiService);
   int StartApp2AppService(std::unique_ptr<ServiceCallback> callback, int localApp2AppPort, int remoteApp2AppPort);

private:
   ServiceManager();
   int NewServiceId();
   template<class type> type* FindServiceCallback(Service *service);
   int FindServiceId(const Service *service);

   int max_services_;
   std::unordered_map<int, std::unique_ptr<Service> > services_;
   std::unordered_map<Service *, std::unique_ptr<ServiceCallback> > callbacks_;
   std::recursive_mutex mutex_;

   // Disallow copy and assign
   ServiceManager(const ServiceManager&) = delete;
   ServiceManager& operator=(const ServiceManager&) = delete;
};
} // namespace NetworkServices

#endif // OBS_NS_SERVICE_MANAGER_

