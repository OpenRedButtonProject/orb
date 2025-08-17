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

#include "IOrbBrowser.h"
#include "OrbConstants.h"

namespace orb
{

class Network;
class MediaSynchroniser;
class AppMgrInterface;
class Configuration;
class Drm;
class BroadcastInterface;

namespace networkServices {
    class JsonRpcService;
}

class Moderator
{
public:
    Moderator(IOrbBrowser* browser, ApplicationType apptype);
    ~Moderator();

    // ----------------------------------------------------------
    // Interface functions to process messages from JS or Browser
    // ----------------------------------------------------------

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

    // --------------------------------------------------------
    // Interface functions to process messages from Live TV app
    // --------------------------------------------------------

    bool handleBridgeEvent(const std::string& etype, const std::string& properties);
    void processAitSection(int32_t aitPid, int32_t serviceId, const std::vector<uint8_t>& section);
    void processXmlAit(const std::vector<uint8_t>& xmlait);

private:
    bool startWebSocketServer();

private:
    IOrbBrowser *mOrbBrowser;
    std::unique_ptr<Network> mNetwork;
    std::unique_ptr<MediaSynchroniser> mMediaSynchroniser;
    std::unique_ptr<AppMgrInterface> mAppMgrInterface;
    std::unique_ptr<Configuration> mConfiguration;
    std::unique_ptr<Drm> mDrm;
    std::unique_ptr<BroadcastInterface> mBroadcastInterface;
    // WebSocket Server is used to communicate with the web client, OpApp and regular HBBTV App
    // should have different WebSocket Server for different permission. OpApp and Video Window
    // will use the same WebSocket Server with different connections.
    std::shared_ptr<orb::networkServices::JsonRpcService> mWebSocketServer;
    ApplicationType mAppType;
}; // class Moderator

} // namespace orb
