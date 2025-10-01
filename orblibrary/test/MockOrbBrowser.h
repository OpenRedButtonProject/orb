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
 */

#ifndef MOCKORBBROWSER_H
#define MOCKORBBROWSER_H

#include "IOrbBrowser.h"
#include <gmock/gmock.h>
#include <string>
#include <vector>

namespace orb
{

/**
 * Mock implementation of IOrbBrowser using Google Mock
 */
class MockOrbBrowser : public IOrbBrowser
{
public:
    MOCK_METHOD(void, loadApplication, (std::string app_id, std::string url), (override));
    MOCK_METHOD(void, showApplication, (), (override));
    MOCK_METHOD(void, hideApplication, (), (override));
    MOCK_METHOD(std::string, sendRequestToClient, (std::string jsonRequest), (override));
    MOCK_METHOD(void, dispatchEvent, (const std::string& etype, const std::string& properties), (override));
    MOCK_METHOD(void, notifyKeySetChange, (uint16_t keyset, (std::vector<uint16_t>) otherkeys), (override));
};

} // namespace orb

#endif // MOCKORBBROWSER_H