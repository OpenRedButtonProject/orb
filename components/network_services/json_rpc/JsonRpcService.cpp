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

static std::string ConvertSecondToISO8601(const long sec);

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

    std::string kind;
    if (HasParam(params, "kind", Json::stringValue))
    {
        kind = params["kind"].asString();
        if (kind != "audio" && kind != "audio-video")
        {
            return JsonRpcStatus::NOTIFICATION_ERROR;
        }
    }
    if (state == "buffering" || state == "paused" || state == "playing" || state == "stopped")
    {
        if (kind == OPTIONAL_STR_NOT_SET)
        {
            return JsonRpcStatus::NOTIFICATION_ERROR;
        }
    }

    std::string type;
    if (HasParam(params, "type", Json::stringValue))
    {
        type = params["type"].asString();
        if (type != "live" && type != "on-demand")
        {
            return JsonRpcStatus::NOTIFICATION_ERROR;
        }
    }
    if (state == "buffering" || state == "paused" || state == "playing" || state == "stopped")
    {
        if (type == OPTIONAL_STR_NOT_SET)
        {
            return JsonRpcStatus::NOTIFICATION_ERROR;
        }
    }

    long long currentTime = -1;
    bool isIntCurrent = false;
    bool isStringCurrent = false;
    int curr = 0;
    if (HasParam(params, "currentTime", Json::intValue) ||
        HasParam(params, "currentTime", Json::uintValue) ||
        HasParam(params, "currentTime", Json::realValue))
    {
        curr = params["currentTime"].asInt();
        currentTime = std::time(nullptr);
        isIntCurrent = true;
    }
    else if (HasParam(params, "currentTime", Json::stringValue))
    {
        std::string currentTimeStr = params["currentTime"].asString();
        currentTime = ConvertISO8601ToSecond(currentTimeStr);
        isStringCurrent = true;
    }
    else if (state == "buffering" || state == "paused" || state == "playing")
    {
        return JsonRpcStatus::NOTIFICATION_ERROR;
    }
    SetConnectionData(connectionId, ConnectionDataType::CurrentTime, currentTime);

    long long startTime = -1;
    long long endTime = -1;
    bool isIntStart = HasParam(params["range"], "start", Json::intValue) ||
        HasParam(params["range"], "start", Json::uintValue) ||
        HasParam(params["range"], "start", Json::realValue);
    bool isIntEnd = HasParam(params["range"], "end", Json::intValue) ||
        HasParam(params["range"], "end", Json::uintValue) ||
        HasParam(params["range"], "end", Json::realValue);
    if (isIntCurrent && isIntStart && isIntEnd)
    {
        int start = params["range"]["start"].asInt();
        int end = params["range"]["end"].asInt();
        startTime = currentTime + (start - curr);
        endTime = currentTime + (end - curr);
    }
    else if (isStringCurrent &&
             HasParam(params["range"], "start", Json::stringValue) &&
             HasParam(params["range"], "end", Json::stringValue))
    {
        startTime = ConvertISO8601ToSecond(params["range"]["start"].asString());
        endTime = ConvertISO8601ToSecond(params["range"]["end"].asString());
    }
    else if (state == "buffering" || state == "paused" || state == "playing")
    {
        return JsonRpcStatus::NOTIFICATION_ERROR;
    }
    SetConnectionData(connectionId, ConnectionDataType::StartTime, startTime);
    SetConnectionData(connectionId, ConnectionDataType::EndTime, endTime);

    SetConnectionData(connectionId, ConnectionDataType::ActionPause, false);
    SetConnectionData(connectionId, ConnectionDataType::ActionPlay, false);
    SetConnectionData(connectionId, ConnectionDataType::ActionFastForward, false);
    SetConnectionData(connectionId, ConnectionDataType::ActionFastReverse, false);
    SetConnectionData(connectionId, ConnectionDataType::ActionStop, false);
    SetConnectionData(connectionId, ConnectionDataType::ActionSeekContent, false);
    SetConnectionData(connectionId, ConnectionDataType::ActionSeekRelative, false);
    SetConnectionData(connectionId, ConnectionDataType::ActionSeekLive, false);
    SetConnectionData(connectionId, ConnectionDataType::ActionSeekWallclock, false);

    if (!HasJsonParam(params, "availableActions"))
    {
        return JsonRpcStatus::NOTIFICATION_ERROR;
    }

    Json::Value actions = params["availableActions"];
    if (HasParam(actions, "pause", Json::booleanValue) &&
        actions["pause"] == true)
    {
        SetConnectionData(connectionId, ConnectionDataType::ActionPause, true);
    }
    if (HasParam(actions, "play", Json::booleanValue) &&
        actions["play"] == true)
    {
        SetConnectionData(connectionId, ConnectionDataType::ActionPlay, true);
    }
    if (HasParam(actions, "fast-forward", Json::booleanValue) &&
        actions["fast-forward"] == true)
    {
        SetConnectionData(connectionId, ConnectionDataType::ActionFastForward, true);
    }
    if (HasParam(actions, "fast-reverse", Json::booleanValue) &&
        actions["fast-reverse"] == true)
    {
        SetConnectionData(connectionId, ConnectionDataType::ActionFastReverse, true);
    }
    if (HasParam(actions, "stop", Json::booleanValue) &&
        actions["stop"] == true)
    {
        SetConnectionData(connectionId, ConnectionDataType::ActionStop, true);
    }
    if (HasParam(actions, "seek-content", Json::booleanValue) &&
        actions["seek-content"] == true)
    {
        SetConnectionData(connectionId, ConnectionDataType::ActionSeekContent, true);
    }
    if (HasParam(actions, "seek-relative", Json::booleanValue) &&
        actions["seek-relative"] == true)
    {
        SetConnectionData(connectionId, ConnectionDataType::ActionSeekRelative, true);
    }
    if (HasParam(actions, "seek-live", Json::booleanValue) &&
        actions["seek-live"] == true)
    {
        SetConnectionData(connectionId, ConnectionDataType::ActionSeekLive, true);
    }
    if (HasParam(actions, "seek-wallclock", Json::booleanValue) &&
        actions["seek-wallclock"] == true)
    {
        SetConnectionData(connectionId, ConnectionDataType::ActionSeekWallclock, true);
    }

    std::string mediaId;
    std::string title;
    std::string secTitle;
    std::string synopsis;
    if (HasJsonParam(params, "metadata"))
    {
        Json::Value metadata = params["metadata"];
        if (HasParam(metadata, "title", Json::stringValue))
        {
            title = metadata["title"].asString();
        }
        if (HasParam(metadata, "mediaId", Json::stringValue))
        {
            mediaId = metadata["mediaId"].asString();
        }
        if (HasParam(metadata, "secondaryTitle", Json::stringValue))
        {
            secTitle = metadata["secondaryTitle"].asString();
        }
        if (HasParam(metadata, "synopsis", Json::stringValue))
        {
            synopsis = metadata["synopsis"].asString();
        }
    }
    if (state == "buffering" || state == "paused" || state == "playing" || state == "stopped")
    {
        if (title == OPTIONAL_STR_NOT_SET)
        {
            return JsonRpcStatus::NOTIFICATION_ERROR;
        }
    }
    SetConnectionData(connectionId, ConnectionDataType::Content, title);

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
    SetConnectionData(connectionId, ConnectionDataType::State, state);
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
    int size, const std::string fontFamily,
    const std::string textColour,
    int textOpacity,
    const std::string edgeType,
    const std::string edgeColour,
    const std::string backgroundColour,
    int backgroundOpacity,
    const std::string windowColour,
    int windowOpacity,
    const std::string language)
{
    Json::Value value;
    value["enabled"] = enabled;
    if (size != OPTIONAL_INT_NOT_SET)
    {
        value["size"] = size;
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
        value["textOpacity"] = textOpacity;
    }
    if (edgeType != OPTIONAL_STR_NOT_SET)
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
        value["backgroundOpacity"] = backgroundOpacity;
    }
    if (windowColour != OPTIONAL_STR_NOT_SET)
    {
        value["windowColour"] = windowColour;
    }
    if (windowOpacity != OPTIONAL_INT_NOT_SET)
    {
        value["windowOpacity"] = windowOpacity;
    }
    if (language != OPTIONAL_STR_NOT_SET)
    {
        value["language"] = language;
    }
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
    Json::Value value;
    value["dialogueEnhancementGainPreference"] = dialogueEnhancementGainPreference;
    value["dialogueEnhancementGain"] = dialogueEnhancementGain;
    Json::Value dialogueEnhancementLimit;
    dialogueEnhancementLimit["min"] = dialogueEnhancementLimitMin;
    dialogueEnhancementLimit["max"] = dialogueEnhancementLimitMax;
    value["dialogueEnhancementLimit"] = dialogueEnhancementLimit;
    Json::Value result = CreateFeatureSettingsQuery(F_DIALOGUE_ENHANCEMENT, value);
    Json::Value response = CreateJsonResponse(id, result);
    SendJsonMessageToClient(connectionId, response);
}

void JsonRpcService::RespondFeatureSettingsUIMagnifier(int connectionId, const std::string &id,
    bool enabled,
    const std::string &magType)
{
    Json::Value value;
    value["enabled"] = enabled;
    if (magType != OPTIONAL_STR_NOT_SET)
    {
        value["magType"] = magType;
    }
    Json::Value result = CreateFeatureSettingsQuery(F_UI_MAGNIFIER, value);
    Json::Value response = CreateJsonResponse(id, result);
    SendJsonMessageToClient(connectionId, response);
}

void JsonRpcService::RespondFeatureSettingsHighContrastUI(int connectionId,
    const std::string &id,
    bool enabled,
    const std::string &hcType)
{
    Json::Value value;
    value["enabled"] = enabled;
    if (hcType != OPTIONAL_STR_NOT_SET)
    {
        value["hcType"] = hcType;
    }
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
    Json::Value value;
    value["enabled"] = enabled;
    if (speed != OPTIONAL_INT_NOT_SET)
    {
        value["speed"] = speed;
    }
    if (voice != OPTIONAL_STR_NOT_SET)
    {
        value["voice"] = voice;
    }
    if (language != OPTIONAL_STR_NOT_SET)
    {
        value["language"] = language;
    }
    Json::Value result = CreateFeatureSettingsQuery(F_SCREEN_READER, value);
    Json::Value response = CreateJsonResponse(id, result);
    SendJsonMessageToClient(connectionId, response);
}

void JsonRpcService::RespondFeatureSettingsResponseToUserAction(int connectionId,
    const std::string &id,
    bool enabled,
    const std::string &type)
{
    Json::Value value;
    value["enabled"] = enabled;
    if (type != OPTIONAL_STR_NOT_SET)
    {
        value["type"] = type;
    }
    Json::Value result = CreateFeatureSettingsQuery(F_RESPONSE_TO_USER_ACTION, value);
    Json::Value response = CreateJsonResponse(id, result);
    SendJsonMessageToClient(connectionId, response);
}

void JsonRpcService::RespondFeatureSettingsAudioDescription(int connectionId, const std::string &id,
    bool enabled,
    int gainPreference,
    int panAzimuthPreference)
{
    Json::Value value;
    value["enabled"] = enabled;
    if (gainPreference != OPTIONAL_INT_NOT_SET)
    {
        value["gainPreference"] = gainPreference;
    }
    if (panAzimuthPreference != OPTIONAL_INT_NOT_SET)
    {
        value["panAzimuthPreference"] = panAzimuthPreference;
    }
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

void JsonRpcService::RequestMediaDescription()
{
    std::vector<int> connectionIds = GetAllConnectionIds();
    for (int connectionId : connectionIds)
    {
        std::string description = "No media is playing";
        Json::Value title = GetConnectionData(connectionId, ConnectionDataType::Content);
        if (title.isString() && !title.asString().empty())
        {
            description = "You're watching " + title.asString();
        }
        else
        {
            LOG(LOG_INFO, "Warning, connection data lost, parameter has wrong type.");
        }
        m_sessionCallback->RespondMessage(description);
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
    Json::Value value;
    value["enabled"] = enabled;
    if (size != OPTIONAL_INT_NOT_SET)
    {
        value["size"] = size;
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
        value["textOpacity"] = textOpacity;
    }
    if (edgeType != OPTIONAL_STR_NOT_SET)
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
        value["backgroundOpacity"] = backgroundOpacity;
    }
    if (windowColour != OPTIONAL_STR_NOT_SET)
    {
        value["windowColour"] = windowColour;
    }
    if (windowOpacity != OPTIONAL_INT_NOT_SET)
    {
        value["windowOpacity"] = windowOpacity;
    }
    if (language != OPTIONAL_STR_NOT_SET)
    {
        value["language"] = language;
    }
    params["value"] = value;
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
    Json::Value value;
    value["dialogueEnhancementGainPreference"] = dialogueEnhancementGainPreference;
    value["dialogueEnhancementGain"] = dialogueEnhancementGain;
    Json::Value dialogueEnhancementLimit;
    dialogueEnhancementLimit["min"] = dialogueEnhancementLimitMin;
    dialogueEnhancementLimit["max"] = dialogueEnhancementLimitMax;
    value["dialogueEnhancementLimit"] = dialogueEnhancementLimit;
    params["value"] = value;
    SendNotifyMessage(GetAccessibilityFeatureId(F_DIALOGUE_ENHANCEMENT), params);
}

void JsonRpcService::NotifyUIMagnifier(bool enabled, const std::string &magType)
{
    Json::Value params;
    params["msgType"] = PC_UI_MAGNIFIER;
    Json::Value value;
    value["enabled"] = enabled;
    if (magType != OPTIONAL_STR_NOT_SET)
    {
        value["magType"] = magType;
    }
    params["value"] = value;
    SendNotifyMessage(GetAccessibilityFeatureId(F_UI_MAGNIFIER), params);
}

void JsonRpcService::NotifyHighContrastUI(bool enabled, const std::string &hcType)
{
    Json::Value params;
    params["msgType"] = PC_HIGH_CONTRAST_UI;
    Json::Value value;
    value["enabled"] = enabled;
    if (hcType != OPTIONAL_STR_NOT_SET)
    {
        value["hcType"] = hcType;
    }
    params["value"] = value;
    SendNotifyMessage(GetAccessibilityFeatureId(F_HIGH_CONTRAST_UI), params);
}

void JsonRpcService::NotifyScreenReader(bool enabled, int speed, const std::string &voice,
    const std::string &language)
{
    Json::Value params;
    params["msgType"] = PC_SCREEN_READER;
    Json::Value value;
    value["enabled"] = enabled;
    if (speed != OPTIONAL_INT_NOT_SET)
    {
        value["speed"] = speed;
    }
    if (voice != OPTIONAL_STR_NOT_SET)
    {
        value["voice"] = voice;
    }
    if (language != OPTIONAL_STR_NOT_SET)
    {
        value["language"] = language;
    }
    params["value"] = value;
    SendNotifyMessage(GetAccessibilityFeatureId(F_SCREEN_READER), params);
}

void JsonRpcService::NotifyResponseToUserAction(bool enabled, const std::string &type)
{
    Json::Value params;
    params["msgType"] = PC_RESPONSE_TO_USER_ACTION;
    Json::Value value;
    value["enabled"] = enabled;
    if (type != OPTIONAL_STR_NOT_SET)
    {
        value["type"] = type;
    }
    params["value"] = value;
    SendNotifyMessage(GetAccessibilityFeatureId(F_RESPONSE_TO_USER_ACTION), params);
}

void JsonRpcService::NotifyAudioDescription(bool enabled, int gainPreference, int
    panAzimuthPreference)
{
    Json::Value params;
    params["msgType"] = PC_AUDIO_DESCRIPTION;
    Json::Value value;
    value["enabled"] = enabled;
    if (gainPreference != OPTIONAL_INT_NOT_SET)
    {
        value["gainPreference"] = gainPreference;
    }
    if (panAzimuthPreference != OPTIONAL_INT_NOT_SET)
    {
        value["panAzimuthPreference"] = panAzimuthPreference;
    }
    params["value"] = value;
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

void JsonRpcService::AdjustTimeRange(int id, const std::string& method, Json::Value &params)
{
    Json::Value current = GetConnectionData(id, ConnectionDataType::CurrentTime);
    Json::Value start = GetConnectionData(id, ConnectionDataType::StartTime);
    Json::Value end = GetConnectionData(id, ConnectionDataType::EndTime);
    if (!current.isInt64() || !start.isInt64() || !end.isInt64())
    {
        LOG(LOG_INFO, "Warning, connection data lost, parameter has wrong type.");
        return;
    }
    long currentTime = (long) current.asInt64();
    long startTime = (long) start.asInt64();
    long endTime = (long) end.asInt64();
    if (currentTime == -1 || startTime == -1 || endTime == -1)
    {
        LOG(LOG_INFO, "Warning, connection data lost, parameter is invalid.");
        return;
    }
    long setTime;
    if (method == MD_INTENT_MEDIA_SEEK_CONTENT || method == MD_INTENT_MEDIA_SEEK_RELATIVE ||
        method == MD_INTENT_MEDIA_SEEK_LIVE || (method == MD_INTENT_PLAYBACK && params.isMember(
            "offset")))
    {
        long offset = 0, anchorTime = 0;
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
            anchorTime = currentTime;
            // TO-DO: real current time
            // for MD_INTENT_MEDIA_SEEK_RELATIVE, MD_INTENT_MEDIA_SEEK_LIVE & MD_INTENT_PLAYBACK
            // anchorTime = GetSystemCurrentTime();
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
            MD_INTENT_PLAYBACK ||
            (method == MD_INTENT_MEDIA_SEEK_CONTENT && actionSeekContentJson.asBool()) ||
            (method == MD_INTENT_MEDIA_SEEK_RELATIVE && actionSeekRelativeJson.asBool()) ||
            (method == MD_INTENT_MEDIA_SEEK_LIVE && actionSeekLiveJson.asBool()) ||
            (method == MD_INTENT_MEDIA_SEEK_WALLCLOCK && actionSeekWallclockJson.asBool()))
        {
            shouldAddConnection = true;
        }
        else if (stateJson.asString() == "no-media" || stateJson.asString() == "error" ||
                 stateJson.asString() == "buffering")
        {
            continue;
        }
        else if (method == MD_INTENT_MEDIA_PAUSE && actionPauseJson.asBool())
        {
            shouldAddConnection = stateJson.asString() != "paused";
        }
        else if ((method == MD_INTENT_MEDIA_PLAY && actionPlayJson.asBool()))
        {
            shouldAddConnection = stateJson.asString() != "playing";
        }
        else if (method == MD_INTENT_MEDIA_STOP && actionStopJson.asBool())
        {
            shouldAddConnection = stateJson.asString() != "stopped";
        }
        else if ((method == MD_INTENT_MEDIA_FAST_FORWARD && actionFastForwardJson.asBool()) ||
                 (method == MD_INTENT_MEDIA_FAST_REVERSE && actionFastReverseJson.asBool()))
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
        case ConnectionDataType::State:
            connectionData.state = value.asString();
            break;
        case ConnectionDataType::Content:
            connectionData.content = value.asString();
            break;
        case ConnectionDataType::VoiceReady:
            connectionData.voiceReady = value.asBool();
            break;
        case ConnectionDataType::ActionPause:
            connectionData.actionPause = value.asBool();
            break;
        case ConnectionDataType::ActionPlay:
            connectionData.actionPlay = value.asBool();
            break;
        case ConnectionDataType::ActionFastForward:
            connectionData.actionFastForward = value.asBool();
            break;
        case ConnectionDataType::ActionFastReverse:
            connectionData.actionFastReverse = value.asBool();
            break;
        case ConnectionDataType::ActionStop:
            connectionData.actionStop = value.asBool();
            break;
        case ConnectionDataType::ActionSeekContent:
            connectionData.actionSeekContent = value.asBool();
            break;
        case ConnectionDataType::ActionSeekRelative:
            connectionData.actionSeekRelative = value.asBool();
            break;
        case ConnectionDataType::ActionSeekLive:
            connectionData.actionSeekLive = value.asBool();
            break;
        case ConnectionDataType::ActionSeekWallclock:
            connectionData.actionSeekWallclock = value.asBool();
            break;
        case ConnectionDataType::CurrentTime:
            connectionData.currentTime = value.asInt64();
            break;
        case ConnectionDataType::StartTime:
            connectionData.startTime = value.asInt64();
            break;
        case ConnectionDataType::EndTime:
            connectionData.endTime = value.asInt64();
            break;
    }
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
 *   - For currentTime, StartTime, EndTime, the value is a long int.
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
            case ConnectionDataType::IntentIdCount:
                value = connectionData.intentIdCount;
                break;
            case ConnectionDataType::State:
                value = connectionData.state;
                break;
            case ConnectionDataType::Content:
                value = connectionData.content;
                break;
            case ConnectionDataType::VoiceReady:
                value = connectionData.voiceReady;
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
            case ConnectionDataType::UnsubscribedMethods:
                break;
            case ConnectionDataType::CurrentTime:
                value = connectionData.currentTime;
                break;
            case ConnectionDataType::StartTime:
                value = connectionData.startTime;
                break;
            case ConnectionDataType::EndTime:
                value = connectionData.endTime;
                break;
        }
    }
    connections_mutex_.unlock();
    return value;
}

// Static functions
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
    // TO-DO check the format of the input
    const char *inputBuff = input.c_str();
    struct tm time;
    strptime(inputBuff, "%FT%TZ", &time);
    time.tm_isdst = 0;
    time_t t = mktime(&time);
    return t;
}

/**
 * Convert seconds to wall-clock time
 *
 * @param sec time in seconds
 * @return time in ISO8601 format
 */
std::string ConvertSecondToISO8601(const long sec)
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