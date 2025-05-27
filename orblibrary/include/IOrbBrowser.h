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

class IOrbBrowser
{
public:
    IOrbBrowser() {}
    virtual ~IOrbBrowser() {}

    // Load new application at URL with new app_id for a reference to this application
    virtual void loadApplication(std::string app_id, std::string url) = 0;

    // Show application
    virtual void showApplication() = 0;

    // Hide application
    virtual void hideApplication() = 0;

    /**
     * Send Orb message request to external client (DVB stack)
     * The request is a string representation of a JSON object with the following form:
     *
     * {
     *    "method": <method>
     *    "token": <app_id>
     *    "params": <params>
     * }
     *
     * The response is also a string representation of a JSON object containing the results, if any.
     *
     * @param jsonRequest String representation of the JSON request
     *
     * @return A string representation of the JSON response
     */
    virtual std::string sendRequestToClient(std::string jsonRequest) = 0;

}; // class IOrbBrowser

} // namespace orb
