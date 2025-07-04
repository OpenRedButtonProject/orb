
/**
 * ORB Software. Copyright (c) 2025 Ocean Blue Software Limited
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

#ifndef OBS_NS_JSON_RPC_SERVICE_UTIL_H
#define OBS_NS_JSON_RPC_SERVICE_UTIL_H


#include <string>
#include <unordered_map>
#include <unordered_set>
#include <json/json.h>
#include "JsonRpcService.h"

/**
* ORB JsonRpcService Utility static Functions
*/
namespace orb
{
namespace networkServices { 
// Constants

    const std::string F_SUBTITLES = "subtitles";
    const std::string F_DIALOGUE_ENHANCEMENT = "dialogueEnhancement";
    const std::string F_UI_MAGNIFIER = "uiMagnifier";
    const std::string F_HIGH_CONTRAST_UI = "highContrastUI";
    const std::string F_SCREEN_READER = "screenReader";
    const std::string F_RESPONSE_TO_USER_ACTION = "responseToUserAction";
    const std::string F_AUDIO_DESCRIPTION = "audioDescription";
    const std::string F_IN_VISION_SIGNING = "inVisionSigning";

    const std::string PC_SUBTITLES = "subtitlesPrefChange";
    const std::string PC_DIALOGUE_ENHANCEMENT = "dialogueEnhancementPrefChange";
    const std::string PC_UI_MAGNIFIER = "uiMagnifierPrefChange";
    const std::string PC_HIGH_CONTRAST_UI = "highContrastUIPrefChange";
    const std::string PC_SCREEN_READER = "screenReaderPrefChange";
    const std::string PC_RESPONSE_TO_USER_ACTION = "responseToUserActionPrefChange";
    const std::string PC_AUDIO_DESCRIPTION = "audioDescriptionPrefChange";
    const std::string PC_IN_VISION_SIGNING = "inVisionSigningPrefChange";

    const std::string MD_NEGOTIATE_METHODS = "org.hbbtv.negotiateMethods";
    const std::string MD_SUBSCRIBE = "org.hbbtv.subscribe";
    const std::string MD_UNSUBSCRIBE = "org.hbbtv.unsubscribe";
    const std::string MD_NOTIFY = "org.hbbtv.notify";

    const std::string MD_AF_FEATURE_SUPPORT_INFO = "org.hbbtv.af.featureSupportInfo";
    const std::string MD_AF_FEATURE_SETTINGS_QUERY = "org.hbbtv.af.featureSettingsQuery";
    const std::string MD_AF_FEATURE_SUPPRESS = "org.hbbtv.af.featureSuppress";

    const std::string MD_AF_DIALOGUE_ENHANCEMENT_OVERRIDE = "org.hbbtv.af.dialogueEnhancementOverride";
    const std::string MD_AF_TRIGGER_RESPONSE_TO_USER_ACTION = "org.hbbtv.af.triggerResponseToUserAction";

    const std::string MD_VOICE_READY = "org.hbbtv.app.voice.ready";
    const std::string MD_STATE_MEDIA = "org.hbbtv.app.state.media";

    const std::string MD_INTENT_MEDIA_PAUSE = "org.hbbtv.app.intent.media.pause";
    const std::string MD_INTENT_MEDIA_PLAY = "org.hbbtv.app.intent.media.play";
    const std::string MD_INTENT_MEDIA_FAST_FORWARD = "org.hbbtv.app.intent.media.fast-forward";
    const std::string MD_INTENT_MEDIA_FAST_REVERSE = "org.hbbtv.app.intent.media.fast-reverse";
    const std::string MD_INTENT_MEDIA_STOP = "org.hbbtv.app.intent.media.stop";
    const std::string MD_INTENT_MEDIA_SEEK_CONTENT = "org.hbbtv.app.intent.media.seek-content";
    const std::string MD_INTENT_MEDIA_SEEK_RELATIVE = "org.hbbtv.app.intent.media.seek-relative";
    const std::string MD_INTENT_MEDIA_SEEK_LIVE = "org.hbbtv.app.intent.media.seek-live";
    const std::string MD_INTENT_MEDIA_SEEK_WALLCLOCK = "org.hbbtv.app.intent.media.seek-wallclock";
    const std::string MD_INTENT_SEARCH = "org.hbbtv.app.intent.search";
    const std::string MD_INTENT_DISPLAY = "org.hbbtv.app.intent.display";
    const std::string MD_INTENT_PLAYBACK = "org.hbbtv.app.intent.playback";

    // OPAPP Video Window ==> TERMINAL
    const std::string MD_IPPLAYBACK_STATUS_UPDATE = "org.hbbtv.ipplayback.statusUpdate";
    const std::string MD_IPPLAYBACK_MEDIA_POSITION_UPDATE = "org.hbbtv.ipplayback.mediaPositionUpdate";
    const std::string MD_IPPLAYBACK_SET_COMPONENTS = "org.hbbtv.ipplayback.setComponents";
    const std::string MD_IPPLAYBACK_SET_TIMELINE_MAPPING = "org.hbbtv.ipplayback.setTimelineMapping";
    const std::string MD_IPPLAYBACK_SET_PRESENT_FOLLOWING = "org.hbbtv.ipplayback.setPresentFollowing";

    // TERMINAL ==> OPAPP Video Window
    const std::string MD_IPPLAYER_SELECT_CHANNEL = "org.hbbtv.ipplayer.selectChannel";
    const std::string MD_IPPLAYER_STOP = "org.hbbtv.ipplayer.stop";
    const std::string MD_IPPLAYER_PLAY = "org.hbbtv.ipplayer.play";
    const std::string MD_IPPLAYER_SET_VIDEO_WINDOW = "org.hbbtv.ipplayer.setVideoWindow";
    const std::string MD_IPPLAYER_SET_RELATIVE_VOLUME = "org.hbbtv.ipplayer.setRelativeVolume";
    const std::string MD_IPPLAYER_PAUSE = "org.hbbtv.ipplayer.pause";
    const std::string MD_IPPLAYER_RESUME = "org.hbbtv.ipplayer.resume";
    const std::string MD_IPPLAYER_SEEK = "org.hbbtv.ipplayer.seek";
    const std::string MD_IPPLAYER_SELECT_COMPONENTS = "org.hbbtv.ipplayer.selectComponents";
    const std::string MD_IPPLAYER_RESOLVE_TIMELINE = "org.hbbtv.ipplayer.resolveTimeline";

    constexpr int OPTIONAL_INT_NOT_SET = -999999;
    const std::string OPTIONAL_STR_NOT_SET = "";

    const std::string JSONRPC_VERSION_KEY = "jsonrpc";
    const std::string JSONRPC_ID_KEY = "id";
    const std::string JSONRPC_METHOD_KEY = "method";
    const std::string JSONRPC_PARAMS_KEY = "params";
    const std::string JSONRPC_RESULT_KEY = "result";
    const std::string JSONRPC_ERROR_KEY = "error";
    const std::string JSONRPC_SESSION_ID_KEY = "sessionID";
    const std::string JSONRPC_MSG_TYPE_KEY = "msgType";
    const std::string JSONRPC_VALUE_KEY = "value";
    const std::string JSONRPC_FEATURE_KEY = "feature";    
    const std::string JSONRPC_ORIGIN_KEY ="origin";
    const std::string JSONRPC_ANCHOR_KEY = "anchor";
    const std::string JSONRPC_STATE_KEY = "state";
    const std::string JSONRPC_AVAILABLE_KEY = "available";
    const std::string JSONRPC_ENABLED_KEY = "enabled";
    const std::string JSONRPC_ACCESSIBILITY_KEY = "accessibility";
    const std::string JSONRPC_OFFSET_KEY = "offset";
    const std::string JSONRPC_CHANNEL_TYPE_KEY = "channelType";
    const std::string JSONRPC_ID_TYPE_KEY = "idType";
    const std::string JSONRPC_IP_BROADCAST_ID_KEY = "ipBroadcastID";
    const std::string JSONRPC_VOLUME_KEY = "volume";

    const std::string JSONRPC_VIDEO_COMPONENTS_KEY = "videoComponents";
    const std::string JSONRPC_AUDIO_COMPONENTS_KEY = "audioComponents";
    const std::string JSONRPC_SUBTITLE_COMPONENTS_KEY = "subtitleComponents";


    const std::string JSONRPC_TERMINAL_TO_APP_KEY = "terminalToApp";
    const std::string JSONRPC_APP_TO_TERMINAL_KEY = "appToTerminal";

    const std::string JSONRPC_VERSION = "2.0";
    const std::string JSONRPC_VOICE = "voice";

    const std::string PLAYER_STATE_NO_MEDIA = "no-media";
    const std::string PLAYER_STATE_ERROR = "error";
    const std::string PLAYER_STATE_BUFFERING = "buffering";
    const std::string PLAYER_STATE_PAUSED = "paused";
    const std::string PLAYER_STATE_PLAYING = "playing";
    const std::string PLAYER_STATE_STOPPED = "stopped";
    

    const static std::map<std::string, int> ACCESSIBILITY_FEATURE_IDS = {
        {F_SUBTITLES, 0},
        {F_DIALOGUE_ENHANCEMENT, 1},
        {F_UI_MAGNIFIER, 2},
        {F_HIGH_CONTRAST_UI, 3},
        {F_SCREEN_READER, 4},
        {F_RESPONSE_TO_USER_ACTION, 5},
        {F_AUDIO_DESCRIPTION, 6},
        {F_IN_VISION_SIGNING, 7},
    };

    const static std::map<int, std::string> ACCESSIBILITY_FEATURE_NAMES = {
        {0, F_SUBTITLES},
        {1, F_DIALOGUE_ENHANCEMENT},
        {2, F_UI_MAGNIFIER},
        {3, F_HIGH_CONTRAST_UI},
        {4, F_SCREEN_READER},
        {5, F_RESPONSE_TO_USER_ACTION},
        {6, F_AUDIO_DESCRIPTION},
        {7, F_IN_VISION_SIGNING},
    }; 
   class JsonRpcServiceUtil {
    public: 
        static Json::Value QuerySettingsSubtitles(bool enabled, int size, const std::string &fontFamily,
            const std::string &textColour, int textOpacity, const std::string &edgeType, const std::string
            &edgeColour, const std::string &backgroundColour, int backgroundOpacity, const std::string
            &windowColour, int windowOpacity, const std::string &language);

        static Json::Value QuerySettingsDialogueEnhancement(int dialogueEnhancementGainPreference,
            int dialogueEnhancementGain, int dialogueEnhancementLimitMin, int dialogueEnhancementLimitMax);

        static Json::Value QuerySettingsUIMagnifier(bool enabled, const std::string &magType);

        static Json::Value QuerySettingsHighContrastUI(bool enabled, const std::string &hcType);

        static Json::Value QuerySettingsScreenReader(bool enabled, int speed, const std::string &voice,
            const std::string &language);

        static Json::Value QuerySettingsResponseToUserAction(bool enabled, const std::string &type);

        static Json::Value QuerySettingsAudioDescription(bool enabled, int gainPreference, int
            panAzimuthPreference);

        static Json::Value GetMethodsInJsonArray(const std::unordered_set<std::string>& set);

        static bool IsMethodInSet(const std::unordered_set<std::string> &set, const std::string& method);

        static bool IsMethodInJsonArray(const Json::Value& array, const std::string& method);

        static bool HasParam(const Json::Value &json, const std::string &param, const
            Json::ValueType& type);

        static bool HasJsonParam(const Json::Value &json, const std::string &param);

        static std::string EncodeJsonId(const Json::Value& id);

        static Json::Value DecodeJsonId(const std::string& id);

        static Json::Value CreateFeatureSettingsQuery(const std::string& feature, const Json::Value& value);

        static Json::Value CreateNotifyRequest(const Json::Value& params);

        static Json::Value CreateClientRequest(const std::string& id, const std::string& method, const
            Json::Value& params);

        static Json::Value CreateJsonResponse(const std::string& id, const Json::Value& result);

        static Json::Value CreateJsonErrorResponse(const std::string& id, const Json::Value& error);

        static std::string GetErrorMessage(JsonRpcService::JsonRpcStatus status);

        static std::string GetAccessibilityFeatureName(int id);

        static int GetAccessibilityFeatureId(const std::string& name);

        static std::time_t ConvertISO8601ToSecond(const std::string& input);

        static std::string ConvertSecondToISO8601(const int sec); 

        static std::string GetId(const Json::Value& obj); 

        static int GetAccessibilityFeatureId(const Json::Value& obj);

        static void AddArrayToJson(Json::Value &json, const std::string &key, const std::vector<int> &array);

        static std::string GetStringValueFromJson(const Json::Value &json, const std::string &key);

        static int GetIntValueFromJson(const Json::Value &json, const std::string &key);

        static bool GetBoolValueFromJson(const Json::Value &json, const std::string &key);
        
    };
} // namespace networkServices
} // namespace orb

#endif // OBS_NS_JSON_RPC_SERVICE_UTIL_H
