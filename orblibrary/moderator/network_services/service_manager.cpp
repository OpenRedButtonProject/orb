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

#include "service_manager.h"
#include "WallClockService.h"
#include "ContentIdentificationService.h"
#include "TimelineSyncService.h"
#include "app2app_local_service.h"
#include "log.h"

#define MAX_SERVICES 1000

namespace orb {
namespace networkServices { 
ServiceManager::ServiceManager() :
    max_services_(MAX_SERVICES)
{
}

ServiceManager &ServiceManager::GetInstance()
{
    static ServiceManager instance;
    return instance;
}

void ServiceManager::StopService(int id)
{
    mutex_.lock();
    Service *service = FindService<Service>(id);
    if (service)
    {
        service->Stop();
    }
    mutex_.unlock();
}

void ServiceManager::OnServiceStopped(Service *service)
{
    mutex_.lock();
    int id = FindServiceId(service);
    ServiceCallback *callback = FindServiceCallback<ServiceCallback>(service);
    if (callback != nullptr)
    {
        callback->OnStopped();
    }
    if (id > -1)
    {
        services_.erase(id);
        callbacks_.erase(service);
    }
    mutex_.unlock();
}

int ServiceManager::NewServiceId()
{
    for (int i = 0; i < max_services_; i++)
    {
        if (services_.find(i) == services_.end())
        {
            return i;
        }
    }
    return -1;
}

template<class type> type * ServiceManager::FindServiceCallback(Service *service)
{
    if (callbacks_.find(service) != callbacks_.end())
    {
        return dynamic_cast<type *>(callbacks_[service].get());
    }
    return nullptr;
}

int ServiceManager::FindServiceId(const Service *service)
{
    for (auto it = services_.begin(); it != services_.end(); ++it)
    {
        if (it->second.get() == service)
        {
            return it->first;
        }
    }
    return -1;
}

int ServiceManager::StartWallClockService(std::unique_ptr<ServiceCallback> callback, const
    int &port, ::SysClock *sysClock)
{
    mutex_.lock();
    int id = NewServiceId();
    if (id > -1)
    {
        std::unique_ptr<WallClockService> service = std::unique_ptr<WallClockService>(
            new WallClockService(port, sysClock));
        if (service->Start())
        {
            callbacks_[service.get()] = std::move(callback);
            services_[id] = std::move(service);
        }
        else
        {
            id = -1;
        }
    }
    mutex_.unlock();
    return id;
}

int ServiceManager::StartContentIdentificationService(std::unique_ptr<ServiceCallback> callback,
    const int &port, ContentIdentificationProperties *props)
{
    mutex_.lock();
    int id = NewServiceId();
    if (id > -1)
    {
        std::unique_ptr<ContentIdentificationService> service =
            std::unique_ptr<ContentIdentificationService>(
                new ContentIdentificationService(port, props));
        if (service->Start())
        {
            callbacks_[service.get()] = std::move(callback);
            services_[id] = std::move(service);
        }
        else
        {
            id = -1;
        }
    }
    mutex_.unlock();
    return id;
}

int ServiceManager::StartTimelineSyncService(std::unique_ptr<ServiceCallback> callback, const
    int &port, ClockBase *wallclock, MediaSynchroniser *ms, int ciiService)
{
    mutex_.lock();
    ContentIdentificationService *cii = FindService<ContentIdentificationService>(ciiService);
    int id = -1;
    if (cii != nullptr)
    {
        id = NewServiceId();
        if (id > -1)
        {
            std::unique_ptr<TimelineSyncService> service = std::unique_ptr<TimelineSyncService>(
                new TimelineSyncService(port,
                    wallclock,
                    ms,
                    cii));
            if (service->Start())
            {
                callbacks_[service.get()] = std::move(callback);
                services_[id] = std::move(service);
            }
            else
            {
                id = -1;
            }
        }
        mutex_.unlock();
    }
    return id;
}

int ServiceManager::StartApp2AppService(std::unique_ptr<ServiceCallback> callback, int
    localApp2AppPort, int remoteApp2AppPort)
{
    mutex_.lock();
    int id = NewServiceId();
    if (id > -1)
    {
        std::unique_ptr<App2AppLocalService> service = std::unique_ptr<App2AppLocalService>(
            new App2AppLocalService(this, localApp2AppPort, remoteApp2AppPort));
        if (service->Start())
        {
            callbacks_[service.get()] = std::move(callback);
            services_[id] = std::move(service);
        }
        else
        {
            id = -1;
        }
    }
    mutex_.unlock();
    return id;
}
} // namespace networkServices
} // namespace orb
  
