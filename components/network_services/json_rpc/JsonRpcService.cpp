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

    void JsonRpcService::registerMethod(const std::string &name, JsonRpcMethod method) {
        m_json_rpc_methods[name] = std::bind(method, this, std::placeholders::_1,
                                             std::placeholders::_2);
    }

    std::map<std::string, int> mapOfFeatures;

    JsonRpcService::JsonRpcService(
            int port,
            const std::string &endpoint,
            std::unique_ptr<SessionCallback> sessionCallback) :
            WebSocketService("JsonRpcService", port, false, "lo"),
            m_endpoint(endpoint),
            m_sessionCallback(std::move(sessionCallback)) {

        mapOfFeatures[F_SUBTITLES] = 0;
        mapOfFeatures[F_DIALOGUE_ENHANCEMENT] = 1;
        mapOfFeatures[F_UI_MAGNIFIER] = 2;
        mapOfFeatures[F_HIGH_CONTRAST_UI] = 3;
        mapOfFeatures[F_SCREEN_READER] = 4;
        mapOfFeatures[F_RESPONSE_TO_USER_ACTION] = 5;
        mapOfFeatures[F_AUDIO_DESCRIPTION] = 6;
        mapOfFeatures[F_IN_VISION_SIGNING] = 7;

        registerMethod(MD_NEGOTIATE_METHODS, &JsonRpcService::RequestNegotiateMethods);
        registerMethod(MD_SUBSCRIBE, &JsonRpcService::RequestSubscribe);
        registerMethod(MD_UNSUBSCRIBE, &JsonRpcService::RequestUnsubscribe);

        registerMethod(MD_AF_FEATURE_SUPPORT_INFO, &JsonRpcService::RequestFeatureSupportInfo);
        registerMethod(MD_AF_FEATURE_SETTINGS_QUERY, &JsonRpcService::RequestFeatureSettingsQuery);
        registerMethod(MD_AF_FEATURE_SUPPRESS, &JsonRpcService::RequestFeatureSuppress);

        registerMethod(MD_AF_DIALOGUE_ENHANCEMENT_OVERRIDE,
                       &JsonRpcService::RequestDialogueEnhancementOverride);
        registerMethod(MD_AF_TRIGGER_RESPONSE_TO_USER_ACTION,
                       &JsonRpcService::RequestTriggerResponseToUserAction);

        registerMethod(MD_VOICE_READY, &JsonRpcService::NotifyVoiceReady);
        registerMethod(MD_STATE_MEDIA, &JsonRpcService::NotifyStateMedia);
        registerMethod(MD_INTENT_MEDIA_PAUSE, &JsonRpcService::ReceiveIntentConfirm);
        registerMethod(MD_INTENT_MEDIA_PLAY, &JsonRpcService::ReceiveIntentConfirm);
        registerMethod(MD_INTENT_MEDIA_FAST_FORWARD, &JsonRpcService::ReceiveIntentConfirm);
        registerMethod(MD_INTENT_MEDIA_FAST_REVERSE, &JsonRpcService::ReceiveIntentConfirm);
        registerMethod(MD_INTENT_MEDIA_STOP, &JsonRpcService::ReceiveIntentConfirm);
        registerMethod(MD_INTENT_MEDIA_SEEK_CONTENT, &JsonRpcService::ReceiveIntentConfirm);
        registerMethod(MD_INTENT_MEDIA_SEEK_RELATIVE, &JsonRpcService::ReceiveIntentConfirm);
        registerMethod(MD_INTENT_MEDIA_SEEK_LIVE, &JsonRpcService::ReceiveIntentConfirm);
        registerMethod(MD_INTENT_MEDIA_SEEK_WALLCLOCK, &JsonRpcService::ReceiveIntentConfirm);
        registerMethod(MD_INTENT_SEARCH, &JsonRpcService::ReceiveIntentConfirm);
        registerMethod(MD_INTENT_DISPLAY, &JsonRpcService::ReceiveIntentConfirm);
        registerMethod(MD_INTENT_PLAYBACK, &JsonRpcService::ReceiveIntentConfirm);

        LOG(LOG_INFO, "Start");
        Start();

    }

    bool JsonRpcService::OnConnection(WebSocketConnection *connection) {
        if (connection->Uri() != m_endpoint) {
            LOG(LOG_INFO, "Unknown endpoint received. Got: %s, expected: %s",
                connection->Uri().c_str(), m_endpoint.c_str());
            return false;
        }
        LOG(LOG_INFO, "Connected: connectionId=%d", connection->Id());
        return true;
    }

    void
    JsonRpcService::OnMessageReceived(WebSocketConnection *connection, const std::string &text) {
        LOG(LOG_INFO, "Message received: connection=%d, text=%s", connection->Id(), text.c_str());
        // Parse request
        Json::Value obj;
        Json::Reader reader;
        JsonRpcStatus status;

        if (reader.parse(text, obj)) {
            if (HasParam(obj, "jsonrpc", Json::stringValue)
                && obj["jsonrpc"] == "2.0") {

                if (HasJsonParam(obj, "error")) {
                    status = JsonRpcService::ReceiveError(connection->Id(), obj);
                }
                else {
                    bool hasMethod = true;
                    std::string method = "";
                    if (HasParam(obj, "method", Json::stringValue)) {
                        method = obj["method"].asString();
                    }
                    else if (HasJsonParam(obj, "result")
                             && HasParam(obj["result"], "method", Json::stringValue)) {
                        method = obj["result"]["method"].asString();
                    }
                    else {
                        LOG(LOG_INFO, "Error, Invalid params");
                        status = JsonRpcStatus::INVALID_PARAMS;
                    }
                    if (method != "") {
                        auto it = m_json_rpc_methods.find(method);
                        if (it != m_json_rpc_methods.end()) {
                            status = it->second(connection->Id(), obj);
                        }
                        else {
                            LOG(LOG_INFO, "Error, Method not found");
                            status = JsonRpcStatus::METHOD_NOT_FOUND;
                        }
                    }
                }
            }
            else {
                LOG(LOG_INFO, "Error, Invalid Request");
                status = JsonRpcStatus::INVALID_REQUEST;
            }
        }
        else {
            LOG(LOG_INFO, "Error, json rpc parse wrong");
            status = JsonRpcStatus::PARSE_ERROR;
        }

        if (status != JsonRpcStatus::SUCCESS) {
            CreateJsonRpcErrorObject(connection->Id(), obj, status);
        }
    }

    //Helper functions

    JsonRpcService::JsonRpcStatus
    JsonRpcService::ReceiveError(int connectionId, const Json::Value &obj) {
        Json::Value error = obj["error"];
        if (!HasParam(obj, "id", Json::stringValue)
            && !HasParam(obj, "id", Json::intValue)
            && !HasParam(obj, "id", Json::uintValue))
            return JsonRpcStatus::INVALID_PARAMS;
        std::string id = AddDataTypeIdentify(obj["id"]);

        if (!HasParam(error, "code", Json::intValue))
            return JsonRpcStatus::INVALID_PARAMS;

        int code = error["code"].asInt();
        std::string message = OPTIONAL_STR_NOT_SET;
        if (HasParam(error, "message", Json::stringValue))
            message = error["message"].asString();

        std::string data = OPTIONAL_STR_NOT_SET;
        if (HasParam(error, "data", Json::stringValue))
            message = error["data"].asString();

        LOG(LOG_INFO, "JSON-RPC-EXAMPLE #2: Service received request. Call session callback...");
        m_sessionCallback->ReceiveError(connectionId, id, code, message);
        return JsonRpcStatus::SUCCESS;
    }

    JsonRpcService::JsonRpcStatus
    JsonRpcService::RequestNegotiateMethods(int connectionId, const Json::Value &obj) {
        if (!HasParam(obj, "id", Json::stringValue)
            && !HasParam(obj, "id", Json::intValue)
            && !HasParam(obj, "id", Json::uintValue))
            return JsonRpcStatus::INVALID_PARAMS;
        std::string id = AddDataTypeIdentify(obj["id"]);

        if (!HasJsonParam(obj, "params"))
            return JsonRpcStatus::INVALID_PARAMS;
        if (!HasParam(obj["params"], "terminalToApp", Json::arrayValue))
            return JsonRpcStatus::INVALID_PARAMS;
        if (!HasParam(obj["params"], "appToTerminal", Json::arrayValue))
            return JsonRpcStatus::INVALID_PARAMS;

        std::string terminalToApp = OPTIONAL_STR_NOT_SET;
        terminalToApp = obj["params"]["terminalToApp"].toStyledString();
        std::string appToTerminal = OPTIONAL_STR_NOT_SET;
        appToTerminal = obj["params"]["appToTerminal"].toStyledString();


        // Remove newline characters
        size_t pos;
        while ((pos = appToTerminal.find('\n')) != std::string::npos) {
            appToTerminal.erase(pos, 2);
        }
        // Remove newline characters
        while ((pos = terminalToApp.find('\n')) != std::string::npos) {
            terminalToApp.erase(pos, 2);
        }

        // Remove square brackets
        appToTerminal.erase(appToTerminal.begin(), appToTerminal.begin() + 1);
        // Remove square brackets
        terminalToApp.erase(terminalToApp.begin(), terminalToApp.begin() + 1);

        LOG(LOG_INFO, "JSON-RPC-EXAMPLE #2: Service received request. Call session callback...");

        m_sessionCallback->RequestNegotiateMethods(connectionId, id, terminalToApp,
                                                   appToTerminal);
        return JsonRpcStatus::SUCCESS;
    }

    JsonRpcService::JsonRpcStatus
    JsonRpcService::RequestSubscribe(int connectionId, const Json::Value &obj) {
        if (!HasParam(obj, "id", Json::stringValue)
            && !HasParam(obj, "id", Json::intValue)
            && !HasParam(obj, "id", Json::uintValue))
            return JsonRpcStatus::INVALID_PARAMS;
        std::string id = AddDataTypeIdentify(obj["id"]);

        if (!HasJsonParam(obj, "params"))
            return JsonRpcStatus::INVALID_PARAMS;
        if (!HasParam(obj["params"], "msgType", Json::arrayValue)) {
            return JsonRpcStatus::INVALID_PARAMS;
        }
        Json::Value msgType = obj["params"]["msgType"];

        bool msgTypeBoolList[8] = {false};
        for (auto msg: msgType) {
            int length = msg.asString().length();
            std::string s = msg.asString().substr(0, length - 10);  //remove the PrefChange
            if (mapOfFeatures.find(s) != mapOfFeatures.end()) {
                msgTypeBoolList[mapOfFeatures[s]] = true;
            }
            else {
                return JsonRpcStatus::INVALID_PARAMS;
            }
        }
        LOG(LOG_INFO, "JSON-RPC-EXAMPLE #2: Service received request. Call session callback...");
        m_sessionCallback->RequestSubscribe(connectionId, id, msgTypeBoolList[0],
                                            msgTypeBoolList[1], msgTypeBoolList[2],
                                            msgTypeBoolList[3], msgTypeBoolList[4],
                                            msgTypeBoolList[5], msgTypeBoolList[6],
                                            msgTypeBoolList[7]);
        return JsonRpcStatus::SUCCESS;
    }

    JsonRpcService::JsonRpcStatus
    JsonRpcService::RequestUnsubscribe(int connectionId, const Json::Value &obj) {
        if (!HasParam(obj, "id", Json::stringValue)
            && !HasParam(obj, "id", Json::intValue)
            && !HasParam(obj, "id", Json::uintValue))
            return JsonRpcStatus::INVALID_PARAMS;
        std::string id = AddDataTypeIdentify(obj["id"]);

        if (!HasJsonParam(obj, "params"))
            return JsonRpcStatus::INVALID_PARAMS;
        if (!HasParam(obj["params"], "msgType", Json::arrayValue)) {
            return JsonRpcStatus::INVALID_PARAMS;
        }
        Json::Value msgType = obj["params"]["msgType"];

        bool msgTypeBoolList[8] = {false};
        for (auto msg: msgType) {
            int length = msg.asString().length();
            std::string s = msg.asString().substr(0, length - 10);  //remove the PrefChange
            if (mapOfFeatures.find(s) != mapOfFeatures.end()) {
                msgTypeBoolList[mapOfFeatures[s]] = true;
            }
            else {
                return JsonRpcStatus::INVALID_PARAMS;
            }
        }
        LOG(LOG_INFO, "JSON-RPC-EXAMPLE #2: Service received request. Call session callback...");
        m_sessionCallback->RequestUnsubscribe(connectionId, id, msgTypeBoolList[0],
                                              msgTypeBoolList[1], msgTypeBoolList[2],
                                              msgTypeBoolList[3], msgTypeBoolList[4],
                                              msgTypeBoolList[5], msgTypeBoolList[6],
                                              msgTypeBoolList[7]);
        return JsonRpcStatus::SUCCESS;
    }

    JsonRpcService::JsonRpcStatus
    JsonRpcService::RequestFeatureSupportInfo(int connectionId, const Json::Value &obj) {
        if (!HasParam(obj, "id", Json::stringValue)
            && !HasParam(obj, "id", Json::intValue)
            && !HasParam(obj, "id", Json::uintValue))
            return JsonRpcStatus::INVALID_PARAMS;
        std::string id = AddDataTypeIdentify(obj["id"]);
        if (!HasJsonParam(obj, "params"))
            return JsonRpcStatus::INVALID_PARAMS;
        if (!HasParam(obj["params"], "feature", Json::stringValue)) {
            return JsonRpcStatus::INVALID_PARAMS;
        }

        std::string feature = obj["params"]["feature"].asString();
        if (mapOfFeatures.find(feature) == mapOfFeatures.end()) {
            return JsonRpcStatus::INVALID_PARAMS;
        }
        LOG(LOG_INFO, "JSON-RPC-EXAMPLE #2: Service received request. Call session callback...");
        m_sessionCallback->RequestFeatureSupportInfo(connectionId, id,
                                                     mapOfFeatures[feature]);
        return JsonRpcStatus::SUCCESS;
    }

    JsonRpcService::JsonRpcStatus
    JsonRpcService::RequestFeatureSettingsQuery(int connectionId, const Json::Value &obj) {
        if (!HasParam(obj, "id", Json::stringValue)
            && !HasParam(obj, "id", Json::intValue)
            && !HasParam(obj, "id", Json::uintValue))
            return JsonRpcStatus::INVALID_PARAMS;
        std::string id = AddDataTypeIdentify(obj["id"]);
        if (!HasJsonParam(obj, "params"))
            return JsonRpcStatus::INVALID_PARAMS;
        if (!HasParam(obj["params"], "feature", Json::stringValue)) {
            return JsonRpcStatus::INVALID_PARAMS;
        }

        std::string feature = obj["params"]["feature"].asString();
        if (mapOfFeatures.find(feature) == mapOfFeatures.end()) {
            return JsonRpcStatus::INVALID_PARAMS;
        }
        LOG(LOG_INFO, "JSON-RPC-EXAMPLE #2: Service received request. Call session callback...");
        m_sessionCallback->RequestFeatureSettingsQuery(connectionId, id,
                                                       mapOfFeatures[feature]);
        return JsonRpcStatus::SUCCESS;
    }

    JsonRpcService::JsonRpcStatus
    JsonRpcService::RequestFeatureSuppress(int connectionId, const Json::Value &obj) {
        if (!HasParam(obj, "id", Json::stringValue)
            && !HasParam(obj, "id", Json::intValue)
            && !HasParam(obj, "id", Json::uintValue))
            return JsonRpcStatus::INVALID_PARAMS;
        std::string id = AddDataTypeIdentify(obj["id"]);
        if (!HasJsonParam(obj, "params"))
            return JsonRpcStatus::INVALID_PARAMS;
        if (!HasParam(obj["params"], "feature", Json::stringValue)) {
            return JsonRpcStatus::INVALID_PARAMS;
        }

        std::string feature = obj["params"]["feature"].asString();
        if (mapOfFeatures.find(feature) == mapOfFeatures.end()) {
            return JsonRpcStatus::INVALID_PARAMS;
        }
        LOG(LOG_INFO, "JSON-RPC-EXAMPLE #2: Service received request. Call session callback...");
        m_sessionCallback->RequestFeatureSuppress(connectionId, id,
                                                  mapOfFeatures[feature]);
        return JsonRpcStatus::SUCCESS;
    }

    JsonRpcService::JsonRpcStatus
    JsonRpcService::RequestDialogueEnhancementOverride(int connectionId, const Json::Value &obj) {
        if (!HasParam(obj, "id", Json::stringValue)
            && !HasParam(obj, "id", Json::intValue)
            && !HasParam(obj, "id", Json::uintValue))
            return JsonRpcStatus::INVALID_PARAMS;
        std::string id = AddDataTypeIdentify(obj["id"]);

        int dialogueEnhancementGain = OPTIONAL_INT_NOT_SET;
        if (HasJsonParam(obj, "params")) {
            Json::Value params = obj["params"];
            if (HasParam(params, "dialogueEnhancementGain", Json::intValue))
                dialogueEnhancementGain = params["dialogueEnhancementGain"].asInt();
        }
        LOG(LOG_INFO, "JSON-RPC-EXAMPLE #2: Service received request. Call session callback...");
        m_sessionCallback->RequestDialogueEnhancementOverride(connectionId, id,
                                                              dialogueEnhancementGain);
        return JsonRpcStatus::SUCCESS;
    }

    JsonRpcService::JsonRpcStatus
    JsonRpcService::RequestTriggerResponseToUserAction(int connectionId, const Json::Value &obj) {
        if (!HasParam(obj, "id", Json::stringValue)
            && !HasParam(obj, "id", Json::intValue)
            && !HasParam(obj, "id", Json::uintValue))
            return JsonRpcStatus::INVALID_PARAMS;
        std::string id = AddDataTypeIdentify(obj["id"]);
        if (!HasJsonParam(obj, "params"))
            return JsonRpcStatus::INVALID_PARAMS;
        if (!HasParam(obj["params"], "magnitude", Json::stringValue))
            return JsonRpcStatus::INVALID_PARAMS;
        std::string magnitude = obj["params"]["magnitude"].asString();
        LOG(LOG_INFO, "JSON-RPC-EXAMPLE #2: Service received request. Call session callback...");
        m_sessionCallback->RequestTriggerResponseToUserAction(connectionId, id, magnitude);
        return JsonRpcStatus::SUCCESS;
    }

    JsonRpcService::JsonRpcStatus
    JsonRpcService::NotifyVoiceReady(int connectionId, const Json::Value &obj) {
        if (!HasJsonParam(obj, "params"))
            return JsonRpcStatus::INVALID_PARAMS;
        if (!HasParam(obj["params"], "ready", Json::booleanValue))
            return JsonRpcStatus::INVALID_PARAMS;
        bool ready = obj["params"]["ready"].asBool();
        LOG(LOG_INFO, "JSON-RPC-EXAMPLE #2: Service received request. Call session callback...");
        m_sessionCallback->NotifyVoiceReady(connectionId, ready);
        return JsonRpcStatus::SUCCESS;
    }

    JsonRpcService::JsonRpcStatus
    JsonRpcService::NotifyStateMedia(int connectionId, const Json::Value &obj) {
        if (!HasJsonParam(obj, "params"))
            return JsonRpcStatus::INVALID_PARAMS;
        Json::Value params = obj["params"];
        if (!HasParam(params, "state", Json::stringValue))
            return JsonRpcStatus::INVALID_PARAMS;

        std::string state = params["state"].asString();
        if (state != "no-media"
            && state != "error"
            && state != "buffering"
            && state != "paused"
            && state != "playing"
            && state != "stopped") {
            return JsonRpcStatus::INVALID_PARAMS;
        }
        std::string kind = OPTIONAL_STR_NOT_SET;
        if (state == "buffering" || state == "paused" || state == "playing"
            || state == "stopped") {
            if (!HasParam(params, "kind", Json::stringValue))
                return JsonRpcStatus::INVALID_PARAMS;
            kind = params["kind"].asString();
            if (kind != "audio" && kind != "audio-video") {
                return JsonRpcStatus::INVALID_PARAMS;
            }
        }

        std::string type = OPTIONAL_STR_NOT_SET;
        if (state == "buffering" || state == "paused" || state == "playing"
            || state == "stopped") {
            if (!HasParam(params, "type", Json::stringValue))
                return JsonRpcStatus::INVALID_PARAMS;
            type = params["type"].asString();
            if (type != "live" && type != "on-demand") {
                return JsonRpcStatus::INVALID_PARAMS;
            }
        }

        std::string currentTimeStr = OPTIONAL_STR_NOT_SET;
        if (state == "buffering" || state == "paused" || state == "playing") {
            if (!HasParam(params, "currentTime", Json::stringValue)
                && !HasParam(params, "currentTime", Json::intValue)
                && !HasParam(params, "currentTime", Json::uintValue)
                && !HasParam(params, "currentTime", Json::realValue))
                return JsonRpcStatus::INVALID_PARAMS;
            currentTimeStr = AddDataTypeIdentify(params["currentTime"]);
        }


        std::string rangeStart = OPTIONAL_STR_NOT_SET;
        std::string rangeEnd = OPTIONAL_STR_NOT_SET;
        if (state == "buffering" || state == "paused" || state == "playing") {
            if (!HasJsonParam(params, "range"))
                return JsonRpcStatus::INVALID_PARAMS;
            if (!HasParam(params["range"], "start", Json::stringValue)
                && !HasParam(params["range"], "start", Json::intValue)
                && !HasParam(params["range"], "start", Json::uintValue)
                && !HasParam(params["range"], "start", Json::realValue))
                return JsonRpcStatus::INVALID_PARAMS;
            if (!HasParam(params["range"], "end", Json::stringValue)
                && !HasParam(params["range"], "end", Json::intValue)
                && !HasParam(params["range"], "end", Json::uintValue)
                && !HasParam(params["range"], "end", Json::realValue))
                return JsonRpcStatus::INVALID_PARAMS;
            rangeStart = AddDataTypeIdentify(params["range"]["start"]);
            rangeEnd = AddDataTypeIdentify(params["range"]["end"]);
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
            return JsonRpcStatus::INVALID_PARAMS;

        Json::Value actions = params["availableActions"];
        if (HasParam(actions, "pause", Json::booleanValue)
            && actions["pause"] == true)
            actPause = true;

        if (HasParam(actions, "play", Json::booleanValue)
            && actions["play"] == true)
            actPlay = true;

        if (HasParam(actions, "fast-forward", Json::booleanValue)
            && actions["fast-forward"] == true)
            actFastForward = true;

        if (HasParam(actions, "fast-reverse", Json::booleanValue)
            && actions["fast-reverse"] == true)
            actFastReverse = true;
        if (HasParam(actions, "stop", Json::booleanValue)
            && actions["stop"] == true)
            actStop = true;

        if (HasParam(actions, "seek-content", Json::booleanValue)
            && actions["seek-content"] == true)
            actSeekContent = true;

        if (HasParam(actions, "seek-relative", Json::booleanValue)
            && actions["seek-relative"] == true)
            actSeekRelative = true;

        if (HasParam(actions, "seek-live", Json::booleanValue)
            && actions["seek-live"] == true)
            actSeekLive = true;

        if (HasParam(actions, "seek-wallclock", Json::booleanValue)
            && actions["seek-wallclock"] == true)
            actWallclock = true;

        std::string mediaId = OPTIONAL_STR_NOT_SET;
        std::string title = OPTIONAL_STR_NOT_SET;
        std::string secTitle = OPTIONAL_STR_NOT_SET;
        std::string synopsis = OPTIONAL_STR_NOT_SET;

        if (state == "buffering" || state == "paused" || state == "playing" ||
            state == "stopped") {

            if (!HasJsonParam(params, "metadata"))
                return JsonRpcStatus::INVALID_PARAMS;
            Json::Value metadata = params["metadata"];

            if (!HasParam(metadata, "title", Json::stringValue))
                return JsonRpcStatus::INVALID_PARAMS;
            title = metadata["title"].asString();

            if (HasParam(metadata, "mediaId", Json::stringValue))
                mediaId = metadata["mediaId"].asString();

            if (HasParam(metadata, "secondaryTitle", Json::stringValue))
                secTitle = metadata["secondaryTitle"].asString();

            if (HasParam(metadata, "synopsis", Json::stringValue))
                synopsis = metadata["synopsis"].asString();
        }


        bool subtitlesEnabled = false;
        bool subtitlesAvailable = false;
        bool audioDescripEnabled = false;
        bool audioDescripAvailable = false;
        bool signLangEnabled = false;
        bool signLangAvailable = false;

        if (state == "buffering" || state == "paused" || state == "playing") {
            if (!HasJsonParam(params, "accessibility"))
                return JsonRpcStatus::INVALID_PARAMS;
            if (!HasJsonParam(params["accessibility"], "subtitles"))
                return JsonRpcStatus::INVALID_PARAMS;
            if (!HasJsonParam(params["accessibility"], "audioDescription"))
                return JsonRpcStatus::INVALID_PARAMS;
            if (!HasJsonParam(params["accessibility"], "signLanguage"))
                return JsonRpcStatus::INVALID_PARAMS;

            Json::Value subtitles = params["accessibility"]["subtitles"];
            Json::Value audioDescription = params["accessibility"]["audioDescription"];
            Json::Value signLanguage = params["accessibility"]["signLanguage"];

            if (!HasParam(subtitles, "enabled", Json::booleanValue))
                return JsonRpcStatus::INVALID_PARAMS;
            if (!HasParam(subtitles, "available", Json::booleanValue))
                return JsonRpcStatus::INVALID_PARAMS;

            subtitlesEnabled = subtitles["enabled"].asBool();
            subtitlesAvailable = subtitles["available"].asBool();

            if (!HasParam(audioDescription, "enabled", Json::booleanValue))
                return JsonRpcStatus::INVALID_PARAMS;
            if (!HasParam(audioDescription, "available", Json::booleanValue))
                return JsonRpcStatus::INVALID_PARAMS;

            audioDescripEnabled = audioDescription["enabled"].asBool();
            audioDescripAvailable = audioDescription["available"].asBool();

            if (!HasParam(signLanguage, "enabled", Json::booleanValue))
                return JsonRpcStatus::INVALID_PARAMS;
            if (!HasParam(signLanguage, "available", Json::booleanValue))
                return JsonRpcStatus::INVALID_PARAMS;

            signLangEnabled = signLanguage["enabled"].asBool();
            signLangAvailable = signLanguage["available"].asBool();
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

    JsonRpcService::JsonRpcStatus
    JsonRpcService::ReceiveIntentConfirm(int connectionId, const Json::Value &obj) {
        Json::Value result = obj["result"];
        if (!HasParam(obj, "id", Json::stringValue)
            && !HasParam(obj, "id", Json::intValue)
            && !HasParam(obj, "id", Json::uintValue))
            return JsonRpcStatus::INVALID_PARAMS;
        std::string id = AddDataTypeIdentify(obj["id"]);

        if (!HasParam(result, "method", Json::stringValue))
            return JsonRpcStatus::INVALID_PARAMS;
        std::string method = result["method"].asString();

        if (method == MD_INTENT_MEDIA_PAUSE
            || method == MD_INTENT_MEDIA_PLAY
            || method == MD_INTENT_MEDIA_FAST_FORWARD
            || method == MD_INTENT_MEDIA_FAST_REVERSE
            || method == MD_INTENT_MEDIA_STOP
            || method == MD_INTENT_MEDIA_SEEK_CONTENT
            || method == MD_INTENT_MEDIA_SEEK_LIVE
            || method == MD_INTENT_MEDIA_SEEK_RELATIVE
            || method == MD_INTENT_MEDIA_SEEK_WALLCLOCK
            || method == MD_INTENT_SEARCH
            || method == MD_INTENT_DISPLAY
            || method == MD_INTENT_PLAYBACK
                ) {
            m_sessionCallback->ReceiveIntentConfirm(connectionId, id, method);
            return JsonRpcStatus::SUCCESS;
        }
    }

    void JsonRpcService::CreateJsonRpcErrorObject(int connectionId, const Json::Value &obj,
                                                  JsonRpcStatus status) {
        Json::Value error;
        std::string id;
        if (HasParam(obj, "id", Json::stringValue)
            || HasParam(obj, "id", Json::intValue)
            || HasParam(obj, "id", Json::uintValue)) {
            id = AddDataTypeIdentify(obj["id"]);
        }

        int code = static_cast<int>(status);
        std::string message = "";
        if (status == JsonRpcStatus::METHOD_NOT_FOUND) {
            message = "Method not found";
        }
        else if (status == JsonRpcStatus::PARSE_ERROR) {
            message = "Parse Error";
        }
        else if (status == JsonRpcStatus::INVALID_PARAMS) {
            message = "Invalid params";
        }
        else if (status == JsonRpcStatus::INVALID_REQUEST) {
            message = "Invalid request";
        }

        RespondError(connectionId, id, code, message);
    }

    bool JsonRpcService::HasParam(const Json::Value &json, const std::string &param,
                                  Json::ValueType type) {
        if (!json.isMember(param)) {
            return false;
        }
        if (json[param].type() != type) {
            return false;
        }
        return true;
    }

    bool JsonRpcService::HasJsonParam(const Json::Value &json, const std::string &param) {
        if (!json.isMember(param)) {
            return false;
        }
        if (!json[param].isObject()) {
            return false;
        }
        return true;
    }

    std::string JsonRpcService::AddDataTypeIdentify(Json::Value value) {
        std::string newValue;
        if (value.type() == Json::stringValue) {
            newValue = "STR" + value.asString();
        }
        else if (value.type() == Json::realValue) {
            std::ostringstream oss;
            oss << std::noshowpoint << value.asDouble();
            newValue = "NUM" + oss.str();
        }
        else if (value.type() == Json::intValue || value.type() == Json::uintValue) {
            newValue = "NUM" + value.asString();
        }
        return newValue;
    }


    void JsonRpcService::OnDisconnected(WebSocketConnection *connection) {

    }

    void JsonRpcService::OnServiceStopped() {

    }

    Json::Value
    JsonRpcService::CreateFeatureSettingsQuery(const std::string feature, Json::Value value) {
        Json::Value result;
        result["method"] = MD_AF_FEATURE_SETTINGS_QUERY;
        result["feature"] = feature;
        result["value"] = value;
        return result;
    }

    Json::Value JsonRpcService::CreateNotifyRequest(Json::Value params) {
        Json::Value jsonResponse;
        jsonResponse["jsonrpc"] = "2.0";
        jsonResponse["method"] = MD_NOTIFY;
        jsonResponse["params"] = params;
        return jsonResponse;
    }

    Json::Value JsonRpcService::CreateJsonResponse(const std::string id, const std::string method,
                                                   Json::Value params) {
        Json::Value jsonResponse;
        int labelStart = 0;
        int labelEnd = 3;
        jsonResponse["jsonrpc"] = "2.0";
        if (id.substr(labelStart, labelEnd) == "STR") {
            jsonResponse["id"] = id.substr(labelEnd, id.length() - labelEnd);
        }
        else {
            jsonResponse["id"] = std::stoll(id.substr(labelEnd, id.length() - labelEnd));
        }
        jsonResponse["params"] = params;
        jsonResponse["method"] = method;
        return jsonResponse;
    }

    Json::Value JsonRpcService::CreateJsonResponse(const std::string id, Json::Value result) {
        Json::Value jsonResponse;
        int labelStart = 0;
        int labelEnd = 3;
        jsonResponse["jsonrpc"] = "2.0";
        if (id.substr(labelStart, labelEnd) == "STR") {
            jsonResponse["id"] = id.substr(labelEnd, id.length() - labelEnd);
        }
        else {
            jsonResponse["id"] = std::stoll(id.substr(labelEnd, id.length() - labelEnd));
        }
        jsonResponse["result"] = result;
        return jsonResponse;
    }

    Json::Value JsonRpcService::CreateJsonErrorResponse(const std::string id, Json::Value error) {
        Json::Value jsonResponse;
        int labelStart = 0;
        int labelEnd = 3;
        jsonResponse["jsonrpc"] = "2.0";
        if (id != OPTIONAL_STR_NOT_SET) {
            if (id.substr(labelStart, labelEnd) == "STR") {
                if (labelEnd != id.length()) {
                    jsonResponse["id"] = id.substr(labelEnd, id.length() - labelEnd);
                }
            }
            else {
                jsonResponse["id"] = std::stoll(id.substr(labelEnd, id.length() - labelEnd));
            }
        }
        jsonResponse["error"] = error;
        return jsonResponse;
    }

    void JsonRpcService::SendJsonMessageToClient(int connectionId, const std::string responseName,
                                                 Json::Value jsonResponse) {
        Json::FastWriter writer;
        std::string message = writer.write(jsonResponse);
        connections_mutex_.lock();
        WebSocketConnection *connection = GetConnection(connectionId);
        if (connection != nullptr) {
            std::ostringstream oss;
            oss << "response=" << responseName << "|" << message;
            connection->SendMessage(oss.str());
        }
        connections_mutex_.unlock();
    }

    const std::string JsonRpcService::accessibilityFeatures[8] =
            {
                    F_SUBTITLES, F_DIALOGUE_ENHANCEMENT,
                    F_UI_MAGNIFIER, F_HIGH_CONTRAST_UI,
                    F_SCREEN_READER, F_RESPONSE_TO_USER_ACTION,
                    F_AUDIO_DESCRIPTION, F_IN_VISION_SIGNING
            };

    void JsonRpcService::RespondFeatureSupportInfo(int connectionId, const std::string id,
                                                   int featureId, const std::string value) {
        Json::Value result;
        result["method"] = MD_AF_FEATURE_SUPPORT_INFO;
        result["feature"] = accessibilityFeatures[featureId];
        result["value"] = value;
        Json::Value response = CreateJsonResponse(id, result);
        SendJsonMessageToClient(connectionId, __func__, response);
    }

    void JsonRpcService::RespondFeatureSettingsSubtitles(int connectionId, const std::string id,
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
                                                         const std::string language) {
        Json::Value value;
        value["enabled"] = enabled;
        if (size != OPTIONAL_INT_NOT_SET) {
            value["size"] = size;
        }
        if (fontFamily != OPTIONAL_STR_NOT_SET) {
            value["fontFamily"] = fontFamily;
        }
        if (textColour != OPTIONAL_STR_NOT_SET) {
            value["textColour"] = textColour;
        }
        if (textOpacity != OPTIONAL_INT_NOT_SET) {
            value["textOpacity"] = textOpacity;
        }
        if (edgeType != OPTIONAL_STR_NOT_SET) {
            value["edgeType"] = edgeType;
        }
        if (edgeColour != OPTIONAL_STR_NOT_SET) {
            value["edgeColour"] = edgeColour;
        }
        if (backgroundColour != OPTIONAL_STR_NOT_SET) {
            value["backgroundColour"] = backgroundColour;
        }
        if (backgroundOpacity != OPTIONAL_INT_NOT_SET) {
            value["backgroundOpacity"] = backgroundOpacity;
        }
        if (windowColour != OPTIONAL_STR_NOT_SET) {
            value["windowColour"] = windowColour;
        }
        if (windowOpacity != OPTIONAL_INT_NOT_SET) {
            value["windowOpacity"] = windowOpacity;
        }
        if (language != OPTIONAL_STR_NOT_SET) {
            value["language"] = language;
        }
        Json::Value result = CreateFeatureSettingsQuery(F_SUBTITLES, value);
        Json::Value response = CreateJsonResponse(id, result);
        SendJsonMessageToClient(connectionId, __func__, response);
    }

    void JsonRpcService::RespondFeatureSettingsDialogueEnhancement(int connectionId,
                                                                   const std::string id,
                                                                   int dialogueEnhancementGainPreference,
                                                                   int dialogueEnhancementGain,
                                                                   int dialogueEnhancementLimitMin,
                                                                   int dialogueEnhancementLimitMax) {
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

    void JsonRpcService::RespondFeatureSettingsUIMagnifier(int connectionId, const std::string id,
                                                           bool enabled,
                                                           const std::string magType) {
        Json::Value value;
        value["enabled"] = enabled;
        if (magType != OPTIONAL_STR_NOT_SET) {
            value["magType"] = magType;
        }
        Json::Value result = CreateFeatureSettingsQuery(F_UI_MAGNIFIER, value);
        Json::Value response = CreateJsonResponse(id, result);
        SendJsonMessageToClient(connectionId, __func__, response);
    }

    void
    JsonRpcService::RespondFeatureSettingsHighContrastUI(int connectionId, const std::string id,
                                                         bool enabled, const std::string hcType) {
        Json::Value value;
        value["enabled"] = enabled;
        if (hcType != OPTIONAL_STR_NOT_SET) {
            value["hcType"] = hcType;
        }
        Json::Value result = CreateFeatureSettingsQuery(F_HIGH_CONTRAST_UI, value);
        Json::Value response = CreateJsonResponse(id, result);
        SendJsonMessageToClient(connectionId, __func__, response);
    }

    void JsonRpcService::RespondFeatureSettingsScreenReader(int connectionId, const std::string id,
                                                            bool enabled, int speed,
                                                            const std::string voice,
                                                            const std::string language) {
        Json::Value value;
        value["enabled"] = enabled;
        if (speed != OPTIONAL_INT_NOT_SET) {
            value["speed"] = speed;
        }
        if (voice != OPTIONAL_STR_NOT_SET) {
            value["voice"] = voice;
        }
        if (language != OPTIONAL_STR_NOT_SET) {
            value["language"] = language;
        }
        Json::Value result = CreateFeatureSettingsQuery(F_SCREEN_READER, value);
        Json::Value response = CreateJsonResponse(id, result);
        SendJsonMessageToClient(connectionId, __func__, response);
    }

    void JsonRpcService::RespondFeatureSettingsResponseToUserAction(int connectionId,
                                                                    const std::string id,
                                                                    bool enabled,
                                                                    const std::string type) {
        Json::Value value;
        value["enabled"] = enabled;
        if (type != OPTIONAL_STR_NOT_SET) {
            value["type"] = type;
        }
        Json::Value result = CreateFeatureSettingsQuery(F_RESPONSE_TO_USER_ACTION, value);
        Json::Value response = CreateJsonResponse(id, result);
        SendJsonMessageToClient(connectionId, __func__, response);
    }

    void
    JsonRpcService::RespondFeatureSettingsAudioDescription(int connectionId, const std::string id,
                                                           bool enabled, int gainPreference,
                                                           int panAzimuthPreference) {
        Json::Value value;
        value["enabled"] = enabled;
        if (gainPreference != OPTIONAL_INT_NOT_SET) {
            value["gainPreference"] = gainPreference;
        }
        if (panAzimuthPreference != OPTIONAL_INT_NOT_SET) {
            value["panAzimuthPreference"] = panAzimuthPreference;
        }
        Json::Value result = CreateFeatureSettingsQuery(F_AUDIO_DESCRIPTION, value);
        Json::Value response = CreateJsonResponse(id, result);
        SendJsonMessageToClient(connectionId, __func__, response);
    }

    void
    JsonRpcService::RespondFeatureSettingsInVisionSigning(int connectionId, const std::string id,
                                                          bool enabled) {
        Json::Value value;
        value["enabled"] = enabled;
        Json::Value result = CreateFeatureSettingsQuery(F_IN_VISION_SIGNING, value);
        Json::Value response = CreateJsonResponse(id, result);
        SendJsonMessageToClient(connectionId, __func__, response);
    }

    void JsonRpcService::RespondFeatureSuppress(int connectionId, const std::string id,
                                                int featureId, const std::string value) {
        Json::Value result;
        result["method"] = MD_AF_FEATURE_SUPPRESS;
        result["feature"] = accessibilityFeatures[featureId];
        result["value"] = value;
        Json::Value response = CreateJsonResponse(id, result);
        SendJsonMessageToClient(connectionId, __func__, response);
    }

    void JsonRpcService::RespondSubscribe(int connectionId, const std::string id,
                                          bool subtitles, bool dialogueEnhancement,
                                          bool uiMagnifier, bool highContrastUI,
                                          bool screenReader, bool responseToUserAction,
                                          bool audioDescription, bool inVisionSigning) {
        Json::Value result;
        Json::Value msgTypeList(Json::arrayValue);
        if (subtitles) {
            msgTypeList.append(PC_SUBTITLES);
        }
        if (dialogueEnhancement) {
            msgTypeList.append(PC_DIALOGUE_ENHANCEMENT);
        }
        if (uiMagnifier) {
            msgTypeList.append(PC_UI_MAGNIFIER);
        }
        if (highContrastUI) {
            msgTypeList.append(PC_HIGH_CONTRAST_UI);
        }
        if (screenReader) {
            msgTypeList.append(PC_SCREEN_READER);
        }
        if (responseToUserAction) {
            msgTypeList.append(PC_RESPONSE_TO_USER_ACTION);
        }
        if (audioDescription) {
            msgTypeList.append(PC_AUDIO_DESCRIPTION);
        }
        if (inVisionSigning) {
            msgTypeList.append(PC_IN_VISION_SIGNING);
        }
        result["msgType"] = msgTypeList;
        Json::Value response = CreateJsonResponse(id, result);
        SendJsonMessageToClient(connectionId, __func__, response);
    }

    void JsonRpcService::RespondUnsubscribe(int connectionId, const std::string id,
                                            bool subtitles, bool dialogueEnhancement,
                                            bool uiMagnifier, bool highContrastUI,
                                            bool screenReader, bool responseToUserAction,
                                            bool audioDescription, bool inVisionSigning) {
        RespondSubscribe(connectionId, id,
                         subtitles, dialogueEnhancement, uiMagnifier, highContrastUI,
                         screenReader, responseToUserAction, audioDescription, inVisionSigning);
    }

    Json::Value JsonRpcService::CreateNegotiatedMethods(const std::string stringList) {
        // Parse the input string
        Json::Value jsonArray(Json::arrayValue);
        size_t startPos = 0;
        size_t endPos = stringList.find(',');
        while (endPos != std::string::npos) {
            std::string element = stringList.substr(startPos + 1, endPos - startPos - 2);
            jsonArray.append(element);
            startPos = endPos + 1;  // Move past the comma and space
            endPos = stringList.find(',', startPos);
        }
        // Append the last element
        std::string lastElement = stringList.substr(startPos + 1,
                                                    stringList.length() - startPos - 2);
        jsonArray.append(lastElement);

        return jsonArray;
    }

    void JsonRpcService::RespondNegotiateMethods(int connectionId, const std::string id,
                                                 const std::string terminalToApp,
                                                 const std::string appToTerminal) {
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

    void JsonRpcService::RespondError(int connectionId, const std::string id,
                                      int code, const std::string message) {
        Json::Value error;
        error["code"] = code;
        error["message"] = message;
        Json::Value response = CreateJsonErrorResponse(id, error);
        SendJsonMessageToClient(connectionId, __func__, response);
    }

    void JsonRpcService::RespondError(int connectionId, const std::string id,
                                      int code, const std::string message, const std::string data) {
        Json::Value error;
        error["code"] = code;
        error["message"] = message;
        if (data != OPTIONAL_STR_NOT_SET) {
            error["data"] = data;
        }
        Json::Value response = CreateJsonErrorResponse(id, error);
        SendJsonMessageToClient(connectionId, __func__, response);
    }

    void JsonRpcService::SendIntentMediaPause(int connectionId, const std::string id,
                                              const std::string origin) {
        Json::Value params;
        params["origin"] = origin;
        Json::Value response = CreateJsonResponse(id, MD_INTENT_MEDIA_PAUSE, params);
        SendJsonMessageToClient(connectionId, __func__, response);
    }

    void JsonRpcService::SendIntentMediaPlay(int connectionId, const std::string id,
                                             const std::string origin) {
        Json::Value params;
        params["origin"] = origin;
        Json::Value response = CreateJsonResponse(id, MD_INTENT_MEDIA_PLAY, params);
        SendJsonMessageToClient(connectionId, __func__, response);
    }

    void JsonRpcService::SendIntentMediaFastForward(int connectionId, const std::string id,
                                                    const std::string origin) {
        Json::Value params;
        params["origin"] = origin;
        Json::Value response = CreateJsonResponse(id, MD_INTENT_MEDIA_FAST_FORWARD, params);
        SendJsonMessageToClient(connectionId, __func__, response);
    }

    void JsonRpcService::SendIntentMediaFastReverse(int connectionId, const std::string id,
                                                    const std::string origin) {
        Json::Value params;
        params["origin"] = origin;
        Json::Value response = CreateJsonResponse(id, MD_INTENT_MEDIA_FAST_REVERSE, params);
        SendJsonMessageToClient(connectionId, __func__, response);
    }

    void JsonRpcService::SendIntentMediaStop(int connectionId, const std::string id,
                                             const std::string origin) {
        Json::Value params;
        params["origin"] = origin;
        Json::Value response = CreateJsonResponse(id, MD_INTENT_MEDIA_STOP, params);
        SendJsonMessageToClient(connectionId, __func__, response);
    }

    void JsonRpcService::SendIntentMediaSeekContent(int connectionId, const std::string id,
                                                    const std::string origin,
                                                    const std::string anchor, int offset) {
        Json::Value params;
        params["origin"] = origin;
        params["anchor"] = anchor;
        params["offset"] = offset;
        Json::Value response = CreateJsonResponse(id, MD_INTENT_MEDIA_SEEK_CONTENT, params);
        SendJsonMessageToClient(connectionId, __func__, response);
    }

    void JsonRpcService::SendIntentMediaSeekRelative(int connectionId, const std::string id,
                                                     const std::string origin, int offset) {
        Json::Value params;
        params["origin"] = origin;
        params["offset"] = offset;
        Json::Value response = CreateJsonResponse(id, MD_INTENT_MEDIA_SEEK_RELATIVE, params);
        SendJsonMessageToClient(connectionId, __func__, response);
    }

    void JsonRpcService::SendIntentMediaSeekLive(int connectionId, const std::string id,
                                                 const std::string origin, int offset) {
        Json::Value params;
        params["origin"] = origin;
        params["offset"] = offset;
        Json::Value response = CreateJsonResponse(id, MD_INTENT_MEDIA_SEEK_LIVE, params);
        SendJsonMessageToClient(connectionId, __func__, response);
    }

    void JsonRpcService::SendIntentMediaSeekWallclock(int connectionId, const std::string id,
                                                      const std::string origin,
                                                      const std::string dateTime) {
        Json::Value params;
        params["origin"] = origin;
        params["date-time"] = dateTime;
        Json::Value response = CreateJsonResponse(id, MD_INTENT_MEDIA_SEEK_WALLCLOCK, params);
        SendJsonMessageToClient(connectionId, __func__, response);
    }

    void JsonRpcService::SendIntentSearch(int connectionId, const std::string id,
                                          const std::string origin, const std::string query) {
        Json::Value params;
        params["origin"] = origin;
        params["query"] = query;
        Json::Value response = CreateJsonResponse(id, MD_INTENT_SEARCH, params);
        SendJsonMessageToClient(connectionId, __func__, response);
    }

    void JsonRpcService::SendIntentDisplay(int connectionId, const std::string id,
                                           const std::string origin, const std::string mediaId) {
        Json::Value params;
        params["origin"] = origin;
        params["mediaId"] = mediaId;
        Json::Value response = CreateJsonResponse(id, MD_INTENT_DISPLAY, params);
        SendJsonMessageToClient(connectionId, __func__, response);
    }

    void JsonRpcService::SendIntentPlayback(int connectionId, const std::string id,
                                            const std::string origin, const std::string mediaId,
                                            const std::string anchor, int offset) {
        Json::Value params;
        params["origin"] = origin;
        params["mediaId"] = mediaId;
        if (anchor != OPTIONAL_STR_NOT_SET) {
            params["anchor"] = anchor;
        }
        if (offset != OPTIONAL_INT_NOT_SET) {
            params["offset"] = offset;
        }
        Json::Value response = CreateJsonResponse(id, MD_INTENT_PLAYBACK, params);
        SendJsonMessageToClient(connectionId, __func__, response);
    }

    void JsonRpcService::NotifySubtitles(int connectionId, bool enabled,
                                         int size, const std::string fontFamily,
                                         const std::string textColour, int textOpacity,
                                         const std::string edgeType, const std::string edgeColour,
                                         const std::string backgroundColour, int backgroundOpacity,
                                         const std::string windowColour, int windowOpacity,
                                         const std::string language) {
        Json::Value params;
        params["msgType"] = PC_SUBTITLES;
        Json::Value value;
        value["enabled"] = enabled;
        if (size != OPTIONAL_INT_NOT_SET) {
            value["size"] = size;
        }
        if (fontFamily != OPTIONAL_STR_NOT_SET) {
            value["fontFamily"] = fontFamily;
        }
        if (textColour != OPTIONAL_STR_NOT_SET) {
            value["textColour"] = textColour;
        }
        if (textOpacity != OPTIONAL_INT_NOT_SET) {
            value["textOpacity"] = textOpacity;
        }
        if (edgeType != OPTIONAL_STR_NOT_SET) {
            value["edgeType"] = edgeType;
        }
        if (edgeColour != OPTIONAL_STR_NOT_SET) {
            value["edgeColour"] = edgeColour;
        }
        if (backgroundColour != OPTIONAL_STR_NOT_SET) {
            value["backgroundColour"] = backgroundColour;
        }
        if (backgroundOpacity != OPTIONAL_INT_NOT_SET) {
            value["backgroundOpacity"] = backgroundOpacity;
        }
        if (windowColour != OPTIONAL_STR_NOT_SET) {
            value["windowColour"] = windowColour;
        }
        if (windowOpacity != OPTIONAL_INT_NOT_SET) {
            value["windowOpacity"] = windowOpacity;
        }
        if (language != OPTIONAL_STR_NOT_SET) {
            value["language"] = language;
        }
        params["value"] = value;
        Json::Value response = CreateNotifyRequest(params);
        SendJsonMessageToClient(connectionId, __func__, response);
    }

    void JsonRpcService::NotifyDialogueEnhancement(int connectionId,
                                                   int dialogueEnhancementGainPreference,
                                                   int dialogueEnhancementGain,
                                                   int dialogueEnhancementLimitMin,
                                                   int dialogueEnhancementLimitMax) {
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
        SendJsonMessageToClient(connectionId, __func__, response);
    }

    void
    JsonRpcService::NotifyUIMagnifier(int connectionId, bool enabled, const std::string magType) {
        Json::Value params;
        params["msgType"] = PC_UI_MAGNIFIER;
        Json::Value value;
        value["enabled"] = enabled;
        if (magType != OPTIONAL_STR_NOT_SET) {
            value["magType"] = magType;
        }
        params["value"] = value;
        Json::Value response = CreateNotifyRequest(params);
        SendJsonMessageToClient(connectionId, __func__, response);
    }

    void JsonRpcService::NotifyHighContrastUI(int connectionId, bool enabled,
                                              const std::string hcType) {
        Json::Value params;
        params["msgType"] = PC_HIGH_CONTRAST_UI;
        Json::Value value;
        value["enabled"] = enabled;
        if (hcType != OPTIONAL_STR_NOT_SET) {
            value["hcType"] = hcType;
        }
        params["value"] = value;
        Json::Value response = CreateNotifyRequest(params);
        SendJsonMessageToClient(connectionId, __func__, response);
    }

    void JsonRpcService::NotifyScreenReader(int connectionId, bool enabled,
                                            int speed, const std::string voice,
                                            const std::string language) {
        Json::Value params;
        params["msgType"] = PC_SCREEN_READER;
        Json::Value value;
        value["enabled"] = enabled;
        if (speed != OPTIONAL_INT_NOT_SET) {
            value["speed"] = speed;
        }
        if (voice != OPTIONAL_STR_NOT_SET) {
            value["voice"] = voice;
        }
        if (language != OPTIONAL_STR_NOT_SET) {
            value["language"] = language;
        }
        params["value"] = value;
        Json::Value response = CreateNotifyRequest(params);
        SendJsonMessageToClient(connectionId, __func__, response);
    }

    void JsonRpcService::NotifyResponseToUserAction(int connectionId, bool enabled,
                                                    const std::string type) {
        Json::Value params;
        params["msgType"] = PC_RESPONSE_TO_USER_ACTION;
        Json::Value value;
        value["enabled"] = enabled;
        if (type != OPTIONAL_STR_NOT_SET) {
            value["type"] = type;
        }
        params["value"] = value;
        Json::Value response = CreateNotifyRequest(params);
        SendJsonMessageToClient(connectionId, __func__, response);
    }

    void JsonRpcService::NotifyAudioDescription(int connectionId, bool enabled,
                                                int gainPreference, int panAzimuthPreference) {
        Json::Value params;
        params["msgType"] = PC_AUDIO_DESCRIPTION;
        Json::Value value;
        value["enabled"] = enabled;
        if (gainPreference != OPTIONAL_INT_NOT_SET) {
            value["gainPreference"] = gainPreference;
        }
        if (panAzimuthPreference != OPTIONAL_INT_NOT_SET) {
            value["panAzimuthPreference"] = panAzimuthPreference;
        }
        params["value"] = value;
        Json::Value response = CreateNotifyRequest(params);
        SendJsonMessageToClient(connectionId, __func__, response);
    }

    void JsonRpcService::NotifyInVisionSigning(int connectionId, bool enabled) {
        Json::Value params;
        params["msgType"] = PC_IN_VISION_SIGNING;
        Json::Value value;
        value["enabled"] = enabled;
        params["value"] = value;
        Json::Value response = CreateNotifyRequest(params);
        SendJsonMessageToClient(connectionId, __func__, response);
    }

    void JsonRpcService::RespondDialogueEnhancementOverride(int connectionId, const std::string id,
                                                            int dialogueEnhancementGain) {
        Json::Value result;
        result["method"] = MD_AF_DIALOGUE_ENHANCEMENT_OVERRIDE;
        if (dialogueEnhancementGain != OPTIONAL_INT_NOT_SET) {
            result["dialogueEnhancementGain"] = dialogueEnhancementGain;
        }
        Json::Value response = CreateJsonResponse(id, result);
        SendJsonMessageToClient(connectionId, __func__, response);
    }

    void JsonRpcService::RespondTriggerResponseToUserAction(int connectionId, const std::string id,
                                                            bool actioned) {
        Json::Value result;
        result["method"] = MD_AF_TRIGGER_RESPONSE_TO_USER_ACTION;
        result["actioned"] = actioned;
        Json::Value response = CreateJsonResponse(id, result);
        SendJsonMessageToClient(connectionId, __func__, response);
    }


} // namespace NetworkServices
