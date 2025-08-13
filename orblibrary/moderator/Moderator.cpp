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
#include "Configuration.hpp"
#include "Drm.hpp"
#include "log.hpp"
#include "JsonUtil.h"
#include "StringUtil.h"
#include "BroadcastInterface.hpp"

#include "JsonRpcCallback.h"
#include "ConfigurationUtil.h"

#include "PlatformAndroid.h"


using namespace std;

namespace orb
{

// Component name constants
const string COMPONENT_MANAGER = "Manager";
const string COMPONENT_NETWORK = "Network";
const string COMPONENT_MEDIA_SYNCHRONISER = "MediaSynchroniser";
const string COMPONENT_CONFIGURATION = "Configuration";
const string COMPONENT_DRM = "Drm";
const string COMPONENT_BROADCAST = "Broadcast";

// Event types
const string CHANNEL_STATUS_CHANGE = "ChannelStatusChanged";
const string NETWORK_STATUS = "NetworkStatus";

Moderator::Moderator(IOrbBrowser* browser, ApplicationType apptype)
    : mOrbBrowser(browser)
    , mPlatform(std::make_shared<AndroidPlatform>(apptype))
    , mNetwork(std::make_unique<Network>())
    , mMediaSynchroniser(std::make_unique<MediaSynchroniser>())
    , mAppMgrInterface(std::make_unique<AppMgrInterface>(browser, apptype))
    , mConfiguration(std::make_unique<Configuration>(mPlatform))
    , mDrm(std::make_unique<Drm>())
    , mBroadcastInterface(std::make_unique<BroadcastInterface>(browser, mPlatform))
    , mAppType(apptype)
{
    LOGI("HbbTV version " << ORB_HBBTV_VERSION);
    if (apptype != orb::APP_TYPE_VIDEO) {
        startWebSocketServer(apptype);
    }
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
    else if (component == COMPONENT_CONFIGURATION)
    {
        return mConfiguration->executeRequest(method, jsonval["token"], jsonval["params"]);
    }
    else if (component == COMPONENT_DRM)
    {
        return mDrm->executeRequest(method, jsonval["token"], jsonval["params"]);
    }
    else if (component == COMPONENT_BROADCAST)
    {
        // Video Window does not support the broadcast interface
        if (mAppType != APP_TYPE_VIDEO) {
            return mBroadcastInterface->executeRequest(method, jsonval["token"], jsonval["params"]);
        } else {
            return "{\"error\": \"Not supported\"}";
        }
    }

    LOGI("Passing request to TIS component: [" << component << "], method: [" << method << "]");
    return mOrbBrowser->sendRequestToClient(jsonRqst);
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

bool Moderator::startWebSocketServer(ApplicationType apptype) {
    std::string endpoint = orb::ConfigurationUtil::getJsonRpcServerEndpoint();
    int port = orb::ConfigurationUtil::getJsonRpcServerPort(apptype);
    LOGI("Create and start WebSocket Server - endpoint: " << endpoint << " port: " << port);

    std::unique_ptr<orb::networkServices::JsonRpcService::ISessionCallback> callback =
        std::make_unique<JsonRpcCallback>(mBroadcastInterface.get());
    mWebSocketServer = std::make_shared<orb::networkServices::JsonRpcService>(port,endpoint,std::move(callback)); // TODO: check if this is correct
    mWebSocketServer->SetOpAppEnabled(apptype != orb::APP_TYPE_HBBTV);
    mBroadcastInterface->SetWebSocketServer(mWebSocketServer);
    return mWebSocketServer->Start();
}

} // namespace orb
