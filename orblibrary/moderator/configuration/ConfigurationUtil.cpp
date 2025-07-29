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

#include "ConfigurationUtil.h"
#include <json/json.h>
#include <sstream>
#include <random>


using namespace std;

namespace orb
{
const int JSON_RPC_SERVER_PORT = 8910;
const string JSON_RPC_ENDPOINT = "/hbbtv/" + ConfigurationUtil::generateRandomNumberStr() + "/";
const string JSON_RPC_SERVER_VERSION = "1.7.1";

std::shared_ptr<Capabilities> ConfigurationUtil::createDefaultCapabilities(ApplicationType apptype)
{

    // TODO: This is a mock implementation.
    std::shared_ptr<Capabilities> capabilities = std::make_shared<Capabilities>();

    std::vector<std::string> optionStrings;
    optionStrings.push_back("+PVR");
    optionStrings.push_back("+DRM");

    std::vector<std::string> profileNameFragments;
    profileNameFragments.push_back("+TRICKMODE"); // +ITV_KEYS is inherited from the base profile
    profileNameFragments.push_back("+DVB_T");
    profileNameFragments.push_back("+DVB_T2");
    profileNameFragments.push_back("+DVB_S");
    profileNameFragments.push_back("+DVB_S2");

    std::vector<std::string> parentalSchemes;
    parentalSchemes.push_back("dvb-si");

    std::vector<std::string> graphicsLevels;
    graphicsLevels.push_back("urn:hbbtv:graphics:performance:level1");
    graphicsLevels.push_back("urn:hbbtv:graphics:performance:level2");

    std::vector<std::string> broadcastUrns;
    broadcastUrns.push_back("urn:dvb:broadcast:ird:video:25_Hz_H.264_AVC_HDTV_IRD");
    broadcastUrns.push_back("urn:dvb:broadcast:ird:video:30_Hz_H.264_AVC_HDTV_IRD");
    broadcastUrns.push_back("urn:dvb:broadcast:ird:video:50_Hz_H.264_AVC_HDTV_IRD");
    broadcastUrns.push_back("urn:dvb:broadcast:ird:video:60_Hz_H.264_AVC_HDTV_IRD");
    broadcastUrns.push_back("urn:dvb:broadcast:ird:video:50_Hz_HEVC_HDTV_8-bit_IRD");
    broadcastUrns.push_back("urn:dvb:broadcast:ird:video:60_Hz_HEVC_HDTV_8-bit_IRD");

    broadcastUrns.push_back("urn:dvb:broadcast:ird:audio:MPEG-1_and_MPEG-2_backwards_compatible");
    broadcastUrns.push_back("urn:dvb:broadcast:ird:audio:AC-3_and_enhanced_AC-3");
    broadcastUrns.push_back("urn:dvb:broadcast:ird:audio:MPEG-4_AAC_family");

    std::string displaySizeWidth = "71"; // Mock 32" TV
    std::string displaySizeHeight = "40"; // Mock 32" TV
    std::string displaySizeMeasurementType = "built-in"; // hdmi-accurate, hdmi-other
    std::string audioOutputFormat = "stereo"; // multichannel, multichannel-preferred
    bool passThroughStatus = false;
    std::string html5MediaVariableRateMin = "0.5";
    std::string html5MediaVariableRateMax = "5.0";

    capabilities->m_optionStrings = optionStrings;
    capabilities->m_profileNameFragments = profileNameFragments;
    capabilities->m_parentalSchemes = parentalSchemes;
    capabilities->m_graphicsLevels = graphicsLevels;
    capabilities->m_broadcastUrns = broadcastUrns;
    capabilities->m_displaySizeWidth = displaySizeWidth;
    capabilities->m_displaySizeHeight = displaySizeHeight;
    capabilities->m_displaySizeMeasurementType = displaySizeMeasurementType;
    capabilities->m_audioOutputFormat = audioOutputFormat;
    capabilities->m_passThroughStatus = passThroughStatus;
    capabilities->m_html5MediaVariableRateMin = html5MediaVariableRateMin;
    capabilities->m_html5MediaVariableRateMax = html5MediaVariableRateMax;
    // HbbTV port is 8910, OpApp port is 8911
    int port = getJsonRpcServerPort(apptype);
    capabilities->m_jsonRpcServerUrl = getJsonRpcServerUrl(port);
    capabilities->m_jsonRpcServerVersion = JSON_RPC_SERVER_VERSION;
    return capabilities;
}

std::vector<AudioProfile> ConfigurationUtil::createDefaultAudioProfiles()
{
    std::vector<AudioProfile> audioProfiles;
    audioProfiles.push_back(createAudioProfile("MPEG1_L3", "audio/mpeg", "", "", ""));
    audioProfiles.push_back(createAudioProfile("HEAAC", "audio/mp4", "", "", ""));
    audioProfiles.push_back(createAudioProfile("MP4_HEAAC", "audio/mp4", "dash", "dash_pr", ""));
    audioProfiles.push_back(createAudioProfile("MP4_E-AC3", "audio/mp4", "", "", ""));
    audioProfiles.push_back(createAudioProfile("MP4_E-AC3", "audio/mp4", "dash", "dash_pr", ""));
    return audioProfiles;
}

AudioProfile ConfigurationUtil::createAudioProfile(
    std::string name,
    std::string type,
    std::string transport,
    std::string syncTl,
    std::string drmSystemId)
{
    AudioProfile audioProfile;
    audioProfile.m_name = name;
    audioProfile.m_type = type;
    audioProfile.m_transport = transport;
    audioProfile.m_syncTl = syncTl;
    audioProfile.m_drmSystemId = drmSystemId;
    return audioProfile;
}

std::vector<VideoProfile> ConfigurationUtil::createDefaultVideoProfiles()
{
    std::vector<VideoProfile> videoProfiles;
    videoProfiles.push_back(createVideoProfile("MP4_AVC_SD_25_HEAAC", "video/mp4", "", "", "", ""));
    videoProfiles.push_back(createVideoProfile("MP4_AVC_HD_25_HEAAC", "video/mp4", "", "", "", ""));
    videoProfiles.push_back(createVideoProfile("MP4_AVC_SD_25_HEAAC_EBUTTD", "video/mp4", "", "", "", ""));
    videoProfiles.push_back(createVideoProfile("MP4_AVC_HD_25_HEAAC_EBUTTD", "video/mp4", "", "", "", ""));
    videoProfiles.push_back(createVideoProfile("TS_AVC_SD_25_HEAAC", "video/mpeg", "", "temi", "", ""));
    videoProfiles.push_back(createVideoProfile("TS_AVC_HD_25_HEAAC", "video/mpeg", "", "temi", "", ""));
    videoProfiles.push_back(createVideoProfile("MP4_AVC_SD_25_HEAAC", "video/mp4", "", "", "", ""));
    videoProfiles.push_back(createVideoProfile("MP4_AVC_HD_25_HEAAC", "video/mp4", "", "", "", ""));
    videoProfiles.push_back(createVideoProfile("TS_AVC_SD_25_E-AC3", "video/mpeg", "", "temi", "", ""));
    videoProfiles.push_back(createVideoProfile("TS_AVC_HD_25_E-AC3", "video/mpeg", "", "temi", "", ""));
    videoProfiles.push_back(createVideoProfile("MP4_AVC_SD_25_E-AC3", "video/mp4", "", "", "", ""));
    videoProfiles.push_back(createVideoProfile("MP4_AVC_HD_25_E-AC3", "video/mp4", "", "", "", ""));
    videoProfiles.push_back(createVideoProfile("MP4_AVC_SD_25_E-AC3_EBUTTD", "video/mp4", "dash", "dash_pr", "", ""));
    videoProfiles.push_back(createVideoProfile("MP4_AVC_HD_25_E-AC3_EBUTTD", "video/mp4", "dash", "dash_pr", "", ""));
    return videoProfiles;
}

VideoProfile ConfigurationUtil::createVideoProfile(
    std::string name,
    std::string type,
    std::string transport,
    std::string syncTl,
    std::string drmSystemId,
    std::string hdr)
{
    VideoProfile videoProfile;
    videoProfile.m_name = name;
    videoProfile.m_type = type;
    videoProfile.m_transport = transport;
    videoProfile.m_syncTl = syncTl;
    videoProfile.m_drmSystemId = drmSystemId;
    videoProfile.m_hdr = hdr;
    return videoProfile;
}

VideoDisplayFormat ConfigurationUtil::createDefaultVideoDisplayFormat()
{
    return VideoDisplayFormat{};
}

std::string ConfigurationUtil::getJsonRpcServerUrl(int port)
{
    std::string rpcUrl = "ws://localhost:"
    + std::to_string(port)
    + JSON_RPC_ENDPOINT;
    return rpcUrl;
}

std::string ConfigurationUtil::generateRandomNumberStr()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(100000, 999999);
    return std::to_string(dis(gen));
}

Json::Value ConfigurationUtil::capabilitiesToJson(const Capabilities& capabilities)
{
    Json::Value jsonCapabilities(Json::objectValue);

    // Convert option strings
    Json::Value jsonOptionStrings = Json::arrayValue;
    for (const auto& optionString : capabilities.m_optionStrings) {
        jsonOptionStrings.append(optionString);
    }
    jsonCapabilities["optionStrings"] = jsonOptionStrings;

    // Convert profile name fragments
    Json::Value jsonProfileNameFragments = Json::arrayValue;
    for (const auto& profileNameFragment : capabilities.m_profileNameFragments) {
        jsonProfileNameFragments.append(profileNameFragment);
    }
    jsonCapabilities["profileNameFragments"] = jsonProfileNameFragments;

    // Convert parental schemes
    Json::Value jsonParentalSchemes = Json::arrayValue;
    for (const auto& parentalScheme : capabilities.m_parentalSchemes) {
        jsonParentalSchemes.append(parentalScheme);
    }
    jsonCapabilities["parentalSchemes"] = jsonParentalSchemes;

    // Convert graphics levels (optional)
    if (!capabilities.m_graphicsLevels.empty()) {
        Json::Value jsonGraphicsLevels = Json::arrayValue;
        for (const auto& graphicsLevel : capabilities.m_graphicsLevels) {
            jsonGraphicsLevels.append(graphicsLevel);
        }
        jsonCapabilities["graphicsLevels"] = jsonGraphicsLevels;
    }

    // Convert broadcast URNs (optional)
    if (!capabilities.m_broadcastUrns.empty()) {
        Json::Value jsonBroadcastUrns;
        for (const auto& broadcastUrn : capabilities.m_broadcastUrns) {
            jsonBroadcastUrns.append(broadcastUrn);
        }
        jsonCapabilities["broadcastUrns"] = jsonBroadcastUrns;
    }

    // Convert other fields
    jsonCapabilities["displaySizeWidth"] = capabilities.m_displaySizeWidth;
    jsonCapabilities["displaySizeHeight"] = capabilities.m_displaySizeHeight;
    jsonCapabilities["displaySizeMeasurementType"] = capabilities.m_displaySizeMeasurementType;

    if (!capabilities.m_audioOutputFormat.empty()) {
        jsonCapabilities["audioOutputFormat"] = capabilities.m_audioOutputFormat;
    }

    jsonCapabilities["passThroughStatus"] = capabilities.m_passThroughStatus;

    if (!capabilities.m_html5MediaVariableRateMin.empty()) {
        jsonCapabilities["html5MediaVariableRateMin"] = capabilities.m_html5MediaVariableRateMin;
    }

    if (!capabilities.m_html5MediaVariableRateMax.empty()) {
        jsonCapabilities["html5MediaVariableRateMax"] = capabilities.m_html5MediaVariableRateMax;
    }

    jsonCapabilities["jsonRpcServerUrl"] = capabilities.m_jsonRpcServerUrl;
    jsonCapabilities["jsonRpcServerVersion"] = capabilities.m_jsonRpcServerVersion;

    return jsonCapabilities;
}

Json::Value ConfigurationUtil::audioProfilesToJson(const std::vector<AudioProfile>& audioProfiles)
{
    Json::Value jsonAudioProfiles = Json::arrayValue;
    for (const auto& audioProfile : audioProfiles) {
        Json::Value jsonAudioProfile = audioProfileToJson(audioProfile);
        jsonAudioProfiles.append(jsonAudioProfile);
    }
    return jsonAudioProfiles;
}

Json::Value ConfigurationUtil::audioProfileToJson(const AudioProfile& audioProfile)
{
    Json::Value jsonAudioProfile;
    jsonAudioProfile["name"] = audioProfile.m_name;
    jsonAudioProfile["type"] = audioProfile.m_type;
    jsonAudioProfile["transport"] = audioProfile.m_transport;
    jsonAudioProfile["syncTl"] = audioProfile.m_syncTl;
    jsonAudioProfile["drmSystemId"] = audioProfile.m_drmSystemId;
    return jsonAudioProfile;
}

Json::Value ConfigurationUtil::videoProfilesToJson(const std::vector<VideoProfile>& videoProfiles)
{
    Json::Value jsonVideoProfiles = Json::arrayValue;
    for (const auto& videoProfile : videoProfiles) {
        Json::Value jsonVideoProfile = audioProfileToJson(videoProfile);
        jsonVideoProfile["hdr"] = videoProfile.m_hdr;
        jsonVideoProfiles.append(jsonVideoProfile);
    }
    return jsonVideoProfiles;
}

std::string ConfigurationUtil::convertJsonToString(const Json::Value& json)
{
    Json::StreamWriterBuilder writer;
    return Json::writeString(writer, json);
}

std::string ConfigurationUtil::getJsonRpcServerEndpoint()
{
    return JSON_RPC_ENDPOINT;
}

int ConfigurationUtil::getJsonRpcServerPort(ApplicationType apptype)
{
    return apptype == ApplicationType::APP_TYPE_HBBTV ? JSON_RPC_SERVER_PORT + 1 : JSON_RPC_SERVER_PORT;
}
} // namespace orb