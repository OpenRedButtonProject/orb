/**
 * ORB Software. Copyright (c) 2023 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

#include "JsonRpcService.h"
#include "log.h"

#include <iostream>
#include <sstream>

// Constants
#define OPTIONAL_INT_NOT_SET -999999
#define OPTIONAL_STR_NOT_SET ""

#define F_SUBTITLES "subtitles"
#define F_DIALOGUE_ENHANCEMENT "dialogueEnhancement"
#define F_UI_MAGNIFIER "uiMagnifier"
#define F_HIGH_CONTRAST_UI "highContrastUI"
#define F_SCREEN_READER "screenReader"
#define F_RESPONSE_TO_USER_ACTION "responseToUserAction"
#define F_AUDIO_DESCRIPTION "audioDescription"
#define F_IN_VISION_SIGNING "inVisionSigning"

#define PC_SUBTITLES "subtitlesPrefChange"
#define PC_DIALOGUE_ENHANCEMENT "dialogueEnhancementPrefChange"
#define PC_UI_MAGNIFIER "uiMagnifierPrefChange"
#define PC_HIGH_CONTRAST_UI "highContrastUIPrefChange"
#define PC_SCREEN_READER "screenReaderPrefChange"
#define PC_RESPONSE_TO_USER_ACTION "responseToUserActionPrefChange"
#define PC_AUDIO_DESCRIPTION "audioDescriptionPrefChange"
#define PC_IN_VISION_SIGNING "inVisionSigningPrefChange"

#define MD_NEGOTIATE_METHODS "org.hbbtv.negotiateMethods"
#define MD_SUBSCRIBE "org.hbbtv.subscribe"
#define MD_UNSUBSCRIBE "org.hbbtv.unsubscribe"
#define MD_NOTIFY "org.hbbtv.notify"

#define MD_AF_FEATURE_SUPPORT_INFO "org.hbbtv.af.featureSupportInfo"
#define MD_AF_FEATURE_SETTINGS_QUERY "org.hbbtv.af.featureSettingsQuery"
#define MD_AF_FEATURE_SUPPRESS "org.hbbtv.af.featureSuppress"

#define MD_AF_DIALOGUE_ENHANCEMENT_OVERRIDE "org.hbbtv.af.dialogueEnhancementOverride"
#define MD_AF_TRIGGER_RESPONSE_TO_USER_ACTION "org.hbbtv.af.triggerResponseToUserAction"

#define MD_VOICE_READY "org.hbbtv.app.voice.ready"
#define MD_STATE_MEDIA "org.hbbtv.app.state.media"

#define MD_INTENT_MEDIA_PAUSE "org.hbbtv.app.intent.media.pause"
#define MD_INTENT_MEDIA_PLAY "org.hbbtv.app.intent.media.play"
#define MD_INTENT_MEDIA_FAST_FORWARD "org.hbbtv.app.intent.media.fast-forward"
#define MD_INTENT_MEDIA_FAST_REVERSE "org.hbbtv.app.intent.media.fast-reverse"
#define MD_INTENT_MEDIA_STOP "org.hbbtv.app.intent.media.stop"
#define MD_INTENT_MEDIA_SEEK_CONTENT "org.hbbtv.app.intent.media.seek-content"
#define MD_INTENT_MEDIA_SEEK_RELATIVE "org.hbbtv.app.intent.media.seek-relative"
#define MD_INTENT_MEDIA_SEEK_LIVE "org.hbbtv.app.intent.media.seek-live"
#define MD_INTENT_MEDIA_SEEK_WALLCLOCK "org.hbbtv.app.intent.media.seek-wallclock"
#define MD_INTENT_SEARCH "org.hbbtv.app.intent.search"
#define MD_INTENT_DISPLAY "org.hbbtv.app.intent.display"
#define MD_INTENT_PLAYBACK "org.hbbtv.app.intent.playback"

namespace NetworkServices {
const int sizeOfAccessibilityFeature = 8;
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

static Json::Value CreateIntentResponse(const std::string& id, const std::string& method, const
    Json::Value& params);

static Json::Value CreateJsonResponse(const std::string& id, const Json::Value& result);

static Json::Value CreateJsonErrorResponse(const std::string& id, const Json::Value& error);

static std::string GetErrorMessage(JsonRpcService::JsonRpcStatus status);

static std::string GetAccessibilityFeatureName(int id);

static int GetAccessibilityFeatureId(const std::string& name);

static std::time_t ConvertISO8601ToSecond(const std::string& input);

static std::string ConvertSecondToISO8601(const int sec);

JsonRpcService::JsonRpcService(
    int port,
    const std::string &endpoint,
    std::unique_ptr<SessionCallback> sessionCallback) :
    WebSocketService("JsonRpcService", port, false, "lo"),
    m_endpoint(endpoint),
    m_sessionCallback(std::move(sessionCallback))
{
    RegisterMethod(MD_NEGOTIATE_METHODS, &JsonRpcService::RequestNegotiateMethods);
    RegisterMethod(MD_SUBSCRIBE, &JsonRpcService::RequestSubscribe);
    RegisterMethod(MD_UNSUBSCRIBE, &JsonRpcService::RequestUnsubscribe);

    RegisterMethod(MD_AF_FEATURE_SUPPORT_INFO, &JsonRpcService::RequestFeatureSupportInfo);
    RegisterMethod(MD_AF_FEATURE_SETTINGS_QUERY, &JsonRpcService::RequestFeatureSettingsQuery);
    RegisterMethod(MD_AF_FEATURE_SUPPRESS, &JsonRpcService::RequestFeatureSuppress);

    RegisterMethod(MD_AF_DIALOGUE_ENHANCEMENT_OVERRIDE,
        &JsonRpcService::RequestDialogueEnhancementOverride);
    RegisterMethod(MD_AF_TRIGGER_RESPONSE_TO_USER_ACTION,
        &JsonRpcService::RequestTriggerResponseToUserAction);

    RegisterMethod(MD_VOICE_READY, &JsonRpcService::NotifyVoiceReady);
    RegisterMethod(MD_STATE_MEDIA, &JsonRpcService::NotifyStateMedia);
    RegisterMethod(MD_INTENT_MEDIA_PAUSE, &JsonRpcService::ReceiveIntentConfirm);
    RegisterMethod(MD_INTENT_MEDIA_PLAY, &JsonRpcService::ReceiveIntentConfirm);
    RegisterMethod(MD_INTENT_MEDIA_FAST_FORWARD, &JsonRpcService::ReceiveIntentConfirm);
    RegisterMethod(MD_INTENT_MEDIA_FAST_REVERSE, &JsonRpcService::ReceiveIntentConfirm);
    RegisterMethod(MD_INTENT_MEDIA_STOP, &JsonRpcService::ReceiveIntentConfirm);
    RegisterMethod(MD_INTENT_MEDIA_SEEK_CONTENT, &JsonRpcService::ReceiveIntentConfirm);
    RegisterMethod(MD_INTENT_MEDIA_SEEK_RELATIVE, &JsonRpcService::ReceiveIntentConfirm);
    RegisterMethod(MD_INTENT_MEDIA_SEEK_LIVE, &JsonRpcService::ReceiveIntentConfirm);
    RegisterMethod(MD_INTENT_MEDIA_SEEK_WALLCLOCK, &JsonRpcService::ReceiveIntentConfirm);
    RegisterMethod(MD_INTENT_SEARCH, &JsonRpcService::ReceiveIntentConfirm);
    RegisterMethod(MD_INTENT_DISPLAY, &JsonRpcService::ReceiveIntentConfirm);
    RegisterMethod(MD_INTENT_PLAYBACK, &JsonRpcService::ReceiveIntentConfirm);

    RegisterSupportedMethods();
}

bool JsonRpcService::OnConnection(WebSocketConnection *connection)
{
    if (connection->Uri() != m_endpoint)
    {
        LOG(LOG_INFO, "Unknown endpoint received. Got: %s, expected: %s",
            connection->Uri().c_str(), m_endpoint.c_str());
        return false;
    }
    LOG(LOG_INFO, "Connected: connectionId=%d", connection->Id());
    InitialConnectionData(connection->Id());
    return true;
}

void JsonRpcService::OnMessageReceived(WebSocketConnection *connection, const std::string &text)
{
    LOG(LOG_INFO, "Message received: connection=%d, text=%s", connection->Id(), text.c_str());
    // Parse request
    Json::Value obj;
    Json::Reader reader;
    JsonRpcStatus status = JsonRpcStatus::UNKNOWN;

    if (!reader.parse(text, obj))
    {
        status = JsonRpcStatus::PARSE_ERROR;
    }

    if (status == JsonRpcStatus::UNKNOWN &&
        (!HasParam(obj, "jsonrpc", Json::stringValue) || obj["jsonrpc"] != "2.0"))
    {
        status = JsonRpcStatus::INVALID_REQUEST;
    }

    //case of error message
    if (status == JsonRpcStatus::UNKNOWN && HasJsonParam(obj, "error"))
    {
        status = JsonRpcService::ReceiveError(connection->Id(), obj);
    }

    //case with method
    std::string method;
    if (status == JsonRpcStatus::UNKNOWN)
    {
        if (HasParam(obj, "method", Json::stringValue))
        {
            method = obj["method"].asString();
            Json::Value negotiateMethods = GetConnectionData(connection->Id(),
                ConnectionDataType::NegotiateMethodsAppToTerminal);
            if (!negotiateMethods.isArray())
            {
                LOG(LOG_INFO, "Warning, connection data lost, parameter has wrong type.");
            }
            if (!IsMethodInJsonArray(negotiateMethods, method))
            {
                status = JsonRpcStatus::METHOD_NOT_FOUND;
            }
        }
        else if (HasJsonParam(obj, "result") &&
                 HasParam(obj["result"], "method", Json::stringValue))
        {
            //case with method in result
            method = obj["result"]["method"].asString();
        }
        else
        {
            //cannot find parameter of "method"
            status = JsonRpcStatus::INVALID_REQUEST;
        }
    }

    if (status == JsonRpcStatus::UNKNOWN)
    {
        auto it = m_json_rpc_methods.find(method);
        if (it != m_json_rpc_methods.end())
        {
            status = it->second(connection->Id(), obj);
        }
        else
        {
            status = JsonRpcStatus::METHOD_NOT_FOUND;
        }
    }

    if (status == JsonRpcStatus::NOTIFICATION_ERROR)
    {
        LOG(LOG_INFO, "Error in notification message");
    }

    if (status != JsonRpcStatus::SUCCESS && status != JsonRpcStatus::NOTIFICATION_ERROR)
    {
        int code = static_cast<int>(status);
        std::string message = GetErrorMessage(status);
        std::string id = OPTIONAL_STR_NOT_SET;
        if (status != JsonRpcStatus::PARSE_ERROR &&
            status != JsonRpcStatus::INVALID_REQUEST)
        {
            if (HasParam(obj, "id", Json::stringValue) ||
                HasParam(obj, "id", Json::intValue) ||
                HasParam(obj, "id", Json::uintValue))
            {
                id = EncodeJsonId(obj["id"]);
            }
        }
        RespondError(connection->Id(), id, code, message);
    }
}

void JsonRpcService::OnDisconnected(WebSocketConnection *connection)
{
    connections_mutex_.lock();
    m_connectionData.erase(connection->Id());
    connections_mutex_.unlock();
}

void JsonRpcService::OnServiceStopped()
{
}

void JsonRpcService::RegisterSupportedMethods()
{
    m_supported_methods_app_to_terminal.insert(MD_NEGOTIATE_METHODS);
    m_supported_methods_app_to_terminal.insert(MD_SUBSCRIBE);
    m_supported_methods_app_to_terminal.insert(MD_UNSUBSCRIBE);
    m_supported_methods_app_to_terminal.insert(MD_AF_FEATURE_SUPPORT_INFO);
    m_supported_methods_app_to_terminal.insert(MD_AF_FEATURE_SETTINGS_QUERY);
    m_supported_methods_app_to_terminal.insert(MD_AF_FEATURE_SUPPRESS);
    m_supported_methods_app_to_terminal.insert(MD_AF_DIALOGUE_ENHANCEMENT_OVERRIDE);
    m_supported_methods_app_to_terminal.insert(MD_AF_TRIGGER_RESPONSE_TO_USER_ACTION);
    m_supported_methods_app_to_terminal.insert(MD_VOICE_READY);
    m_supported_methods_app_to_terminal.insert(MD_STATE_MEDIA);

    m_supported_methods_terminal_to_app.insert(MD_NOTIFY);
    m_supported_methods_terminal_to_app.insert(MD_INTENT_MEDIA_PAUSE);
    m_supported_methods_terminal_to_app.insert(MD_INTENT_MEDIA_PLAY);
    m_supported_methods_terminal_to_app.insert(MD_INTENT_MEDIA_FAST_FORWARD);
    m_supported_methods_terminal_to_app.insert(MD_INTENT_MEDIA_FAST_REVERSE);
    m_supported_methods_terminal_to_app.insert(MD_INTENT_MEDIA_STOP);
    m_supported_methods_terminal_to_app.insert(MD_INTENT_MEDIA_SEEK_CONTENT);
    m_supported_methods_terminal_to_app.insert(MD_INTENT_MEDIA_SEEK_RELATIVE);
    m_supported_methods_terminal_to_app.insert(MD_INTENT_MEDIA_SEEK_LIVE);
    m_supported_methods_terminal_to_app.insert(MD_INTENT_MEDIA_SEEK_WALLCLOCK);
    m_supported_methods_terminal_to_app.insert(MD_INTENT_SEARCH);
    m_supported_methods_terminal_to_app.insert(MD_INTENT_DISPLAY);
    m_supported_methods_terminal_to_app.insert(MD_INTENT_PLAYBACK);
}

JsonRpcService::JsonRpcStatus JsonRpcService::RequestNegotiateMethods(int connectionId, const
    Json::Value &obj)
{
    if (!HasParam(obj, "id", Json::stringValue) &&
        !HasParam(obj, "id", Json::intValue) &&
        !HasParam(obj, "id", Json::uintValue))
    {
        return JsonRpcStatus::INVALID_PARAMS;
    }
    std::string id = EncodeJsonId(obj["id"]);

    if (!HasJsonParam(obj, "params"))
    {
        return JsonRpcStatus::INVALID_PARAMS;
    }
    if (!HasParam(obj["params"], "terminalToApp", Json::arrayValue))
    {
        return JsonRpcStatus::INVALID_PARAMS;
    }
    if (!HasParam(obj["params"], "appToTerminal", Json::arrayValue))
    {
        return JsonRpcStatus::INVALID_PARAMS;
    }
    Json::Value terminalToApp = obj["params"]["terminalToApp"];
    Json::Value appToTerminal = obj["params"]["appToTerminal"];

    //add method to set
    Json::Value filteredTerminalToApp = FilterMethods(connectionId, terminalToApp,
        false);
    Json::Value filteredAppToTerminal = FilterMethods(connectionId, appToTerminal,
        true);

    m_sessionCallback->RequestNegotiateMethods();
    RespondNegotiateMethods(connectionId, id,
        filteredTerminalToApp,
        filteredAppToTerminal);
    return JsonRpcStatus::SUCCESS;
}

JsonRpcService::JsonRpcStatus JsonRpcService::RequestSubscribe(int connectionId, const
    Json::Value &obj)
{
    if (!HasParam(obj, "id", Json::stringValue) &&
        !HasParam(obj, "id", Json::intValue) &&
        !HasParam(obj, "id", Json::uintValue))
    {
        return JsonRpcStatus::INVALID_PARAMS;
    }
    std::string id = EncodeJsonId(obj["id"]);

    if (!HasJsonParam(obj, "params"))
    {
        return JsonRpcStatus::INVALID_PARAMS;
    }
    if (!HasParam(obj["params"], "msgType", Json::arrayValue))
    {
        return JsonRpcStatus::INVALID_PARAMS;
    }
    Json::Value msgType = obj["params"]["msgType"];

    std::vector<bool> msgTypeBoolList(sizeOfAccessibilityFeature, false);
    for (const auto& msg: msgType)
    {
        int length = msg.asString().length();
        std::string feature = msg.asString().substr(0, length - 10);     // Remove PrefChange
        int featureId = GetAccessibilityFeatureId(feature);
        if (featureId > -1 && featureId < sizeOfAccessibilityFeature)
        {
            msgTypeBoolList[featureId] = true;
            SetConnectionData(connectionId,
                ConnectionDataType::SubscribedMethods,
                msg.asString());
        }
        else
        {
            return JsonRpcStatus::INVALID_PARAMS;
        }
    }
    m_sessionCallback->RequestSubscribe(msgTypeBoolList[0],
        msgTypeBoolList[1], msgTypeBoolList[2],
        msgTypeBoolList[3], msgTypeBoolList[4],
        msgTypeBoolList[5], msgTypeBoolList[6],
        msgTypeBoolList[7]);
    RespondSubscribe(connectionId, id, msgType);
    return JsonRpcStatus::SUCCESS;
}

JsonRpcService::JsonRpcStatus JsonRpcService::RequestUnsubscribe(int connectionId, const
    Json::Value &obj)
{
    if (!HasParam(obj, "id", Json::stringValue) &&
        !HasParam(obj, "id", Json::intValue) &&
        !HasParam(obj, "id", Json::uintValue))
    {
        return JsonRpcStatus::INVALID_PARAMS;
    }
    std::string id = EncodeJsonId(obj["id"]);

    if (!HasJsonParam(obj, "params"))
    {
        return JsonRpcStatus::INVALID_PARAMS;
    }
    if (!HasParam(obj["params"], "msgType", Json::arrayValue))
    {
        return JsonRpcStatus::INVALID_PARAMS;
    }
    Json::Value msgType = obj["params"]["msgType"];

    std::vector<bool> msgTypeBoolList(sizeOfAccessibilityFeature, false);
    for (const auto& msg: msgType)
    {
        int length = msg.asString().length();
        std::string feature = msg.asString().substr(0, length - 10);     // Remove PrefChange
        int featureId = GetAccessibilityFeatureId(feature);
        if (featureId > -1 && featureId < sizeOfAccessibilityFeature)
        {
            msgTypeBoolList[featureId] = true;
            SetConnectionData(connectionId, ConnectionDataType::UnsubscribedMethods,
                msg.asString());
        }
        else
        {
            return JsonRpcStatus::INVALID_PARAMS;
        }
    }
    m_sessionCallback->RequestUnsubscribe(msgTypeBoolList[0],
        msgTypeBoolList[1], msgTypeBoolList[2],
        msgTypeBoolList[3], msgTypeBoolList[4],
        msgTypeBoolList[5], msgTypeBoolList[6],
        msgTypeBoolList[7]);
    RespondUnsubscribe(connectionId, id, msgType);
    return JsonRpcStatus::SUCCESS;
}

JsonRpcService::JsonRpcStatus JsonRpcService::RequestFeatureSupportInfo(int connectionId, const
    Json::Value &obj)
{
    if (!HasParam(obj, "id", Json::stringValue) &&
        !HasParam(obj, "id", Json::intValue) &&
        !HasParam(obj, "id", Json::uintValue))
    {
        return JsonRpcStatus::INVALID_PARAMS;
    }
    std::string id = EncodeJsonId(obj["id"]);
    if (!HasJsonParam(obj, "params"))
    {
        return JsonRpcStatus::INVALID_PARAMS;
    }
    if (!HasParam(obj["params"], "feature", Json::stringValue))
    {
        return JsonRpcStatus::INVALID_PARAMS;
    }

    std::string feature = obj["params"]["feature"].asString();
    if (GetAccessibilityFeatureId(feature) == -1)
    {
        return JsonRpcStatus::INVALID_PARAMS;
    }
    m_sessionCallback->RequestFeatureSupportInfo(connectionId, id,
        GetAccessibilityFeatureId(feature));
    return JsonRpcStatus::SUCCESS;
}

JsonRpcService::JsonRpcStatus JsonRpcService::RequestFeatureSettingsQuery(int connectionId, const
    Json::Value &obj)
{
    if (!HasParam(obj, "id", Json::stringValue) &&
        !HasParam(obj, "id", Json::intValue) &&
        !HasParam(obj, "id", Json::uintValue))
    {
        return JsonRpcStatus::INVALID_PARAMS;
    }
    std::string id = EncodeJsonId(obj["id"]);
    if (!HasJsonParam(obj, "params"))
    {
        return JsonRpcStatus::INVALID_PARAMS;
    }
    if (!HasParam(obj["params"], "feature", Json::stringValue))
    {
        return JsonRpcStatus::INVALID_PARAMS;
    }

    std::string feature = obj["params"]["feature"].asString();
    if (GetAccessibilityFeatureId(feature) == -1)
    {
        return JsonRpcStatus::INVALID_PARAMS;
    }
    m_sessionCallback->RequestFeatureSettingsQuery(connectionId, id,
        GetAccessibilityFeatureId(feature));
    return JsonRpcStatus::SUCCESS;
}

JsonRpcService::JsonRpcStatus JsonRpcService::RequestFeatureSuppress(int connectionId, const
    Json::Value &obj)
{
    if (!HasParam(obj, "id", Json::stringValue) &&
        !HasParam(obj, "id", Json::intValue) &&
        !HasParam(obj, "id", Json::uintValue))
    {
        return JsonRpcStatus::INVALID_PARAMS;
    }
    std::string id = EncodeJsonId(obj["id"]);
    if (!HasJsonParam(obj, "params"))
    {
        return JsonRpcStatus::INVALID_PARAMS;
    }
    if (!HasParam(obj["params"], "feature", Json::stringValue))
    {
        return JsonRpcStatus::INVALID_PARAMS;
    }

    std::string feature = obj["params"]["feature"].asString();
    if (GetAccessibilityFeatureId(feature) == -1)
    {
        return JsonRpcStatus::INVALID_PARAMS;
    }
    m_sessionCallback->RequestFeatureSuppress(connectionId, id,
        GetAccessibilityFeatureId(feature));
    return JsonRpcStatus::SUCCESS;
}

JsonRpcService::JsonRpcStatus JsonRpcService::RequestDialogueEnhancementOverride(int connectionId,
    const Json::Value &obj)
{
    if (!HasParam(obj, "id", Json::stringValue) &&
        !HasParam(obj, "id", Json::intValue) &&
        !HasParam(obj, "id", Json::uintValue))
    {
        return JsonRpcStatus::INVALID_PARAMS;
    }
    std::string id = EncodeJsonId(obj["id"]);

    int dialogueEnhancementGain = OPTIONAL_INT_NOT_SET;
    if (HasJsonParam(obj, "params"))
    {
        Json::Value params = obj["params"];
        if (HasParam(params, "dialogueEnhancementGain", Json::intValue))
        {
            dialogueEnhancementGain = params["dialogueEnhancementGain"].asInt();
        }
    }
    m_sessionCallback->RequestDialogueEnhancementOverride(connectionId, id,
        dialogueEnhancementGain);
    return JsonRpcStatus::SUCCESS;
}

JsonRpcService::JsonRpcStatus JsonRpcService::RequestTriggerResponseToUserAction(
    int connectionId, const Json::Value &obj)
{
    if (!HasParam(obj, "id", Json::stringValue) &&
        !HasParam(obj, "id", Json::intValue) &&
        !HasParam(obj, "id", Json::uintValue))
    {
        return JsonRpcStatus::INVALID_PARAMS;
    }
    std::string id = EncodeJsonId(obj["id"]);
    if (!HasJsonParam(obj, "params"))
    {
        return JsonRpcStatus::INVALID_PARAMS;
    }
    if (!HasParam(obj["params"], "magnitude", Json::stringValue))
    {
        return JsonRpcStatus::INVALID_PARAMS;
    }
    std::string magnitude = obj["params"]["magnitude"].asString();
    if (magnitude != "triggerPrimary" && magnitude != "triggerSecondary" && magnitude !=
        "triggerException")
    {
        return JsonRpcStatus::INVALID_PARAMS;
    }
    m_sessionCallback->RequestTriggerResponseToUserAction(connectionId, id, magnitude);
    return JsonRpcStatus::SUCCESS;
}

JsonRpcService::JsonRpcStatus JsonRpcService::NotifyVoiceReady(int connectionId, const
    Json::Value &obj)
{
    if (!HasJsonParam(obj, "params") ||
        !HasParam(obj["params"], "ready", Json::booleanValue))
    {
        return JsonRpcStatus::NOTIFICATION_ERROR;
    }
    bool voiceReady = obj["params"]["ready"].asBool();
    SetConnectionData(connectionId, ConnectionDataType::VoiceReady, voiceReady);
    m_sessionCallback->NotifyVoiceReady(voiceReady);
    return JsonRpcStatus::SUCCESS;
}

JsonRpcService::JsonRpcStatus JsonRpcService::NotifyStateMedia(int connectionId, const
    Json::Value &obj)
{
    Json::Value params;
    if (!HasJsonParam(obj, "params"))
    {
        return JsonRpcStatus::NOTIFICATION_ERROR;
    }
    params = obj["params"];

    if (!HasParam(params, "state", Json::stringValue))
    {
        return JsonRpcStatus::NOTIFICATION_ERROR;
    }
    std::string state = params["state"].asString();
    if (state != "no-media" &&
        state != "error" &&
        state != "buffering" &&
        state != "paused" &&
        state != "playing" &&
        state != "stopped")
    {
        return JsonRpcStatus::NOTIFICATION_ERROR;
    }
    std::unique_ptr<ConnectionData> mediaData(new ConnectionData);
    mediaData->state = state;
    if (!HasJsonParam(params, "availableActions"))
    {
        return JsonRpcStatus::NOTIFICATION_ERROR;
    }
    Json::Value actions = params["availableActions"];
    if (HasParam(actions, "pause", Json::booleanValue) &&
        actions["pause"] == true)
    {
        mediaData->actionPause = true;
    }
    if (HasParam(actions, "play", Json::booleanValue) &&
        actions["play"] == true)
    {
        mediaData->actionPlay = true;
    }
    if (HasParam(actions, "fast-forward", Json::booleanValue) &&
        actions["fast-forward"] == true)
    {
        mediaData->actionFastForward = true;
    }
    if (HasParam(actions, "fast-reverse", Json::booleanValue) &&
        actions["fast-reverse"] == true)
    {
        mediaData->actionFastReverse = true;
    }
    if (HasParam(actions, "stop", Json::booleanValue) &&
        actions["stop"] == true)
    {
        mediaData->actionStop = true;
    }
    if (HasParam(actions, "seek-content", Json::booleanValue) &&
        actions["seek-content"] == true)
    {
        mediaData->actionSeekContent = true;
    }
    if (HasParam(actions, "seek-relative", Json::booleanValue) &&
        actions["seek-relative"] == true)
    {
        mediaData->actionSeekRelative = true;
    }
    if (HasParam(actions, "seek-live", Json::booleanValue) &&
        actions["seek-live"] == true)
    {
        mediaData->actionSeekLive = true;
    }
    if (HasParam(actions, "seek-wallclock", Json::booleanValue) &&
        actions["seek-wallclock"] == true)
    {
        mediaData->actionSeekWallclock = true;
    }

    if (state == "no-media" || state == "error")
    {
        SetStateMediaToConnectionData(connectionId, *mediaData);
        m_sessionCallback->NotifyStateMedia(state);
        return JsonRpcStatus::SUCCESS;
    }

    // state is "buffering", "paused", "playing" or "stopped"
    std::string kind;
    if (HasParam(params, "kind", Json::stringValue))
    {
        kind = params["kind"].asString();
    }
    std::string type;
    if (HasParam(params, "type", Json::stringValue))
    {
        type = params["type"].asString();
    }
    if (kind.empty() || (kind != "audio" && kind != "audio-video") ||
        type.empty() || (type != "live" && type != "on-demand"))
    {
        LOG(LOG_INFO, "Invalid settings for either type or kind in notification message");
        return JsonRpcStatus::NOTIFICATION_ERROR;
    }

    if (!HasJsonParam(params, "metadata"))
    {
        return JsonRpcStatus::NOTIFICATION_ERROR;
    }
    Json::Value metadata = params["metadata"];
    if (HasParam(metadata, "mediaId", Json::stringValue))
    {
        mediaData->mediaId = metadata["mediaId"].asString();
    }
    if (HasParam(metadata, "title", Json::stringValue))
    {
        mediaData->title = metadata["title"].asString();
    }
    if (HasParam(metadata, "secondaryTitle", Json::stringValue))
    {
        mediaData->secondTitle = metadata["secondaryTitle"].asString();
    }
    if (HasParam(metadata, "synopsis", Json::stringValue))
    {
        mediaData->synopsis = metadata["synopsis"].asString();
    }
    if (mediaData->title.empty())
    {
        LOG(LOG_INFO, "Invalid value for metadata title in notification message");
        return JsonRpcStatus::NOTIFICATION_ERROR;
    }

    if (state == "stopped")
    {
        SetStateMediaToConnectionData(connectionId, *mediaData);
        m_sessionCallback->NotifyStateMedia(state);
        return JsonRpcStatus::SUCCESS;
    }

    // state is "buffering", "paused" or "playing"
    Json::Value current = params["currentTime"];
    Json::Value range = params["range"];
    if (!params.isMember("currentTime") || !HasJsonParam(params, "range") ||
        !range.isMember("start") || !range.isMember("end"))
    {
        LOG(LOG_INFO, "A required item is missing in notification message");
        return JsonRpcStatus::NOTIFICATION_ERROR;
    }
    Json::Value start = range["start"];
    Json::Value end = range["end"];
    int biasTime = 0;
    int systemTime = std::time(nullptr);
    int currentTime = -1;
    int startTime = -1;
    int endTime = -1;
    bool isCurrentNum = current.type() == Json::intValue || current.type() == Json::realValue;
    bool isStartNum = start.type() == Json::intValue || start.type() == Json::realValue;
    bool isEndNum = end.type() == Json::intValue || end.type() == Json::realValue;
    if (isCurrentNum && isStartNum && isEndNum)
    {
        double curr = params["currentTime"].asDouble();
        double st = systemTime + (start.asDouble() - curr);
        double e = systemTime + (end.asDouble() - curr);
        if (st < INT_MIN || st > INT_MAX || e < INT_MIN || e > INT_MAX)
        {
            LOG(LOG_INFO, "Invalid time and range of media in notification message");
            return JsonRpcStatus::NOTIFICATION_ERROR;
        }
        currentTime = systemTime;
        startTime = (int) st;
        endTime = (int) e;
    }
    else if (current.type() == Json::stringValue && current.type() == Json::stringValue &&
             current.type() == Json::stringValue)
    {
        currentTime = ConvertISO8601ToSecond(current.asString());
        biasTime = currentTime - systemTime;
        startTime = ConvertISO8601ToSecond(start.asString());
        endTime = ConvertISO8601ToSecond(end.asString());
    }
    if (systemTime < 0 || currentTime < 0 || startTime < 0 || endTime < 0 ||
        !(currentTime >= startTime && currentTime <= endTime))
    {
        LOG(LOG_INFO, "Invalid time and range of media in notification message");
        return JsonRpcStatus::NOTIFICATION_ERROR;
    }
    mediaData->biasToSystemTime = biasTime;
    mediaData->startTime = startTime;
    mediaData->endTime = endTime;

    Json::Value subtitles;
    Json::Value audioDescription;
    Json::Value signLanguage;
    bool subtitlesEnabled = false;
    bool subtitlesAvailable = false;
    bool audioDescripEnabled = false;
    bool audioDescripAvailable = false;
    bool signLangEnabled = false;
    bool signLangAvailable = false;

    if (HasJsonParam(params, "accessibility"))
    {
        if (HasJsonParam(params["accessibility"], "subtitles"))
        {
            subtitles = params["accessibility"]["subtitles"];
            if (HasParam(subtitles, "enabled", Json::booleanValue))
            {
                subtitlesEnabled = subtitles["enabled"].asBool();
            }
            if (HasParam(subtitles, "available", Json::booleanValue))
            {
                subtitlesAvailable = subtitles["available"].asBool();
            }
        }

        if (HasJsonParam(params["accessibility"], "audioDescription"))
        {
            audioDescription = params["accessibility"]["audioDescription"];
            if (HasParam(audioDescription, "enabled", Json::booleanValue))
            {
                audioDescripEnabled = audioDescription["enabled"].asBool();
            }
            if (HasParam(audioDescription, "available", Json::booleanValue))
            {
                audioDescripAvailable = audioDescription["available"].asBool();
            }
        }

        if (HasJsonParam(params["accessibility"], "signLanguage"))
        {
            signLanguage = params["accessibility"]["signLanguage"];
            if (HasParam(signLanguage, "enabled", Json::booleanValue))
            {
                signLangEnabled = signLanguage["enabled"].asBool();
            }
            if (HasParam(signLanguage, "available", Json::booleanValue))
            {
                signLangAvailable = signLanguage["available"].asBool();
            }
        }
    }

    if (state == "buffering" || state == "paused" || state == "playing")
    {
        if (!HasParam(subtitles, "enabled", Json::booleanValue) ||
            !HasParam(subtitles, "available", Json::booleanValue) ||
            !HasParam(audioDescription, "enabled", Json::booleanValue) ||
            !HasParam(audioDescription, "available", Json::booleanValue) ||
            !HasParam(signLanguage, "enabled", Json::booleanValue) ||
            !HasParam(signLanguage, "available", Json::booleanValue))
        {
            return JsonRpcStatus::NOTIFICATION_ERROR;
        }
    }
    SetStateMediaToConnectionData(connectionId, *mediaData);
    m_sessionCallback->NotifyStateMedia(state);
    return JsonRpcStatus::SUCCESS;
}

JsonRpcService::JsonRpcStatus JsonRpcService::ReceiveIntentConfirm(int connectionId, const
    Json::Value &obj)
{
    Json::Value result = obj["result"];
    if (!HasParam(obj, "id", Json::stringValue) &&
        !HasParam(obj, "id", Json::intValue) &&
        !HasParam(obj, "id", Json::uintValue))
    {
        return JsonRpcStatus::INVALID_PARAMS;
    }
    std::string id = EncodeJsonId(obj["id"]);

    if (!HasParam(result, "method", Json::stringValue))
    {
        return JsonRpcStatus::INVALID_PARAMS;
    }
    std::string method = result["method"].asString();

    m_sessionCallback->ReceiveIntentConfirm(connectionId, id, method);
    return JsonRpcStatus::SUCCESS;
}

JsonRpcService::JsonRpcStatus JsonRpcService::ReceiveError(int connectionId,
    const Json::Value &obj)
{
    Json::Value error = obj["error"];
    if (!HasParam(obj, "id", Json::stringValue) &&
        !HasParam(obj, "id", Json::intValue) &&
        !HasParam(obj, "id", Json::uintValue))
    {
        return JsonRpcStatus::INVALID_PARAMS;
    }
    std::string id = EncodeJsonId(obj["id"]);

    if (!HasParam(error, "code", Json::intValue))
    {
        return JsonRpcStatus::INVALID_PARAMS;
    }

    int code = error["code"].asInt();
    std::string message;
    if (HasParam(error, "message", Json::stringValue))
    {
        message = error["message"].asString();
    }

    std::string method;
    if (HasParam(error, "method", Json::stringValue))
    {
        method = error["method"].asString();
    }

    std::string data;
    if (HasParam(error, "data", Json::stringValue))
    {
        data = error["data"].asString();
    }

    if (method != OPTIONAL_STR_NOT_SET || data != OPTIONAL_STR_NOT_SET)
    {
        m_sessionCallback->ReceiveError(code, message, method, data);
    }
    else
    {
        m_sessionCallback->ReceiveError(code, message);
    }
    return JsonRpcStatus::SUCCESS;
}

void JsonRpcService::RespondFeatureSupportInfo(int connectionId, const std::string& id,
    int featureId,
    const std::string& value)
{
    Json::Value result;

    result["method"] = MD_AF_FEATURE_SUPPORT_INFO;
    if (featureId < 0 || featureId >= sizeOfAccessibilityFeature)
    {
        return;
    }
    result["feature"] = GetAccessibilityFeatureName(featureId);
    result["value"] = value;
    Json::Value response = CreateJsonResponse(id, result);
    SendJsonMessageToClient(connectionId, response);
}

void JsonRpcService::RespondFeatureSettingsSubtitles(int connectionId, const std::string &id,
    bool enabled,
    int size, const std::string &fontFamily,
    const std::string &textColour,
    int textOpacity,
    const std::string &edgeType,
    const std::string &edgeColour,
    const std::string &backgroundColour,
    int backgroundOpacity,
    const std::string &windowColour,
    int windowOpacity,
    const std::string &language)
{
    Json::Value value = QuerySettingsSubtitles(enabled, size, fontFamily, textColour, textOpacity,
        edgeType, edgeColour, backgroundColour, backgroundOpacity, windowColour, windowOpacity,
        language);
    Json::Value result = CreateFeatureSettingsQuery(F_SUBTITLES, value);
    Json::Value response = CreateJsonResponse(id, result);
    SendJsonMessageToClient(connectionId, response);
}

void JsonRpcService::RespondFeatureSettingsDialogueEnhancement(int connectionId,
    const std::string &id,
    int dialogueEnhancementGainPreference,
    int dialogueEnhancementGain,
    int dialogueEnhancementLimitMin,
    int dialogueEnhancementLimitMax)
{
    Json::Value value = QuerySettingsDialogueEnhancement(dialogueEnhancementGainPreference,
        dialogueEnhancementGain, dialogueEnhancementLimitMin, dialogueEnhancementLimitMax);
    Json::Value result = CreateFeatureSettingsQuery(F_DIALOGUE_ENHANCEMENT, value);
    Json::Value response = CreateJsonResponse(id, result);
    SendJsonMessageToClient(connectionId, response);
}

void JsonRpcService::RespondFeatureSettingsUIMagnifier(int connectionId, const std::string &id,
    bool enabled,
    const std::string &magType)
{
    Json::Value value = QuerySettingsUIMagnifier(enabled, magType);
    Json::Value result = CreateFeatureSettingsQuery(F_UI_MAGNIFIER, value);
    Json::Value response = CreateJsonResponse(id, result);
    SendJsonMessageToClient(connectionId, response);
}

void JsonRpcService::RespondFeatureSettingsHighContrastUI(int connectionId,
    const std::string &id,
    bool enabled,
    const std::string &hcType)
{
    Json::Value value = QuerySettingsHighContrastUI(enabled, hcType);
    Json::Value result = CreateFeatureSettingsQuery(F_HIGH_CONTRAST_UI, value);
    Json::Value response = CreateJsonResponse(id, result);
    SendJsonMessageToClient(connectionId, response);
}

void JsonRpcService::RespondFeatureSettingsScreenReader(int connectionId,
    const std::string &id,
    bool enabled,
    int speed,
    const std::string &voice,
    const std::string &language)
{
    Json::Value value = QuerySettingsScreenReader(enabled, speed, voice, language);
    Json::Value result = CreateFeatureSettingsQuery(F_SCREEN_READER, value);
    Json::Value response = CreateJsonResponse(id, result);
    SendJsonMessageToClient(connectionId, response);
}

void JsonRpcService::RespondFeatureSettingsResponseToUserAction(int connectionId,
    const std::string &id,
    bool enabled,
    const std::string &type)
{
    Json::Value value = QuerySettingsResponseToUserAction(enabled, type);
    Json::Value result = CreateFeatureSettingsQuery(F_RESPONSE_TO_USER_ACTION, value);
    Json::Value response = CreateJsonResponse(id, result);
    SendJsonMessageToClient(connectionId, response);
}

void JsonRpcService::RespondFeatureSettingsAudioDescription(int connectionId, const std::string &id,
    bool enabled,
    int gainPreference,
    int panAzimuthPreference)
{
    Json::Value value = QuerySettingsAudioDescription(enabled, gainPreference,
        panAzimuthPreference);
    Json::Value result = CreateFeatureSettingsQuery(F_AUDIO_DESCRIPTION, value);
    Json::Value response = CreateJsonResponse(id, result);
    SendJsonMessageToClient(connectionId, response);
}

void JsonRpcService::RespondFeatureSettingsInVisionSigning(int connectionId, const std::string &id,
    bool enabled)
{
    Json::Value value;
    value["enabled"] = enabled;
    Json::Value result = CreateFeatureSettingsQuery(F_IN_VISION_SIGNING, value);
    Json::Value response = CreateJsonResponse(id, result);
    SendJsonMessageToClient(connectionId, response);
}

void JsonRpcService::RespondFeatureSuppress(int connectionId, const std::string &id,
    int featureId,
    const std::string &value)
{
    Json::Value result;
    result["method"] = MD_AF_FEATURE_SUPPRESS;
    if (featureId < 0 || featureId >= sizeOfAccessibilityFeature)
    {
        return;
    }
    result["feature"] = GetAccessibilityFeatureName(featureId);
    result["value"] = value;
    Json::Value response = CreateJsonResponse(id, result);
    SendJsonMessageToClient(connectionId, response);
}

void JsonRpcService::RespondSubscribe(int connectionId, const std::string &id,
    const Json::Value &msgTypeList)
{
    Json::Value result;
    result["msgType"] = msgTypeList;
    Json::Value response = CreateJsonResponse(id, result);
    SendJsonMessageToClient(connectionId, response);
}

void JsonRpcService::RespondUnsubscribe(int connectionId, const std::string &id,
    const Json::Value &msgTypeList)
{
    Json::Value result;
    result["msgType"] = msgTypeList;
    Json::Value response = CreateJsonResponse(id, result);
    SendJsonMessageToClient(connectionId, response);
}

void JsonRpcService::RespondNegotiateMethods(int connectionId, const std::string &id,
    const Json::Value& terminalToApp,
    const Json::Value& appToTerminal)
{
    Json::Value result;
    result["method"] = MD_NEGOTIATE_METHODS;
    result["terminalToApp"] = terminalToApp;
    result["appToTerminal"] = appToTerminal;
    Json::Value response = CreateJsonResponse(id, result);
    SendJsonMessageToClient(connectionId, response);
}

void JsonRpcService::RespondError(int connectionId, const std::string &id,
    int code, const std::string &message)
{
    Json::Value error;
    error["code"] = code;
    error["message"] = message;
    Json::Value response = CreateJsonErrorResponse(id, error);
    SendJsonMessageToClient(connectionId, response);
}

void JsonRpcService::RespondError(int connectionId, const std::string &id,
    int code, const std::string &message, const std::string &data)
{
    Json::Value error;
    error["code"] = code;
    error["message"] = message;
    if (data != OPTIONAL_STR_NOT_SET)
    {
        error["data"] = data;
    }
    Json::Value response = CreateJsonErrorResponse(id, error);
    SendJsonMessageToClient(connectionId, response);
}

void JsonRpcService::VoiceRequestDescription()
{
    std::vector<int> connectionIds = GetAllConnectionIds();
    for (int connectionId : connectionIds)
    {
        Json::Value title = GetConnectionData(connectionId, ConnectionDataType::Title);
        Json::Value secTitle = GetConnectionData(connectionId, ConnectionDataType::SecondTitle);
        Json::Value synopsis = GetConnectionData(connectionId, ConnectionDataType::Synopsis);
        if (!title.isString() || title.asString().empty())
        {
            LOG(LOG_INFO, "Warning, connection data lost, parameter has the wrong type.");
            m_sessionCallback->RespondMessage("No media is playing");
            return;
        }
        std::stringstream message;
        message << "You're watching " << title.asString();
        if (secTitle.isString() && !secTitle.asString().empty())
        {
            message << " {Secondary title: " << secTitle.asString() << "}";
        }
        if (synopsis.isString() && !synopsis.asString().empty())
        {
            message << " \nSynopsis: " << synopsis.asString();
        }
        m_sessionCallback->RespondMessage(message.str());
    }
}

void JsonRpcService::SendIntentMediaPause()
{
    Json::Value params;
    params["origin"] = "voice";
    SendIntentMessage(MD_INTENT_MEDIA_PAUSE, params);
}

void JsonRpcService::SendIntentMediaPlay()
{
    Json::Value params;
    params["origin"] = "voice";
    SendIntentMessage(MD_INTENT_MEDIA_PLAY, params);
}

void JsonRpcService::SendIntentMediaFastForward()
{
    Json::Value params;
    params["origin"] = "voice";
    SendIntentMessage(MD_INTENT_MEDIA_FAST_FORWARD, params);
}

void JsonRpcService::SendIntentMediaFastReverse()
{
    Json::Value params;
    params["origin"] = "voice";
    SendIntentMessage(MD_INTENT_MEDIA_FAST_REVERSE, params);
}

void JsonRpcService::SendIntentMediaStop()
{
    Json::Value params;
    params["origin"] = "voice";
    SendIntentMessage(MD_INTENT_MEDIA_STOP, params);
}

void JsonRpcService::SendIntentMediaSeekContent(const std::string &anchor, int offset)
{
    Json::Value params;
    params["origin"] = "voice";
    params["anchor"] = anchor;
    params["offset"] = offset;
    SendIntentMessage(MD_INTENT_MEDIA_SEEK_CONTENT, params);
}

void JsonRpcService::SendIntentMediaSeekRelative(int offset)
{
    Json::Value params;
    params["origin"] = "voice";
    params["offset"] = offset;
    SendIntentMessage(MD_INTENT_MEDIA_SEEK_RELATIVE, params);
}

void JsonRpcService::SendIntentMediaSeekLive(int offset)
{
    Json::Value params;
    params["origin"] = "voice";
    params["offset"] = offset;
    SendIntentMessage(MD_INTENT_MEDIA_SEEK_LIVE, params);
}

void JsonRpcService::SendIntentMediaSeekWallclock(const std::string &dateTime)
{
    Json::Value params;
    params["origin"] = "voice";
    params["date-time"] = dateTime;
    SendIntentMessage(MD_INTENT_MEDIA_SEEK_WALLCLOCK, params);
}

void JsonRpcService::SendIntentSearch(const std::string &query)
{
    Json::Value params;
    params["origin"] = "voice";
    params["query"] = query;
    SendIntentMessage(MD_INTENT_SEARCH, params);
}

void JsonRpcService::SendIntentDisplay(const std::string &mediaId)
{
    Json::Value params;
    params["origin"] = "voice";
    params["mediaId"] = mediaId;
    SendIntentMessage(MD_INTENT_DISPLAY, params);
}

void JsonRpcService::SendIntentPlayback(const std::string &mediaId,
    const std::string &anchor,
    int offset)
{
    Json::Value params;
    params["origin"] = "voice";
    params["mediaId"] = mediaId;
    if (anchor != OPTIONAL_STR_NOT_SET)
    {
        params["anchor"] = anchor;
    }
    if (offset != OPTIONAL_INT_NOT_SET)
    {
        params["offset"] = offset;
    }
    SendIntentMessage(MD_INTENT_PLAYBACK, params);
}

void JsonRpcService::NotifySubtitles(bool enabled,
    int size, const std::string &fontFamily,
    const std::string &textColour, int textOpacity,
    const std::string &edgeType, const std::string &edgeColour,
    const std::string &backgroundColour, int backgroundOpacity,
    const std::string &windowColour, int windowOpacity,
    const std::string &language)
{
    Json::Value params;
    params["msgType"] = PC_SUBTITLES;
    params["value"] = QuerySettingsSubtitles(enabled, size, fontFamily, textColour, textOpacity,
        edgeType, edgeColour, backgroundColour, backgroundOpacity, windowColour, windowOpacity,
        language);
    SendNotifyMessage(GetAccessibilityFeatureId(F_SUBTITLES), params);
}

void JsonRpcService::NotifyDialogueEnhancement(
    int dialogueEnhancementGainPreference,
    int dialogueEnhancementGain,
    int dialogueEnhancementLimitMin,
    int dialogueEnhancementLimitMax)
{
    Json::Value params;
    params["msgType"] = PC_DIALOGUE_ENHANCEMENT;
    params["value"] = QuerySettingsDialogueEnhancement(dialogueEnhancementGainPreference,
        dialogueEnhancementGain, dialogueEnhancementLimitMin, dialogueEnhancementLimitMax);
    SendNotifyMessage(GetAccessibilityFeatureId(F_DIALOGUE_ENHANCEMENT), params);
}

void JsonRpcService::NotifyUIMagnifier(bool enabled, const std::string &magType)
{
    Json::Value params;
    params["msgType"] = PC_UI_MAGNIFIER;
    params["value"] = QuerySettingsUIMagnifier(enabled, magType);
    SendNotifyMessage(GetAccessibilityFeatureId(F_UI_MAGNIFIER), params);
}

void JsonRpcService::NotifyHighContrastUI(bool enabled, const std::string &hcType)
{
    Json::Value params;
    params["msgType"] = PC_HIGH_CONTRAST_UI;
    params["value"] = QuerySettingsUIMagnifier(enabled, hcType);
    SendNotifyMessage(GetAccessibilityFeatureId(F_HIGH_CONTRAST_UI), params);
}

void JsonRpcService::NotifyScreenReader(bool enabled, int speed, const std::string &voice,
    const std::string &language)
{
    Json::Value params;
    params["msgType"] = PC_SCREEN_READER;
    params["value"] = QuerySettingsScreenReader(enabled, speed, voice, language);
    SendNotifyMessage(GetAccessibilityFeatureId(F_SCREEN_READER), params);
}

void JsonRpcService::NotifyResponseToUserAction(bool enabled, const std::string &type)
{
    Json::Value params;
    params["msgType"] = PC_RESPONSE_TO_USER_ACTION;
    params["value"] = QuerySettingsResponseToUserAction(enabled, type);
    SendNotifyMessage(GetAccessibilityFeatureId(F_RESPONSE_TO_USER_ACTION), params);
}

void JsonRpcService::NotifyAudioDescription(bool enabled, int gainPreference, int
    panAzimuthPreference)
{
    Json::Value params;
    params["msgType"] = PC_AUDIO_DESCRIPTION;
    params["value"] = QuerySettingsAudioDescription(enabled, gainPreference, panAzimuthPreference);
    SendNotifyMessage(GetAccessibilityFeatureId(F_AUDIO_DESCRIPTION), params);
}

void JsonRpcService::NotifyInVisionSigning(bool enabled)
{
    Json::Value params;
    params["msgType"] = PC_IN_VISION_SIGNING;
    Json::Value value;
    value["enabled"] = enabled;
    params["value"] = value;
    SendNotifyMessage(GetAccessibilityFeatureId(F_IN_VISION_SIGNING), params);
}

void JsonRpcService::RespondDialogueEnhancementOverride(int connectionId,
    const std::string &id,
    int dialogueEnhancementGain)
{
    Json::Value result;
    result["method"] = MD_AF_DIALOGUE_ENHANCEMENT_OVERRIDE;
    if (dialogueEnhancementGain != OPTIONAL_INT_NOT_SET)
    {
        result["dialogueEnhancementGain"] = dialogueEnhancementGain;
    }
    Json::Value response = CreateJsonResponse(id, result);
    SendJsonMessageToClient(connectionId, response);
}

void JsonRpcService::RespondTriggerResponseToUserAction(int connectionId,
    const std::string &id,
    bool actioned)
{
    Json::Value result;
    result["method"] = MD_AF_TRIGGER_RESPONSE_TO_USER_ACTION;
    result["actioned"] = actioned;
    Json::Value response = CreateJsonResponse(id, result);
    SendJsonMessageToClient(connectionId, response);
}

// Helper functions
/**
 * Generate a unique ID for an intent associated with a given connection.
 *
 * This method increments the intent ID count for the certain connection,
 * generates a unique ID based on the incremented count.
 * Start from "IntentId1", "IntentId2", ...
 *
 * @param connectionId The ID of the connection for which the intent ID is generated.
 *
 * @return A unique JSON-RPC intent ID as a string.
 */
std::string JsonRpcService::GenerateId(int connectionId)
{
    Json::Value data = GetConnectionData(connectionId, ConnectionDataType::IntentIdCount);
    if (data.isInt())
    {
        int intentId = data.asInt();
        intentId++;    //Generate the unique 'id' for intent media method
        std::string id = EncodeJsonId("IntentId" + std::to_string(intentId));
        SetConnectionData(connectionId,
            ConnectionDataType::IntentIdCount,
            intentId);
        return id;
    }
    LOG(LOG_INFO, "Warning, connection data lost, parameter has wrong type.");
    return OPTIONAL_STR_NOT_SET;
}

/**
 * This method retrieves the connection IDs who have subscribed and have
 * negotiated support for the 'MD_NOTIFY' method (used for notifications).
 *
 * @param result A vector to store the connection IDs of satisfied clients.
 * @param msgTypeIndex The index of the notification message type in
 *                     ACCESSIBILITY_FEATURE_NAMES predefined list.
 */
void JsonRpcService::GetNotifyConnectionIds(std::vector<int> &result,
    const int msgTypeIndex)
{
    std::string suffix = "PrefChange";
    std::string msgType = ACCESSIBILITY_FEATURE_NAMES.at(msgTypeIndex) + suffix;
    std::vector<int> connectionIds = GetAllConnectionIds();
    for (int i : connectionIds)
    {
        Json::Value subscribedMethods = GetConnectionData(i,
            ConnectionDataType::SubscribedMethods);
        Json::Value negotiateMethods = GetConnectionData(i,
            ConnectionDataType::NegotiateMethodsTerminalToApp);
        if (!subscribedMethods.isArray() || !negotiateMethods.isArray())
        {
            LOG(LOG_INFO, "Warning, connection data lost, parameter has wrong type.");
            continue;
        }

        if (IsMethodInJsonArray(subscribedMethods, msgType) &&
            IsMethodInJsonArray(negotiateMethods, MD_NOTIFY))
        {
            result.push_back(i);
        }
    }
}

/**
 * This method allows the registration of a JSON-RPC method by associating it with
 * its name. The registered method can later be invoked using the provided name
 * in JSON-RPC requests.
 *
 * @param name The name of the method to register.
 * @param method A callable pointer representing the JSON-RPC method to be registered.
 */
void JsonRpcService::RegisterMethod(const std::string &name, JsonRpcMethod method)
{
    m_json_rpc_methods[name] = std::bind(method, this, std::placeholders::_1,
        std::placeholders::_2);
}

/**
 * This method takes a connection ID and a JSON response, sends the formatted
 * response message to the client with the specified connection ID.
 * This method is the exit point of class JsonRpcService.
 *
 * @param connectionId The unique identifier of the client connection.
 * @param jsonResponse The JSON data to be sent as a response to the client.
 */
void JsonRpcService::SendJsonMessageToClient(int connectionId,
    const Json::Value &jsonResponse)
{
    Json::FastWriter writer;
    std::string message = writer.write(jsonResponse);
    connections_mutex_.lock();
    WebSocketConnection *connection = GetConnection(connectionId);
    if (connection != nullptr)
    {
        std::ostringstream oss;
        oss << message;
        connection->SendMessage(oss.str());
    }
    connections_mutex_.unlock();
}

/**
 * Send an intent message to available clients.
 *
 * @param method The name of the intent method to send.
 * @param params The JSON parameters associated with the intent.
 */
void JsonRpcService::SendIntentMessage(const std::string& method, Json::Value &params)
{
    std::vector<int> connectionIds;
    CheckIntentMethod(connectionIds, method);
    for (int connectionId : connectionIds)
    {
        std::string id = GenerateId(connectionId);
        AdjustTimeRange(connectionId, method, params);
        Json::Value response = CreateIntentResponse(id, method, params);
        SendJsonMessageToClient(connectionId, response);
    }
}

/**
 * Send an intent message to available clients.
 *
 * @param method The name of the intent method to send.
 * @param params The JSON parameters associated with the intent.
 */
void JsonRpcService::AdjustTimeRange(int id, const std::string& method, Json::Value &params)
{
    Json::Value bias = GetConnectionData(id, ConnectionDataType::BiasToSystemTime);
    Json::Value start = GetConnectionData(id, ConnectionDataType::StartTime);
    Json::Value end = GetConnectionData(id, ConnectionDataType::EndTime);
    if (!bias.isInt() || !start.isInt() || !end.isInt())
    {
        LOG(LOG_INFO, "Warning, connection data lost, parameter has wrong type.");
        return;
    }
    int biasTime = bias.asInt();
    int startTime = start.asInt();
    int endTime = end.asInt();
    if (startTime == -1 || endTime == -1)
    {
        LOG(LOG_INFO, "Warning, connection data lost, parameter is invalid.");
        return;
    }
    int setTime;
    if (method == MD_INTENT_MEDIA_SEEK_CONTENT || method == MD_INTENT_MEDIA_SEEK_RELATIVE ||
        method == MD_INTENT_MEDIA_SEEK_LIVE || (method == MD_INTENT_PLAYBACK && params.isMember(
            "offset")))
    {
        int offset = 0, anchorTime = 0;
        if (params.isMember("anchor") && params["anchor"].asString() == "start")
        {
            anchorTime = startTime;
        }
        else if (params.isMember("anchor") && params["anchor"].asString() == "end")
        {
            anchorTime = endTime;
        }
        else
        {
            anchorTime = std::time(nullptr) + biasTime;
        }
        offset = params["offset"].asInt();
        setTime = anchorTime + offset;
        setTime = std::max(setTime, startTime);
        setTime = std::min(setTime, endTime);
        params["offset"] = (int) (setTime - anchorTime);
    }
    else if (method == MD_INTENT_MEDIA_SEEK_WALLCLOCK)
    {
        setTime = ConvertISO8601ToSecond(params["date-time"].asString());
        setTime = std::max(setTime, startTime);
        setTime = std::min(setTime, endTime);
        std::string setWallclock = ConvertSecondToISO8601(setTime);
        params["date-time"] = setWallclock;
    }
}

/**
 * Send a notification message to available clients.
 *
 * @param msgTypeIndex The index of the notification message type in a predefined list.
 * @param params The JSON parameters associated with the notification.
 */
void JsonRpcService::SendNotifyMessage(int msgTypeIndex, const Json::Value &params)
{
    Json::Value response = CreateNotifyRequest(params);
    std::vector<int> connectionIds;
    GetNotifyConnectionIds(connectionIds, msgTypeIndex);
    for (int connectionId : connectionIds)
    {
        SendJsonMessageToClient(connectionId, response);
    }
}

/**
 * Remove the method not in table.
 *
 * @param connectionId The connection ID.
 * @param methodsList The Json::arrayValue of methods.
 * @param isAppToTerminal The bool shows appToTerminal or terminalToApp.
 *
 * @return The Json::arrayValue of filtered methods
 */
Json::Value JsonRpcService::FilterMethods(int connectionId,
    const Json::Value& methodsList,
    bool isAppToTerminal)
{
    Json::Value newMethodsList;
    for (const auto& method : methodsList)
    {
        if (isAppToTerminal && IsMethodInSet(
            m_supported_methods_app_to_terminal,
            method.asString()))
        {
            SetConnectionData(connectionId,
                ConnectionDataType::NegotiateMethodsAppToTerminal,
                method.asString());
            newMethodsList.append(method);
        }
        else if (!isAppToTerminal && IsMethodInSet(
            m_supported_methods_terminal_to_app,
            method.asString()))
        {
            SetConnectionData(connectionId,
                ConnectionDataType::NegotiateMethodsTerminalToApp,
                method.asString());
            newMethodsList.append(method);
        }
    }
    return newMethodsList;
}

/**
 * This method retrieves connection IDs that are VoiceReady, negotiated,
 * and have been available actions from state media.
 *
 * @param result A vector to store the connection IDs of satisfied clients.
 * @param msgTypeIndex The index of the notification message type in
 *                     ACCESSIBILITY_FEATURE_NAMES predefined list.
 */
void JsonRpcService::CheckIntentMethod(std::vector<int> &result, const std::string& method)
{
    std::vector<int> connectionIds = GetAllConnectionIds();
    for (int i : connectionIds)
    {
        Json::Value voiceReadyJson = GetConnectionData(i, ConnectionDataType::VoiceReady);
        Json::Value negotiateMethodsJson = GetConnectionData(i,
            ConnectionDataType::NegotiateMethodsTerminalToApp);

        if (!voiceReadyJson.isBool() || !negotiateMethodsJson.isArray())
        {
            LOG(LOG_INFO, "Warning, connection data lost, parameter has wrong type.");
            continue;
        }
        if (!voiceReadyJson.asBool() || !IsMethodInJsonArray(negotiateMethodsJson, method))
        {
            continue;
        }

        Json::Value stateJson = GetConnectionData(i, ConnectionDataType::State);
        Json::Value actionPauseJson = GetConnectionData(i, ConnectionDataType::ActionPause);
        Json::Value actionPlayJson = GetConnectionData(i, ConnectionDataType::ActionPlay);
        Json::Value actionFastForwardJson = GetConnectionData(i,
            ConnectionDataType::ActionFastForward);
        Json::Value actionFastReverseJson = GetConnectionData(i,
            ConnectionDataType::ActionFastReverse);
        Json::Value actionStopJson = GetConnectionData(i, ConnectionDataType::ActionStop);
        Json::Value actionSeekContentJson = GetConnectionData(i,
            ConnectionDataType::ActionSeekContent);
        Json::Value actionSeekRelativeJson = GetConnectionData(i,
            ConnectionDataType::ActionSeekRelative);
        Json::Value actionSeekLiveJson = GetConnectionData(i, ConnectionDataType::ActionSeekLive);
        Json::Value actionSeekWallclockJson = GetConnectionData(i,
            ConnectionDataType::ActionSeekWallclock);

        if (!stateJson.isString() || !actionPauseJson.isBool() ||
            !actionPlayJson.isBool() || !actionFastForwardJson.isBool() ||
            !actionFastReverseJson.isBool() || !actionStopJson.isBool() ||
            !actionSeekContentJson.isBool() || !actionSeekRelativeJson.isBool() ||
            !actionSeekLiveJson.isBool() || !actionSeekWallclockJson.isBool())
        {
            LOG(LOG_INFO, "Warning, connection data lost, parameter has wrong type.");
            continue;
        }

        bool shouldAddConnection = false;
        if (method == MD_INTENT_SEARCH || method == MD_INTENT_DISPLAY || method ==
            MD_INTENT_PLAYBACK)
        {
            shouldAddConnection = true;
        }
        else if (stateJson.asString().empty() || stateJson.asString() == "no-media" ||
                 stateJson.asString() == "error")
        {
            continue;
        }
        else if (stateJson.asString() == "stopped")
        {
            shouldAddConnection = (method == MD_INTENT_MEDIA_PLAY && actionPlayJson.asBool());
        }
        else if (method == MD_INTENT_MEDIA_PAUSE && actionPauseJson.asBool())
        {
            shouldAddConnection = stateJson.asString() != "paused";
        }
        else if ((method == MD_INTENT_MEDIA_PLAY && actionPlayJson.asBool()) ||
                 (method == MD_INTENT_MEDIA_FAST_FORWARD && actionFastForwardJson.asBool()) ||
                 (method == MD_INTENT_MEDIA_FAST_REVERSE && actionFastReverseJson.asBool()) ||
                 (method == MD_INTENT_MEDIA_STOP && actionStopJson.asBool()) ||
                 (method == MD_INTENT_MEDIA_SEEK_CONTENT && actionSeekContentJson.asBool()) ||
                 (method == MD_INTENT_MEDIA_SEEK_RELATIVE && actionSeekRelativeJson.asBool()) ||
                 (method == MD_INTENT_MEDIA_SEEK_LIVE && actionSeekLiveJson.asBool()) ||
                 (method == MD_INTENT_MEDIA_SEEK_WALLCLOCK && actionSeekWallclockJson.asBool()))
        {
            shouldAddConnection = true;
        }

        if (shouldAddConnection)
        {
            result.push_back(i);
        }
    }
}

/**
 * Getter Setter function for connection data map. These methods locks the
 * connections_mutex_ to ensure thread safety while updating the connection data.
 */


/**
 * This method get all register connection IDs.
 *
 * @return A list of connection ids.
 */
std::vector<int> JsonRpcService::GetAllConnectionIds()
{
    connections_mutex_.lock();
    std::vector<int> connectionIds;
    for (const auto& entry : m_connectionData)
    {
        connectionIds.push_back(entry.first);
    }
    connections_mutex_.unlock();
    return connectionIds;
}

/**
 * This method get all register connection IDs.
 *
 * @return A list of connection ids.
 */
void JsonRpcService::InitialConnectionData(int connectionId)
{
    connections_mutex_.lock();
    m_connectionData[connectionId].intentIdCount = 0;
    m_connectionData[connectionId].negotiateMethodsAppToTerminal.insert(MD_NEGOTIATE_METHODS);
    connections_mutex_.unlock();
}

/**
 * This method updates connection data based on the specified type and value.
 *
 * @param connectionId The unique identifier of the connection to update.
 * @param type The type of connection data to update.
 * @param value The new value for the specified connection data.
 */
void JsonRpcService::SetConnectionData(int connectionId, ConnectionDataType type, const
    Json::Value& value)
{
    connections_mutex_.lock();
    if (m_connectionData.find(connectionId) == m_connectionData.end())
    {
        return;
    }
    ConnectionData& connectionData = m_connectionData[connectionId];

    switch (type)
    {
        case ConnectionDataType::NegotiateMethodsAppToTerminal:
            connectionData.negotiateMethodsAppToTerminal.insert(value.asString());
            break;
        case ConnectionDataType::NegotiateMethodsTerminalToApp:
            connectionData.negotiateMethodsTerminalToApp.insert(value.asString());
            break;
        case ConnectionDataType::SubscribedMethods:
            connectionData.subscribedMethods.insert(value.asString());
            break;
        case ConnectionDataType::UnsubscribedMethods:
            connectionData.subscribedMethods.erase(value.asString());
            break;
        case ConnectionDataType::IntentIdCount:
            connectionData.intentIdCount = value.asInt();
            break;
        case ConnectionDataType::VoiceReady:
            connectionData.voiceReady = value.asBool();
            break;
        default:
            break;
    }
    connections_mutex_.unlock();
}

/**
 * This method updates connection data based on the specified type and value.
 *
 * @param connectionId The unique identifier of the connection to update.
 * @param mediaData The data for state media
 */
void JsonRpcService::SetStateMediaToConnectionData(int connectionId, const
    ConnectionData& mediaData)
{
    connections_mutex_.lock();
    if (m_connectionData.find(connectionId) == m_connectionData.end())
    {
        return;
    }
    ConnectionData& connectionData = m_connectionData[connectionId];
    connectionData.state = mediaData.state;
    connectionData.actionPause = mediaData.actionPause;
    connectionData.actionPlay = mediaData.actionPlay;
    connectionData.actionFastForward = mediaData.actionFastForward;
    connectionData.actionFastReverse = mediaData.actionFastReverse;
    connectionData.actionStop = mediaData.actionStop;
    connectionData.actionSeekContent = mediaData.actionSeekContent;
    connectionData.actionSeekRelative = mediaData.actionSeekRelative;
    connectionData.actionSeekLive = mediaData.actionSeekLive;
    connectionData.actionSeekWallclock = mediaData.actionSeekWallclock;
    connectionData.biasToSystemTime = mediaData.biasToSystemTime;
    connectionData.startTime = mediaData.startTime;
    connectionData.endTime = mediaData.endTime;
    connectionData.title = mediaData.title;
    connectionData.mediaId = mediaData.mediaId;
    connectionData.secondTitle = mediaData.secondTitle;
    connectionData.synopsis = mediaData.synopsis;
    connections_mutex_.unlock();
}

/**
 * This method retrieves connection data of the specified type for a given connection ID.
 *
 * @param connectionId The unique identifier of the connection to retrieve data for.
 * @param type The type of connection data to retrieve.
 *
 * @return A Json::Value containing the requested connection data. The structure and content
 * of the Json::Value depend on the specified 'type':
 *   - For NegotiateMethodsAppToTerminal, NegotiateMethodsTerminalToApp, and SubscribedMethods,
 *     the Json::Value is an array of strings.
 *   - For IntentIdCount, the Json::Value is an integer value.
 *   - For State, the Json::Value is a single string value.
 *   - For VoiceReady, ActionPause, ActionPlay, ActionFastForward, ActionFastReverse, ActionStop,
 *     ActionSeekContent, ActionSeekRelative, ActionSeekLive, and ActionSeekWallclock, the Json::Value
 *     is a boolean value.
 *   - For biasToSystemTime, StartTime, EndTime, the value is an int.
 *   - For UnsubscribedMethods, an empty Json::Value is returned (indicating no data).
 */
Json::Value JsonRpcService::GetConnectionData(int connectionId, ConnectionDataType type)
{
    connections_mutex_.lock();
    Json::Value value;

    if (m_connectionData.find(connectionId) != m_connectionData.end())
    {
        const ConnectionData& connectionData = m_connectionData[connectionId];

        switch (type)
        {
            case ConnectionDataType::NegotiateMethodsAppToTerminal:
            {
                value = GetMethodsInJsonArray(connectionData.negotiateMethodsAppToTerminal);
                break;
            }
            case ConnectionDataType::NegotiateMethodsTerminalToApp:
            {
                value = GetMethodsInJsonArray(connectionData.negotiateMethodsTerminalToApp);
                break;
            }
            case ConnectionDataType::SubscribedMethods:
            {
                value = GetMethodsInJsonArray(connectionData.subscribedMethods);
                break;
            }
            case ConnectionDataType::UnsubscribedMethods:
                break;
            case ConnectionDataType::IntentIdCount:
                value = connectionData.intentIdCount;
                break;
            case ConnectionDataType::VoiceReady:
                value = connectionData.voiceReady;
                break;
            case ConnectionDataType::State:
                value = connectionData.state;
                break;
            case ConnectionDataType::ActionPause:
                value = connectionData.actionPause;
                break;
            case ConnectionDataType::ActionPlay:
                value = connectionData.actionPlay;
                break;
            case ConnectionDataType::ActionFastForward:
                value = connectionData.actionFastForward;
                break;
            case ConnectionDataType::ActionFastReverse:
                value = connectionData.actionFastReverse;
                break;
            case ConnectionDataType::ActionStop:
                value = connectionData.actionStop;
                break;
            case ConnectionDataType::ActionSeekContent:
                value = connectionData.actionSeekContent;
                break;
            case ConnectionDataType::ActionSeekRelative:
                value = connectionData.actionSeekRelative;
                break;
            case ConnectionDataType::ActionSeekLive:
                value = connectionData.actionSeekLive;
                break;
            case ConnectionDataType::ActionSeekWallclock:
                value = connectionData.actionSeekWallclock;
                break;
            case ConnectionDataType::MediaId:
                value = connectionData.mediaId;
                break;
            case ConnectionDataType::Title:
                value = connectionData.title;
                break;
            case ConnectionDataType::SecondTitle:
                value = connectionData.secondTitle;
                break;
            case ConnectionDataType::Synopsis:
                value = connectionData.synopsis;
                break;
            case ConnectionDataType::BiasToSystemTime:
                value = static_cast<int>(connectionData.biasToSystemTime);
                break;
            case ConnectionDataType::StartTime:
                value = static_cast<int>(connectionData.startTime);
                break;
            case ConnectionDataType::EndTime:
                value = static_cast<int>(connectionData.endTime);
                break;
            default:
                break;
        }
    }
    connections_mutex_.unlock();
    return value;
}

// Static functions

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
Json::Value QuerySettingsSubtitles(bool enabled, int size, const std::string &fontFamily, const
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
Json::Value QuerySettingsDialogueEnhancement(int dialogueEnhancementGainPreference, int
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
Json::Value QuerySettingsUIMagnifier(bool enabled, const std::string &magType)
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
Json::Value QuerySettingsHighContrastUI(bool enabled, const std::string &hcType)
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
Json::Value QuerySettingsScreenReader(bool enabled, int speed, const std::string &voice, const
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
        value["voice"] = voice;
    }
    else if (enabled)
    {
        value["voice"] = "default";
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
Json::Value QuerySettingsResponseToUserAction(bool enabled, const std::string &type)
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
Json::Value QuerySettingsAudioDescription(bool enabled, int gainPreference, int
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
Json::Value GetMethodsInJsonArray(const std::unordered_set<std::string>& set)
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
bool IsMethodInJsonArray(const Json::Value& array, const std::string& method)
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
bool IsMethodInSet(const std::unordered_set<std::string> &set, const std::string& method)
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
bool HasParam(const Json::Value &json, const std::string &param, const Json::ValueType& type)
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
bool HasJsonParam(const Json::Value &json, const std::string &param)
{
    return json.isMember(param) && json[param].isObject();
}

/**
 * Encode a JSON value to a string representation.
 *
 * @param id The JSON value to encode into a string.
 * @return A string representation of the 'id'.
 */
std::string EncodeJsonId(const Json::Value& id)
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
Json::Value DecodeJsonId(const std::string& id)
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
Json::Value CreateFeatureSettingsQuery(const std::string& feature, const Json::Value& value)
{
    Json::Value result;
    result["method"] = MD_AF_FEATURE_SETTINGS_QUERY;
    result["feature"] = feature;
    result["value"] = value;
    return result;
}

/**
 * Create a JSON request for notify message.
 *
 * @param params The Json value of parameters associated with the notify message.
 * @return A JSON request containing full information for notify message.
 */
Json::Value CreateNotifyRequest(const Json::Value& params)
{
    Json::Value jsonResponse;
    jsonResponse["jsonrpc"] = "2.0";
    jsonResponse["method"] = MD_NOTIFY;
    jsonResponse["params"] = params;
    return jsonResponse;
}

/**
 * Create a JSON response for an intent media response.
 *
 * @param id The id for the intent request.
 * @param method The method associated with the intent media response.
 * @param params The parameters associated with the intent media response.
 * @return A JSON response object with the specified id, method, and parameters.
 */
Json::Value CreateIntentResponse(const std::string& id, const std::string& method, const
    Json::Value& params)
{
    Json::Value jsonResponse;
    jsonResponse["jsonrpc"] = "2.0";
    jsonResponse["id"] = DecodeJsonId(id);
    jsonResponse["params"] = params;
    jsonResponse["method"] = method;
    return jsonResponse;
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
Json::Value CreateJsonResponse(const std::string& id, const Json::Value& result)
{
    Json::Value jsonResponse;
    jsonResponse["jsonrpc"] = "2.0";
    jsonResponse["id"] = DecodeJsonId(id);
    jsonResponse["result"] = result;
    return jsonResponse;
}

/**
 * Create a JSON error response with a specific id and error information.
 *
 * @param id The unique identifier for the JSON error response.
 * @param error The error information or details to include in the response.
 * @return A JSON error response object with the id and error details.
 */
Json::Value CreateJsonErrorResponse(const std::string& id, const Json::Value& error)
{
    Json::Value jsonResponse;
    jsonResponse["jsonrpc"] = "2.0";
    jsonResponse["id"] = DecodeJsonId(id);
    jsonResponse["error"] = error;
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
std::string GetErrorMessage(JsonRpcService::JsonRpcStatus status)
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
    LOG(LOG_INFO, "Error, %s", message.c_str());
    return message;
}

/**
 * Get the name of an accessibility feature.
 *
 * @param id The feature ID.
 * @return The name, or "" if the feature is unknown.
 */
std::string GetAccessibilityFeatureName(int id)
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
static int GetAccessibilityFeatureId(const std::string& name)
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
std::time_t ConvertISO8601ToSecond(const std::string& input)
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
std::string ConvertSecondToISO8601(const int sec)
{
    std::time_t input = std::time(nullptr);
    input = sec;
    char time[25];
    struct tm *convertTm;
    convertTm = gmtime(&input);
    strftime(time, sizeof(time), "%FT%TZ", convertTm);
    std::string result = time;
    return result;
}
} // namespace NetworkServices