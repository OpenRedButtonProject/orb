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

Configuration::Configuration(std::shared_ptr<IPlatform> platform)
    : ComponentBase(), mPlatform(platform)
{
    LOGI("Configuration constructor");
}

string Configuration::executeRequest(string method, Json::Value token, Json::Value params)
{
    LOGI("Configuration::executeRequest - method: " << method);
    const string RESULT_KEY = "result";
    // create an empty json object
    Json::Value response;
    if (method == CONFIGURATION_GET_CAPABILITIES) {
        response[RESULT_KEY] = handleGetCapabilities();
    }
    else if (method == CONFIGURATION_GET_AUDIO_PROFILES) {
        response[RESULT_KEY] = handleGetAudioProfiles();
    }
    else if (method == CONFIGURATION_GET_VIDEO_PROFILES) {
        response[RESULT_KEY] = handleGetVideoProfiles();
    }
    else {
        response[RESULT_KEY] = "Configuration method '" + method + "' received";
    }

    // TODO: Implement configuration-specific methods
    // For now, return a basic response indicating the method was received
    string responseString = JsonUtil::convertJsonToString(response);
    // LOGI("Configuration::executeRequest - response: " << responseString);
    return responseString;
}

Json::Value Configuration::handleGetCapabilities()
{
    std::shared_ptr<Capabilities> capabilities = mPlatform->Configuration_GetCapabilities();
    return ConfigurationUtil::capabilitiesToJson(*capabilities);
}

Json::Value Configuration::handleGetAudioProfiles()
{
    std::vector<AudioProfile> audioProfiles = mPlatform->Configuration_GetAudioProfiles();
    return ConfigurationUtil::audioProfilesToJson(audioProfiles);
}

Json::Value Configuration::handleGetVideoProfiles()
{
    std::vector<VideoProfile> videoProfiles = mPlatform->Configuration_GetVideoProfiles();
    return ConfigurationUtil::videoProfilesToJson(videoProfiles);
}


} // namespace orb