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

#include <json/json.h>

#include "Moderator.h"
#include "AppMgrInterface.hpp"
#include "Network.hpp"
#include "MediaSynchroniser.hpp"
#include "Drm.hpp"
#include "log.hpp"
#include "JsonUtil.h"
#include "StringUtil.h"
#include "BroadcastInterface.hpp"

#include "JsonRpcCallback.h"
#include "JsonUtil.h"


using namespace std;

namespace orb
{

// Component name constants
const string COMPONENT_MANAGER = "Manager";
const string COMPONENT_NETWORK = "Network";
const string COMPONENT_MEDIA_SYNCHRONISER = "MediaSynchroniser";
const string COMPONENT_DRM = "Drm";
const string COMPONENT_BROADCAST = "Broadcast";

Moderator::Moderator(IOrbBrowser* browser, ApplicationType apptype)
    : mOrbBrowser(browser)
    , mNetwork(std::make_unique<Network>())
    , mMediaSynchroniser(std::make_unique<MediaSynchroniser>())
    , mAppMgrInterface(std::make_unique<AppMgrInterface>(browser, apptype))
    , mDrm(std::make_unique<Drm>())
    , mBroadcastInterface(std::make_unique<BroadcastInterface>(browser))
    , mAppType(apptype)
{
    LOGI("HbbTV version " << ORB_HBBTV_VERSION);
}

Moderator::~Moderator()
{
    if (mWebSocketServer) {
        mWebSocketServer->Stop();
    }
}

string Moderator::handleOrbRequest(string jsonRqst)
{
    Json::Value jsonval;

    LOGI("json: " << jsonRqst);

    if (!JsonUtil::decodeJson(jsonRqst, &jsonval))
    {
        return "{\"error\": \"Invalid Request\"}";
    }

    if (JsonUtil::HasJsonParam(jsonval, "error"))
    {
        LOGE("Json request reports error");
        return "{\"error\": \"Error Request\"}";
    }

    if (!JsonUtil::HasParam(jsonval, "method", Json::stringValue))
    {
        LOGE("Request has no method");
        return "{\"error\": \"No method\"}";
    }

    // add application type to params
    jsonval["params"]["applicationType"] = mAppType;

    string component;
    string method;
    if (!StringUtil::ResolveMethod(jsonval["method"].asString(), component, method))
    {
        return "{\"error\": \"Invalid method\"}";
    }

    LOGI(component << ", method: " << method);
    if (component == COMPONENT_MANAGER)
    {
        return mAppMgrInterface->executeRequest(method, jsonval["token"], jsonval["params"]);
    }
    else if (component == COMPONENT_NETWORK)
    {
        return mNetwork->executeRequest(method, jsonval["token"], jsonval["params"]);
    }
    else if (component == COMPONENT_MEDIA_SYNCHRONISER)
    {
        return mMediaSynchroniser->executeRequest(method, jsonval["token"], jsonval["params"]);
    }
    else if (component == COMPONENT_DRM)
    {
        return mDrm->executeRequest(method, jsonval["token"], jsonval["params"]);
    }
    else if (component == COMPONENT_BROADCAST)
    {
        return mBroadcastInterface->executeRequest(method, jsonval["token"], jsonval["params"]);
    }

    LOGI("Passing request to Live TV App");
    return mOrbBrowser->sendRequestToClient(JsonUtil::convertJsonToString(jsonval));
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
    Json::Value jsonval;

    LOGI("etype: " << etype << " props: " << properties);
    if (etype == CHANNEL_STATUS_CHANGE) {
        if (JsonUtil::decodeJson(properties, &jsonval)) {
            int status = JsonUtil::getIntegerValue(jsonval, "statusCode");
            if (status == CHANNEL_STATUS_CONNECTING) {
                uint16_t onetId = JsonUtil::getIntegerValue(jsonval, "onetId");
                uint16_t transId = JsonUtil::getIntegerValue(jsonval, "transId");
                uint16_t serviceId = JsonUtil::getIntegerValue(jsonval, "servId");
                mAppMgrInterface->onChannelChange(onetId, transId, serviceId);
            }
        }
        // Javascript also needs this event
    }
    else if (etype == NETWORK_STATUS) {
        if (JsonUtil::decodeJson(properties, &jsonval)) {
            mAppMgrInterface->onNetworkStatusChange(JsonUtil::getBoolValue(jsonval, "available"));
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

bool Moderator::startWebSocketServer() {
    const std::string CONFIGURATION_GETCAPABILITIES = "Configuration.getCapabilities";
    // request capabilities from Live App
    Json::Value request;
    request["method"] = CONFIGURATION_GETCAPABILITIES;
    request["params"]["applicationType"] = mAppType;
    std::string response = mOrbBrowser->sendRequestToClient(JsonUtil::convertJsonToString(request));

    Json::Value capabilities;
    if (!JsonUtil::decodeJson(response, &capabilities)) {
        LOGE("Failed to decode capabilities");
        return false;
    }

    // Get the endpoint and port from the capabilities
    std::string endpoint = JsonUtil::getStringValue(capabilities["result"], "jsonRpcServerEndpoint");
    int port = JsonUtil::getIntegerValue(capabilities["result"], "jsonRpcServerPort");

    LOGI("Create and start WebSocket Server - endpoint: " << endpoint << ", port: " << port);
    // Create and start the WebSocket Server
    std::unique_ptr<orb::networkServices::JsonRpcService::ISessionCallback> callback =
        std::make_unique<JsonRpcCallback>();
    mWebSocketServer = std::make_shared<orb::networkServices::JsonRpcService>(port,endpoint,std::move(callback));
    mWebSocketServer->SetOpAppEnabled(mAppType == orb::APP_TYPE_OPAPP);
    return mWebSocketServer->Start();
}

} // namespace orb
