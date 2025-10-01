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
 * Mock ComponentBase for testing using Google Mock
 */

#ifndef MOCK_COMPONENT_BASE_H
#define MOCK_COMPONENT_BASE_H

#include "ComponentBase.hpp"
#include <gmock/gmock.h>

namespace orb
{

class MockComponentBase : public ComponentBase
{
public:
    MockComponentBase() = default;
    virtual ~MockComponentBase() = default;

    // ComponentBase interface using Google Mock
    MOCK_METHOD(std::string, executeRequest, (std::string method, std::string token, std::unique_ptr<IJson> params), (override));
};

} // namespace orb

#endif // MOCK_COMPONENT_BASE_H
