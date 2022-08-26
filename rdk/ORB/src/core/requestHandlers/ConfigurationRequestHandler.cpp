/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

#include "ConfigurationRequestHandler.h"
#include "ORBPlatform.h"
#include "ORB.h"

using namespace WPEFramework::Plugin;

#define CONFIGURATION_GET_LOCAL_SYSTEM "getLocalSystem"
#define CONFIGURATION_GET_PREFERRED_AUDIO_LANGUAGE "getPreferredAudioLanguage"
#define CONFIGURATION_GET_PREFERRED_SUBTITLE_LANGUAGE "getPreferredSubtitleLanguage"
#define CONFIGURATION_GET_PREFERRED_UI_LANGUAGE "getPreferredUILanguage"
#define CONFIGURATION_GET_COUNTRY_ID "getCountryId"
#define CONFIGURATION_GET_SUBTITLES_ENABLED "getSubtitlesEnabled"
#define CONFIGURATION_GET_AUDIO_DESCRIPTION_ENABLED "getAudioDescriptionEnabled"
#define CONFIGURATION_GET_DTT_NETWORK_IDS "getDttNetworkIds"
#define CONFIGIRATION_GET_DEVICE_ID "getDeviceId"
#define CONFIGURATION_REQUEST_ACCESS_TO_DISTINCTIVE_IDENTIFIER "requestAccessToDistinctiveIdentifier"

namespace orb {

/**
 * Constructor.
 */
ConfigurationRequestHandler::ConfigurationRequestHandler()
{}

/**
 * Destructor.
 */
ConfigurationRequestHandler::~ConfigurationRequestHandler()
{}

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
  JsonObject token,
  std::string method,
  JsonObject params,
  JsonObject& response)
{
  bool ret = true;

  // Configuration.getLocalSystem
  if (method == CONFIGURATION_GET_LOCAL_SYSTEM) {
    std::shared_ptr<LocalSystem> localSystem = ORB::instance(nullptr)->GetORBPlatform()->Configuration_GetLocalSystem();
    response.Set("result", localSystem->ToJsonObject());
  }

  // Configuration.getPreferredAudioLanguage
  else if (method == CONFIGURATION_GET_PREFERRED_AUDIO_LANGUAGE) {
    std::string preferredAudioLanguage = ORB::instance(nullptr)->GetORBPlatform()->Configuration_GetPreferredAudioLanguage();
    response["result"] = preferredAudioLanguage;
  }
  
  // Configuration.getPreferredSubtitleLanguage
  else if (method == CONFIGURATION_GET_PREFERRED_SUBTITLE_LANGUAGE) {
    std::string preferredSubtitleLanguage = ORB::instance(nullptr)->GetORBPlatform()->Configuration_GetPreferredSubtitleLanguage();
    response["result"] = preferredSubtitleLanguage;
  }

  // Configuration.getPreferredUILanguage
  else if (method == CONFIGURATION_GET_PREFERRED_UI_LANGUAGE) {
    std::string preferredUILanguage = ORB::instance(nullptr)->GetORBPlatform()->Configuration_GetPreferredUILanguage();
    response["result"] = preferredUILanguage;
  }

  // Configuration.getCountryId
  else if (method == CONFIGURATION_GET_COUNTRY_ID) {
    std::string countryId = ORB::instance(nullptr)->GetORBPlatform()->Configuration_GetCountryId();
    response["result"] = countryId;
  }

  // Configuration.getSubtitlesEnabled
  else if (method == CONFIGURATION_GET_SUBTITLES_ENABLED) {
    bool subtitlesEnabled = ORB::instance(nullptr)->GetORBPlatform()->Configuration_GetSubtitlesEnabled();
    response["result"] = subtitlesEnabled;
  }

  // Configuration.getAudioDescriptionEnabled
  else if (method == CONFIGURATION_GET_AUDIO_DESCRIPTION_ENABLED) {
    bool audioDescriptionEnabled = ORB::instance(nullptr)->GetORBPlatform()->Configuration_GetAudioDescriptionEnabled();
    response["result"] = audioDescriptionEnabled;
  }

  // Configuration.getDttNetworkIds
  else if (method == CONFIGURATION_GET_DTT_NETWORK_IDS) {
    std::vector<int> dttNetworkIds = ORB::instance(nullptr)->GetORBPlatform()->Configuration_GetDttNetworkIds();
    ArrayType<JsonValue> array;
    JsonValue jsonDttNetworkIds;
    for (int nid : dttNetworkIds) {
      array.Add(nid);
    }
    jsonDttNetworkIds.Array(array);
    response["result"] = jsonDttNetworkIds;
  }

  // Configuration.getDeviceId
  else if (method == CONFIGIRATION_GET_DEVICE_ID) {
    std::string deviceId = ORB::instance(nullptr)->GetORBPlatform()->Configuration_GetDeviceId();
    response["result"] = deviceId;
  }

  // Configuration.requestAccessToDistinctiveIdentifier
  else if (method == CONFIGURATION_REQUEST_ACCESS_TO_DISTINCTIVE_IDENTIFIER) {
    JsonObject jsonPayload = token["payload"].Object();
    std::string origin = jsonPayload["origin"].String();
    ORB::instance(nullptr)->GetORBPlatform()->Configuration_RequestAccessToDistinctiveIdentifier(origin);
    response.FromString("{}");
  }

  // UnknownMethod
  else {
    response = RequestHandler::MakeErrorResponse("UnknownMethod");
    ret = false;
  }

  return ret;
}

} // namespace orb
