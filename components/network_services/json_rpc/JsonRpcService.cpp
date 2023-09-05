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
static bool CheckMethodInSet(const std::unordered_set<std::string> &set, const std::string& method);

static bool CheckMethodInJson(const Json::Value& array, const std::string& method);

static bool HasParam(const Json::Value &json, const std::string &param, const
    Json::ValueType& type);

static bool HasJsonParam(const Json::Value &json, const std::string &param);

static std::string EncodeJsonId(const Json::Value& id);

static Json::Value DecodeJsonId(const std::string& id);

static Json::Value CreateFeatureSettingsQuery(const std::string& feature, const Json::Value& value);

static Json::Value CreateNotifyRequest(const Json::Value& params);

static Json::Value CreateJsonResponse(const std::string& id, const std::string& method, const
    Json::Value& params);

static Json::Value CreateJsonResponse(const std::string& id, const Json::Value& result);

static Json::Value CreateJsonErrorResponse(const std::string& id, const Json::Value& error);

static std::string GetErrorMessage(JsonRpcService::JsonRpcStatus status);

static std::string GetAccessibilityFeatureName(int id);

static int GetAccessibilityFeatureId(const std::string& name);

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
        LOG(LOG_INFO, "Error, json rpc parse wrong");
        status = JsonRpcStatus::PARSE_ERROR;
    }

    if (status == JsonRpcStatus::UNKNOWN &&
        (!HasParam(obj, "jsonrpc", Json::stringValue) || obj["jsonrpc"] != "2.0"))
    {
        LOG(LOG_INFO, "Error, Invalid Request");
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
            if (!CheckMethodInJson(
                GetConnectionData(connection->Id(),
                    ConnectionDataType::NegotiateMethodsAppToTerminal),
                method))
            {
                LOG(LOG_INFO, "Error, Method not found");
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
            LOG(LOG_INFO, "Error, Invalid Request");
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
            LOG(LOG_INFO, "Error, Method not found");
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
    RespondSubscribe(connectionId, id, msgTypeBoolList[0],
        msgTypeBoolList[1], msgTypeBoolList[2],
        msgTypeBoolList[3], msgTypeBoolList[4],
        msgTypeBoolList[5], msgTypeBoolList[6],
        msgTypeBoolList[7]);
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
    RespondUnsubscribe(connectionId, id, msgTypeBoolList[0],
        msgTypeBoolList[1], msgTypeBoolList[2],
        msgTypeBoolList[3], msgTypeBoolList[4],
        msgTypeBoolList[5], msgTypeBoolList[6],
        msgTypeBoolList[7]);
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

    std::string currentTimeStr;
    bool isIntCurrent;
    if (HasParam(params, "currentTime", Json::intValue) ||
        HasParam(params, "currentTime", Json::uintValue) ||
        HasParam(params, "currentTime", Json::realValue))
    {
        currentTimeStr = EncodeJsonId(params["currentTime"]);
        isIntCurrent = true;
    }
    else if (HasParam(params, "currentTime", Json::stringValue))
    {
        currentTimeStr = EncodeJsonId(params["currentTime"]);
        isIntCurrent = false;
    }
    if (state == "buffering" || state == "paused" || state == "playing")
    {
        if (currentTimeStr == OPTIONAL_STR_NOT_SET)
        {
            return JsonRpcStatus::NOTIFICATION_ERROR;
        }
    }

    std::string rangeStart;
    std::string rangeEnd;
    if (HasJsonParam(params, "range"))
    {
        bool isIntStart = HasParam(params["range"], "start", Json::intValue) ||
            HasParam(params["range"], "start", Json::uintValue) ||
            HasParam(params["range"], "start", Json::realValue);
        bool isIntEnd = HasParam(params["range"], "end", Json::intValue) ||
            HasParam(params["range"], "end", Json::uintValue) ||
            HasParam(params["range"], "end", Json::realValue);

        if (isIntCurrent && isIntStart && isIntEnd)
        {
            rangeStart = EncodeJsonId(params["range"]["start"]);
            rangeEnd = EncodeJsonId(params["range"]["end"]);
        }
        else if (!isIntCurrent &&
                 HasParam(params["range"], "start", Json::stringValue) &&
                 HasParam(params["range"], "end", Json::stringValue))
        {
            rangeStart = EncodeJsonId(params["range"]["start"]);
            rangeEnd = EncodeJsonId(params["range"]["end"]);
        }
        else
        {
            return JsonRpcStatus::NOTIFICATION_ERROR;
        }
    }
    if (state == "buffering" || state == "paused" || state == "playing")
    {
        if (rangeStart == OPTIONAL_STR_NOT_SET ||
            rangeEnd == OPTIONAL_STR_NOT_SET)
        {
            return JsonRpcStatus::NOTIFICATION_ERROR;
        }
    }

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
        if (title == OPTIONAL_STR_NOT_SET ||
            mediaId == OPTIONAL_STR_NOT_SET ||
            secTitle == OPTIONAL_STR_NOT_SET ||
            synopsis == OPTIONAL_STR_NOT_SET)
        {
            return JsonRpcStatus::NOTIFICATION_ERROR;
        }
    }

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
    SendJsonMessageToClient(connectionId, __func__, response);
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
    SendJsonMessageToClient(connectionId, __func__, response);
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
    SendJsonMessageToClient(connectionId, __func__, response);
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
    SendJsonMessageToClient(connectionId, __func__, response);
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
    SendJsonMessageToClient(connectionId, __func__, response);
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
    SendJsonMessageToClient(connectionId, __func__, response);
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
    SendJsonMessageToClient(connectionId, __func__, response);
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
    SendJsonMessageToClient(connectionId, __func__, response);
}

void JsonRpcService::RespondFeatureSettingsInVisionSigning(int connectionId, const std::string &id,
    bool enabled)
{
    Json::Value value;
    value["enabled"] = enabled;
    Json::Value result = CreateFeatureSettingsQuery(F_IN_VISION_SIGNING, value);
    Json::Value response = CreateJsonResponse(id, result);
    SendJsonMessageToClient(connectionId, __func__, response);
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
    SendJsonMessageToClient(connectionId, __func__, response);
}

void JsonRpcService::RespondSubscribe(int connectionId, const std::string &id,
    bool subtitles, bool dialogueEnhancement,
    bool uiMagnifier, bool highContrastUI,
    bool screenReader, bool responseToUserAction,
    bool audioDescription, bool inVisionSigning)
{
    Json::Value result;
    Json::Value msgTypeList(Json::arrayValue);
    if (subtitles)
    {
        msgTypeList.append(PC_SUBTITLES);
    }
    if (dialogueEnhancement)
    {
        msgTypeList.append(PC_DIALOGUE_ENHANCEMENT);
    }
    if (uiMagnifier)
    {
        msgTypeList.append(PC_UI_MAGNIFIER);
    }
    if (highContrastUI)
    {
        msgTypeList.append(PC_HIGH_CONTRAST_UI);
    }
    if (screenReader)
    {
        msgTypeList.append(PC_SCREEN_READER);
    }
    if (responseToUserAction)
    {
        msgTypeList.append(PC_RESPONSE_TO_USER_ACTION);
    }
    if (audioDescription)
    {
        msgTypeList.append(PC_AUDIO_DESCRIPTION);
    }
    if (inVisionSigning)
    {
        msgTypeList.append(PC_IN_VISION_SIGNING);
    }
    result["msgType"] = msgTypeList;
    Json::Value response = CreateJsonResponse(id, result);
    SendJsonMessageToClient(connectionId, __func__, response);
}

void JsonRpcService::RespondUnsubscribe(int connectionId, const std::string &id,
    bool subtitles, bool dialogueEnhancement,
    bool uiMagnifier, bool highContrastUI,
    bool screenReader, bool responseToUserAction,
    bool audioDescription, bool inVisionSigning)
{
    Json::Value result;
    Json::Value msgTypeList(Json::arrayValue);
    if (subtitles)
    {
        msgTypeList.append(PC_SUBTITLES);
    }
    if (dialogueEnhancement)
    {
        msgTypeList.append(PC_DIALOGUE_ENHANCEMENT);
    }
    if (uiMagnifier)
    {
        msgTypeList.append(PC_UI_MAGNIFIER);
    }
    if (highContrastUI)
    {
        msgTypeList.append(PC_HIGH_CONTRAST_UI);
    }
    if (screenReader)
    {
        msgTypeList.append(PC_SCREEN_READER);
    }
    if (responseToUserAction)
    {
        msgTypeList.append(PC_RESPONSE_TO_USER_ACTION);
    }
    if (audioDescription)
    {
        msgTypeList.append(PC_AUDIO_DESCRIPTION);
    }
    if (inVisionSigning)
    {
        msgTypeList.append(PC_IN_VISION_SIGNING);
    }
    result["msgType"] = msgTypeList;
    Json::Value response = CreateJsonResponse(id, result);
    SendJsonMessageToClient(connectionId, __func__, response);
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
    SendJsonMessageToClient(connectionId, __func__, response);
}

void JsonRpcService::RespondError(int connectionId, const std::string &id,
    int code, const std::string &message)
{
    Json::Value error;
    error["code"] = code;
    error["message"] = message;
    Json::Value response = CreateJsonErrorResponse(id, error);
    SendJsonMessageToClient(connectionId, __func__, response);
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
    SendJsonMessageToClient(connectionId, __func__, response);
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
    SendNotifyMessage(0, params);
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
    SendNotifyMessage(1, params);
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
    SendNotifyMessage(2, params);
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
    SendNotifyMessage(3, params);
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
    SendNotifyMessage(4, params);
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
    SendNotifyMessage(5, params);
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
    SendNotifyMessage(6, params);
}

void JsonRpcService::NotifyInVisionSigning(bool enabled)
{
    Json::Value params;
    params["msgType"] = PC_IN_VISION_SIGNING;
    Json::Value value;
    value["enabled"] = enabled;
    params["value"] = value;
    SendNotifyMessage(7, params);
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
    SendJsonMessageToClient(connectionId, __func__, response);
}

void JsonRpcService::RespondTriggerResponseToUserAction(int connectionId,
    const std::string &id,
    bool actioned)
{
    Json::Value result;
    result["method"] = MD_AF_TRIGGER_RESPONSE_TO_USER_ACTION;
    result["actioned"] = actioned;
    Json::Value response = CreateJsonResponse(id, result);
    SendJsonMessageToClient(connectionId, __func__, response);
}

// Helper functions
std::string JsonRpcService::GenerateID(int connectionId)
{
    int intentId = GetConnectionData(connectionId,
        ConnectionDataType::IntentIdCount).asInt();
    std::string id = EncodeJsonId("IntentId" + std::to_string(++intentId));
    SetConnectionData(connectionId,
        ConnectionDataType::IntentIdCount,
        intentId);
    return id;
}

void JsonRpcService::GetSubscribedConnectionIds(std::vector<int> &subscribedConnectionIds,
    const int msgTypeIndex)
{
    std::string suffix = "PrefChange";
    std::string msgType = ACCESSIBILITY_FEATURE_NAMES.at(msgTypeIndex) + suffix;
    std::vector<int> connectionIds = GetAllConnectionIds();
    for (int i : connectionIds)
    {
        Json::Value subscribedMethods = GetConnectionData(i,
            ConnectionDataType::SubscribedMethods);
        if (CheckMethodInJson(subscribedMethods, msgType))
        {
            subscribedConnectionIds.push_back(i);
        }
    }
}

void JsonRpcService::RegisterMethod(const std::string &name, JsonRpcMethod method)
{
    m_json_rpc_methods[name] = std::bind(method, this, std::placeholders::_1,
        std::placeholders::_2);
}

void JsonRpcService::SendJsonMessageToClient(int connectionId,
    const std::string &responseName,
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

void JsonRpcService::SendIntentMessage(const std::string& method, const Json::Value &params)
{
    std::vector<int> connectionIds;
    CheckIntentMethod(connectionIds, method);
    if (connectionIds.empty())
    {
        return;
    }
    for (int connectionId : connectionIds)
    {
        std::string id = GenerateID(connectionId);
        Json::Value response = CreateJsonResponse(id, method, params);
        SendJsonMessageToClient(connectionId, __func__, response);
    }
}

void JsonRpcService::SendNotifyMessage(int msgTypeIndex, const Json::Value &params)
{
    Json::Value response = CreateNotifyRequest(params);
    std::vector<int> subscribedConnectionIds;
    GetSubscribedConnectionIds(subscribedConnectionIds, msgTypeIndex);
    if (subscribedConnectionIds.empty())
    {
        return;
    }
    for (int connectionId : subscribedConnectionIds)
    {
        SendJsonMessageToClient(connectionId, __func__, response);
    }
}

/**
 * Remove the method not in table
 *
 * @param connectionId The connection ID.
 * @param oldString The Json::arrayValue of methods.
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
        if (isAppToTerminal && CheckMethodInSet(
            m_supported_methods_app_to_terminal,
            method.asString()))
        {
            SetConnectionData(connectionId,
                ConnectionDataType::NegotiateMethodsAppToTerminal,
                method.asString());
            newMethodsList.append(method);
        }
        else if (!isAppToTerminal && CheckMethodInSet(
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

void JsonRpcService::CheckIntentMethod(std::vector<int> &result, const std::string& method)
{
    std::vector<int> connectionIds = GetAllConnectionIds();
    for (int i : connectionIds)
    {
        if (!GetConnectionData(i, ConnectionDataType::VoiceReady))
        {
            continue;
        }
        if (!CheckMethodInJson(GetConnectionData(i,
            ConnectionDataType::NegotiateMethodsTerminalToApp),
            method))
        {
            continue;
        }

        bool shouldAddConnection = false;

        if (method == MD_INTENT_MEDIA_PAUSE &&
            GetConnectionData(i, ConnectionDataType::ActionPause))
        {
            shouldAddConnection =
                GetConnectionData(i, ConnectionDataType::State) != "paused";
        }
        else if ((method == MD_INTENT_MEDIA_PLAY &&
                  GetConnectionData(i, ConnectionDataType::ActionPlay)) ||
                 (method == MD_INTENT_MEDIA_FAST_FORWARD &&
                  GetConnectionData(i, ConnectionDataType::ActionFastForward)) ||
                 (method == MD_INTENT_MEDIA_FAST_REVERSE &&
                  GetConnectionData(i, ConnectionDataType::ActionFastReverse)))
        {
            shouldAddConnection =
                GetConnectionData(i, ConnectionDataType::State) != "playing";
        }
        else if (method == MD_INTENT_MEDIA_STOP && GetConnectionData(i,
            ConnectionDataType::ActionStop))
        {
            shouldAddConnection =
                GetConnectionData(i, ConnectionDataType::State) != "stopped";
        }
        else if ((method == MD_INTENT_MEDIA_SEEK_CONTENT && GetConnectionData(i,
            ConnectionDataType::ActionSeekContent)) ||
                 (method == MD_INTENT_MEDIA_SEEK_RELATIVE && GetConnectionData(i,
                     ConnectionDataType::ActionSeekRelative)) ||
                 (method == MD_INTENT_MEDIA_SEEK_LIVE && GetConnectionData(i,
                     ConnectionDataType::ActionSeekLive)) ||
                 (method == MD_INTENT_MEDIA_SEEK_WALLCLOCK && GetConnectionData(i,
                     ConnectionDataType::ActionSeekWallclock)))
        {
            shouldAddConnection = true;
        }
        else if (method == MD_INTENT_SEARCH || method == MD_INTENT_DISPLAY || method ==
                 MD_INTENT_PLAYBACK)
        {
            shouldAddConnection = true;
        }

        if (shouldAddConnection)
        {
            result.push_back(i);
        }
    }
}

// Getter Setter function
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

void JsonRpcService::InitialConnectionData(int connectionId)
{
    connections_mutex_.lock();
    m_connectionData[connectionId].intentIdCount = 0;
    m_connectionData[connectionId].negotiateMethodsAppToTerminal.insert(MD_NEGOTIATE_METHODS);
    connections_mutex_.unlock();
}

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
        {
            connectionData.negotiateMethodsAppToTerminal.insert(value.asString());
            break;
        }
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
    }
    connections_mutex_.unlock();
}

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
                int index = 0;
                for (const std::string& item : connectionData.negotiateMethodsAppToTerminal)
                {
                    value[index++] = Json::Value(item);
                }
                break;
            }
            case ConnectionDataType::NegotiateMethodsTerminalToApp:
            {
                int index = 0;
                for (const std::string& item : connectionData.negotiateMethodsTerminalToApp)
                {
                    value[index++] = Json::Value(item);
                }
                break;
            }
            case ConnectionDataType::SubscribedMethods:
            {
                int index = 0;
                for (const std::string& item : connectionData.subscribedMethods)
                {
                    value[index++] = Json::Value(item);
                }
                break;
            }
            case ConnectionDataType::IntentIdCount:
                value = connectionData.intentIdCount;
                break;
            case ConnectionDataType::State:
                value = connectionData.state;
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
        }
    }
    connections_mutex_.unlock();
    return value;
}

// Static functions
bool CheckMethodInJson(const Json::Value& array, const std::string& method)
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

bool CheckMethodInSet(const std::unordered_set<std::string> &set, const std::string& method)
{
    return set.find(method) != set.end();
}

bool HasParam(const Json::Value &json, const std::string &param, const Json::ValueType& type)
{
    if (!json.isMember(param))
    {
        return false;
    }
    if (json[param].type() != type)
    {
        return false;
    }
    return true;
}

bool HasJsonParam(const Json::Value &json, const std::string &param)
{
    if (!json.isMember(param))
    {
        return false;
    }
    if (!json[param].isObject())
    {
        return false;
    }
    return true;
}

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
    writerBuilder["indentation"] = "";
    return Json::writeString(writerBuilder, id);
}

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

Json::Value CreateFeatureSettingsQuery(const std::string& feature, const Json::Value& value)
{
    Json::Value result;
    result["method"] = MD_AF_FEATURE_SETTINGS_QUERY;
    result["feature"] = feature;
    result["value"] = value;
    return result;
}

Json::Value CreateNotifyRequest(const Json::Value& params)
{
    Json::Value jsonResponse;
    jsonResponse["jsonrpc"] = "2.0";
    jsonResponse["method"] = MD_NOTIFY;
    jsonResponse["params"] = params;
    return jsonResponse;
}

Json::Value CreateJsonResponse(const std::string& id, const std::string& method, const
    Json::Value& params)
{
    Json::Value jsonResponse;
    jsonResponse["jsonrpc"] = "2.0";
    jsonResponse["id"] = DecodeJsonId(id);
    jsonResponse["params"] = params;
    jsonResponse["method"] = method;
    return jsonResponse;
}

Json::Value CreateJsonResponse(const std::string& id, const Json::Value& result)
{
    Json::Value jsonResponse;
    jsonResponse["jsonrpc"] = "2.0";
    jsonResponse["id"] = DecodeJsonId(id);
    jsonResponse["result"] = result;
    return jsonResponse;
}

Json::Value CreateJsonErrorResponse(const std::string& id, const Json::Value& error)
{
    Json::Value jsonResponse;
    jsonResponse["jsonrpc"] = "2.0";
    jsonResponse["id"] = DecodeJsonId(id);
    jsonResponse["error"] = error;
    return jsonResponse;
}

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
    return "";
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
} // namespace NetworkServices