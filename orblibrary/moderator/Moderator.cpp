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
#include "Configuration.h"
#include "Drm.h"
#include "log.h"
#include "JsonUtil.h"
#include "StringUtil.h"
#include "BroadcastInterface.hpp"

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

Moderator::Moderator(IOrbBrowser* browser, ApplicationType apptype)
    : mOrbBrowser(browser)
    , mNetwork(std::make_unique<Network>())
    , mMediaSynchroniser(std::make_unique<MediaSynchroniser>())
    , mAppMgrInterface(std::make_unique<AppMgrInterface>(browser, apptype))
    , mConfiguration(std::make_unique<Configuration>(apptype))
    , mDrm(std::make_unique<Drm>()
    , mBroadcastInterface(std::make_unique<BroadcastInterface>(browser))
{
    LOGI("HbbTV version " << ORB_HBBTV_VERSION);
}

Moderator::~Moderator()
{
}

string Moderator::handleOrbRequest(string jsonRqst)
{
    Json::Value jsonval;
    Json::CharReaderBuilder builder;
    const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
    string err;
    int rlen = static_cast<int>(jsonRqst.length());

    LOGI("json: " << jsonRqst);

    if (!reader->parse(jsonRqst.c_str(), jsonRqst.c_str() + rlen, &jsonval, &err))
    {
        LOGE("Json parsing failed: " << err);
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
        return mBroadcastInterface->executeRequest(method, jsonval["token"], jsonval["params"]);
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


} // namespace orb
