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
 * ORB Moderator
 *
 */

#include "Moderator.h"
#include "AppMgrInterface.hpp"
#include "Network.hpp"
#include "MediaSynchroniser.hpp"
#include "log.h"
#include "StringUtil.h"
#include "IJson.h"
#include "Drm.hpp"
#include "JsonRpcService.h"

using namespace std;

namespace orb
{

// Component name constants
const string COMPONENT_MANAGER = "Manager";
const string COMPONENT_NETWORK = "Network";
const string COMPONENT_MEDIA_SYNCHRONISER = "MediaSynchroniser";
const string COMPONENT_DRM = "Drm";

Moderator::Moderator(IOrbBrowser* browser, ApplicationType apptype)
    : mOrbBrowser(browser)
    , mNetwork(std::make_unique<Network>())
    , mMediaSynchroniser(std::make_unique<MediaSynchroniser>())
    , mAppMgrInterface(std::make_unique<AppMgrInterface>(browser, apptype))
    , mDrm(std::make_unique<Drm>())
{
    LOGI("HbbTV version " << ORB_HBBTV_VERSION);
}

Moderator::Moderator(
    IOrbBrowser* browser,
    ApplicationType apptype,
    std::unique_ptr<IAppMgrInterface> appMgrInterface,
    std::unique_ptr<ComponentBase> drm)
    : mOrbBrowser(browser)
    , mNetwork(std::make_unique<Network>())
    , mMediaSynchroniser(std::make_unique<MediaSynchroniser>())
    , mAppMgrInterface(std::move(appMgrInterface))
    , mDrm(std::move(drm))
{}

Moderator::~Moderator() {}

string Moderator::handleOrbRequest(string jsonRqst)
{
    std::unique_ptr<IJson> json = IJson::create();

    if (!json->parse(jsonRqst))
    {
        return "{\"error\": \"Invalid Request\"}";
    }

    if (json->hasParam("error"))
    {
        LOGE("Json request reports error");
        return "{\"error\": \"Error Request\"}";
    }

    if (!json->hasParam("method", IJson::JSON_TYPE_STRING))
    {
        LOGE("Request has no method");
        return "{\"error\": \"No method\"}";
    }

    // add application type to params. Guard against missing params.
    if (!json->hasParam("params", IJson::JSON_TYPE_OBJECT)) {
        LOGE("Request has no params");
        return "{\"error\": \"No params\"}";
    }

    json->setInteger("params", mAppMgrInterface->GetApplicationType(), "applicationType");

    std::string component;
    std::string method;
    if (!StringUtil::ResolveMethod(json->getString("method"), component, method))
    {
        return "{\"error\": \"Invalid method\"}";
    }

    std::unique_ptr<IJson> params = json->getObject("params");
    std::string token = json->getString("token");
    if (component == COMPONENT_MANAGER)
    {
        return mAppMgrInterface->executeRequest(method, token, *params);
    }
    else if (component == COMPONENT_NETWORK)
    {
        return mNetwork->executeRequest(method, token, *params);
    }
    else if (component == COMPONENT_MEDIA_SYNCHRONISER)
    {
        return mMediaSynchroniser->executeRequest(method, token, *params);
    }
    else if (component == COMPONENT_DRM)
    {
        return mDrm->executeRequest(method, token, *params);
    }

    LOGI("Passing request to Live TV App");
    return mOrbBrowser->sendRequestToClient(json->toString());
}

void Moderator::notifyApplicationPageChanged(string url)
{
    LOGI("url: " << url);
}

void Moderator::notifyApplicationLoadFailed(string url, string errorText)
{
    LOGI("url: " << url << " err: " << errorText);
}

bool Moderator::handleBridgeEvent(const std::string& etype, const std::string& properties) {
    bool consumed = false;
    LOGI("etype: " << etype << " props: " << properties);
    if (etype == CHANNEL_STATUS_CHANGE) {
        std::unique_ptr<IJson> json = IJson::create();
        if (json->parse(properties)) {
            int status = json->getInteger("statusCode");
            // Check if we have DVB triplet information (indicates DVB channel change)
            // Note: hasParam defaults to JSON_TYPE_OBJECT, but these are integers, so we check for integer type
            if (json->hasParam("onetId", IJson::JSON_TYPE_INTEGER) && 
                json->hasParam("transId", IJson::JSON_TYPE_INTEGER) && 
                json->hasParam("servId", IJson::JSON_TYPE_INTEGER)) {
                uint16_t onetId = json->getInteger("onetId");
                uint16_t transId = json->getInteger("transId");
                uint16_t serviceId = json->getInteger("servId");
                // Call onChannelChange for DVB channel changes
                // We call it for CONNECTING status to stop IP player when switching from IP to DVB
                // Note: When switching from IP to DVB, the event might come with PRESENTING status
                // if the switch already happened, but we still need to stop the IP player
                if (status == CHANNEL_STATUS_CONNECTING) {
                    LOGI("Moderator::handleBridgeEvent - DVB channel change detected (CONNECTING), calling onChannelChange");
                    mAppMgrInterface->onChannelChange(onetId, transId, serviceId);
                } else if (status == CHANNEL_STATUS_PRESENTING) {
                    // Also call onChannelChange for PRESENTING status if we're switching from IP to DVB
                    // This ensures we stop the IP player even if the event comes with PRESENTING status
                    LOGI("Moderator::handleBridgeEvent - DVB channel change detected (PRESENTING), calling onChannelChange to stop IP player");
                    mAppMgrInterface->onChannelChange(onetId, transId, serviceId);
                } else {
                    LOGI("Moderator::handleBridgeEvent - DVB channel change detected but status is " << status << ", skipping onChannelChange");
                }
            } else {
                LOGI("Moderator::handleBridgeEvent - ChannelStatusChanged event without DVB triplet information");
            }
        }
        // Javascript also needs this event
    }
    else if (etype == NETWORK_STATUS) {
        std::unique_ptr<IJson> json = IJson::create();
        if (json->parse(properties)) {
            mAppMgrInterface->onNetworkStatusChange(json->getBool("available"));
        }
        consumed = true; // This event is consumed here and is not passed to Javascript
    }
    return consumed;
}

void Moderator::processAitSection(int32_t aitPid, int32_t serviceId, const vector<uint8_t>& section)
{
    LOGI("pid: " << aitPid << "serviceId: " << serviceId);
    mAppMgrInterface->processAitSection(aitPid, serviceId, section);
}

void Moderator::processXmlAit(const vector<uint8_t>& xmlait)
{
    LOGI("");
    mAppMgrInterface->processXmlAit(xmlait);
}

void Moderator::setWebSocketService(std::shared_ptr<networkServices::JsonRpcService> webSocketService)
{
    mAppMgrInterface->setWebSocketService(webSocketService);
}
} // namespace orb
