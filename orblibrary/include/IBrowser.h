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
#pragma once

#include <vector>
#include <cstdint>
#include <string>

namespace orb
{

class IBrowser
{
public:
    // Enum used with dispatchKeyEvent
    enum { ACTION_DOWN, ACTION_UP };

    // Load new application at URL with new app_id for a reference to this application
    virtual void loadApplication(std::string app_id, std::string url) = 0;

    // Show application
    virtual void showApplication() = 0;

    // Hide application
    virtual void hideApplication() = 0;

    // Dispatch event
    virtual void dispatchEvent(std::string type, std::string properties) = 0;

    // Dispatch key event
    virtual bool dispatchKeyEvent(int32_t action, int32_t key_code) = 0;

    // Provide DSM-CC content
    virtual void provideDsmccContent(std::string url, const std::vector<uint8_t>& content) = 0;

}; // class IOrbBrowser

} // namespace orb
