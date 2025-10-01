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
 * Generic Factory Interface
 *
 * Note: This file defines a generic factory interface for creating objects.
 */

#ifndef IFACTORY_H
#define IFACTORY_H

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include "IJson.h"
#include "OrbConstants.h"

namespace orb
{

/**
 * Generic Factory Interface
 *
 */

class ComponentBase;
class IAppMgrInterface;
class IOrbBrowser;

class IFactory
{
public:
    /**
     * Virtual destructor
     */
    virtual ~IFactory() = default;

    /**
     * Create an instance of IJson
     *
     * @param jsonString The JSON string to create the instance from
     * @return A unique pointer to the created instance
     */
    virtual std::unique_ptr<IJson> createJson(const std::string& jsonString = {}) = 0;

    /**
     * Create an instance of Drm
     *
     * @return A unique pointer to the created instance
     */
    virtual std::unique_ptr<ComponentBase> createDrm() = 0;

    /**
     * Create an instance of AppMgrInterface
     *
     * @return A unique pointer to the created instance
     */
    virtual std::unique_ptr<IAppMgrInterface> createAppMgrInterface(IOrbBrowser* browser, ApplicationType apptype) = 0;
};

} // namespace orb

#endif // IFACTORY_H
