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

#ifndef OBS_NS_SERVICE_MANAGER_H
#define OBS_NS_SERVICE_MANAGER_H

#include <memory>
#include <unordered_map>
#include <mutex>

class ClockBase;
class SysClock;

namespace orb {
namespace networkServices {    
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
            // TODO: Call this method when Service manager is integated into Project
            // to avoid link errors.
            // ServiceManager::GetInstance().OnServiceStopped(this);
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

    int StartWallClockService(std::unique_ptr<ServiceCallback> callback, const int &port,
        ::SysClock *sysClock);
    int StartContentIdentificationService(std::unique_ptr<ServiceCallback> callback, const
        int &port, ContentIdentificationProperties *props);
    int StartTimelineSyncService(std::unique_ptr<ServiceCallback> callback, const int &port,
        ClockBase *wallclock, MediaSynchroniser *ms, int ciiService);
    int StartApp2AppService(std::unique_ptr<ServiceCallback> callback, int localApp2AppPort, int
        remoteApp2AppPort);

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
} // namespace networkServices
} // namespace orb  

#endif // OBS_NS_SERVICE_MANAGER_H

