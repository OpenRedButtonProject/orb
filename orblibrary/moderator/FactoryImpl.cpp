/**
 * ORB Software. Copyright (c) 2025 Ocean Blue Software Limited
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
 * Factory Implementation
 *
 * Note: This file implements the IFactory interface for creating ORB components.
 */


#include "FactoryImpl.h"
#include "app_mgr/application_manager.h"
#include "JsonImpl.h"
#include "Drm.hpp"
#include "AppMgrInterface.hpp"
#include "log.h"
#include "IOrbBrowser.h"

namespace orb
{

std::unique_ptr<IJson> FactoryImpl::createJson(const std::string& jsonString)
{
    LOGI("FactoryImpl: Creating IJson instance");
    return std::make_unique<JsonImpl>(jsonString);
}

std::unique_ptr<ComponentBase> FactoryImpl::createDrm()
{
    LOGI("FactoryImpl: Creating Drm instance");
    return std::make_unique<Drm>();
}

std::unique_ptr<IAppMgrInterface> FactoryImpl::createAppMgrInterface(IOrbBrowser* browser, ApplicationType apptype)

{
    LOGI("FactoryImpl: Creating AppMgrInterface instance");
    return std::make_unique<AppMgrInterface>(browser, apptype);
}

} // namespace orb
