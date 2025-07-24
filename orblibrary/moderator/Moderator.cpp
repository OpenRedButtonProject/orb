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
#include "BroadcastInterface.hpp"
#include "log.h"

using namespace std;

namespace orb
{
/**
 * Check if a JSON object has a specified parameter with a certain data type.
 *
 * @param json The JSON object to check for the presence of the parameter.
 * @param param The name of the parameter to search for within the JSON object.
 * @param type The expected data type of the parameter.
 * @return 'true' if the parameter 'param' exists within the JSON object
 *          and has the specified data type, 'false' otherwise.
 */
static bool HasParam(const Json::Value &json, const string &param, const Json::ValueType& type)
{
    return json.isMember(param) && json[param].type() == type;
}

/**
 * Check if a JSON object has a specified parameter with a json data type.
 *
 * @param json The JSON object to check for the presence of the parameter.
 * @param param The name of the parameter to search for within the JSON object.
 * @return 'true' if the parameter 'param' exists within the JSON object
 *          and has the json data type, 'false' otherwise.
 */
static bool HasJsonParam(const Json::Value &json, const string &param)
{
    return json.isMember(param) && json[param].isObject();
}

/**
 * Resolves the component and method from the specified input, which has the following form:
 *
 * <component>.<method>
 *
 * @param input  (in)  The input string
 * @param component (out) Holds the resolved component in success
 * @param method (out) Holds the resolved method in success
 *
 * @return true in success, otherwise false
 */
static bool ResolveMethod(string input, string& component, string& method)
{
    vector<string> tokens;
    for (auto i = strtok(&input[0], "."); i != NULL; i = strtok(NULL, "."))
    {
        tokens.push_back(i);
    }
    if (tokens.size() != 2)
    {
        return false;
    }

    component = tokens[0];
    method = tokens[1];

    return true;
}

Moderator::Moderator(IOrbBrowser* browser, ApplicationType apptype)
    : mOrbBrowser(browser)
    , mNetwork(std::make_unique<Network>())
    , mMediaSynchroniser(std::make_unique<MediaSynchroniser>())
    , mAppMgrInterface(std::make_unique<AppMgrInterface>(browser, apptype))
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

    if (HasJsonParam(jsonval, "error"))
    {
        LOGE("Json request reports error");
        return "{\"error\": \"Error Request\"}";
    }

    if (!HasParam(jsonval, "method", Json::stringValue))
    {
        LOGE("Request has no method");
        return "{\"error\": \"No method\"}";
    }

    string component;
    string method;
    if (!ResolveMethod(jsonval["method"].asString(), component, method))
    {
        return "{\"error\": \"Invalid method\"}";
    }

    if (component == "Manager")
    {
        LOGI("App Manager, method: " << method);
        return mAppMgrInterface->executeRequest(method, jsonval["token"], jsonval["params"]);
    }
    else if (component == "Network")
    {
        LOGI("Network, method: " << method);
        return mNetwork->executeRequest(method, jsonval["token"], jsonval["params"]);
    }
    else if (component == "MediaSynchroniser")
    {
        LOGI("MediaSynchroniser, method: " << method);
        return mMediaSynchroniser->executeRequest(method, jsonval["token"], jsonval["params"]);
    }
    else if (component == "Broadcast")
    {
        LOGI("Broadcast, method: " << method);
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
