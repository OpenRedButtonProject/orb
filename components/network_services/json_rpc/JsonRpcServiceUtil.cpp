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
 * ORB JsonRpcServiceUtil
 *
 */

#include "JsonRpcServiceUtil.h"
#include "log.h"
#include <iomanip>
#include <sstream> 

namespace orb
{
namespace networkServices
{
   /**
    * Query the feature settings of subtitles
    *
    * @param enabled           Enabled subtitles
    * @param size              The font size
    * @param fontFamily        The description of the font family
    * @param textColour        The text colour in RGB24 format
    * @param textOpacity       The test opacity with the percentage from 0 to 100
    * @param edgeType          The description of edge type
    * @param edgeColour        The edge colour in RGB24 format
    * @param backgroundColour  The background colour in RGB24 format
    * @param backgroundOpacity The background opacity with the percentage from 0 to 100
    * @param windowColour      The window colour in RGB24 format
    * @param windowOpacity     The window opacity with the percentage from 0 to 100
    * @param language          The description of language in ISO639-2 3-character code
    * @return A Json object containing the settings.
    */
    Json::Value JsonRpcServiceUtil::QuerySettingsSubtitles(bool enabled, int size, const std::string &fontFamily, const
        std::string &textColour, int textOpacity, const std::string &edgeType, const std::string
        &edgeColour, const std::string &backgroundColour, int backgroundOpacity, const std::string
        &windowColour, int windowOpacity, const std::string &language)
    {
        Json::Value value;
        value["enabled"] = enabled;
        // Optional Parameters shall only be present if the user has explicitly modified the setting
        // value to something other than the terminal default value.
        if (!enabled)
        {
            return value;
        }
        if (size != OPTIONAL_INT_NOT_SET)
        {
            value["size"] = std::min(std::max(size, 25), 300);
        }
        if (fontFamily != OPTIONAL_STR_NOT_SET)
        {
            value["fontFamily"] = fontFamily;
        }
        if (textColour != OPTIONAL_STR_NOT_SET)
        {
            value["textColour"] = textColour;
        }
        if (textOpacity != OPTIONAL_INT_NOT_SET)
        {
            value["textOpacity"] = std::min(std::max(size, 0), 100);
        }
        if (edgeType == "outline" || edgeType == "dropShadow" || edgeType == "raised" || edgeType ==
            "depressed")
        {
            value["edgeType"] = edgeType;
        }
        if (edgeColour != OPTIONAL_STR_NOT_SET)
        {
            value["edgeColour"] = edgeColour;
        }
        if (backgroundColour != OPTIONAL_STR_NOT_SET)
        {
            value["backgroundColour"] = backgroundColour;
        }
        if (backgroundOpacity != OPTIONAL_INT_NOT_SET)
        {
            value["backgroundOpacity"] = std::min(std::max(backgroundOpacity, 0), 100);
        }
        if (windowColour != OPTIONAL_STR_NOT_SET)
        {
            value["windowColour"] = windowColour;
        }
        if (windowOpacity != OPTIONAL_INT_NOT_SET)
        {
            value["windowOpacity"] = std::min(std::max(windowOpacity, 0), 100);
        }
        if (language != OPTIONAL_STR_NOT_SET)
        {
            value["language"] = language;
        }
        return value;
    }

    /**
    * Query the feature settings of dialogue enhancement
    *
    * @param gainPreference The dialogue enhancement gain preference in dB
    * @param gain           The currently-active gain value in dB
    * @param limitMin       The current allowed minimum gain value in dB
    * @param limitMax       The current allowed maximum gain value in dB
    * @return A Json object containing the settings.
    */
    Json::Value JsonRpcServiceUtil::QuerySettingsDialogueEnhancement(int dialogueEnhancementGainPreference, int
        dialogueEnhancementGain, int dialogueEnhancementLimitMin, int dialogueEnhancementLimitMax)
    {
        Json::Value value;
        value["dialogueEnhancementGainPreference"] = dialogueEnhancementGainPreference;
        value["dialogueEnhancementGain"] = dialogueEnhancementGain;
        Json::Value dialogueEnhancementLimit;
        dialogueEnhancementLimit["min"] = dialogueEnhancementLimitMin;
        dialogueEnhancementLimit["max"] = dialogueEnhancementLimitMax;
        value["dialogueEnhancementLimit"] = dialogueEnhancementLimit;
        return value;
    }

    /**
    * Query the feature settings of UI magnifier
    *
    * @param enabled    Enabled a screen magnification UI setting
    * @param magType    The description of the type of magnification scheme currently set
    * @return A Json object containing the settings.
    */
    Json::Value JsonRpcServiceUtil::QuerySettingsUIMagnifier(bool enabled, const std::string &magType)
    {
        Json::Value value;
        value["enabled"] = enabled;
        // It shall be present if the "enabled" parameter is set to true
        // and shall not be present if the "enabled" parameter is set to false.
        if (!enabled)
        {
            return value;
        }
        std::vector<std::string> magTypes = {"textMagnification", "magnifierGlass", "screenZoom",
                                            "largeLayout", "other"};
        if (count(magTypes.begin(), magTypes.end(), magType))
        {
            value["magType"] = magType;
        }
        else
        {
            value["magType"] = "other";
        }
        return value;
    }

    /**
    * Query the feature settings of high contrast UI
    *
    * @param enabled    Enabled a high contrast UI
    * @param hcType     The description of the type of high contrast scheme currently set
    * @return A Json object containing the settings.
    */
    Json::Value JsonRpcServiceUtil::QuerySettingsHighContrastUI(bool enabled, const std::string &hcType)
    {
        Json::Value value;
        value["enabled"] = enabled;
        // It shall be present if the "enabled" parameter is set to "true".
        if (hcType == "monochrome" || hcType == "other")
        {
            value["hcType"] = hcType;
        }
        else if (enabled)
        {
            value["hcType"] = "other";
        }
        return value;
    }

    /**
    * Query the feature settings of screen reader
    *
    * @param enabled    Enabled a screen reader preference
    * @param speed      A percentage scaling factor of the default speech speed, 100% considered normal speed
    * @param voice      The description of the voice
    * @param language   The description of language in ISO639-2 3-character code
    * @return A Json object containing the settings.
    */
    Json::Value JsonRpcServiceUtil::QuerySettingsScreenReader(bool enabled, int speed, const std::string &voice, const
        std::string &language)
    {
        Json::Value value;
        value["enabled"] = enabled;
        if (speed != OPTIONAL_INT_NOT_SET)
        {
            value["speed"] = std::min(std::max(speed, 10), 1000);
        }
        // The voice value should be present if the enabled value is set to true.
        if (voice == "default" || voice == "female" || voice == "male")
        {
            value[JSONRPC_VOICE] = voice;
        }
        else if (enabled)
        {
            value[JSONRPC_VOICE] = "default";
        }
        if (language != OPTIONAL_STR_NOT_SET)
        {
            value["language"] = language;
        }
        return value;
    }

    /**
    * Query the feature settings of response-to-user-action
    *
    * @param enabled    Enabled a "response to a user action" preference
    * @param type       The description of the mechanism the terminal uses to feedback to the user that the user action has occurred.
    * @return A Json object containing the settings.
    */
    Json::Value JsonRpcServiceUtil::QuerySettingsResponseToUserAction(bool enabled, const std::string &type)
    {
        Json::Value value;
        value["enabled"] = enabled;
        // If the "enabled" value is set to true, then the "type" field shall be present
        std::vector<std::string> types = {"audio", "visual", "haptic", "other", "none"};
        if (count(types.begin(), types.end(), type))
        {
            value["type"] = type;
        }
        else if (enabled)
        {
            value["type"] = "other";
        }
        return value;
    }

    /**
    * Query the feature settings of audio description
    *
    * @param enabled              Enabled audio description
    * @param gainPreference       The audio description gain preference set by the user in dB.
    * @param panAzimuthPreference The degree of the azimuth pan preference set by the user
    * @return A Json object containing the settings.
    */
    Json::Value JsonRpcServiceUtil::QuerySettingsAudioDescription(bool enabled, int gainPreference, int
        panAzimuthPreference)
    {
        Json::Value value;
        value["enabled"] = enabled;
        // If Audio Description is not enabled, this field shall not be present.
        if (!enabled)
        {
            return value;
        }
        if (gainPreference != OPTIONAL_INT_NOT_SET)
        {
            value["gainPreference"] = gainPreference;
        }
        if (panAzimuthPreference != OPTIONAL_INT_NOT_SET)
        {
            value["panAzimuthPreference"] = std::min(std::max(panAzimuthPreference, -180), 180);
        }
        return value;
    }

    /**
    * Convert a std::unordered_set of methods strings into a JSON array.
    *
    * @param set The unordered set of methods strings to convert to a JSON array.
    * @return A Json::Value containing the converted JSON array.
    */
    Json::Value JsonRpcServiceUtil::GetMethodsInJsonArray(const std::unordered_set<std::string>& set)
    {
        Json::Value value;
        int index = 0;
        // Loop through the unordered_set 'set' and copy each string item into 'value' as a Json::arrayValue
        for (const std::string& item : set)
        {
            // Add each 'item' as a Json::Value to 'value' at the current 'index'
            value[index++] = Json::Value(item);
        }
        return value;
    }

    /**
    * Check if a given 'method' string exists within a JSON array.
    *
    * @param array A JSON array to search for the 'method' string.
    * @param method The string to search for within the JSON array.
    * @return 'true' if the 'method' string is found within the JSON array, 'false' otherwise.
    */
    bool JsonRpcServiceUtil::IsMethodInJsonArray(const Json::Value& array, const std::string& method)
    {
        if (array.type() == Json::arrayValue)
        {
            for (const auto& element : array)
            {
                if (element.asString() == method)
                {
                    return true;
                }
            }
        }
        return false;
    }

    /**
    * Check if a given 'method' string exists within an unordered set.
    *
    * @param set An unordered set to search for the 'method' string.
    * @param method The string to search for within the unordered set.
    * @return 'true' if the 'method' string is found within the unordered set, 'false' otherwise.
    */
    bool JsonRpcServiceUtil::IsMethodInSet(const std::unordered_set<std::string> &set, const std::string& method)
    {
        return set.find(method) != set.end();
    }

    /**
    * Check if a JSON object has a specified parameter with a certain data type.
    *
    * @param json The JSON object to check for the presence of the parameter.
    * @param param The name of the parameter to search for within the JSON object.
    * @param type The expected data type of the parameter.
    * @return 'true' if the parameter 'param' exists within the JSON object
    *          and has the specified data type, 'false' otherwise.
    */
    bool JsonRpcServiceUtil::HasParam(const Json::Value &json, const std::string &param, const Json::ValueType& type)
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
    bool JsonRpcServiceUtil::HasJsonParam(const Json::Value &json, const std::string &param)
    {
        return json.isMember(param) && json[param].isObject();
    }

    /**
    * Encode a JSON value to a string representation.
    *
    * @param id The JSON value to encode into a string.
    * @return A string representation of the 'id'.
    */
    std::string JsonRpcServiceUtil::EncodeJsonId(const Json::Value& id)
    {
        //Avoid floating problem
        if (id.type() == Json::realValue)
        {
            std::ostringstream oss;
            oss << std::noshowpoint << id.asDouble();
            return oss.str();
        }
        Json::StreamWriterBuilder writerBuilder;
        writerBuilder["indentation"] = OPTIONAL_STR_NOT_SET;
        return Json::writeString(writerBuilder, id);
    }

    /**
    * Decode a string representation to a JSON value.
    *
    * @param The JSON string to decode into a JSON value.
    * @return A JSON value decoded from the input.
    */
    Json::Value JsonRpcServiceUtil::DecodeJsonId(const std::string& id)
    {
        Json::CharReaderBuilder builder;
        Json::Value jsonId;
        std::string errs;
        std::istringstream ss(id);
        if (!Json::parseFromStream(builder, ss, &jsonId, &errs))
        {
            return Json::nullValue;
        }
        return jsonId;
    }

    /**
    * Create a JSON request for querying feature settings.
    *
    * @param feature The name of the feature.
    * @param value The Json value of parameters associated with the feature query.
    * @return A JSON request containing full information for feature setting query.
    */
    Json::Value JsonRpcServiceUtil::CreateFeatureSettingsQuery(const std::string& feature, const Json::Value& value)
    {
        Json::Value result;
        result[JSONRPC_METHOD_KEY] = MD_AF_FEATURE_SETTINGS_QUERY;
        result[JSONRPC_FEATURE_KEY] = feature;
        result[JSONRPC_VALUE_KEY] = value;
        return result;
    }

    /**
    * Create a JSON request for notify message.
    *
    * @param params The Json value of parameters associated with the notify message.
    * @return A JSON request containing full information for notify message.
    */
    Json::Value JsonRpcServiceUtil::CreateNotifyRequest(const Json::Value& params)
    {
        Json::Value jsonResponse;
        jsonResponse[JSONRPC_VERSION_KEY] = JSONRPC_VERSION;
        jsonResponse[JSONRPC_METHOD_KEY] = MD_NOTIFY;
        jsonResponse[JSONRPC_PARAMS_KEY] = params;
        return jsonResponse;
    }

    /**
    * Create a client JSON response.
    *
    * @param id The id for the request.
    * @param method The method associated with the request.
    * @param params The parameters associated with the request.
    * @return A JSON client request object with the specified id, method, and parameters.
    */
    Json::Value JsonRpcServiceUtil::CreateClientRequest(const std::string& id, const std::string& method, const
        Json::Value& params)
    {
        Json::Value jsonClientResquest;
        jsonClientResquest[JSONRPC_VERSION_KEY] = JSONRPC_VERSION;
        jsonClientResquest[JSONRPC_ID_KEY] = DecodeJsonId(id);
        jsonClientResquest[JSONRPC_PARAMS_KEY] = params;
        jsonClientResquest[JSONRPC_METHOD_KEY] = method;
        return jsonClientResquest;
    }

    /**
    * Create a JSON response with a specific id and result data.
    * This is a general case.
    *
    * @param id The id for the JSON response.
    * @param result The result data to include in the response.
    * @return A JSON response object with the specified id and result.
    *
    */
    Json::Value JsonRpcServiceUtil::CreateJsonResponse(const std::string& id, const Json::Value& result)
    {
        Json::Value jsonResponse;
        jsonResponse[JSONRPC_VERSION_KEY] = JSONRPC_VERSION;
        jsonResponse[JSONRPC_ID_KEY] = DecodeJsonId(id);
        jsonResponse[JSONRPC_RESULT_KEY] = result;
        return jsonResponse;
    }

    /**
    * Create a JSON error response with a specific id and error information.
    *
    * @param id The unique identifier for the JSON error response.
    * @param error The error information or details to include in the response.
    * @return A JSON error response object with the id and error details.
    */
    Json::Value JsonRpcServiceUtil::CreateJsonErrorResponse(const std::string& id, const Json::Value& error)
    {
        Json::Value jsonResponse;
        jsonResponse[JSONRPC_VERSION_KEY] = JSONRPC_VERSION;
        jsonResponse[JSONRPC_ID_KEY] = DecodeJsonId(id);
        jsonResponse[JSONRPC_ERROR_KEY] = error;
        return jsonResponse;
    }

    /**
    * Takes a JsonRpcStatus as input and returns a descriptive error message that
    * corresponds to the provided status. It includes cases for common JSON-RPC errors
    * such as "Method not found," "Parse Error," "Invalid params,", "Invalid request."
    * or "Unknown."
    *
    * @param status The JsonRpcStatus enumeration indicating the error condition.
    * @return A human-readable error message corresponding to the provided status.
    */
    std::string JsonRpcServiceUtil::GetErrorMessage(JsonRpcService::JsonRpcStatus status)
    {
        std::string message;
        switch (status)
        {
            case JsonRpcService::JsonRpcStatus::METHOD_NOT_FOUND:
                message = "Method not found";
                break;
            case JsonRpcService::JsonRpcStatus::PARSE_ERROR:
                message = "Parse Error";
                break;
            case JsonRpcService::JsonRpcStatus::INVALID_PARAMS:
                message = "Invalid params";
                break;
            case JsonRpcService::JsonRpcStatus::INVALID_REQUEST:
                message = "Invalid request";
                break;
            default:
                message = "Unknown";
        }
        LOGE("Error, " + message);
        return message;
    }

    /**
    * Get the name of an accessibility feature.
    *
    * @param id The feature ID.
    * @return The name, or "" if the feature is unknown.
    */
    std::string JsonRpcServiceUtil::GetAccessibilityFeatureName(int id)
    {
        auto it = ACCESSIBILITY_FEATURE_NAMES.find(id);
        if (it != ACCESSIBILITY_FEATURE_NAMES.end())
        {
            return it->second;
        }
        return OPTIONAL_STR_NOT_SET;
    }

    /**
    * Get the ID of an accessibility feature.
    *
    * @param name The feature name.
    * @return The ID, or -1 if the feature is unknown.
    */
    int JsonRpcServiceUtil::GetAccessibilityFeatureId(const std::string& name)
    {
        auto it = ACCESSIBILITY_FEATURE_IDS.find(name);
        if (it != ACCESSIBILITY_FEATURE_IDS.end())
        {
            return it->second;
        }
        return -1;
    }

    /**
    * Convert wall-clock time to seconds
    *
    * @param input time in ISO8601 format
    * @return seconds with time_t
    */
    std::time_t JsonRpcServiceUtil::ConvertISO8601ToSecond(const std::string& input)
    {
        if (input.empty())
        {
            return -1;
        }
        int zh = 0;
        int zm = 0;
        int len = input.length();
        int lenDateTime = len - strlen("+00:00");
        lenDateTime = (lenDateTime < 0) ? 0 : lenDateTime;
        if (input[lenDateTime] == '+' || input[lenDateTime] == '-')
        {
            sscanf(input.substr(lenDateTime, len).c_str(), "%d:%d", &zh, &zm);
            if (zh < -23 || zh > 23 || zm < 0 || zm > 59)
            {
                return -1;
            }
            if (zh < 0)
            {
                zm = -zm;
            }
        }
        const char *inputBuff = input.c_str();
        struct tm time;
        strptime(inputBuff, "%FT%TZ", &time);
        time.tm_isdst = 0;
        time.tm_hour -= zh;
        time.tm_min -= zm;
        time_t t = mktime(&time);
        return t;
    }

    /**
    * Convert seconds to wall-clock time
    *
    * @param sec time in seconds
    * @return time in ISO8601 format
    */
    std::string JsonRpcServiceUtil::ConvertSecondToISO8601(const int sec)
    {
        std::time_t time_sec = static_cast<std::time_t>(sec);
        struct tm* p_tm = std::gmtime(&time_sec);
        if (p_tm != nullptr) 
        {
            std::ostringstream oss;
            oss << std::put_time(p_tm, "%Y-%m-%dT%H:%M:%SZ");
            return oss.str();         
        }
       return "";
    }

    /**
    * Get the id from a JSON object.
    * @param obj The JSON object to extract the id from.
    * @return The id as a string, or an empty string if the id is not present or invalid.
    */
    std::string JsonRpcServiceUtil::GetId(const Json::Value& obj) 
    {
      if (!HasParam(obj, JSONRPC_ID_KEY, Json::stringValue) &&
          !HasParam(obj, JSONRPC_ID_KEY, Json::intValue) &&
          !HasParam(obj, JSONRPC_ID_KEY, Json::uintValue))
        {
            return "";
        }
        return EncodeJsonId(obj[JSONRPC_ID_KEY]);
    }

    /**
    * Get the accessibility feature ID from a JSON object.
    * @param obj The JSON object containing the feature information.
    * @return The feature ID as an integer, or -1 if the feature is not specified or invalid.
    */
    int JsonRpcServiceUtil::GetAccessibilityFeatureId(const Json::Value& obj) {
        if (!HasJsonParam(obj, JSONRPC_PARAMS_KEY))
        {
            return -1;
        }
        if (!HasParam(obj[JSONRPC_PARAMS_KEY], JSONRPC_FEATURE_KEY, Json::stringValue))
        {
            return -1;
        }

        std::string feature = obj[JSONRPC_PARAMS_KEY][JSONRPC_FEATURE_KEY].asString();
        return GetAccessibilityFeatureId(feature);
    }

    /**
     * Add an array of strings to a JSON object under a specified key.
     * @param json The JSON object to which the array will be added.
     * @param key The key under which the array will be stored in the JSON object.
     * @param array The vector of int to be added as a JSON array.
     */
     void JsonRpcServiceUtil::AddArrayToJson(Json::Value &json, const std::string &key, const std::vector<int> &array)
     {
        Json::Value jsonArray(Json::arrayValue);
        for (const auto& item : array)
        {
            jsonArray.append(item);
        }
        json[key] = jsonArray;
    }

    /**
     * Get a string value from a JSON object by key.
     * If the key does not exist or is not a string, returns OPTIONAL_STR_NOT_SET.
     * @param json The JSON object to search.
     * @param key The key to look for in the JSON object.
     * @return The string value associated with the key, or OPTIONAL_STR_NOT_SET if not found.
     */
    std::string JsonRpcServiceUtil::GetStringValueFromJson(const Json::Value &json, const std::string &key) {
        if (HasParam(json, key, Json::stringValue)) {
            return json[key].asString();
        }
        return OPTIONAL_STR_NOT_SET;
    }

    /**
     * Get an integer value from a JSON object by key.
     * If the key does not exist or is not an integer, returns OPTIONAL_INT_NOT_SET.
     * @param json The JSON object to search.
     * @param key The key to look for in the JSON object.
     * @return The integer value associated with the key, or OPTIONAL_INT_NOT_SET if not found.
     */
    int JsonRpcServiceUtil::GetIntValueFromJson(const Json::Value &json, const std::string &key)
    {
        if (HasParam(json, key, Json::intValue)) {
            return json[key].asInt();
        }
        return OPTIONAL_INT_NOT_SET;
    }

    /**
     * Get a boolean value from a JSON object by key.
     * If the key does not exist or is not a boolean, returns false.
     * @param json The JSON object to search.
     * @param key The key to look for in the JSON object.
     * @return The boolean value associated with the key, or false if not found.
     */
    bool JsonRpcServiceUtil::GetBoolValueFromJson(const Json::Value &json, const std::string &key)
    {
        if (HasParam(json, key, Json::booleanValue)) {
            return json[key].asBool();
        }
        return false;
    }
} // namespace networkServices
} // namespace orb