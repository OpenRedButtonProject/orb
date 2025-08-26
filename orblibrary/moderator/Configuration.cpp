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

#include "Configuration.hpp"
#include "log.hpp"
#include "ConfigurationUtil.h"
#include "JsonUtil.h"

using namespace std;

namespace orb
{
// change to const string
const string CONFIGURATION_GET_CAPABILITIES = "getCapabilities";
const string CONFIGURATION_GET_AUDIO_PROFILES = "getAudioProfiles";
const string CONFIGURATION_GET_VIDEO_PROFILES = "getVideoProfiles";
const string CONFIGURATION_METHOD_PREFIX = "Configuration.";

Configuration::Configuration(ApplicationType apptype, IOrbBrowser* browser)
    : ComponentBase(), mAppType(apptype), mOrbBrowser(browser)
{
    LOGI("Configuration constructor - apptype: " << apptype);
}

string Configuration::executeRequest(string method, Json::Value token, Json::Value params)
{
    LOGI("Configuration::executeRequest - method: " << method);
    const string RESULT_KEY = "result";
    // create an empty json object

    if (method == CONFIGURATION_GET_CAPABILITIES) {
        return handleGetCapabilities();
    }
    else if (method == CONFIGURATION_GET_AUDIO_PROFILES) {
        return handleGetAudioProfiles();
    }
    else if (method == CONFIGURATION_GET_VIDEO_PROFILES) {
        return handleGetVideoProfiles();
    }

    Json::Value response;
    response[RESULT_KEY] = "Configuration method '" + method + "' Not implemented";
    return JsonUtil::convertJsonToString(response);
}

string Configuration::handleGetCapabilities()
{
    std::string request = ConfigurationUtil::generateRequest(CONFIGURATION_METHOD_PREFIX+CONFIGURATION_GET_CAPABILITIES, mAppType);
    return mOrbBrowser->sendRequestToClient(request);
}

string Configuration::handleGetAudioProfiles()
{
    std::string request = ConfigurationUtil::generateRequest(CONFIGURATION_METHOD_PREFIX+CONFIGURATION_GET_AUDIO_PROFILES, mAppType);
    return mOrbBrowser->sendRequestToClient(request);
}

string Configuration::handleGetVideoProfiles()
{
    std::string request = ConfigurationUtil::generateRequest(CONFIGURATION_METHOD_PREFIX+CONFIGURATION_GET_VIDEO_PROFILES, mAppType);
    return mOrbBrowser->sendRequestToClient(request);
}


} // namespace orb