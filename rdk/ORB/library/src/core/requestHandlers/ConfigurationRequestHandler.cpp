/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

#include "ConfigurationRequestHandler.h"
#include "ORBPlatform.h"
#include "ORBEngine.h"
#include "JsonUtil.h"

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

    // Configuration.getLocalSystem
    if (method == CONFIGURATION_GET_LOCAL_SYSTEM)
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
        std::string preferredUILanguage = ORBEngine::GetSharedInstance().GetPreferredUILanguage();
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
        json array;
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
        ORBEngine::GetSharedInstance().GetORBPlatform()->
        Configuration_RequestAccessToDistinctiveIdentifier(origin);
    }
    // UnknownMethod
    else
    {
        response = RequestHandler::MakeErrorResponse("UnknownMethod");
        ret = false;
    }

    return ret;
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
