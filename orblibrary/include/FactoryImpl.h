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

#ifndef FACTORY_IMPL_H
#define FACTORY_IMPL_H

#include "IFactory.h"

namespace orb
{

/**
 * Factory Implementation
 *
 * This class implements the IFactory interface to create ORB components
 */
class FactoryImpl : public IFactory
{
public:
    /**
     * Constructor
     */
    FactoryImpl() = default;

    /**
     * Destructor
     */
    virtual ~FactoryImpl() = default;

    /**
     * Create an instance of IJson
     *
     * @param jsonString The JSON string to create the instance from
     * @return A unique pointer to the created instance
     */
    std::unique_ptr<IJson> createJson(const std::string& jsonString = {}) override;

    /**
     * Create an instance of Drm
     *
     * @param jsonString The JSON string to create the instance from
     * @return A unique pointer to the created instance
     */
    std::unique_ptr<ComponentBase> createDrm() override;

    /**
     * Create an instance of AppMgrInterface
     *
     * @param browser The browser instance
     * @param apptype The application type
     * @return A unique pointer to the created instance
     */
    std::unique_ptr<IAppMgrInterface> createAppMgrInterface(IOrbBrowser* browser, ApplicationType apptype) override;

};

} // namespace orb

#endif // FACTORY_IMPL_H
