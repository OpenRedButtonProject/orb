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

#include "IBrowser.h"

namespace orb
{

class Moderator
{
public:
    Moderator();
    ~Moderator();

    // Set Browser callback object
    void setBrowserCallback(IBrowser* browser);

    // Execute the given request from browser.
    // The request is a string representation of a JSON object with the following form:
    // {
    //    "token": <app_id>
    //    "method": <method>
    //    "params": <params>
    // }
    //
    // The response is also a string representation of a JSON object containing the results, if any.
    //
    // @param jsonRequest String representation of the JSON request
    // @return A string representation of the JSON response
    std::string executeRequest(std::string jsonRequest);

    // Notify that URL has been loaded for an application
    void notifyApplicationPageChanged(std::string url);

    // Notify that URL has failed to load for an application
    void notifyApplicationLoadFailed(std::string url, std::string errorText);

    void getDvbContent(std::string url);

    std::string getUserAgentString();

private:
    IBrowser* mBrowser;

}; // class Moderator

} // namespace orb
