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
 * Mock Factory Implementation
 *
 * Note: This file provides mock implementations for testing the IFactory interface.
 */

#ifndef MOCK_FACTORY_H
#define MOCK_FACTORY_H

#include "IFactory.h"
#include "MockJson.h"
#include <gmock/gmock.h>
#include <memory>

namespace orb
{

/**
 * Mock implementation of IFactory using Google Mock
 */
class MockFactory : public IFactory
{
public:
    MOCK_METHOD(std::unique_ptr<IJson>, createJson, (const std::string& jsonString), (override));

    MOCK_METHOD(std::unique_ptr<ComponentBase>, createDrm, (), (override));

    MOCK_METHOD(std::unique_ptr<IAppMgrInterface>, createAppMgrInterface, (IOrbBrowser* browser, ApplicationType apptype), (override));

};


} // namespace orb

#endif // MOCK_FACTORY_H
