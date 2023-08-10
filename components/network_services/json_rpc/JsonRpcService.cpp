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
#define LOG_TAG "JsonRpcService"

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

static bool HasParam(const Json::Value &json, const std::string &name, const Json::ValueType& type);

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

static Json::Value CreateNegotiatedMethods(const std::string& stringList);

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
    m_negotiate_methods_app_to_terminal[connection->Id()].insert(MD_NEGOTIATE_METHODS);
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
            if (!CheckMethodInNegotiatedMap("appToTerminal", connection->Id(), method))
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
    m_negotiate_methods_terminal_to_app.erase(connection->Id());
    m_negotiate_methods_app_to_terminal.erase(connection->Id());
    m_subscribed_method.erase(connection->Id());
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
    std::string terminalToApp;
    terminalToApp = obj["params"]["terminalToApp"].toStyledString();
    std::string appToTerminal;
    appToTerminal = obj["params"]["appToTerminal"].toStyledString();

    //add method to set
    std::string filteredTerminalToApp = ResetStylesFilterMethods(connectionId, terminalToApp,
        "terminalToApp");
    std::string filteredAppToTerminal = ResetStylesFilterMethods(connectionId, appToTerminal,
        "appToTerminal");

    m_sessionCallback->RequestNegotiateMethods(connectionId, id, filteredTerminalToApp,
        filteredAppToTerminal);
    RespondNegotiateMethods(connectionId, id, filteredTerminalToApp, filteredAppToTerminal);
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
            m_subscribed_method[connectionId].insert(msg.asString());
        }
        else
        {
            return JsonRpcStatus::INVALID_PARAMS;
        }
    }
    m_sessionCallback->RequestSubscribe(connectionId, id, msgTypeBoolList[0],
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
            m_subscribed_method[connectionId].erase(msg.asString());
        }
        else
        {
            return JsonRpcStatus::INVALID_PARAMS;
        }
    }
    m_sessionCallback->RequestUnsubscribe(connectionId, id, msgTypeBoolList[0],
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

JsonRpcService::JsonRpcStatus JsonRpcService::RequestTriggerResponseToUserAction(int connectionId,
    const Json::Value &obj)
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
    bool ready = obj["params"]["ready"].asBool();
    m_sessionCallback->NotifyVoiceReady(connectionId, ready);
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
    if (state == "buffering" || state == "paused" || state == "playing" ||
        state == "stopped")
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
    if (state == "buffering" || state == "paused" || state == "playing" ||
        state == "stopped")
    {
        if (type == OPTIONAL_STR_NOT_SET)
        {
            return JsonRpcStatus::NOTIFICATION_ERROR;
        }
    }

    std::string currentTimeStr;
    if (HasParam(params, "currentTime", Json::stringValue) ||
        HasParam(params, "currentTime", Json::intValue) ||
        HasParam(params, "currentTime", Json::uintValue) ||
        HasParam(params, "currentTime", Json::realValue))
    {
        currentTimeStr = EncodeJsonId(params["currentTime"]);
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
        if (HasParam(params["range"], "start", Json::stringValue) ||
            HasParam(params["range"], "start", Json::intValue) ||
            HasParam(params["range"], "start", Json::uintValue) ||
            HasParam(params["range"], "start", Json::realValue))
        {
            rangeStart = EncodeJsonId(params["range"]["start"]);
        }
        if (HasParam(params["range"], "end", Json::stringValue) ||
            HasParam(params["range"], "end", Json::intValue) ||
            HasParam(params["range"], "end", Json::uintValue) ||
            HasParam(params["range"], "end", Json::realValue))
        {
            rangeEnd = EncodeJsonId(params["range"]["end"]);
        }
        if (params["range"]["start"].type() != params["range"]["end"].type() ||
            params["range"]["start"].type() != params["currentTime"].type())
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

    bool actPause = false;
    bool actPlay = false;
    bool actFastForward = false;
    bool actFastReverse = false;
    bool actStop = false;
    bool actSeekContent = false;
    bool actSeekRelative = false;
    bool actSeekLive = false;
    bool actWallclock = false;

    if (!HasJsonParam(params, "availableActions"))
    {
        return JsonRpcStatus::NOTIFICATION_ERROR;
    }

    Json::Value actions = params["availableActions"];
    if (HasParam(actions, "pause", Json::booleanValue) &&
        actions["pause"] == true)
    {
        actPause = true;
    }

    if (HasParam(actions, "play", Json::booleanValue) &&
        actions["play"] == true)
    {
        actPlay = true;
    }

    if (HasParam(actions, "fast-forward", Json::booleanValue) &&
        actions["fast-forward"] == true)
    {
        actFastForward = true;
    }

    if (HasParam(actions, "fast-reverse", Json::booleanValue) &&
        actions["fast-reverse"] == true)
    {
        actFastReverse = true;
    }
    if (HasParam(actions, "stop", Json::booleanValue) &&
        actions["stop"] == true)
    {
        actStop = true;
    }

    if (HasParam(actions, "seek-content", Json::booleanValue) &&
        actions["seek-content"] == true)
    {
        actSeekContent = true;
    }

    if (HasParam(actions, "seek-relative", Json::booleanValue) &&
        actions["seek-relative"] == true)
    {
        actSeekRelative = true;
    }

    if (HasParam(actions, "seek-live", Json::booleanValue) &&
        actions["seek-live"] == true)
    {
        actSeekLive = true;
    }

    if (HasParam(actions, "seek-wallclock", Json::booleanValue) &&
        actions["seek-wallclock"] == true)
    {
        actWallclock = true;
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
    if (state == "buffering" || state == "paused" || state == "playing" ||
        state == "stopped")
    {
        if (title == OPTIONAL_STR_NOT_SET ||
            mediaId == OPTIONAL_STR_NOT_SET ||
            secTitle == OPTIONAL_STR_NOT_SET ||
            secTitle == OPTIONAL_STR_NOT_SET)
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

    m_sessionCallback->NotifyStateMedia(connectionId,
        state, kind, type, currentTimeStr,
        rangeStart, rangeEnd,
        actPause, actPlay, actFastForward,
        actFastReverse, actStop, actSeekContent,
        actSeekRelative, actSeekLive, actWallclock,
        mediaId, title, secTitle, synopsis,
        subtitlesEnabled, subtitlesAvailable,
        audioDescripEnabled, audioDescripAvailable,
        signLangEnabled, signLangAvailable);
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

JsonRpcService::JsonRpcStatus JsonRpcService::ReceiveError(int connectionId, const Json::Value &obj)
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
        m_sessionCallback->ReceiveError(connectionId, id, code, message, method, data);
    }
    else
    {
        m_sessionCallback->ReceiveError(connectionId, id, code, message);
    }
    return JsonRpcStatus::SUCCESS;
}

void JsonRpcService::RespondFeatureSupportInfo(int connectionId, const std::string& id,
    int featureId, const std::string& value)
{
    Json::Value result;

    result["method"] = MD_AF_FEATURE_SUPPORT_INFO;
    if (featureId > -1 && featureId < sizeOfAccessibilityFeature)
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

void JsonRpcService::RespondFeatureSettingsHighContrastUI(int connectionId, const std::string &id,
    bool enabled, const std::string &hcType)
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

void JsonRpcService::RespondFeatureSettingsScreenReader(int connectionId, const std::string &id,
    bool enabled, int speed,
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
    bool enabled, int gainPreference,
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
    int featureId, const std::string &value)
{
    Json::Value result;
    result["method"] = MD_AF_FEATURE_SUPPRESS;
    if (featureId > -1 && featureId < sizeOfAccessibilityFeature)
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
    const std::string &terminalToApp,
    const std::string &appToTerminal)
{
    Json::Value result;
    result["method"] = MD_NEGOTIATE_METHODS;
    // terminalToApp methods
    Json::Value terminalToAppJsonArray(Json::arrayValue);
    terminalToAppJsonArray = CreateNegotiatedMethods(terminalToApp);
    result["terminalToApp"] = terminalToAppJsonArray;
    // appToTerminal methods
    Json::Value appToTerminalJsonArray(Json::arrayValue);
    appToTerminalJsonArray = CreateNegotiatedMethods(appToTerminal);
    result["appToTerminal"] = appToTerminalJsonArray;
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

void JsonRpcService::SendIntentMediaPause(int connectionId, const std::string &id,
    const std::string &origin)
{
    if (!CheckMethodInNegotiatedMap("terminalToApp", connectionId, MD_INTENT_MEDIA_PAUSE))
    {
        return;
    }
    Json::Value params;
    params["origin"] = origin;
    Json::Value response = CreateJsonResponse(id, MD_INTENT_MEDIA_PAUSE, params);
    SendJsonMessageToClient(connectionId, __func__, response);
}

void JsonRpcService::SendIntentMediaPlay(int connectionId, const std::string &id,
    const std::string &origin)
{
    if (!CheckMethodInNegotiatedMap("terminalToApp", connectionId, MD_INTENT_MEDIA_PLAY))
    {
        return;
    }
    Json::Value params;
    params["origin"] = origin;
    Json::Value response = CreateJsonResponse(id, MD_INTENT_MEDIA_PLAY, params);
    SendJsonMessageToClient(connectionId, __func__, response);
}

void JsonRpcService::SendIntentMediaFastForward(int connectionId, const std::string &id,
    const std::string &origin)
{
    if (!CheckMethodInNegotiatedMap("terminalToApp", connectionId, MD_INTENT_MEDIA_FAST_FORWARD))
    {
        return;
    }
    Json::Value params;
    params["origin"] = origin;
    Json::Value response = CreateJsonResponse(id, MD_INTENT_MEDIA_FAST_FORWARD, params);
    SendJsonMessageToClient(connectionId, __func__, response);
}

void JsonRpcService::SendIntentMediaFastReverse(int connectionId, const std::string &id,
    const std::string &origin)
{
    if (!CheckMethodInNegotiatedMap("terminalToApp", connectionId, MD_INTENT_MEDIA_FAST_REVERSE))
    {
        return;
    }
    Json::Value params;
    params["origin"] = origin;
    Json::Value response = CreateJsonResponse(id, MD_INTENT_MEDIA_FAST_REVERSE, params);
    SendJsonMessageToClient(connectionId, __func__, response);
}

void JsonRpcService::SendIntentMediaStop(int connectionId, const std::string &id,
    const std::string &origin)
{
    if (!CheckMethodInNegotiatedMap("terminalToApp", connectionId, MD_INTENT_MEDIA_STOP))
    {
        return;
    }
    Json::Value params;
    params["origin"] = origin;
    Json::Value response = CreateJsonResponse(id, MD_INTENT_MEDIA_STOP, params);
    SendJsonMessageToClient(connectionId, __func__, response);
}

void JsonRpcService::SendIntentMediaSeekContent(int connectionId, const std::string &id,
    const std::string &origin,
    const std::string &anchor, int offset)
{
    if (!CheckMethodInNegotiatedMap("terminalToApp", connectionId, MD_INTENT_MEDIA_SEEK_CONTENT))
    {
        return;
    }
    Json::Value params;
    params["origin"] = origin;
    params["anchor"] = anchor;
    params["offset"] = offset;
    Json::Value response = CreateJsonResponse(id, MD_INTENT_MEDIA_SEEK_CONTENT, params);
    SendJsonMessageToClient(connectionId, __func__, response);
}

void JsonRpcService::SendIntentMediaSeekRelative(int connectionId, const std::string &id,
    const std::string &origin, int offset)
{
    if (!CheckMethodInNegotiatedMap("terminalToApp", connectionId, MD_INTENT_MEDIA_SEEK_RELATIVE))
    {
        return;
    }
    Json::Value params;
    params["origin"] = origin;
    params["offset"] = offset;
    Json::Value response = CreateJsonResponse(id, MD_INTENT_MEDIA_SEEK_RELATIVE, params);
    SendJsonMessageToClient(connectionId, __func__, response);
}

void JsonRpcService::SendIntentMediaSeekLive(int connectionId, const std::string &id,
    const std::string &origin, int offset)
{
    if (!CheckMethodInNegotiatedMap("terminalToApp", connectionId, MD_INTENT_MEDIA_SEEK_LIVE))
    {
        return;
    }
    Json::Value params;
    params["origin"] = origin;
    params["offset"] = offset;
    Json::Value response = CreateJsonResponse(id, MD_INTENT_MEDIA_SEEK_LIVE, params);
    SendJsonMessageToClient(connectionId, __func__, response);
}

void JsonRpcService::SendIntentMediaSeekWallclock(int connectionId, const std::string &id,
    const std::string &origin,
    const std::string &dateTime)
{
    if (!CheckMethodInNegotiatedMap("terminalToApp", connectionId, MD_INTENT_MEDIA_SEEK_WALLCLOCK))
    {
        return;
    }
    Json::Value params;
    params["origin"] = origin;
    params["date-time"] = dateTime;
    Json::Value response = CreateJsonResponse(id, MD_INTENT_MEDIA_SEEK_WALLCLOCK, params);
    SendJsonMessageToClient(connectionId, __func__, response);
}

void JsonRpcService::SendIntentSearch(int connectionId, const std::string &id,
    const std::string &origin, const std::string &query)
{
    if (!CheckMethodInNegotiatedMap("terminalToApp", connectionId, MD_INTENT_SEARCH))
    {
        return;
    }
    Json::Value params;
    params["origin"] = origin;
    params["query"] = query;
    Json::Value response = CreateJsonResponse(id, MD_INTENT_SEARCH, params);
    SendJsonMessageToClient(connectionId, __func__, response);
}

void JsonRpcService::SendIntentDisplay(int connectionId, const std::string &id,
    const std::string &origin, const std::string &mediaId)
{
    if (!CheckMethodInNegotiatedMap("terminalToApp", connectionId, MD_INTENT_DISPLAY))
    {
        return;
    }
    Json::Value params;
    params["origin"] = origin;
    params["mediaId"] = mediaId;
    Json::Value response = CreateJsonResponse(id, MD_INTENT_DISPLAY, params);
    SendJsonMessageToClient(connectionId, __func__, response);
}

void JsonRpcService::SendIntentPlayback(int connectionId, const std::string &id,
    const std::string &origin, const std::string &mediaId,
    const std::string &anchor, int offset)
{
    if (!CheckMethodInNegotiatedMap("terminalToApp", connectionId, MD_INTENT_PLAYBACK))
    {
        return;
    }
    Json::Value params;
    params["origin"] = origin;
    params["mediaId"] = mediaId;
    if (anchor != OPTIONAL_STR_NOT_SET)
    {
        params["anchor"] = anchor;
    }
    if (offset != OPTIONAL_INT_NOT_SET)
    {
        params["offset"] = offset;
    }
    Json::Value response = CreateJsonResponse(id, MD_INTENT_PLAYBACK, params);
    SendJsonMessageToClient(connectionId, __func__, response);
}

void JsonRpcService::NotifySubtitles(bool enabled,
    int size, const std::string &fontFamily,
    const std::string &textColour, int textOpacity,
    const std::string &edgeType, const std::string &edgeColour,
    const std::string &backgroundColour, int backgroundOpacity,
    const std::string &windowColour, int windowOpacity,
    const std::string &language)
{
    std::vector<int> subscribedConnectionIds;
    GetSubscribedConnectionIds(subscribedConnectionIds, 0);
    if (subscribedConnectionIds.empty())
    {
        return;
    }

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
    Json::Value response = CreateNotifyRequest(params);
    for (int connectionId : subscribedConnectionIds)
    {
        SendJsonMessageToClient(connectionId, __func__, response);
    }
}

void JsonRpcService::NotifyDialogueEnhancement(
    int dialogueEnhancementGainPreference,
    int dialogueEnhancementGain,
    int dialogueEnhancementLimitMin,
    int dialogueEnhancementLimitMax)
{
    std::vector<int> subscribedConnectionIds;
    GetSubscribedConnectionIds(subscribedConnectionIds, 1);
    if (subscribedConnectionIds.empty())
    {
        return;
    }
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
    Json::Value response = CreateNotifyRequest(params);
    for (int connectionId : subscribedConnectionIds)
    {
        SendJsonMessageToClient(connectionId, __func__, response);
    }
}

void JsonRpcService::NotifyUIMagnifier(bool enabled, const std::string &magType)
{
    std::vector<int> subscribedConnectionIds;
    GetSubscribedConnectionIds(subscribedConnectionIds, 2);
    if (subscribedConnectionIds.empty())
    {
        return;
    }
    Json::Value params;
    params["msgType"] = PC_UI_MAGNIFIER;
    Json::Value value;
    value["enabled"] = enabled;
    if (magType != OPTIONAL_STR_NOT_SET)
    {
        value["magType"] = magType;
    }
    params["value"] = value;
    Json::Value response = CreateNotifyRequest(params);
    for (int connectionId : subscribedConnectionIds)
    {
        SendJsonMessageToClient(connectionId, __func__, response);
    }
}

void JsonRpcService::NotifyHighContrastUI(bool enabled,
    const std::string &hcType)
{
    std::vector<int> subscribedConnectionIds;
    GetSubscribedConnectionIds(subscribedConnectionIds, 3);
    if (subscribedConnectionIds.empty())
    {
        return;
    }
    Json::Value params;
    params["msgType"] = PC_HIGH_CONTRAST_UI;
    Json::Value value;
    value["enabled"] = enabled;
    if (hcType != OPTIONAL_STR_NOT_SET)
    {
        value["hcType"] = hcType;
    }
    params["value"] = value;
    Json::Value response = CreateNotifyRequest(params);
    for (int connectionId : subscribedConnectionIds)
    {
        SendJsonMessageToClient(connectionId, __func__, response);
    }
}

void JsonRpcService::NotifyScreenReader(bool enabled,
    int speed, const std::string &voice,
    const std::string &language)
{
    std::vector<int> subscribedConnectionIds;
    GetSubscribedConnectionIds(subscribedConnectionIds, 4);
    if (subscribedConnectionIds.empty())
    {
        return;
    }
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
    Json::Value response = CreateNotifyRequest(params);
    for (int connectionId : subscribedConnectionIds)
    {
        SendJsonMessageToClient(connectionId, __func__, response);
    }
}

void JsonRpcService::NotifyResponseToUserAction(bool enabled,
    const std::string &type)
{
    std::vector<int> subscribedConnectionIds;
    GetSubscribedConnectionIds(subscribedConnectionIds, 5);
    if (subscribedConnectionIds.empty())
    {
        return;
    }
    Json::Value params;
    params["msgType"] = PC_RESPONSE_TO_USER_ACTION;
    Json::Value value;
    value["enabled"] = enabled;
    if (type != OPTIONAL_STR_NOT_SET)
    {
        value["type"] = type;
    }
    params["value"] = value;
    Json::Value response = CreateNotifyRequest(params);
    for (int connectionId : subscribedConnectionIds)
    {
        SendJsonMessageToClient(connectionId, __func__, response);
    }
}

void JsonRpcService::NotifyAudioDescription(bool enabled,
    int gainPreference, int panAzimuthPreference)
{
    std::vector<int> subscribedConnectionIds;
    GetSubscribedConnectionIds(subscribedConnectionIds, 6);
    if (subscribedConnectionIds.empty())
    {
        return;
    }
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
    Json::Value response = CreateNotifyRequest(params);
    for (int connectionId : subscribedConnectionIds)
    {
        SendJsonMessageToClient(connectionId, __func__, response);
    }
}

void JsonRpcService::NotifyInVisionSigning(bool enabled)
{
    std::vector<int> subscribedConnectionIds;
    GetSubscribedConnectionIds(subscribedConnectionIds, 7);
    if (subscribedConnectionIds.empty())
    {
        return;
    }
    Json::Value params;
    params["msgType"] = PC_IN_VISION_SIGNING;
    Json::Value value;
    value["enabled"] = enabled;
    params["value"] = value;
    Json::Value response = CreateNotifyRequest(params);
    for (int connectionId : subscribedConnectionIds)
    {
        SendJsonMessageToClient(connectionId, __func__, response);
    }
}

void JsonRpcService::RespondDialogueEnhancementOverride(int connectionId, const std::string &id,
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

void JsonRpcService::RespondTriggerResponseToUserAction(int connectionId, const std::string &id,
    bool actioned)
{
    Json::Value result;
    result["method"] = MD_AF_TRIGGER_RESPONSE_TO_USER_ACTION;
    result["actioned"] = actioned;
    Json::Value response = CreateJsonResponse(id, result);
    SendJsonMessageToClient(connectionId, __func__, response);
}

// Helper functions
void JsonRpcService::GetSubscribedConnectionIds(std::vector<int> &subscribedConnectionIds, const
    int msgTypeIndex)
{
    std::string suffix = "PrefChange";
    std::string msgType = ACCESSIBILITY_FEATURE_NAMES.at(msgTypeIndex) + suffix;
    for (auto & i : m_subscribed_method)
    {
        if (i.second.find(msgType) != i.second.end())
        {
            subscribedConnectionIds.push_back(i.first);
        }
    }
}

void JsonRpcService::RegisterMethod(const std::string &name, JsonRpcMethod method)
{
    m_json_rpc_methods[name] = std::bind(method, this, std::placeholders::_1,
        std::placeholders::_2);
}

void JsonRpcService::SendJsonMessageToClient(int connectionId, const std::string &responseName,
    const Json::Value &jsonResponse)
{
    Json::FastWriter writer;
    std::string message = writer.write(jsonResponse);
    connections_mutex_.lock();
    WebSocketConnection *connection = GetConnection(connectionId);
    if (connection != nullptr)
    {
        std::ostringstream oss;
        oss << "response=" << responseName << "|" << message;
        connection->SendMessage(oss.str());
    }
    connections_mutex_.unlock();
}

bool HasParam(const Json::Value &json, const std::string &param,
    const Json::ValueType& type)
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

/**
 * Reset styles and remove the method not in table
 *
 * @param connectionId The connection ID.
 * @param oldString The string of methods in json style.
 * @param direction The string, should be "terminalToApp" or "appToTerminal".
 *
 * @return The string of filtered methods and separated by ","
 */
std::string JsonRpcService::ResetStylesFilterMethods(int connectionId, std::string &oldString, const
    std::string &direction)
{
    oldString.erase(std::remove(oldString.begin(), oldString.end(), '['), oldString.end());
    oldString.erase(std::remove(oldString.begin(), oldString.end(), ']'), oldString.end());
    oldString.erase(std::remove(oldString.begin(), oldString.end(), '\t'), oldString.end());
    oldString.erase(std::remove(oldString.begin(), oldString.end(), '\n'), oldString.end());
    oldString.erase(std::remove(oldString.begin(), oldString.end(), '"'), oldString.end());

    std::string newString;
    std::stringstream ss(oldString);
    std::string str;
    while (std::getline(ss, str, ','))
    {
        if (CheckMethodInSupportedSet(direction, str))
        {
            if (direction == "appToTerminal")
            {
                m_negotiate_methods_app_to_terminal[connectionId].insert(str);
            }
            else if (direction == "terminalToApp")
            {
                m_negotiate_methods_terminal_to_app[connectionId].insert(str);
            }
            if (!newString.empty())
            {
                newString += ",";
            }
            newString += str;
        }
    }
    return newString;
}

bool JsonRpcService::CheckMethodInNegotiatedMap(const std::string& direction, int connectionId,
    const std::string& method)
{
    if (direction == "appToTerminal")
    {
        if (m_negotiate_methods_app_to_terminal[connectionId].find(method) ==
            m_negotiate_methods_app_to_terminal[connectionId].end())
        {
            return false;
        }
    }
    else if (direction == "terminalToApp")
    {
        if (m_negotiate_methods_terminal_to_app[connectionId].find(method) ==
            m_negotiate_methods_terminal_to_app[connectionId].end())
        {
            return false;
        }
    }
    return true;
}

bool JsonRpcService::CheckMethodInSupportedSet(const std::string& direction,
    const std::string& method)
{
    if (direction == "appToTerminal")
    {
        if (m_supported_methods_app_to_terminal.find(method) ==
            m_supported_methods_app_to_terminal.end())
        {
            return false;
        }
    }
    else if (direction == "terminalToApp")
    {
        if (m_supported_methods_terminal_to_app.find(method) ==
            m_supported_methods_terminal_to_app.end())
        {
            return false;
        }
    }
    return true;
}

// Static functions

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

Json::Value CreateNegotiatedMethods(const std::string& stringList)
{
    // Parse the input string
    Json::Value jsonArray(Json::arrayValue);
    if (stringList.empty())
    {
        return jsonArray;
    }
    size_t startPos = 0;
    size_t endPos = stringList.find(',');
    while (endPos != std::string::npos)
    {
        std::string element = stringList.substr(startPos, endPos - startPos);
        jsonArray.append(element);
        startPos = endPos + 1;          // Move past the comma and space
        endPos = stringList.find(',', startPos);
    }
    // Append the last element
    std::string lastElement = stringList.substr(startPos,
        stringList.length() - startPos);
    jsonArray.append(lastElement);
    return jsonArray;
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