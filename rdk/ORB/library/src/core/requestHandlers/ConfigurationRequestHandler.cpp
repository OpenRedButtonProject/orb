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

#include "ConfigurationRequestHandler.h"
#include "ORBPlatform.h"
#include "ORBEngine.h"
#include "JsonUtil.h"
#include "DisplayInfo.h"

#define CONFIGURATION_GET_CAPABILITIES "getCapabilities"
#define CONFIGURATION_GET_AUDIO_PROFILES "getAudioProfiles"
#define CONFIGURATION_GET_VIDEO_PROFILES "getVideoProfiles"
#define CONFIGURATION_GET_VIDEO_DISPLAY_FORMATS "getVideoDisplayFormats"
#define CONFIGURATION_GET_EXTRA_SD_VIDEO_DECODES "getExtraSDVideoDecodes"
#define CONFIGURATION_GET_EXTRA_HD_VIDEO_DECODES "getExtraHDVideoDecodes"
#define CONFIGURATION_GET_EXTRA_UHD_VIDEO_DECODES "getExtraUHDVideoDecodes"
#define CONFIGURATION_GET_LOCAL_SYSTEM "getLocalSystem"
#define CONFIGURATION_GET_PREFERRED_AUDIO_LANGUAGE "getPreferredAudioLanguage"
#define CONFIGURATION_GET_PREFERRED_SUBTITLE_LANGUAGE "getPreferredSubtitleLanguage"
#define CONFIGURATION_GET_PREFERRED_UI_LANGUAGE "getPreferredUILanguage"
#define CONFIGURATION_GET_COUNTRY_ID "getCountryId"
#define CONFIGURATION_GET_SUBTITLES_ENABLED "getSubtitlesEnabled"
#define CONFIGURATION_GET_AUDIO_DESCRIPTION_ENABLED "getAudioDescriptionEnabled"
#define CONFIGURATION_GET_DTT_NETWORK_IDS "getDttNetworkIds"
#define CONFIGIRATION_GET_DEVICE_ID "getDeviceId"
#define CONFIGURATION_REQUEST_ACCESS_TO_DISTINCTIVE_IDENTIFIER \
    "requestAccessToDistinctiveIdentifier"
#define CONFIGURATION_GET_PRIMARY_DISPLAY "getPrimaryDisplay"
#define CONFIGURATION_GET_CLEAN_AUDIO_ENABLED "getCleanAudioEnabled"

namespace orb {
/**
 * Constructor.
 */
ConfigurationRequestHandler::ConfigurationRequestHandler()
{
}

/**
 * Destructor.
 */
ConfigurationRequestHandler::~ConfigurationRequestHandler()
{
}

/**
 * @brief ConfigurationRequestHandler::Handle
 *
 * Handles the given Configuration request.
 *
 * @param token    (in)  The request token
 * @param method   (in)  The requested method
 * @param params   (in)  A JSON object containing the input parameters (if any)
 * @param response (out) A JSON object containing the response
 *
 * @return true in success, otherwise false
 */
bool ConfigurationRequestHandler::Handle(
    json token,
    std::string method,
    json params,
    json& response)
{
    bool ret = true;
    response = "{}"_json;

    // Configuration.getCapabilities
    if (method == CONFIGURATION_GET_CAPABILITIES)
    {
        response["result"] = GetCapabilities();
    }
    // Configuration.getAudioProfiles
    else if (method == CONFIGURATION_GET_AUDIO_PROFILES)
    {
        response["result"] = GetAudioProfiles();
    }
    // Configuration.getVideoProfiles
    else if (method == CONFIGURATION_GET_VIDEO_PROFILES)
    {
        response["result"] = GetVideoProfiles();
    }
    // Configuration.getVideoDisplayFormats
    else if (method == CONFIGURATION_GET_VIDEO_DISPLAY_FORMATS)
    {
        response["result"] = GetVideoDisplayFormats();
    }
    // Configuration.getExtraSDVideoDecodes
    else if (method == CONFIGURATION_GET_EXTRA_SD_VIDEO_DECODES)
    {
        response["result"] = GetExtraSDVideoDecodes();
    }
    // Configuration.getExtraHDVideoDecodes
    else if (method == CONFIGURATION_GET_EXTRA_HD_VIDEO_DECODES)
    {
        response["result"] = GetExtraHDVideoDecodes();
    }
    // Configuration.getExtraUHDVideoDecodes
    else if (method == CONFIGURATION_GET_EXTRA_UHD_VIDEO_DECODES)
    {
        response["result"] = GetExtraUHDVideoDecodes();
    }
    // Configuration.getLocalSystem
    else if (method == CONFIGURATION_GET_LOCAL_SYSTEM)
    {
        std::shared_ptr<LocalSystem> localSystem =
            ORBEngine::GetSharedInstance().GetORBPlatform()->Configuration_GetLocalSystem();
        response["result"] = JsonUtil::LocalSystemToJsonObject(*(localSystem.get()));
    }
    // Configuration.getPreferredAudioLanguage
    else if (method == CONFIGURATION_GET_PREFERRED_AUDIO_LANGUAGE)
    {
        std::string preferredAudioLanguage =
            ORBEngine::GetSharedInstance().GetORBPlatform()->Configuration_GetPreferredAudioLanguage();
        response["result"] = preferredAudioLanguage;
    }
    // Configuration.getPreferredSubtitleLanguage
    else if (method == CONFIGURATION_GET_PREFERRED_SUBTITLE_LANGUAGE)
    {
        std::string preferredSubtitleLanguage =
            ORBEngine::GetSharedInstance().GetORBPlatform()->
            Configuration_GetPreferredSubtitleLanguage();
        response["result"] = preferredSubtitleLanguage;
    }
    // Configuration.getPreferredUILanguage
    else if (method == CONFIGURATION_GET_PREFERRED_UI_LANGUAGE)
    {
        std::string preferredUILanguage =
            ORBEngine::GetSharedInstance().GetORBPlatform()->Configuration_GetPreferredUILanguage();
        if (preferredUILanguage.empty())
        {
            preferredUILanguage = ORBEngine::GetSharedInstance().GetPreferredUILanguage();
        }
        response["result"] = preferredUILanguage;
    }
    // Configuration.getCountryId
    else if (method == CONFIGURATION_GET_COUNTRY_ID)
    {
        std::string countryId =
            ORBEngine::GetSharedInstance().GetORBPlatform()->Configuration_GetCountryId();
        response["result"] = countryId;
    }
    // Configuration.getSubtitlesEnabled
    else if (method == CONFIGURATION_GET_SUBTITLES_ENABLED)
    {
        bool subtitlesEnabled =
            ORBEngine::GetSharedInstance().GetORBPlatform()->Configuration_GetSubtitlesEnabled();
        response["result"] = subtitlesEnabled;
    }
    // Configuration.getAudioDescriptionEnabled
    else if (method == CONFIGURATION_GET_AUDIO_DESCRIPTION_ENABLED)
    {
        bool audioDescriptionEnabled =
            ORBEngine::GetSharedInstance().GetORBPlatform()->
            Configuration_GetAudioDescriptionEnabled();
        response["result"] = audioDescriptionEnabled;
    }
    // Configuration.getDttNetworkIds
    else if (method == CONFIGURATION_GET_DTT_NETWORK_IDS)
    {
        std::vector<int> dttNetworkIds = GetDttNetworkIds();
        json array = json::array();
        for (int nid : dttNetworkIds)
        {
            array.push_back(nid);
        }
        response["result"] = array;
    }
    // Configuration.getDeviceId
    else if (method == CONFIGIRATION_GET_DEVICE_ID)
    {
        std::string deviceId =
            ORBEngine::GetSharedInstance().GetORBPlatform()->Configuration_GetDeviceId();
        response["result"] = deviceId;
    }
    // Configuration.requestAccessToDistinctiveIdentifier
    else if (method == CONFIGURATION_REQUEST_ACCESS_TO_DISTINCTIVE_IDENTIFIER)
    {
        json jsonPayload = token["payload"];
        std::string origin = jsonPayload.value("origin", "");
        std::map<std::string, std::string> appNames =
            ORBEngine::GetSharedInstance().GetApplicationManager()->GetCurrentAppNames();
        ORBEngine::GetSharedInstance().GetORBPlatform()->
        Configuration_RequestAccessToDistinctiveIdentifier(origin, appNames);
    }
        // Configuration.getCleanAudioEnabled
    else if (method == CONFIGURATION_GET_CLEAN_AUDIO_ENABLED)
    {
        json jsonPayload = token["payload"];
        std::string origin = jsonPayload.value("origin", "");
        bool enabled =
            ORBEngine::GetSharedInstance().GetORBPlatform()->Configuration_GetCleanAudioEnabled();
        response["result"] = enabled;
    }
#ifdef BBC_API_ENABLE
    // Configuration.getPrimaryDisplay
    else if (method == CONFIGURATION_GET_PRIMARY_DISPLAY)
    {
        std::shared_ptr<DisplayInfo> displayInfo =
            ORBEngine::GetSharedInstance().GetORBPlatform()->Configuration_GetPrimaryDisplay();
        response["result"] = JsonUtil::DisplayInfoToJsonObject(*(displayInfo.get()));
    }
#endif
    // UnknownMethod
    else
    {
        response = ORBBridgeRequestHandler::MakeErrorResponse("UnknownMethod");
        ret = false;
    }

    return ret;
}

/**
 * Get the current capabilities of the terminal.
 *
 * @return A JSON representation of the capabilities object
 */
json ConfigurationRequestHandler::GetCapabilities()
{
    json result;
    std::shared_ptr<Capabilities> capabilities =
        ORBEngine::GetSharedInstance().GetORBPlatform()->Configuration_GetCapabilities();
    result = JsonUtil::CapabilitiesToJsonObject(capabilities);
    return result;
}

/**
 * Get a list of audio profiles supported by the terminal, as defined by HBBTV 10.2.4.7 for
 * the audio_profile element.
 *
 * @return A JSON array with the audio profiles
 */
json ConfigurationRequestHandler::GetAudioProfiles()
{
    json result = json::array();
    std::vector<AudioProfile> audioProfiles =
        ORBEngine::GetSharedInstance().GetORBPlatform()->Configuration_GetAudioProfiles();
    for (AudioProfile audioProfile : audioProfiles)
    {
        result.push_back(JsonUtil::AudioProfileToJsonObject(audioProfile));
    }
    return result;
}

/**
 * Get a list of video profiles supported by the terminal, as defined by HBBTV 10.2.4.7 for
 * the video_profile element.
 *
 * @return A JSON array with the video profiles
 */
json ConfigurationRequestHandler::GetVideoProfiles()
{
    json result = json::array();
    std::vector<VideoProfile> videoProfiles =
        ORBEngine::GetSharedInstance().GetORBPlatform()->Configuration_GetVideoProfiles();
    for (VideoProfile videoProfile : videoProfiles)
    {
        result.push_back(JsonUtil::VideoProfileToJsonObject(videoProfile));
    }
    return result;
}

/**
 * If the terminal supports UHD, get a list that describes the highest quality video format the
 * terminal supports, as defined by HBBTV 10.2.4.7 for the video_display_format element;
 * otherwise get an empty list.
 *
 * @return A JSON array with the video display formats
 */
json ConfigurationRequestHandler::GetVideoDisplayFormats()
{
    json result = json::array();
    std::vector<VideoDisplayFormat> videoDisplayFormats =
        ORBEngine::GetSharedInstance().GetORBPlatform()->Configuration_GetVideoDisplayFormats();
    for (VideoDisplayFormat videoDisplayFormat : videoDisplayFormats)
    {
        result.push_back(JsonUtil::VideoDisplayFormatToJsonObject(videoDisplayFormat));
    }
    return result;
}

/**
 * Get the current number of additional media streams containing SD video accompanied by audio
 * that can be decoded and presented by an A/V control object or HTML5 media element.
 *
 * @return The extra SD video decodes
 */
int ConfigurationRequestHandler::GetExtraSDVideoDecodes()
{
    return ORBEngine::GetSharedInstance().GetORBPlatform()->Configuration_GetExtraSDVideoDecodes();
}

/**
 * Get the current number of additional media streams containing HD video accompanied by audio
 * that can be decoded and presented by an A/V control object or HTML5 media element.
 *
 * @return The extra HD video decodes
 */
int ConfigurationRequestHandler::GetExtraHDVideoDecodes()
{
    return ORBEngine::GetSharedInstance().GetORBPlatform()->Configuration_GetExtraHDVideoDecodes();
}

/**
 * Get the current number of additional media streams containing UHD video accompanied by audio
 * that can be decoded and presented by an A/V control object or HTML5 media element.
 *
 * @return The extra UHD video decodes
 */
int ConfigurationRequestHandler::GetExtraUHDVideoDecodes()
{
    return ORBEngine::GetSharedInstance().GetORBPlatform()->Configuration_GetExtraUHDVideoDecodes();
}

/**
 * Get the DTT network ids from the DVB_T & DVB_T2 channels found in the channel list.
 *
 * @return A vector containing the DTT network ids
 */
std::vector<int> ConfigurationRequestHandler::GetDttNetworkIds()
{
    std::vector<int> dttNetworkIds;
    std::vector<Channel> channelList =
        ORBEngine::GetSharedInstance().GetORBPlatform()->Broadcast_GetChannelList();
    for (auto channel : channelList)
    {
        int idType = channel.GetIdType();
        if (idType == Channel::IdType::CHANNEL_ID_DVB_T || idType ==
            Channel::IdType::CHANNEL_ID_DVB_T2)
        {
            int nid = channel.GetNid();
            if (nid != 0)
            {
                dttNetworkIds.push_back(nid);
            }
        }
    }
    return dttNetworkIds;
}
} // namespace orb
