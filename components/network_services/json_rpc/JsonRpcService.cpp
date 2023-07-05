/**
 * ORB Software. Copyright (c) 2023 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */
#define LOG_TAG "JsonRpcService"
#define NullIntValue -999999
#define NullStrValue ""

#include "JsonRpcService.h"
#include "log.h"

#include <iostream>
#include <sstream>

namespace NetworkServices {

    JsonRpcService::JsonRpcService(
            int port,
            const std::string &endpoint,
            std::unique_ptr<SessionCallback> sessionCallback) :
            WebSocketService("JsonRpcService", port, false, "lo"),
            m_endpoint(endpoint),
            m_sessionCallback(std::move(sessionCallback))
    {
        LOG(LOG_INFO, "Start");
        Start();
    }

    bool JsonRpcService::OnConnection(WebSocketConnection *connection)
    {
        if (connection->Uri() != m_endpoint) {
            LOG(LOG_INFO, "Unknown endpoint received. Got: %s, expected: %s",
                connection->Uri().c_str(), m_endpoint.c_str());
            return false;
        }
        LOG(LOG_INFO, "Connected: connectionId=%d", connection->Id());
        return true;
    }
    bool JsonRpcService::checkIDIsMemberAndType(int connectionID, Json::Value json, std::string variable, int errorCode, const std::string& errorMessage){
        if (!json.isMember(variable)){
            LOG(LOG_INFO, "Error, missing parameter %s.", variable.c_str());
            const std::string& id = "STR";
            RespondError(connectionID, id, errorCode, errorMessage);
            return false;
        }
        if (json[variable].type() != Json::stringValue
              and json[variable].type() != Json::intValue
              and json[variable].type() != Json::uintValue){
            LOG(LOG_INFO, "Error, %s has wrong type.", variable.c_str());
            const std::string& id = "STR";
            RespondError(connectionID, id, errorCode, errorMessage);
            return false;
        }
        return true;
    }
    bool JsonRpcService::checkTimeVariableIsMemberAndType(int connectionID, std::string id, Json::Value json, std::string variable, int errorCode, const std::string& errorMessage){
        if (!json.isMember(variable)){
            if (json[variable].type()!=Json::intValue
                and json[variable].type()!=Json::uintValue
                and json[variable].type()!=Json::realValue
                and json[variable].type()!=Json::stringValue)
            LOG(LOG_INFO, "Error, missing parameter ' %s ', or wrong type.", variable.c_str());
            RespondError(connectionID, id, errorCode, errorMessage);
            return false;
        }
        return true;
    }
    bool JsonRpcService::checkVariableIsMemberAndType(int connectionID, std::string id, Json::Value json, std::string variable, Json::ValueType type, int errorCode, const std::string& errorMessage){
        if (!json.isMember(variable) or json[variable].type()!=type){
            LOG(LOG_INFO, "Error, missing parameter ' %s ', or wrong type.", variable.c_str());
            RespondError(connectionID, id, errorCode, errorMessage);
            return false;
        }
        return true;
    }
    bool JsonRpcService::checkVariableIsMemberAndIsJson(int connectionID, std::string id, Json::Value json, std::string variable, int errorCode, const std::string& errorMessage){
        if (!json.isMember(variable) or !json[variable].isObject()){
            LOG(LOG_INFO, "Error, missing parameter ' %s ', or wrong type.", variable.c_str());
            RespondError(connectionID, id, errorCode, errorMessage);
            return false;
        }
        return true;
    }

    void JsonRpcService::OnMessageReceived(WebSocketConnection *connection, const std::string &text)
    {
        LOG(LOG_INFO, "Message received: connection=%d, text=%s", connection->Id(), text.c_str());

        Json::Value obj;
        Json::Reader reader;
        bool b = reader.parse(text, obj);

        if (!b){
            LOG(LOG_INFO, "Error, json rpc parse wrong");
            //call return -32700 Error
            RespondError(connection->Id(), "STR", -32700, "Parse Error");
            return;
        }
        if (!obj.isMember("jsonrpc") or obj["jsonrpc"]!="2.0"){
            LOG(LOG_INFO, "Error, json rpc version not 2.0");
            if (!checkIDIsMemberAndType(connection->Id(), obj, "id", -32600, "Invalid request")) return;
            std::string id = AddIdentify(obj["id"]);
            RespondError(connection->Id(), id, -32600, "Invalid request");
            return;
        }
        if (obj.isMember("result")){
            Json::Value result = obj["result"];
            if (!checkIDIsMemberAndType(connection->Id(), obj, "id", -32602, "Invalid params")) return;
            std::string id = AddIdentify(obj["id"]);

            if (!checkVariableIsMemberAndType(connection->Id(), id, result, "method", Json::stringValue, -32602, "Invalid params")) return;
            std::string method = result["method"].asString();

            if (method == MD_INTENT_MEDIA_PAUSE
                or	method == MD_INTENT_MEDIA_PLAY
                or	method == MD_INTENT_MEDIA_FAST_FORWARD
                or	method == MD_INTENT_MEDIA_FAST_REVERSE
                or	method == MD_INTENT_MEDIA_STOP
                or	method == MD_INTENT_MEDIA_SEEK_CONTENT
                or  method == MD_INTENT_MEDIA_SEEK_LIVE
                or	method == MD_INTENT_MEDIA_SEEK_RELATIVE
                or	method == MD_INTENT_MEDIA_SEEK_WALLCLOCK
                or	method == MD_INTENT_SEARCH
                or	method == MD_INTENT_DISPLAY
                or	method == MD_INTENT_PLAYBACK
            ){
                m_sessionCallback->ReceiveIntentConfirm(connection->Id(), id, method);
                return;
            }
        }

        if (obj.isMember("error")) {
            Json::Value error = obj["error"];
            if (!checkIDIsMemberAndType(connection->Id(), obj, "id", -32602, "Invalid params")) return;
            std::string id = AddIdentify(obj["id"]);
            if (!checkVariableIsMemberAndType(connection->Id(), id, error, "code", Json::intValue, -32602, "Invalid params")) return;
            int code = error["code"].asInt();
            std::string message = NullStrValue;
            if (error.isMember("message") or error["message"].type()==Json::stringValue)
                message = error["message"].asString();

            std::string data = NullStrValue;
            if (error.isMember("data") or error["data"].type()==Json::stringValue)
                message = error["data"].asString();

            m_sessionCallback->ReceiveError(connection->Id(), id, code, message);
            return;
        }
        if (!obj.isMember("method") or obj["method"].type()!=Json::stringValue){
            LOG(LOG_INFO, "Cannot find method");
            if (!checkIDIsMemberAndType(connection->Id(), obj, "id", -32600, "Invalid request")) return;
            std::string id = AddIdentify(obj["id"]);
            RespondError(connection->Id(), id, -32600, "Invalid request");
            return;
        }
        std::string method = obj["method"].asString();

        LOG(LOG_INFO, "Method = %s",method.c_str());

        if (method == MD_AF_DIALOGUE_ENHANCEMENT_OVERRIDE){
            if (!checkIDIsMemberAndType(connection->Id(), obj, "id", -32602, "Invalid params")) return;
            std::string id = AddIdentify(obj["id"]);
            int dialogueEnhancementGain = NullIntValue;
            if (obj.isMember("params")){
                Json::Value params = obj["params"];
                if (params.isMember("dialogueEnhancementGain")
                and params["dialogueEnhancementGain"].type()==Json::intValue)
                    dialogueEnhancementGain = params["dialogueEnhancementGain"].asInt();
            }

            LOG(LOG_INFO, "JSON-RPC-EXAMPLE #2: Service received request. Call session callback...");
            m_sessionCallback->RequestDialogueEnhancementOverride(connection->Id(), id,
                                                                  dialogueEnhancementGain);
            return;
        }
        if (!obj.isMember("params")){
            LOG(LOG_INFO, "Error, missing parameter 'params'.");
            if (!checkIDIsMemberAndType(connection->Id(), obj, "id", -32602, "Invalid params")) return;
            std::string id = AddIdentify(obj["id"]);
            //call return -32602 Error
            RespondError(connection->Id(), id, -32602, "Invalid params");
            return;
        }
        Json::Value params = obj["params"];

        std::map<std::string, int> mapOfFeatures;
        mapOfFeatures[F_SUBTITLES]               = 0;
        mapOfFeatures[F_DIALOGUE_ENHANCEMENT]    = 1;
        mapOfFeatures[F_UI_MAGNIFIER]            = 2;
        mapOfFeatures[F_HIGH_CONTRAST_UI]        = 3;
        mapOfFeatures[F_SCREEN_READER]           = 4;
        mapOfFeatures[F_RESPONSE_TO_USER_ACTION] = 5;
        mapOfFeatures[F_AUDIO_DESCRIPTION]       = 6;
        mapOfFeatures[F_IN_VISION_SIGNING]       = 7;


        if (method == MD_AF_FEATURE_SUPPORT_INFO){
            if (!checkIDIsMemberAndType(connection->Id(), obj, "id", -32602, "Invalid params")) return;
            std::string id = AddIdentify(obj["id"]);
            if (!checkVariableIsMemberAndType(connection->Id(), id, params, "feature", Json::stringValue, -32602, "Invalid params")) return;
            std::string feature = params["feature"].asString();
            if (mapOfFeatures.find(feature)==mapOfFeatures.end()){
                LOG(LOG_INFO, "Error, this feature is not recognized.");
                //call return -32602 Error
                RespondError(connection->Id(), id, -32602, "Invalid params");
                return;
            }
            LOG(LOG_INFO, "JSON-RPC-EXAMPLE #2: Service received request. Call session callback...");
            m_sessionCallback->RequestFeatureSupportInfo(connection->Id(), id,
                                                         mapOfFeatures[feature]);
        }
        else if (method == MD_AF_FEATURE_SETTINGS_QUERY){
            if (!checkIDIsMemberAndType(connection->Id(), obj, "id", -32602, "Invalid params")) return;
            std::string id = AddIdentify(obj["id"]);
            if (!checkVariableIsMemberAndType(connection->Id(), id, params, "feature", Json::stringValue, -32602, "Invalid params")) return;
            std::string feature = params["feature"].asString();
            if (mapOfFeatures.find(feature)==mapOfFeatures.end()){
                LOG(LOG_INFO, "Error, this feature is not recognized.");
                //call return -32602 Error
                RespondError(connection->Id(), id, -32602, "Invalid params");
                return;
            }
            LOG(LOG_INFO, "JSON-RPC-EXAMPLE #2: Service received request. Call session callback...");
            m_sessionCallback->RequestFeatureSettingsQuery(connection->Id(), id,
                                                           mapOfFeatures[feature]);
        }
        else if (method == MD_AF_FEATURE_SUPPRESS){
            if (!checkIDIsMemberAndType(connection->Id(), obj, "id", -32602, "Invalid params")) return;
            std::string id = AddIdentify(obj["id"]);
            if (!checkVariableIsMemberAndType(connection->Id(), id, params, "feature", Json::stringValue, -32602, "Invalid params")) return;
            std::string feature = params["feature"].asString();
            if (mapOfFeatures.find(feature)==mapOfFeatures.end()){
                LOG(LOG_INFO, "Error, this feature is not recognized.");
                //call return -32602 Error
                RespondError(connection->Id(), id, -32602, "Invalid params");
                return;
            }
            LOG(LOG_INFO, "JSON-RPC-EXAMPLE #2: Service received request. Call session callback...");
            m_sessionCallback->RequestFeatureSuppress(connection->Id(), id,
                                                      mapOfFeatures[feature]);
        }
        else if (method == MD_SUBSCRIBE){
            if (!checkIDIsMemberAndType(connection->Id(), obj, "id", -32602, "Invalid params")) return;
            std::string id = AddIdentify(obj["id"]);

            if (!checkVariableIsMemberAndType(connection->Id(), id, params, "msgType", Json::arrayValue, -32602, "Invalid params")) return;
            Json::Value msgType = params["msgType"];

            bool msgTypeBoolList[8]={false};
            for (auto msg : msgType){
                int length = msg.asString().length();
                std::string s = msg.asString().substr(0,length - 10);  //remove the PrefChange
                if (mapOfFeatures.find(s)!=mapOfFeatures.end()){
                    msgTypeBoolList[mapOfFeatures[s]]=true;
                    LOG(LOG_INFO, "One added");
                }
                else{
                    LOG(LOG_INFO, "Error, One of msgType parameter is not recognized.");
                    //call return -32602 Error
                    RespondError(connection->Id(), id, -32602, "Invalid params");
                    return;
                }
            }
            m_sessionCallback->RequestSubscribe(connection->Id(), id, msgTypeBoolList[0],
                                                msgTypeBoolList[1],msgTypeBoolList[2],
                                                msgTypeBoolList[3],msgTypeBoolList[4],
                                                msgTypeBoolList[5],msgTypeBoolList[6],
                                                msgTypeBoolList[7]);
        }
        else if (method == MD_UNSUBSCRIBE){
            if (!checkIDIsMemberAndType(connection->Id(), obj, "id", -32602, "Invalid params")) return;
            std::string id = AddIdentify(obj["id"]);

            if (!checkVariableIsMemberAndType(connection->Id(), id, params, "msgType", Json::arrayValue, -32602, "Invalid params")) return;
            Json::Value msgType = params["msgType"];

            bool msgTypeBoolList[8]={false};
            for (auto msg : msgType){
                int length = msg.asString().length();
                std::string s = msg.asString().substr(0,length - 10);  //remove the PrefChange
                if (mapOfFeatures.find(s)!=mapOfFeatures.end()){
                    msgTypeBoolList[mapOfFeatures[s]]=true;
                }
                else{
                    LOG(LOG_INFO, "Error, One of msgType parameter is not recognized.");
                    //call return -32602 Error
                    RespondError(connection->Id(), id, -32602, "Invalid params");
                    return;
                }
            }
            m_sessionCallback->RequestUnsubscribe(connection->Id(), id, msgTypeBoolList[0],
                                                  msgTypeBoolList[1],msgTypeBoolList[2],
                                                  msgTypeBoolList[3],msgTypeBoolList[4],
                                                  msgTypeBoolList[5],msgTypeBoolList[6],
                                                  msgTypeBoolList[7]);
        }
        else if (method == MD_VOICE_READY){
            if (!checkVariableIsMemberAndType(connection->Id(), "STR", params, "ready", Json::booleanValue, -32602, "Invalid params")) return;
            bool ready = params["ready"].asBool();
            LOG(LOG_INFO, "JSON-RPC-EXAMPLE #2: Service received request. Call session callback...");
            m_sessionCallback->NotifyVoiceReady(connection->Id(), ready);
            return;
        }
        else if (method == MD_STATE_MEDIA){
            if (!checkVariableIsMemberAndType(connection->Id(), "STR", params, "state", Json::stringValue, -32602, "Invalid params")) return;
            std::string state = params["state"].asString();
            if (state!="no-media"
                and state!="error"
                and state!="buffering"
                and state!="paused"
                and state!="playing"
                and state!="stopped"){
                LOG(LOG_INFO, "Error, state - %s - not recognized.", state.c_str());
                //call return -32602 Error
                RespondError(connection->Id(), "STR", -32602, "Invalid params");
                return;
            }
            std::string kind = NullStrValue;
            if (state=="buffering" or state=="paused" or state=="playing" or state=="stopped"){
                if (!checkVariableIsMemberAndType(connection->Id(), "STR", params, "kind", Json::stringValue, -32602, "Invalid params")) return;
                kind = params["kind"].asString();
                if (kind != "audio" and kind != "audio-video"){
                    LOG(LOG_INFO, "Error, kind - %s - not recognized.", kind.c_str());
                    //call return -32602 Error
                    RespondError(connection->Id(), "STR", -32602, "Invalid params");
                    return;
                }
            }

            std::string type = NullStrValue;
            if (state=="buffering" or state=="paused" or state=="playing" or state=="stopped"){
                if (!checkVariableIsMemberAndType(connection->Id(), "STR", params, "type", Json::stringValue, -32602, "Invalid params")) return;
                type = params["type"].asString();
                if (type != "live" and type != "on-demand"){
                    LOG(LOG_INFO, "Error, type - %s - not recognized.", type.c_str());
                    //call return -32602 Error
                    RespondError(connection->Id(), "STR", -32602, "Invalid params");
                    return;
                }
            }

            std::string currentTimeStr = NullStrValue;
            if (state=="buffering" or state=="paused" or state=="playing"){
                if (!checkTimeVariableIsMemberAndType(connection->Id(), "STR", params, "currentTime", -32602, "Invalid params")) return;
                currentTimeStr = AddIdentify(params["currentTime"]);
            }


            std::string rangeStart = NullStrValue;
            std::string rangeEnd = NullStrValue;
            if (state=="buffering" or state=="paused" or state=="playing"){
                if (!checkVariableIsMemberAndIsJson(connection->Id(), "STR", params, "range", -32602, "Invalid params")) return;
                if (!checkTimeVariableIsMemberAndType(connection->Id(), "STR", params["range"], "start", -32602, "Invalid params")) return;
                if (!checkTimeVariableIsMemberAndType(connection->Id(), "STR", params["range"], "end", -32602, "Invalid params")) return;
                rangeStart = AddIdentify(params["range"]["start"]);
                rangeEnd = AddIdentify(params["range"]["end"]);
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
            if (!checkVariableIsMemberAndIsJson(connection->Id(), "STR", params, "availableActions", -32602, "Invalid params")) return;
            if (!params.isMember("availableActions")){
                LOG(LOG_INFO, "Error, missing parameter 'availableActions'. ");
                //call return -32602 Error
                RespondError(connection->Id(), "STR", -32602, "Invalid params");
                return;
            }
            Json::Value actions =  params["availableActions"];
            if (actions.isMember("pause")
                and actions["pause"].type() == Json::booleanValue
                and actions["pause"]==true) actPause=true;

            if (actions.isMember("play")
                and actions["play"].type() == Json::booleanValue
                and actions["play"]==true) actPlay=true;

            if (actions.isMember("fast-forward")
                and actions["fast-forward"].type() == Json::booleanValue
                and actions["fast-forward"]==true) actFastForward=true;

            if (actions.isMember("fast-reverse")
                and actions["fast-reverse"].type() == Json::booleanValue
                and actions["fast-reverse"]==true) actFastReverse=true;

            if (actions.isMember("stop")
                and actions["stop"].type() == Json::booleanValue
                and actions["stop"]==true) actStop=true;

            if (actions.isMember("seek-content")
                and actions["seek-content"].type() == Json::booleanValue
                and actions["seek-content"]==true) actSeekContent=true;

            if (actions.isMember("seek-relative")
                and actions["seek-relative"].type() == Json::booleanValue
                and actions["seek-relative"]==true) actSeekRelative=true;

            if (actions.isMember("seek-live")
                and actions["seek-live"].type() == Json::booleanValue
                and actions["seek-live"]==true) actSeekLive=true;

            if (actions.isMember("seek-wallclock")
                and actions["seek-wallclock"].type() == Json::booleanValue
                and actions["seek-wallclock"]==true) actWallclock=true;

            std::string mediaId = NullStrValue;
            std::string title = NullStrValue;
            std::string secTitle = NullStrValue;
            std::string synopsis = NullStrValue;

            if (state=="buffering" or state=="paused" or state=="playing" or state=="stopped"){
                if (!checkVariableIsMemberAndIsJson(connection->Id(), "STR", params, "metadata", -32602, "Invalid params")) return;
                if (!checkVariableIsMemberAndType(connection->Id(), "STR", params["metadata"], "title", Json::stringValue, -32602, "Invalid params")) return;
                title = params["metadata"]["title"].asString();

                if (params["metadata"].isMember("mediaId")
                    and params["metadata"]["mediaId"].type() == Json::stringValue)
                    mediaId = params["metadata"]["mediaId"].asString();
                if (params["metadata"].isMember("secTitle")
                    and params["metadata"]["secTitle"].type() == Json::stringValue)
                    secTitle = params["metadata"]["secTitle"].asString();
                if (params["metadata"].isMember("synopsis")
                    and params["metadata"]["synopsis"].type() == Json::stringValue)
                    synopsis = params["metadata"]["synopsis"].asString();
            }


            bool subtitlesEnabled = false;
            bool subtitlesAvailable = false;
            bool audioDescripEnabled = false;
            bool audioDescripAvailable = false;
            bool signLangEnabled = false;
            bool signLangAvailable = false;

            if (state=="buffering" or state=="paused" or state=="playing"){
                if (!checkVariableIsMemberAndIsJson(connection->Id(), "STR", params, "accessibility", -32602, "Invalid params")) return;
                if (!checkVariableIsMemberAndIsJson(connection->Id(), "STR", params["accessibility"], "subtitles", -32602, "Invalid params")) return;
                Json::Value subtitles = params["accessibility"]["subtitles"];
                if (!checkVariableIsMemberAndIsJson(connection->Id(), "STR", params["accessibility"], "audioDescription", -32602, "Invalid params")) return;
                Json::Value audioDescription = params["accessibility"]["audioDescription"];
                if (!checkVariableIsMemberAndIsJson(connection->Id(), "STR", params["accessibility"], "signLanguage", -32602, "Invalid params")) return;
                Json::Value signLanguage = params["accessibility"]["signLanguage"];
                if (!checkVariableIsMemberAndType(connection->Id(), "STR", subtitles, "enabled", Json::booleanValue, -32602, "Invalid params")) return;
                subtitlesEnabled = subtitles["enabled"].asBool();
                if (!checkVariableIsMemberAndType(connection->Id(), "STR", subtitles, "available", Json::booleanValue, -32602, "Invalid params")) return;
                subtitlesAvailable = subtitles["available"].asBool();
                if (!checkVariableIsMemberAndType(connection->Id(), "STR", audioDescription, "enabled", Json::booleanValue, -32602, "Invalid params")) return;
                audioDescripEnabled = audioDescription["enabled"].asBool();
                if (!checkVariableIsMemberAndType(connection->Id(), "STR", audioDescription, "available", Json::booleanValue, -32602, "Invalid params")) return;
                audioDescripAvailable = audioDescription["available"].asBool();
                if (!checkVariableIsMemberAndType(connection->Id(), "STR", signLanguage, "enabled", Json::booleanValue, -32602, "Invalid params")) return;
                signLangEnabled = signLanguage["enabled"].asBool();
                if (!checkVariableIsMemberAndType(connection->Id(), "STR", signLanguage, "available", Json::booleanValue, -32602, "Invalid params")) return;
                signLangAvailable = signLanguage["available"].asBool();
            }
            m_sessionCallback->NotifyStateMedia(connection->Id(),
                                                state, kind, type, currentTimeStr,
                                                rangeStart, rangeEnd,
                                                actPause, actPlay, actFastForward,
                                                actFastReverse, actStop, actSeekContent,
                                                actSeekRelative, actSeekLive, actWallclock,
                                                mediaId, title, secTitle, synopsis,
                                                subtitlesEnabled, subtitlesAvailable,
                                                audioDescripEnabled, audioDescripAvailable,
                                                signLangEnabled, signLangAvailable);
        }
        else if (method == MD_NEGOTIATE_METHODS){
            if (!checkIDIsMemberAndType(connection->Id(), obj, "id", -32602, "Invalid params")) return;
            std::string id = AddIdentify(obj["id"]);
            std::string terminalToApp = NullStrValue;
            if (!checkVariableIsMemberAndType(connection->Id(), id, params, "terminalToApp", Json::arrayValue, -32602, "Invalid params")) return;
            terminalToApp = params["terminalToApp"].toStyledString();
            std::string appToTerminal = NullStrValue;
            if (!checkVariableIsMemberAndType(connection->Id(), id, params, "appToTerminal", Json::arrayValue, -32602, "Invalid params")) return;
            appToTerminal = params["appToTerminal"].toStyledString();


            // Remove newline characters
            size_t pos;
            while ((pos = appToTerminal.find('\n')) != std::string::npos) {
                appToTerminal.erase(pos, 2);
            }
            // Remove newline characters
            while ((pos = terminalToApp.find('\n')) != std::string::npos) {
                terminalToApp.erase(pos, 2);
            }
            LOG(LOG_INFO, "JSON-RPC-EXAMPLE #2: %s \n %s", terminalToApp.c_str(), appToTerminal.c_str());

            // Remove square brackets
            appToTerminal.erase(appToTerminal.begin(), appToTerminal.begin() + 1);
            // Remove square brackets
            terminalToApp.erase(terminalToApp.begin(), terminalToApp.begin() + 1);

            LOG(LOG_INFO, "JSON-RPC-EXAMPLE #2: Service received request. Call session callback...");

            m_sessionCallback->RequestNegotiateMethods(connection->Id(), id, terminalToApp, appToTerminal);
        }
        else if (method == MD_AF_TRIGGER_RESPONSE_TO_USER_ACTION){
            if (!checkIDIsMemberAndType(connection->Id(), obj, "id", -32602, "Invalid params")) return;
            std::string id = AddIdentify(obj["id"]);
            if (!checkVariableIsMemberAndType(connection->Id(), id, params, "magnitude", Json::stringValue, -32602, "Invalid params")) return;
            std::string magnitude = params["magnitude"].asString();
            LOG(LOG_INFO, "JSON-RPC-EXAMPLE #2: Service received request. Call session callback...");
            m_sessionCallback->RequestTriggerResponseToUserAction(connection->Id(), id, magnitude);
        }
        else{
            LOG(LOG_INFO, "Error, The terminal cannot recognize the method name.");
            if (!checkIDIsMemberAndType(connection->Id(), obj, "id", -32602, "Invalid params")) return;
            std::string id = AddIdentify(obj["id"]);
            RespondError(connection->Id(), id, -32601, "Method not found");
            return;
        }
    }

    std::string JsonRpcService::AddIdentify(Json::Value value)
    {
        std::string newValue;
        if (value.type()==Json::stringValue){
            newValue = "STR" + value.asString();
        }
        else if (value.type() == Json::realValue){
            std::ostringstream oss;
            oss << std::noshowpoint << value.asDouble();
            newValue = "NUM" + oss.str();
        }
        else if (value.type()==Json::intValue or value.type()==Json::uintValue ){
            newValue = "NUM" + value.asString();
        }
        LOG(LOG_INFO,"new Value = %s", newValue.c_str());
        return newValue;
    }

    void JsonRpcService::OnDisconnected(WebSocketConnection *connection)
    {

    }

    void JsonRpcService::OnServiceStopped()
    {

    }

    void JsonRpcService::writeFeatureSettingsQuery(
            Json::Value& result,
            const std::string feature,
            Json::Value value
    ) {
        result["method"] = MD_AF_FEATURE_SETTINGS_QUERY;
        result["feature"] = feature;
        result["value"] = value;
    }

    std::string JsonRpcService::writeJsonForNotify(Json::Value params)
    {
        Json::Value jsonResponse;
        jsonResponse["jsonrpc"] = "2.0";
        jsonResponse["method"] = MD_NOTIFY;
        jsonResponse["params"] = params;

        Json::StyledWriter writer;
        return writer.write(jsonResponse);
    }

    std::string JsonRpcService::writeJson(const std::string& id, const std::string& method, Json::Value params)
    {
        Json::Value jsonResponse;
        int labelStart = 0;
        int labelEnd = 3;
        jsonResponse["jsonrpc"] = "2.0";
        if (id.substr(labelStart,labelEnd)=="STR"){
            jsonResponse["id"] = id.substr(labelEnd, id.length()-labelEnd);
        }
        else {
            jsonResponse["id"] =  std::stoll(id.substr(labelEnd, id.length()-labelEnd));
        }
        jsonResponse["params"] = params;
        jsonResponse["method"] = method;
        Json::StyledWriter writer;
        return writer.write(jsonResponse);
    }

    std::string JsonRpcService::writeJson(const std::string& id, Json::Value result)
    {
        Json::Value jsonResponse;
        int labelStart = 0;
        int labelEnd = 3;
        jsonResponse["jsonrpc"] = "2.0";
        if (id.substr(labelStart,labelEnd)=="STR"){
            jsonResponse["id"] = id.substr(labelEnd, id.length()-labelEnd);
        }
        else {
            jsonResponse["id"] =  std::stoll(id.substr(labelEnd, id.length()-labelEnd));
        }
        jsonResponse["result"] = result;
        Json::StyledWriter writer;
        return writer.write(jsonResponse);
    }

    std::string JsonRpcService::writeError(const std::string& id, Json::Value error)
    {
        Json::Value jsonResponse;
        int labelStart = 0;
        int labelEnd = 3;
        jsonResponse["jsonrpc"] = "2.0";
        if (id.substr(labelStart,labelEnd)=="STR"){
            if (labelEnd != id.length()) {
                jsonResponse["id"] = id.substr(labelEnd, id.length()-labelEnd);
            }
        }
        else {
            jsonResponse["id"] =  std::stoll(id.substr(labelEnd, id.length()-labelEnd));
        }
        jsonResponse["error"] = error;
        Json::StyledWriter writer;
        return writer.write(jsonResponse);
    }

    void JsonRpcService::sendMessageTo(WebSocketConnection *connection, std::string responseName, std::string out_string) {

        std::ostringstream oss;
        oss << "response=" << responseName << "|" << out_string;
        connection->SendMessage(oss.str());
    }

    void JsonRpcService::RespondMessageTo(
            int connectionId,
            const std::string responseName,
            const std::string out_string
    ){
        connections_mutex_.lock();
        WebSocketConnection *connection = GetConnection(connectionId);
        if (connection != nullptr)
        {
            sendMessageTo(connection, responseName , out_string);
        }
        connections_mutex_.unlock();
    }
    void JsonRpcService::addOptionalProp(
            Json::Value& json,
            const std::string key,
            const int value
    ){
        if (value != NullIntValue) json[key] = value;
    }

    void JsonRpcService::addOptionalProp(
            Json::Value& json,
            const std::string key,
            const std::string value
    ){
        if (value != NullStrValue) json[key] = value;
    }

    void JsonRpcService::RespondFeatureSupportInfo(
            int connectionId,
            const std::string& id,
            int feature,
            const std::string& value){

        Json::Value result;
        result["method"] = MD_AF_FEATURE_SUPPORT_INFO;
        std::vector<std::string> resultStringVector = {
                F_SUBTITLES, F_DIALOGUE_ENHANCEMENT,
                F_UI_MAGNIFIER, F_HIGH_CONTRAST_UI,
                F_SCREEN_READER, F_RESPONSE_TO_USER_ACTION,
                F_AUDIO_DESCRIPTION, F_IN_VISION_SIGNING};
        result["feature"] = resultStringVector[feature];
        result["value"] = value;

        std::string out_string = writeJson(id, result);
        RespondMessageTo(connectionId, __func__, out_string);
    }

    void JsonRpcService::RespondFeatureSettingsSubtitles(
            int connectionId,
            const std::string& id,
            bool enabled,
            int size,
            const std::string& fontFamily,
            const std::string& textColour,
            int textOpacity,
            const std::string& edgeType,
            const std::string& edgeColour,
            const std::string& backgroundColour,
            int backgroundOpacity,
            const std::string& windowColour,
            int windowOpacity,
            const std::string& language
    ){
        Json::Value value;
        value["enabled"] = enabled;

        addOptionalProp(value, "size", size);
        addOptionalProp(value, "fontFamily", fontFamily);
        addOptionalProp(value, "textColour", textColour);
        addOptionalProp(value, "textOpacity",textOpacity);
        addOptionalProp(value, "edgeType", edgeType);
        addOptionalProp(value, "edgeColour", edgeColour);
        addOptionalProp(value, "backgroundColour", backgroundColour);
        addOptionalProp(value, "backgroundOpacity", backgroundOpacity);
        addOptionalProp(value, "windowColour", windowColour);
        addOptionalProp(value, "windowOpacity", windowOpacity);
        addOptionalProp(value, "language", language);

        Json::Value result;
        writeFeatureSettingsQuery(result, F_SUBTITLES, value);

        std::string out_string = writeJson(id, result);
        RespondMessageTo(connectionId, __func__, out_string);
    }

    void JsonRpcService::RespondFeatureSettingsDialogueEnhancement(
            int connectionId,
            const std::string& id,
            int dialogueEnhancementGainPreference,
            int dialogueEnhancementGain,
            int dialogueEnhancementLimitMin,
            int dialogueEnhancementLimitMax
    ){
        Json::Value value;
        value["dialogueEnhancementGainPreference"] = dialogueEnhancementGainPreference;
        value["dialogueEnhancementGain"] = dialogueEnhancementGain;
        Json::Value dialogueEnhancementLimit;
        dialogueEnhancementLimit["min"] = dialogueEnhancementLimitMin;
        dialogueEnhancementLimit["max"] = dialogueEnhancementLimitMax;
        value["dialogueEnhancementLimit"] = dialogueEnhancementLimit;

        Json::Value result;
        writeFeatureSettingsQuery(result, F_DIALOGUE_ENHANCEMENT, value);

        std::string out_string = writeJson(id, result);
        RespondMessageTo(connectionId, __func__, out_string);
    }

    void JsonRpcService::RespondFeatureSettingsUIMagnifier(
            int connectionId,
            const std::string& id,
            bool enabled,
            const std::string& magType
    ){
        Json::Value value;
        value["enabled"] = enabled;
        addOptionalProp(value, "magType", magType);

        Json::Value result;
        writeFeatureSettingsQuery(result, F_UI_MAGNIFIER, value);

        std::string out_string = writeJson(id, result);
        RespondMessageTo(connectionId, __func__, out_string);
    }

    void JsonRpcService::RespondFeatureSettingsHighContrastUI(
            int connectionId,
            const std::string& id,
            bool enabled,
            const std::string& hcType
    ){
        Json::Value value;
        value["enabled"] = enabled;
        addOptionalProp(value, "hcType", hcType);

        Json::Value result;
        writeFeatureSettingsQuery(result, F_HIGH_CONTRAST_UI, value);

        std::string out_string = writeJson(id, result);
        RespondMessageTo(connectionId, __func__, out_string);
    }

    void JsonRpcService::RespondFeatureSettingsScreenReader(
            int connectionId,
            const std::string& id,
            bool enabled,
            int speed,
            const std::string& voice,
            const std::string& language
    ){
        Json::Value value;
        value["enabled"] = enabled;
        addOptionalProp(value, "speed", speed);
        addOptionalProp(value, "voice", voice);
        addOptionalProp(value, "language", language);

        Json::Value result;
        writeFeatureSettingsQuery(result, F_SCREEN_READER, value);

        std::string out_string = writeJson(id, result);
        RespondMessageTo(connectionId, __func__, out_string);
    }

    void JsonRpcService::RespondFeatureSettingsResponseToUserAction(
            int connectionId,
            const std::string& id,
            bool enabled,
            const std::string& type
    ){
        Json::Value value;
        value["enabled"] = enabled;
        addOptionalProp(value, "hcType", type);

        Json::Value result;
        writeFeatureSettingsQuery(result, F_RESPONSE_TO_USER_ACTION, value);

        std::string out_string = writeJson(id, result);
        RespondMessageTo(connectionId, __func__, out_string);
    }

    void JsonRpcService::RespondFeatureSettingsAudioDescription(
            int connectionId,
            const std::string& id,
            bool enabled,
            int gainPreference,
            int panAzimuthPreference
    ){
        Json::Value value;
        value["enabled"] = enabled;
        addOptionalProp(value, "gainPreference", gainPreference);
        addOptionalProp(value, "panAzimuthPreference", panAzimuthPreference);

        Json::Value result;
        writeFeatureSettingsQuery(result, F_AUDIO_DESCRIPTION, value);

        std::string out_string = writeJson(id, result);
        RespondMessageTo(connectionId, __func__, out_string);
    }

    void JsonRpcService::RespondFeatureSettingsInVisionSigning(
            int connectionId,
            const std::string& id,
            bool enabled
    ){
        Json::Value value;
        value["enabled"] = enabled;

        Json::Value result;
        writeFeatureSettingsQuery(result, F_IN_VISION_SIGNING, value);

        std::string out_string = writeJson(id, result);
        RespondMessageTo(connectionId, __func__, out_string);
    }

    void JsonRpcService::RespondFeatureSuppress(
            int connectionId,
            const std::string& id,
            int feature,
            const std::string& value
    ){
        Json::Value result;
        result["method"] = MD_AF_FEATURE_SUPPRESS;
        std::vector<std::string> resultStringVector = {
                F_SUBTITLES, F_DIALOGUE_ENHANCEMENT,
                F_UI_MAGNIFIER, F_HIGH_CONTRAST_UI,
                F_SCREEN_READER, F_RESPONSE_TO_USER_ACTION,
                F_AUDIO_DESCRIPTION, F_IN_VISION_SIGNING};
        result["feature"] = resultStringVector[feature];
        result["value"] = value;

        std::string out_string = writeJson(id, result);
        RespondMessageTo(connectionId, __func__, out_string);
    }

    void JsonRpcService::RespondSubscribe(
            int connectionId,
            const std::string& id,
            bool subtitles, bool dialogueEnhancement,
            bool uiMagnifier, bool highContrastUI,
            bool screenReader, bool responseToUserAction,
            bool audioDescription, bool inVisionSigning
    ){
        Json::Value result;
        Json::Value msgTypeList(Json::arrayValue);
        if (subtitles)
            msgTypeList.append(PC_SUBTITLES);
        if (dialogueEnhancement)
            msgTypeList.append(PC_DIALOGUE_ENHANCEMENT);
        if (uiMagnifier)
            msgTypeList.append(PC_UI_MAGNIFIER);
        if (highContrastUI)
            msgTypeList.append(PC_HIGH_CONTRAST_UI);
        if (screenReader)
            msgTypeList.append(PC_SCREEN_READER);
        if (responseToUserAction)
            msgTypeList.append(PC_RESPONSE_TO_USER_ACTION);
        if (audioDescription)
            msgTypeList.append(PC_AUDIO_DESCRIPTION);
        if (inVisionSigning)
            msgTypeList.append(PC_IN_VISION_SIGNING);

        result["msgType"] = msgTypeList;
        std::string out_string = writeJson(id, result);
        RespondMessageTo(connectionId, __func__, out_string);
    }

    void JsonRpcService::RespondUnsubscribe(
            int connectionId,
            const std::string& id,
            bool subtitles, bool dialogueEnhancement,
            bool uiMagnifier, bool highContrastUI,
            bool screenReader, bool responseToUserAction,
            bool audioDescription, bool inVisionSigning
    ){
        Json::Value result;
        Json::Value msgTypeList(Json::arrayValue);
        if (subtitles)
            msgTypeList.append(PC_SUBTITLES);
        if (dialogueEnhancement)
            msgTypeList.append(PC_DIALOGUE_ENHANCEMENT);
        if (uiMagnifier)
            msgTypeList.append(PC_UI_MAGNIFIER);
        if (highContrastUI)
            msgTypeList.append(PC_HIGH_CONTRAST_UI);
        if (screenReader)
            msgTypeList.append(PC_SCREEN_READER);
        if (responseToUserAction)
            msgTypeList.append(PC_RESPONSE_TO_USER_ACTION);
        if (audioDescription)
            msgTypeList.append(PC_AUDIO_DESCRIPTION);
        if (inVisionSigning)
            msgTypeList.append(PC_IN_VISION_SIGNING);

        result["msgType"] = msgTypeList;

        std::string out_string = writeJson(id, result);
        RespondMessageTo(connectionId, __func__, out_string);
    }

    void JsonRpcService::addMethodsToJsonArray(Json::Value& jsonArray, const std::string stringList
    ){
        // Parse the input string
        LOG(LOG_INFO,"%s",stringList.c_str());
        size_t startPos = 0;
        size_t endPos = stringList.find(',');
        while (endPos != std::string::npos) {
            std::string element = stringList.substr(startPos + 1, endPos - startPos - 2);
            jsonArray.append(element);
            startPos = endPos + 1;  // Move past the comma and space
            endPos = stringList.find(',', startPos);
        }
        // Append the last element
        std::string lastElement = stringList.substr(startPos + 1, stringList.length() - startPos - 2);
        jsonArray.append(lastElement);
    }

    void JsonRpcService::RespondNegotiateMethods(
            int connectionId,
            const std::string& id,
            const std::string& terminalToApp,
            const std::string& appToTerminal
    ){
        Json::Value result;
        result["method"] = MD_NEGOTIATE_METHODS;

        Json::Value terminalToAppJsonArray(Json::arrayValue);
        addMethodsToJsonArray(terminalToAppJsonArray, terminalToApp);
        result["terminalToApp"] = terminalToAppJsonArray;

        Json::Value appToTerminalJsonArray(Json::arrayValue);
        addMethodsToJsonArray(appToTerminalJsonArray, appToTerminal);
        result["appToTerminal"] = appToTerminalJsonArray;

        std::string out_string = writeJson(id, result);
        RespondMessageTo(connectionId, __func__, out_string);
    }

    void JsonRpcService::RespondError(
            int connectionId,
            const std::string& id,
            int code,
            const std::string& message
    ){
        Json::Value error;
        error["code"] = code;
        error["message"] = message;

        std::string out_string = writeError(id, error);
        RespondMessageTo(connectionId, __func__, out_string);
    }

    void JsonRpcService::RespondError(
            int connectionId,
            const std::string& id,
            int code,
            const std::string& message,
            const std::string& data
    ){
        Json::Value error;
        error["code"] = code;
        error["message"] = message;
        addOptionalProp(error, "data", data);

        std::string out_string = writeError(id, error);
        RespondMessageTo(connectionId, __func__, out_string);
    }

    void JsonRpcService::SendIntentMediaPause(
            int connectionId,
            const std::string& id,
            const std::string& origin
    ){
        Json::Value params;
        params["origin"] = origin;

        std::string out_string = writeJson(id, MD_INTENT_MEDIA_PAUSE, params);
        RespondMessageTo(connectionId, __func__, out_string);
    }

    void JsonRpcService::SendIntentMediaPlay(
            int connectionId,
            const std::string& id,
            const std::string& origin
    ){
        Json::Value params;
        params["origin"] = origin;

        std::string out_string = writeJson(id, MD_INTENT_MEDIA_PLAY, params);
        RespondMessageTo(connectionId, __func__, out_string);
    }

    void JsonRpcService::SendIntentMediaFastForward(
            int connectionId,
            const std::string& id,
            const std::string& origin
    ){
        Json::Value params;
        params["origin"] = origin;

        std::string out_string = writeJson(id, MD_INTENT_MEDIA_FAST_FORWARD, params);
        RespondMessageTo(connectionId, __func__, out_string);
    }

    void JsonRpcService::SendIntentMediaFastReverse(
            int connectionId,
            const std::string& id,
            const std::string& origin
    ){
        Json::Value params;
        params["origin"] = origin;

        std::string out_string = writeJson(id, MD_INTENT_MEDIA_FAST_REVERSE, params);
        RespondMessageTo(connectionId, __func__, out_string);
    }

    void JsonRpcService::SendIntentMediaStop(
            int connectionId,
            const std::string& id,
            const std::string& origin
    ){
        Json::Value params;
        params["origin"] = origin;

        std::string out_string = writeJson(id, MD_INTENT_MEDIA_STOP, params);
        RespondMessageTo(connectionId, __func__, out_string);
    }

    void JsonRpcService::SendIntentMediaSeekContent(
            int connectionId,
            const std::string& id,
            const std::string& origin,
            const std::string& anchor,
            int offset
    ){
        Json::Value params;
        params["origin"] = origin;
        params["anchor"] = anchor;
        params["offset"] = offset;

        std::string out_string = writeJson(id, MD_INTENT_MEDIA_SEEK_CONTENT, params);
        RespondMessageTo(connectionId, __func__, out_string);
    }

    void JsonRpcService::SendIntentMediaSeekRelative(
            int connectionId,
            const std::string& id,
            const std::string& origin,
            int offset
    ){
        Json::Value params;
        params["origin"] = origin;
        params["offset"] = offset;

        std::string out_string = writeJson(id, MD_INTENT_MEDIA_SEEK_RELATIVE, params);
        RespondMessageTo(connectionId, __func__, out_string);
    }

    void JsonRpcService::SendIntentMediaSeekLive(
            int connectionId,
            const std::string& id,
            const std::string& origin,
            int offset
    ){
        Json::Value params;
        params["origin"] = origin;
        params["offset"] = offset;

        std::string out_string = writeJson(id, MD_INTENT_MEDIA_SEEK_LIVE, params);
        RespondMessageTo(connectionId, __func__, out_string);
    }

    void JsonRpcService::SendIntentMediaSeekWallclock(
            int connectionId,
            const std::string& id,
            const std::string& origin,
            const std::string& dateTime
    ){
        Json::Value params;
        params["origin"] = origin;
        params["date-time"] = dateTime;

        std::string out_string = writeJson(id, MD_INTENT_MEDIA_SEEK_WALLCLOCK, params);
        RespondMessageTo(connectionId, __func__, out_string);
    }

    void JsonRpcService::SendIntentSearch(
            int connectionId,
            const std::string& id,
            const std::string& origin,
            const std::string& query
    ){
        Json::Value params;
        params["origin"] = origin;
        params["query"] = query;

        std::string out_string = writeJson(id, MD_INTENT_SEARCH, params);
        RespondMessageTo(connectionId, __func__, out_string);
    }

    void JsonRpcService::SendIntentDisplay(
            int connectionId,
            const std::string& id,
            const std::string& origin,
            const std::string& mediaId
    ){
        Json::Value params;
        params["origin"] = origin;
        params["mediaId"] = mediaId;

        std::string out_string = writeJson(id, MD_INTENT_DISPLAY, params);
        RespondMessageTo(connectionId, __func__, out_string);
    }

    void JsonRpcService::SendIntentPlayback(
            int connectionId,
            const std::string& id,
            const std::string& origin,
            const std::string& mediaId,
            const std::string& anchor,
            int offset
    ){
        Json::Value params;
        params["origin"] = origin;
        params["mediaId"] = mediaId;
        addOptionalProp(params, "anchor", anchor);
        addOptionalProp(params, "offset", offset);

        std::string out_string = writeJson(id, MD_INTENT_PLAYBACK, params);
        RespondMessageTo(connectionId, __func__, out_string);
    }

    void JsonRpcService::NotifySubtitles(
            int connectionId,
            bool enabled,
            int size,
            const std::string& fontFamily,
            const std::string& textColour,
            int textOpacity,
            const std::string& edgeType,
            const std::string& edgeColour,
            const std::string& backgroundColour,
            int backgroundOpacity,
            const std::string& windowColour,
            int windowOpacity,
            const std::string& language
    ){
        Json::Value params;
        params["msgType"] = PC_SUBTITLES;
        Json::Value value;
        value["enabled"] = enabled;
        addOptionalProp(value, "size", size);
        addOptionalProp(value, "fontFamily", fontFamily);
        addOptionalProp(value, "textColour", textColour);
        addOptionalProp(value, "textOpacity", textOpacity);
        addOptionalProp(value, "edgeType", edgeType);
        addOptionalProp(value, "edgeColour", edgeColour);
        addOptionalProp(value, "backgroundColour", backgroundColour);
        addOptionalProp(value, "backgroundOpacity", backgroundOpacity);
        addOptionalProp(value, "windowColour", windowColour);
        addOptionalProp(value, "windowOpacity", windowOpacity);
        addOptionalProp(value, "language", language);
        params["value"] = value;

        std::string out_string = writeJsonForNotify(params);
        RespondMessageTo(connectionId, __func__, out_string);
    }

    void JsonRpcService::NotifyDialogueEnhancement(
            int connectionId,
            int dialogueEnhancementGainPreference,
            int dialogueEnhancementGain,
            int dialogueEnhancementLimitMin,
            int dialogueEnhancementLimitMax
    ){
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

        std::string out_string = writeJsonForNotify(params);
        RespondMessageTo(connectionId, __func__, out_string);
    }

    void JsonRpcService::NotifyUIMagnifier(
            int connectionId,
            bool enabled,
            const std::string& magType
    ){
        Json::Value params;
        params["msgType"] = PC_UI_MAGNIFIER;
        Json::Value value;
        value["enabled"] = enabled;
        addOptionalProp(value, "magType", magType);
        params["value"] = value;

        std::string out_string = writeJsonForNotify(params);
        RespondMessageTo(connectionId, __func__, out_string);
    }

    void JsonRpcService::NotifyHighContrastUI(
            int connectionId,
            bool enabled,
            const std::string& hcType
    ){
        Json::Value params;
        params["msgType"] = PC_HIGH_CONTRAST_UI;
        Json::Value value;
        value["enabled"] = enabled;
        addOptionalProp(value, "hcType", hcType);
        params["value"] = value;

        std::string out_string = writeJsonForNotify(params);
        RespondMessageTo(connectionId, __func__, out_string);
    }

    void JsonRpcService::NotifyScreenReader(
            int connectionId,
            bool enabled,
            int speed,
            const std::string& voice,
            const std::string& language
    ){
        Json::Value params;
        params["msgType"] = PC_SCREEN_READER;

        Json::Value value;
        value["enabled"] = enabled;
        addOptionalProp(value, "speed", speed);
        addOptionalProp(value, "voice", voice);
        addOptionalProp(value, "language", language);
        params["value"] = value;

        std::string out_string = writeJsonForNotify(params);
        RespondMessageTo(connectionId, __func__, out_string);
    }

    void JsonRpcService::NotifyResponseToUserAction(
            int connectionId,
            bool enabled,
            const std::string& type
    ){
        Json::Value params;
        params["msgType"] = PC_RESPONSE_TO_USER_ACTION;

        Json::Value value;
        value["enabled"] = enabled;
        addOptionalProp(value, "hcType", type);

        params["value"] = value;

        std::string out_string = writeJsonForNotify(params);
        RespondMessageTo(connectionId, __func__, out_string);
    }

    void JsonRpcService::NotifyAudioDescription(
            int connectionId,
            bool enabled,
            int gainPreference,
            int panAzimuthPreference
    ){
        Json::Value params;
        params["msgType"] = PC_AUDIO_DESCRIPTION;

        Json::Value value;
        value["enabled"] = enabled;
        addOptionalProp(value, "gainPreference", gainPreference);
        addOptionalProp(value, "panAzimuthPreference", panAzimuthPreference);

        params["value"] = value;

        std::string out_string = writeJsonForNotify(params);
        RespondMessageTo(connectionId, __func__, out_string);
    }

    void JsonRpcService::NotifyInVisionSigning(
            int connectionId,
            bool enabled
    ){
        Json::Value params;
        params["msgType"] = PC_IN_VISION_SIGNING;

        Json::Value value;
        value["enabled"] = enabled;

        params["value"] = value;

        std::string out_string = writeJsonForNotify(params);
        RespondMessageTo(connectionId, __func__, out_string);
    }


    void JsonRpcService::RespondDialogueEnhancementOverride(
            int connectionId,
            const std::string& id,
            int dialogueEnhancementGain
    ){
        Json::Value result;
        result["method"] = MD_AF_DIALOGUE_ENHANCEMENT_OVERRIDE;
        result["dialogueEnhancementGain"] = dialogueEnhancementGain;

        std::string out_string = writeJson(id, result);
        RespondMessageTo(connectionId, __func__, out_string);
    }

    void JsonRpcService::RespondTriggerResponseToUserAction(
            int connectionId,
            const std::string& id,
            bool actioned
    ){
        Json::Value result;
        result["method"] = MD_AF_TRIGGER_RESPONSE_TO_USER_ACTION;
        result["actioned"] = actioned;

        std::string out_string = writeJson(id, result);
        RespondMessageTo(connectionId, __func__, out_string);
    }
} // namespace NetworkServices
