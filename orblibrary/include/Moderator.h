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

#include <memory>

#include "IBrowser.h"
#include "IDvbClient.h"

namespace orb
{

class AppManager;
class Network;
class MediaSynchroniser;

class Moderator
{
public:
    Moderator();
    ~Moderator();

    Moderator (const Moderator&) = delete;
    Moderator& operator= (const Moderator&) = delete;

    // Set Browser callback object
    void setBrowserCallback(IBrowser* browser);

    // Set DVB integration callback object
    void setDvbClient(IDvbClient *dvb_client);

    /** Handle ORB request from Javascript.
     * The request is a string representation of a JSON object with the following form:
     * {
     *    "method": <method>
     *    "token": <app_id>
     *    "params": <params>
     * }
     *
     * The response is also a string representation of a JSON object containing the results, if any.
     *
     * @param request String representation of the JSON request
     * @return A string representation of the JSON response
     */
    std::string handleOrbRequest(std::string request);

    // Notify that URL has been loaded for an application
    void notifyApplicationPageChanged(std::string url);

    // Notify that URL has failed to load for an application
    void notifyApplicationLoadFailed(std::string url, std::string errorText);

    void getDvbContent(std::string url);

    std::string getUserAgentString();

private:
    IBrowser *mBrowser;
    IDvbClient* mDvbClient;
    std::unique_ptr<AppManager> mAppManager;
    std::unique_ptr<Network> mNetwork;
    std::unique_ptr<MediaSynchroniser> mMediaSynchroniser;

}; // class Moderator

} // namespace orb
