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
#include <list>

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

    void JsonRpcService::OnMessageReceived(WebSocketConnection *connection, const std::string &text)
    {
        LOG(LOG_INFO, "Message received: connection=%d, text=%s", connection->Id(), text.c_str());

        Json::Value obj;
        Json::Reader reader;
        bool b = reader.parse(text, obj);
        std::map<std::string, int> mapOfFeatures;
        mapOfFeatures["subtitles"]            = 0;
        mapOfFeatures["dialogueEnhancement"]  = 1;
        mapOfFeatures["uiMagnifier"]          = 2;
        mapOfFeatures["highContrastUI"]       = 3;
        mapOfFeatures["screenReader"]         = 4;
        mapOfFeatures["responseToUserAction"] = 5;
        mapOfFeatures["audioDescription"]     = 6;
        mapOfFeatures["inVisionSigning"]      = 7;

        if (!b){
            LOG(LOG_INFO, "Error, json rpc parse wrong");
            //call return
        }
        if (obj["jsonrpc"]!="2.0"){
            LOG(LOG_INFO, "Error, json rpc version not 2.0");
            //call RespondDialogueEnhancementOverride to return
        }
        if (!obj.isMember("result")){
            LOG(LOG_INFO, "It is a request");
        }
        else{
            //case Cr
            Json::Value result = obj["result"];
            std::string method = result["method"].asString();
            std::string id = obj["method"].asString();
            if (obj["id"].type()==Json::stringValue){
                id = "STR" + id;
            }
            else{
                id = "NUM" + id;
            }
//            m_sessionCallback->ReceiveIntentConfirm(connection->Id(), id, method);
        }

        if (!obj.isMember("error")) {
            Json::Value error = obj["error"];
            int code = error["code"].asInt();
            std::string message = error["message"].asString();
            std::string id = obj["id"].asString();
            if (obj["id"].type()==Json::stringValue){
                id = "STR" + id;
            }
            else{
                id = "NUM" + id;
            }
//            m_sessionCallback->ReceiveError(connection->Id(), id, code, message);
            //call return
        }
        if (!obj.isMember("method")){
            LOG(LOG_INFO, "Cannot find method");
            //call return
        }
        std::string method = obj["method"].asString();

        if (method == "org.hbbtv.af.dialogueEnhancementOverride"){
            //CASE F 1

            int dialogueEnhancementGain;
            if (obj.isMember("dialogueEnhancementGain")) dialogueEnhancementGain = obj["dialogueEnhancementGain"].asInt();

            if (!obj.isMember("id")){
                LOG(LOG_INFO, "Cannot find id");
            }

            std::string id = obj["id"].asString();

            if (obj["id"].type()==Json::stringValue){
                id = "STR" + id;
            }
            else{
                id = "NUM" + id;
            }
            LOG(LOG_INFO, "JSON-RPC-EXAMPLE #2: Service received request. Call session callback...");
//            m_sessionCallback->ResquestDialogueEnhancementOverride(connection->Id(), id, dialogueEnhancementGain);
        }


        Json::Value params = obj["params"];
        if (method == "org.hbbtv.af.featureSupportInfo"){
            //CASE A1
            if (!params.isMember("feature")){
                LOG(LOG_INFO, "Cannot find feature");
                //call return
            }
            std::string feature = params["feature"].asString();
            if (!obj.isMember("id")){
                LOG(LOG_INFO, "Cannot find id");
            }
            std::string id = obj["id"].asString();

            if (obj["id"].type()==Json::stringValue){
                id = "STR" + id;
            }
            else{
                id = "NUM" + id;
            }
            LOG(LOG_INFO, "JSON-RPC-EXAMPLE #2: Service received request. Call session callback...");
//            m_sessionCallback->ResquestFeatureSupportInfo(connection->Id(), id, mapOfFeatures[feature]);
        }
        else if (method == "org.hbbtv.af.featureSettingsQuery"){
            //CASE A2
            if (!params.isMember("feature") ){
                LOG(LOG_INFO, "Cannot find feature");
                //call return
            }
            std::string feature = params["feature"].asString();
            if (!obj.isMember("id")){
                LOG(LOG_INFO, "Cannot find id");
            }
            std::string id = obj["id"].asString();

            if (obj["id"].type()==Json::stringValue){
                id = "STR" + id;
            }
            else{
                id = "NUM" + id;
            }
            LOG(LOG_INFO, "JSON-RPC-EXAMPLE #2: Service received request. Call session callback...");
//            m_sessionCallback->ResquestFeatureSettingsQuery(connection->Id(), id, mapOfFeatures[feature]);
        }
        else if (method == "org.hbbtv.af.featureSuppress"){
            //CASE A3
            if (!params.isMember("feature")){
                LOG(LOG_INFO, "Cannot find feature");
                //call return
            }
            std::string feature = params["feature"].asString();
            if (!obj.isMember("id")){
                LOG(LOG_INFO, "Cannot find id");
            }
            std::string id = obj["id"].asString();

            if (obj["id"].type()==Json::stringValue){
                id = "STR" + id;
            }
            else{
                id = "NUM" + id;
            }
            LOG(LOG_INFO, "JSON-RPC-EXAMPLE #2: Service received request. Call session callback...");
//            m_sessionCallback->ResquestFeatureSuppress(connection->Id(), id, mapOfFeatures[feature]);
        }
        else if (method == "org.hbbtv.subscribe"){
            //CASE B1
            if (!params.isMember("msgType")){
                LOG(LOG_INFO, "Cannot find msgType");
                //call return
            }
            Json::Value msgType = params["msgType"];
            if (!obj.isMember("id")){
                LOG(LOG_INFO, "Cannot find id");
            }
            std::string id = obj["id"].asString();

            if (obj["id"].type()==Json::stringValue){
                id = "STR" + id;
            }
            else{
                id = "NUM" + id;
            }

            bool msgTypeBoolList[8]={false};
            for (auto msg : msgType){
                int length = msg.asString().length();
                std::string s = msg.asString().substr(0,length - 10);  //remove the PrefChange
                if (mapOfFeatures.find(s)!=mapOfFeatures.end()){
                    msgTypeBoolList[mapOfFeatures[s]]=true;
                    LOG(LOG_INFO, "Changed one to true");
                }
                else{
                    LOG(LOG_INFO, "Cannot find msgType");
                }
            }
//            m_sessionCallback->RequestSubscribe(connection->Id(), id, msgTypeBoolList[0], msgTypeBoolList[1],msgTypeBoolList[2],msgTypeBoolList[3],msgTypeBoolList[4],msgTypeBoolList[5],msgTypeBoolList[6],msgTypeBoolList[7]);
        }
        else if (method == "org.hbbtv.unsubscribe"){
            //CASE B2
            if (!params.isMember("msgType")){
                LOG(LOG_INFO, "Cannot find msgType");
                //call return
            }
            Json::Value msgType = params["msgType"];
            if (!obj.isMember("id")){
                LOG(LOG_INFO, "Cannot find id");
            }
            std::string id = obj["id"].asString();

            if (obj["id"].type()==Json::stringValue){
                id = "STR" + id;
            }
            else{
                id = "NUM" + id;
            }
            bool msgTypeBoolList[8]={false};
            for (auto msg : msgType){
                int length = msg.asString().length();
                std::string s = msg.asString().substr(0,length - 10);  //remove the PrefChange
                if (mapOfFeatures.find(s)!=mapOfFeatures.end()){
                    msgTypeBoolList[mapOfFeatures[s]]=true;
                }
                else{
                    LOG(LOG_INFO, "Cannot find msgType");
                }
            }
//            m_sessionCallback->RequestUnsubscribe(connection->Id(), id, msgTypeBoolList[0], msgTypeBoolList[1],msgTypeBoolList[2],msgTypeBoolList[3],msgTypeBoolList[4],msgTypeBoolList[5],msgTypeBoolList[6],msgTypeBoolList[7]);
        }
        else if (method == "org.hbbtv.app.voice.ready"){
            //CASE D1
            if (!params.isMember("ready")){
                LOG(LOG_INFO, "Cannot find ready");
                //call return
            }
            std::string ready = params["ready"].asString();
            LOG(LOG_INFO, "JSON-RPC-EXAMPLE #2: Service received request. Call session callback...");
    //        m_sessionCallback->NotifyVoiceReady(connection->Id(), ready);

        }
        else if (method == "org.hbbtv.app.state.media"){
            //CASE D2
            if (!params.isMember("state")){
                LOG(LOG_INFO, "Cannot find state");
                //call return
            }
            std::string state = params["state"].asString();

            std::string kind = "";
            if (!params.isMember("kind")){
                LOG(LOG_INFO, "Cannot find kind");
                if (state=="buffering" or state=="paused" or state=="playing" or state=="stopped"){
                    //call return
                }
            }
            else{
                kind = params["kind"].asString();
            }

            std::string type = "";
            if (!params.isMember("type")){
                LOG(LOG_INFO, "Cannot find type");
                if (state=="buffering" or state=="paused" or state=="playing" or state=="stopped"){
                    //call return
                }
            }
            else{
                type = params["type"].asString();
            }

            std::string currentTime = "";
            if (!params.isMember("currentTime")){
                LOG(LOG_INFO, "Cannot find currentTime");
                if (state=="buffering" or state=="paused" or state=="playing"){
                    //call return
                }
            }
            else{
                currentTime = params["currentTime"].asString();
            }

            std::string rangeStart = "";
            std::string rangeEnd = "";
            if (state=="buffering" or state=="paused" or state=="playing"){
                if (!params.isMember("range")){
                    LOG(LOG_INFO, "Cannot find range");
                    //call return
                }
                if (!params["range"].isMember("rangeStart")){
                    LOG(LOG_INFO, "Cannot find rangeStart");
                    //call return
                }
                if (!params["range"].isMember("rangeEnd")){
                    LOG(LOG_INFO, "Cannot find rangeEnd");
                    //call return
                }
                rangeStart = params["range"]["start"].asString();
                rangeEnd = params["range"]["end"].asString();
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


            Json::Value actions =  params["availableActions"];
            if (actions.isMember("pause") and actions["pause"]==true) actPause=true;
            if (actions.isMember("play") and actions["play"]==true) actPlay=true;
            if (actions.isMember("fast-forward") and actions["fast-forward"]==true) actFastForward=true;
            if (actions.isMember("fast-reverse") and actions["fast-reverse"]==true) actFastReverse=true;
            if (actions.isMember("stop") and actions["stop"]==true) actStop=true;
            if (actions.isMember("seek-content") and actions["seek-content"]==true) actSeekContent=true;
            if (actions.isMember("seek-relative") and actions["seek-relative"]==true) actSeekRelative=true;
            if (actions.isMember("seek-live") and actions["seek-live"]==true) actSeekLive=true;
            if (actions.isMember("seek-wallclock") and actions["seek-wallclock"]==true) actWallclock=true;

            std::string mediaId = NullStrValue;
            std::string title = NullStrValue;
            std::string secTitle = NullStrValue;
            std::string synopsis = NullStrValue;

            if (state=="buffering" or state=="paused" or state=="playing" or state=="stopped"){
                if (!params.isMember("metadata")){
                    LOG(LOG_INFO, "Cannot find metadata");
                    //call return
                }
                if (!params["metadata"].isMember("title")){
                    LOG(LOG_INFO, "Cannot find title");
                    //call return
                }
                if (params["metadata"].isMember("mediaId")) mediaId = params["metadata"]["mediaId"].asString();
                title = params["metadata"]["title"].asString();
                if (params["metadata"].isMember("secTitle")) secTitle = params["metadata"]["secTitle"].asString();
                if (params["metadata"].isMember("synopsis")) synopsis = params["metadata"]["synopsis"].asString();
            }


            bool subtitlesEnabled = false;
            bool subtitlesAvailable = false;
            bool audioDescripEnabled = false;
            bool audioDescripAvailable = false;
            bool signLangEnabled = false;
            bool signLangAvailable = false;

            if (state=="buffering" or state=="paused" or state=="playing"){
                if (!params.isMember("accessibility")){
                    LOG(LOG_INFO, "Cannot find accessibility");
                    //call return
                }
                if (!params["accessibility"].isMember("subtitles")){
                    LOG(LOG_INFO, "Cannot find subtitles");
                    //call return
                }
                if (!params["accessibility"].isMember("audioDescription")){
                    LOG(LOG_INFO, "Cannot find audioDescription");
                    //call return
                }
                if (!params["accessibility"].isMember("signLanguage")){
                    LOG(LOG_INFO, "Cannot find signLanguage");
                    //call return
                }
                subtitlesEnabled = params["accessibility"]["subtitles"]["enabled"].asBool();
                subtitlesAvailable = params["accessibility"]["subtitles"]["available"].asBool();
                audioDescripEnabled = params["accessibility"]["audioDescription"]["enabled"].asBool();
                audioDescripAvailable = params["accessibility"]["audioDescription"]["available"].asBool();
                signLangEnabled = params["accessibility"]["signLanguage"]["enabled"].asBool();
                signLangAvailable = params["accessibility"]["signLanguage"]["available"].asBool();
            }
//            m_sessionCallback->NotifyStateMedia(connection->Id(), state, kind, type, currentTime, rangeStart, rangeEnd, actPause, actPlay, actFastForward, actFastReverse, actStop, actSeekContent, actSeekRelative, actSeekLive, actWallclock, mediaId, title, secTitle, synopsis, subtitlesEnabled, subtitlesAvailable, audioDescripEnabled, audioDescripAvailable, signLangEnabled, signLangAvailable);

        }
        else if (method == "org.hbbtv.negotiateMethods"){
            //CASE N negotiateMethods
            std::string terminalToApp = NullStrValue;

            if (!params.isMember("terminalToApp")){
                LOG(LOG_INFO, "Cannot find terminalToApp");
            }
            else{
                terminalToApp = params["terminalToApp"].asString();
            }
            std::string appToTerminal = NullStrValue;
            if (!params.isMember("appToTerminal")){
                LOG(LOG_INFO, "Cannot find appToTerminal");
            }
            else{
                appToTerminal = params["appToTerminal"].asString();
            }

            // Remove square brackets
            appToTerminal.erase(appToTerminal.begin(), appToTerminal.begin() + 1);
            appToTerminal.erase(appToTerminal.end() - 1, appToTerminal.end());

            // Remove newline characters
            size_t pos;
            while ((pos = appToTerminal.find('\n')) != std::string::npos) {
                appToTerminal.erase(pos, 1);
            }

            // Remove square brackets
            terminalToApp.erase(terminalToApp.begin(), terminalToApp.begin() + 1);
            terminalToApp.erase(terminalToApp.end() - 1, terminalToApp.end());

            // Remove newline characters
            while ((pos = terminalToApp.find('\n')) != std::string::npos) {
                terminalToApp.erase(pos, 1);
            }

            LOG(LOG_INFO, "JSON-RPC-EXAMPLE #2: Service received request. Call session callback...");

            std::string id = obj["id"].asString();

            if (obj["id"].type()==Json::stringValue){
                id = "STR" + id;
            }
            else{
                id = "NUM" + id;
            }
//            m_sessionCallback->RequestNegotiateMethods(connection->Id(), id, terminalToApp, appToTerminal);
        }
        else if (method == "org.hbbtv.af.triggerResponseToUserAction"){
            //CASE F 2

            if (!params.isMember("magnitude")){
                LOG(LOG_INFO, "Cannot find magnitude");
            }
            std::string magnitude = params["magnitude"].asString();

            LOG(LOG_INFO, "JSON-RPC-EXAMPLE #2: Service received request. Call session callback...");

            std::string id = obj["id"].asString();

            if (obj["id"].type()==Json::stringValue){
                id = "STR" + id;
            }
            else{
                id = "NUM" + id;
            }
//            m_sessionCallback->RequestTriggerResponseToUserAction(connection->Id(), id, magnitude);
        }
    }

    void JsonRpcService::OnDisconnected(WebSocketConnection *connection)
    {

    }

    void JsonRpcService::OnServiceStopped()
    {

    }

    void JsonRpcService::RespondFeatureSupportInfo(
            int connectionId,
            const std::string& id,
            int feature,
            const std::string& value
            ){
        LOG(LOG_INFO, "JSON-RPC-EXAMPLE #9: Service called with response. Send response to client...");
        //Case Ar 1
        connections_mutex_.lock();
        WebSocketConnection *connection = GetConnection(connectionId);
        if (connection != nullptr)
        {
            Json::Value jsonResponse;
            jsonResponse["jsonrpc"] = "2.0";
            Json::Value result;
            result["method"] = "org.hbbtv.af.featureSupportInfo";
            std::vector<std::string> resultStringVector = {"subtitles", "dialogueEnhancement", "uiMagnifier", "highContrastUI", "screenReader", "responseToUserAction", "audioDescription", "inVisionSigning"};
            result["feature"] = resultStringVector[feature];
            result["value"] = value;
            jsonResponse["result"] = result;
            if (id.substr(0,3)=="STR"){
                jsonResponse["id"] = id.substr(3, id.length()-3);
            }
            else{
                jsonResponse["id"] =  std::stoll(id.substr(3, id.length()-3));
            }
            connection->SendMessage(jsonResponse.asString());
        }
        connections_mutex_.unlock();
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
        LOG(LOG_INFO, "JSON-RPC-EXAMPLE #9: Service called with response. Send response to client...");
        //case Arc 1
        connections_mutex_.lock();
        WebSocketConnection *connection = GetConnection(connectionId);
        if (connection != nullptr)
        {
            Json::Value jsonResponse;
            jsonResponse["jsonrpc"] = "2.0";
            Json::Value result;
            result["method"] = "org.hbbtv.af.featureSettingsQuery";
            result["feature"] = "subtitles";

            Json::Value value;
            value["enabled"] = enabled;
            if (size != NullIntValue) value["size"] = size;
            if (fontFamily != NullStrValue) value["fontFamily"] = fontFamily;
            if (textColour != NullStrValue) value["textColour"] = textColour;
            if (textOpacity != NullIntValue) value["textOpacity"] = textOpacity;
            if (textColour != NullStrValue) value["edgeType"] = edgeType;
            if (edgeColour != NullStrValue) value["edgeColour"] = edgeColour;
            if (backgroundColour != NullStrValue) value["backgroundColour"] = backgroundColour;
            if (backgroundOpacity != NullIntValue) value["backgroundOpacity"] = backgroundOpacity;
            if (windowColour != NullStrValue) value["windowColour"] = windowColour;
            if (windowOpacity != NullIntValue) value["windowOpacity"] = windowOpacity;
            if (language != NullStrValue) value["language"] = language;


            result["value"] = value;
            jsonResponse["result"] = result;

            if (id.substr(0,3)=="STR"){
                jsonResponse["id"] = id.substr(3, id.length()-3);
            }
            else{
                jsonResponse["id"] =  std::stoll(id.substr(3, id.length()-3));
            }
            connection->SendMessage(jsonResponse.asString());
        }
        connections_mutex_.unlock();
    }

    void JsonRpcService::RespondFeatureSettingsDialogueEnhancement(
            int connectionId,
            const std::string& id,
            int dialogueEnhancementGainPreference,
            int dialogueEnhancementGain,
            int dialogueEnhancementLimitMin,
            int dialogueEnhancementLimitMax
    ){
        //case Arc 2
        LOG(LOG_INFO, "JSON-RPC-EXAMPLE #9: Service called with response. Send response to client...");
        connections_mutex_.lock();
        WebSocketConnection *connection = GetConnection(connectionId);
        if (connection != nullptr)
        {
            Json::Value jsonResponse;
            jsonResponse["jsonrpc"] = "2.0";
            Json::Value result;
            result["method"] = "org.hbbtv.af.featureSettingsQuery";
            result["feature"] = "dialogueEnhancement";

            Json::Value value;
            value["dialogueEnhancementGainPreference"] = dialogueEnhancementGainPreference;
            value["dialogueEnhancementGain"] = dialogueEnhancementGain;
            Json::Value dialogueEnhancementLimit;
            dialogueEnhancementLimit["min"] = dialogueEnhancementLimitMin;
            dialogueEnhancementLimit["max"] = dialogueEnhancementLimitMax;

            value["dialogueEnhancementLimit"] = dialogueEnhancementLimit;
            result["value"] = value;
            jsonResponse["result"] = result;

            if (id.substr(0,3)=="STR"){
                jsonResponse["id"] = id.substr(3, id.length()-3);
            }
            else{
                jsonResponse["id"] =  std::stoll(id.substr(3, id.length()-3));
            }
            connection->SendMessage(jsonResponse.asString());
        }
        connections_mutex_.unlock();
    }

    void JsonRpcService::RespondFeatureSettingsUIMagnifier(
            int connectionId,
            const std::string& id,
            bool enabled,
            const std::string& magType
    ){
        //case Arc 3
        LOG(LOG_INFO, "JSON-RPC-EXAMPLE #9: Service called with response. Send response to client...");
        connections_mutex_.lock();
        WebSocketConnection *connection = GetConnection(connectionId);
        if (connection != nullptr)
        {
            Json::Value jsonResponse;
            jsonResponse["jsonrpc"] = "2.0";
            Json::Value result;
            result["method"] = "org.hbbtv.af.featureSettingsQuery";
            result["feature"] = "uiMagnifier";

            Json::Value value;
            value["enabled"] = enabled;
            if (magType != NullStrValue) value["magType"] = magType;

            result["value"] = value;
            jsonResponse["result"] = result;

            if (id.substr(0,3)=="STR"){
                jsonResponse["id"] = id.substr(3, id.length()-3);
            }
            else{
                jsonResponse["id"] =  std::stoll(id.substr(3, id.length()-3));
            }
            connection->SendMessage(jsonResponse.asString());
        }
        connections_mutex_.unlock();
    }

    void JsonRpcService::RespondFeatureSettingsHighContrastUI(
            int connectionId,
            const std::string& id,
            bool enabled,
            const std::string& hcType
    ){
        //case Arc 4
        LOG(LOG_INFO, "JSON-RPC-EXAMPLE #9: Service called with response. Send response to client...");
        connections_mutex_.lock();
        WebSocketConnection *connection = GetConnection(connectionId);
        if (connection != nullptr)
        {
            Json::Value jsonResponse;
            jsonResponse["jsonrpc"] = "2.0";
            Json::Value result;
            result["method"] = "org.hbbtv.af.featureSettingsQuery";
            result["feature"] = "highContrastUI";

            Json::Value value;
            value["enabled"] = enabled;
            if (hcType != NullStrValue) value["hcType"] = hcType;

            result["value"] = value;
            jsonResponse["result"] = result;

            if (id.substr(0,3)=="STR"){
                jsonResponse["id"] = id.substr(3, id.length()-3);
            }
            else{
                jsonResponse["id"] =  std::stoll(id.substr(3, id.length()-3));
            }
            connection->SendMessage(jsonResponse.asString());
        }
        connections_mutex_.unlock();
    }

    void JsonRpcService::RespondFeatureSettingsScreenReader(
            int connectionId,
            const std::string& id,
            bool enabled,
            int speed,
            const std::string& voice,
            const std::string& language
    ){
        //case Arc 5
        LOG(LOG_INFO, "JSON-RPC-EXAMPLE #9: Service called with response. Send response to client...");
        connections_mutex_.lock();
        WebSocketConnection *connection = GetConnection(connectionId);
        if (connection != nullptr)
        {
            Json::Value jsonResponse;
            jsonResponse["jsonrpc"] = "2.0";
            Json::Value result;
            result["method"] = "org.hbbtv.af.featureSettingsQuery";
            result["feature"] = "screenReader";

            Json::Value value;
            value["enabled"] = enabled;
            if (speed != NullIntValue) value["speed"] = speed;
            if (voice != NullStrValue) value["voice"] = voice;
            if (language != NullStrValue) value["language"] = language;

            result["value"] = value;
            jsonResponse["result"] = result;

            if (id.substr(0,3)=="STR"){
                jsonResponse["id"] = id.substr(3, id.length()-3);
            }
            else{
                jsonResponse["id"] =  std::stoll(id.substr(3, id.length()-3));
            }
            connection->SendMessage(jsonResponse.asString());
        }
        connections_mutex_.unlock();
    }
    void JsonRpcService::RespondFeatureSettingsResponseToUserAction(
            int connectionId,
            const std::string& id,
            bool enabled,
            const std::string& type
    ){
        //case Arc 6
        LOG(LOG_INFO, "JSON-RPC-EXAMPLE #9: Service called with response. Send response to client...");
        connections_mutex_.lock();
        WebSocketConnection *connection = GetConnection(connectionId);
        if (connection != nullptr)
        {
            Json::Value jsonResponse;
            jsonResponse["jsonrpc"] = "2.0";
            Json::Value result;
            result["method"] = "org.hbbtv.af.featureSettingsQuery";
            result["feature"] = "responseToUserAction";

            Json::Value value;
            value["enabled"] = enabled;
            if (type != NullStrValue) value["hcType"] = type;

            result["value"] = value;
            jsonResponse["result"] = result;

            if (id.substr(0,3)=="STR"){
                jsonResponse["id"] = id.substr(3, id.length()-3);
            }
            else{
                jsonResponse["id"] =  std::stoll(id.substr(3, id.length()-3));
            }
            connection->SendMessage(jsonResponse.asString());
        }
        connections_mutex_.unlock();
    }

    void JsonRpcService::RespondFeatureSettingsAudioDescription(
            int connectionId,
            const std::string& id,
            bool enabled,
            int gainPreference,
            int panAzimuthPreference
    ){
        //case Arc 7
        LOG(LOG_INFO, "JSON-RPC-EXAMPLE #9: Service called with response. Send response to client...");
        connections_mutex_.lock();
        WebSocketConnection *connection = GetConnection(connectionId);
        if (connection != nullptr)
        {
            Json::Value jsonResponse;
            jsonResponse["jsonrpc"] = "2.0";
            Json::Value result;
            result["method"] = "org.hbbtv.af.featureSettingsQuery";
            result["feature"] = "audioDescription";

            Json::Value value;
            value["enabled"] = enabled;
            if (gainPreference != NullIntValue) value["gainPreference"] = gainPreference;
            if (panAzimuthPreference != NullIntValue) value["panAzimuthPreference"] = panAzimuthPreference;

            result["value"] = value;
            jsonResponse["result"] = result;

            if (id.substr(0,3)=="STR"){
                jsonResponse["id"] = id.substr(3, id.length()-3);
            }
            else{
                jsonResponse["id"] =  std::stoll(id.substr(3, id.length()-3));
            }
            connection->SendMessage(jsonResponse.asString());
        }
        connections_mutex_.unlock();
    }

    void JsonRpcService::RespondFeatureSettingsInVisionSigning(
            int connectionId,
            const std::string& id,
            bool enabled
    ){
        //case Arc 8
        LOG(LOG_INFO, "JSON-RPC-EXAMPLE #9: Service called with response. Send response to client...");
        connections_mutex_.lock();
        WebSocketConnection *connection = GetConnection(connectionId);
        if (connection != nullptr)
        {
            Json::Value jsonResponse;
            jsonResponse["jsonrpc"] = "2.0";
            Json::Value result;
            result["method"] = "org.hbbtv.af.featureSettingsQuery";
            result["feature"] = "inVisionSigning";

            Json::Value value;
            value["enabled"] = enabled;

            result["value"] = value;
            jsonResponse["result"] = result;

            if (id.substr(0,3)=="STR"){
                jsonResponse["id"] = id.substr(3, id.length()-3);
            }
            else{
                jsonResponse["id"] =  std::stoll(id.substr(3, id.length()-3));
            }
            connection->SendMessage(jsonResponse.asString());
        }
        connections_mutex_.unlock();
    }

    void JsonRpcService::RespondFeatureSuppress(
            int connectionId,
            const std::string& id,
            int feature,
            const std::string& value
    ){
        LOG(LOG_INFO, "JSON-RPC-EXAMPLE #9: Service called with response. Send response to client...");
        //Case Ar 2
        connections_mutex_.lock();
        WebSocketConnection *connection = GetConnection(connectionId);
        if (connection != nullptr)
        {
            Json::Value jsonResponse;
            jsonResponse["jsonrpc"] = "2.0";
            Json::Value result;
            result["method"] = "org.hbbtv.af.featureSuppress";
            std::vector<std::string> resultStringVector = {"subtitles", "dialogueEnhancement", "uiMagnifier", "highContrastUI", "screenReader", "responseToUserAction", "audioDescription", "inVisionSigning"};
            result["feature"] = resultStringVector[feature];
            result["value"] = value;
            jsonResponse["result"] = result;
            if (id.substr(0,3)=="STR"){
                jsonResponse["id"] = id.substr(3, id.length()-3);
            }
            else{
                jsonResponse["id"] =  std::stoll(id.substr(3, id.length()-3));
            }
            connection->SendMessage(jsonResponse.asString());
        }
        connections_mutex_.unlock();
    }
    void JsonRpcService::RespondSubscribe(
            int connectionId,
            const std::string& id,
            bool msgType0,
            bool msgType1,
            bool msgType2,
            bool msgType3,
            bool msgType4,
            bool msgType5,
            bool msgType6,
            bool msgType7
    ){
        LOG(LOG_INFO, "JSON-RPC-EXAMPLE #9: Service called with response. Send response to client...");
        //Case Br
        connections_mutex_.lock();
        WebSocketConnection *connection = GetConnection(connectionId);
        if (connection != nullptr)
        {
            Json::Value jsonResponse;
            jsonResponse["jsonrpc"] = "2.0";
            Json::Value result;
            Json::Value msgTypeList(Json::arrayValue);

            if (msgType0) msgTypeList.append("subtitlesPrefChange");
            if (msgType1) msgTypeList.append("dialogueEnhancementPrefChange");
            if (msgType2) msgTypeList.append("uiMagnifierPrefChange");
            if (msgType3) msgTypeList.append("highContrastUIPrefChange");
            if (msgType4) msgTypeList.append("screenReaderPrefChange");
            if (msgType5) msgTypeList.append("responseToUserActionPrefChange");
            if (msgType6) msgTypeList.append("audioDescriptionPrefChange");
            if (msgType7) msgTypeList.append("inVisionSigningPrefChange");

            result["msgType"] = msgTypeList;
            jsonResponse["result"] = result;

            if (id.substr(0,3)=="STR"){
                jsonResponse["id"] = id.substr(3, id.length()-3);
            }
            else{
                jsonResponse["id"] =  std::stoll(id.substr(3, id.length()-3));
            }
            connection->SendMessage(jsonResponse.asString());
        }
        connections_mutex_.unlock();
    }
    void JsonRpcService::RespondUnsubscribe(
            int connectionId,
            const std::string& id,
            bool msgType0,
            bool msgType1,
            bool msgType2,
            bool msgType3,
            bool msgType4,
            bool msgType5,
            bool msgType6,
            bool msgType7
    ){
        LOG(LOG_INFO, "JSON-RPC-EXAMPLE #9: Service called with response. Send response to client...");
        //Case Br
        connections_mutex_.lock();
        WebSocketConnection *connection = GetConnection(connectionId);
        if (connection != nullptr)
        {
            Json::Value jsonResponse;
            jsonResponse["jsonrpc"] = "2.0";
            Json::Value result;
            Json::Value msgTypeList(Json::arrayValue);

            if (msgType0) msgTypeList.append("subtitlesPrefChange");
            if (msgType1) msgTypeList.append("dialogueEnhancementPrefChange");
            if (msgType2) msgTypeList.append("uiMagnifierPrefChange");
            if (msgType3) msgTypeList.append("highContrastUIPrefChange");
            if (msgType4) msgTypeList.append("screenReaderPrefChange");
            if (msgType5) msgTypeList.append("responseToUserActionPrefChange");
            if (msgType6) msgTypeList.append("audioDescriptionPrefChange");
            if (msgType7) msgTypeList.append("inVisionSigningPrefChange");

            result["msgType"] = msgTypeList;
            jsonResponse["result"] = result;

            if (id.substr(0,3)=="STR"){
                jsonResponse["id"] = id.substr(3, id.length()-3);
            }
            else{
                jsonResponse["id"] =  std::stoll(id.substr(3, id.length()-3));
            }
            connection->SendMessage(jsonResponse.asString());
        }
        connections_mutex_.unlock();
    }
    void JsonRpcService::RespondNegotiateMethods(
            int connectionId,
            const std::string& id,
            const std::string& terminalToApp,
            const std::string& appToTerminal
    ){
        LOG(LOG_INFO, "JSON-RPC-EXAMPLE #9: Service called with response. Send response to client...");
        //Case N
        connections_mutex_.lock();
        WebSocketConnection *connection = GetConnection(connectionId);
        if (connection != nullptr)
        {
            Json::Value jsonResponse;
            jsonResponse["jsonrpc"] = "2.0";
            Json::Value result;

            result["method"] = "org.hbbtv.negotiateMethods";

            Json::Value terminalToAppJsonArray(Json::arrayValue);
            // Parse the input string
            size_t startPos = 0;
            size_t endPos = terminalToApp.find(',');
            while (endPos != std::string::npos) {
                std::string element = terminalToApp.substr(startPos, endPos - startPos);
                terminalToAppJsonArray.append(element);
                startPos = endPos + 2;  // Move past the comma and space
                endPos = terminalToApp.find(',', startPos);
            }
            // Append the last element
            std::string lastElement = terminalToApp.substr(startPos);
            terminalToAppJsonArray.append(lastElement);
            result["terminalToApp"] = terminalToAppJsonArray;
            Json::Value appToTerminalJsonArray(Json::arrayValue);

            // Parse the input string
            startPos = 0;
            endPos = appToTerminal.find(',');
            while (endPos != std::string::npos) {
                std::string element = appToTerminal.substr(startPos, endPos - startPos);
                appToTerminalJsonArray.append(element);
                startPos = endPos + 2;  // Move past the comma and space
                endPos = appToTerminal.find(',', startPos);
            }
            // Append the last element
            lastElement = appToTerminal.substr(startPos);
            appToTerminalJsonArray.append(lastElement);
            result["appToTerminal"] = appToTerminalJsonArray;

            jsonResponse["result"] = result;

            if (id.substr(0,3)=="STR"){
                jsonResponse["id"] = id.substr(3, id.length()-3);
            }
            else{
                jsonResponse["id"] =  std::stoll(id.substr(3, id.length()-3));
            }
            connection->SendMessage(jsonResponse.asString());
        }
        connections_mutex_.unlock();
    }

    void JsonRpcService::RespondError(
            int connectionId,
            const std::string& id,
            int code,
            const std::string& message
    ){
        LOG(LOG_INFO, "JSON-RPC-EXAMPLE #9: Service called with response. Send response to client...");
        //Case E1
        connections_mutex_.lock();
        WebSocketConnection *connection = GetConnection(connectionId);
        if (connection != nullptr)
        {
            Json::Value jsonResponse;
            jsonResponse["jsonrpc"] = "2.0";
            Json::Value error;
            error["code"] = code;
            error["message"] = message;
            jsonResponse["error"] = error;

            if (id.substr(0,3)=="STR"){
                jsonResponse["id"] = id.substr(3, id.length()-3);
            }
            else{
                jsonResponse["id"] =  std::stoll(id.substr(3, id.length()-3));
            }
            connection->SendMessage(jsonResponse.asString());
        }
        connections_mutex_.unlock();
    }

    void JsonRpcService::RespondError(
            int connectionId,
            const std::string& id,
            int code,
            const std::string& message,
            const std::string& method
    ){
        LOG(LOG_INFO, "JSON-RPC-EXAMPLE #9: Service called with response. Send response to client...");
        //Case E2
        connections_mutex_.lock();
        WebSocketConnection *connection = GetConnection(connectionId);
        if (connection != nullptr)
        {
            Json::Value jsonResponse;
            jsonResponse["jsonrpc"] = "2.0";
            Json::Value error;
            error["code"] = code;
            error["message"] = message;
            error["method"] = method;
            jsonResponse["error"] = error;

            if (id.substr(0,3)=="STR"){
                jsonResponse["id"] = id.substr(3, id.length()-3);
            }
            else{
                jsonResponse["id"] =  std::stoll(id.substr(3, id.length()-3));
            }
            connection->SendMessage(jsonResponse.asString());
        }
        connections_mutex_.unlock();
    }

    void JsonRpcService::SendIntentMediaPause(
            int connectionId,
            const std::string& id,
            const std::string& origin
    ){
        LOG(LOG_INFO, "JSON-RPC-EXAMPLE #9: Service called with response. Send response to client...");
        //Case c1
        connections_mutex_.lock();
        WebSocketConnection *connection = GetConnection(connectionId);
        if (connection != nullptr)
        {
            Json::Value jsonResponse;
            jsonResponse["jsonrpc"] = "2.0";
            jsonResponse["method"] = "org.hbbtv.app.intent.media.pause";
            Json::Value params;
            params["origin"] = origin;
            jsonResponse["params"] = params;
            if (id.substr(0,3)=="STR"){
                jsonResponse["id"] = id.substr(3, id.length()-3);
            }
            else{
                jsonResponse["id"] = std::stoll(id.substr(3, id.length()-3));
            }
            connection->SendMessage(jsonResponse.asString());
        }
        connections_mutex_.unlock();
    }

    void JsonRpcService::SendIntentMediaPlay(
            int connectionId,
            const std::string& id,
            const std::string& origin
    ){
        LOG(LOG_INFO, "JSON-RPC-EXAMPLE #9: Service called with response. Send response to client...");
        //Case c2
        connections_mutex_.lock();
        WebSocketConnection *connection = GetConnection(connectionId);
        if (connection != nullptr)
        {
            Json::Value jsonResponse;
            jsonResponse["jsonrpc"] = "2.0";
            jsonResponse["method"] = "org.hbbtv.app.intent.media.play";
            Json::Value params;
            params["origin"] = origin;
            jsonResponse["params"] = params;
            if (id.substr(0,3)=="STR"){
                jsonResponse["id"] = id.substr(3, id.length()-3);
            }
            else{
                jsonResponse["id"] = std::stoll(id.substr(3, id.length()-3));
            }
            connection->SendMessage(jsonResponse.asString());
        }
        connections_mutex_.unlock();
    }

    void JsonRpcService::SendIntentMediaFastForward(
            int connectionId,
            const std::string& id,
            const std::string& origin
    ){
        LOG(LOG_INFO, "JSON-RPC-EXAMPLE #9: Service called with response. Send response to client...");
        //Case c3
        connections_mutex_.lock();
        WebSocketConnection *connection = GetConnection(connectionId);
        if (connection != nullptr)
        {
            Json::Value jsonResponse;
            jsonResponse["jsonrpc"] = "2.0";
            jsonResponse["method"] = "org.hbbtv.app.intent.media.fast-forward";
            Json::Value params;
            params["origin"] = origin;
            jsonResponse["params"] = params;
            if (id.substr(0,3)=="STR"){
                jsonResponse["id"] = id.substr(3, id.length()-3);
            }
            else{
                jsonResponse["id"] = std::stoll(id.substr(3, id.length()-3));
            }
            connection->SendMessage(jsonResponse.asString());
        }
        connections_mutex_.unlock();
    }

    void JsonRpcService::SendIntentMediaFastReverse(
            int connectionId,
            const std::string& id,
            const std::string& origin
    ){
        LOG(LOG_INFO, "JSON-RPC-EXAMPLE #9: Service called with response. Send response to client...");
        //Case c4
        connections_mutex_.lock();
        WebSocketConnection *connection = GetConnection(connectionId);
        if (connection != nullptr)
        {
            Json::Value jsonResponse;
            jsonResponse["jsonrpc"] = "2.0";
            jsonResponse["method"] = "org.hbbtv.app.intent.media.fast-reverse";
            Json::Value params;
            params["origin"] = origin;
            jsonResponse["params"] = params;
            if (id.substr(0,3)=="STR"){
                jsonResponse["id"] = id.substr(3, id.length()-3);
            }
            else{
                jsonResponse["id"] = std::stoll(id.substr(3, id.length()-3));
            }
            connection->SendMessage(jsonResponse.asString());
        }
        connections_mutex_.unlock();
    }

    void JsonRpcService::SendIntentMediaStop(
            int connectionId,
            const std::string& id,
            const std::string& origin
    ){
        LOG(LOG_INFO, "JSON-RPC-EXAMPLE #9: Service called with response. Send response to client...");
        //Case c5
        connections_mutex_.lock();
        WebSocketConnection *connection = GetConnection(connectionId);
        if (connection != nullptr)
        {
            Json::Value jsonResponse;
            jsonResponse["jsonrpc"] = "2.0";
            jsonResponse["method"] = "org.hbbtv.app.intent.media.stop";
            Json::Value params;
            params["origin"] = origin;
            jsonResponse["params"] = params;
            if (id.substr(0,3)=="STR"){
                jsonResponse["id"] = id.substr(3, id.length()-3);
            }
            else{
                jsonResponse["id"] = std::stoll(id.substr(3, id.length()-3));
            }
            connection->SendMessage(jsonResponse.asString());
        }
        connections_mutex_.unlock();
    }

    void JsonRpcService::SendIntentMediaSeekContent(
            int connectionId,
            const std::string& id,
            const std::string& origin,
            const std::string& anchor,
            int offset
    ){
        LOG(LOG_INFO, "JSON-RPC-EXAMPLE #9: Service called with response. Send response to client...");
        //Case c6
        connections_mutex_.lock();
        WebSocketConnection *connection = GetConnection(connectionId);
        if (connection != nullptr)
        {
            Json::Value jsonResponse;
            jsonResponse["jsonrpc"] = "2.0";
            jsonResponse["method"] = "org.hbbtv.app.intent.media.seek-content";
            Json::Value params;
            params["origin"] = origin;
            params["anchor"] = anchor;
            params["offset"] = offset;
            jsonResponse["params"] = params;
            if (id.substr(0,3)=="STR"){
                jsonResponse["id"] = id.substr(3, id.length()-3);
            }
            else{
                jsonResponse["id"] = std::stoll(id.substr(3, id.length()-3));
            }
            connection->SendMessage(jsonResponse.asString());
        }
        connections_mutex_.unlock();
    }
    void JsonRpcService::SendIntentMediaSeekRelative(
            int connectionId,
            const std::string& id,
            const std::string& origin,
            int offset
    ){
        LOG(LOG_INFO, "JSON-RPC-EXAMPLE #9: Service called with response. Send response to client...");
        //Case c7
        connections_mutex_.lock();
        WebSocketConnection *connection = GetConnection(connectionId);
        if (connection != nullptr)
        {
            Json::Value jsonResponse;
            jsonResponse["jsonrpc"] = "2.0";
            jsonResponse["method"] = "org.hbbtv.app.intent.media.seek-relative";
            Json::Value params;
            params["origin"] = origin;
            params["offset"] = offset;
            jsonResponse["params"] = params;
            if (id.substr(0,3)=="STR"){
                jsonResponse["id"] = id.substr(3, id.length()-3);
            }
            else{
                jsonResponse["id"] = std::stoll(id.substr(3, id.length()-3));
            }
            connection->SendMessage(jsonResponse.asString());
        }
        connections_mutex_.unlock();
    }

    void JsonRpcService::SendIntentMediaSeekLive(
            int connectionId,
            const std::string& id,
            const std::string& origin,
            int offset
    ){
        LOG(LOG_INFO, "JSON-RPC-EXAMPLE #9: Service called with response. Send response to client...");
        //Case c8
        connections_mutex_.lock();
        WebSocketConnection *connection = GetConnection(connectionId);
        if (connection != nullptr)
        {
            Json::Value jsonResponse;
            jsonResponse["jsonrpc"] = "2.0";
            jsonResponse["method"] = "org.hbbtv.app.intent.media.seek-live";
            Json::Value params;
            params["origin"] = origin;
            params["offset"] = offset;
            jsonResponse["params"] = params;
            if (id.substr(0,3)=="STR"){
                jsonResponse["id"] = id.substr(3, id.length()-3);
            }
            else{
                jsonResponse["id"] = std::stoll(id.substr(3, id.length()-3));
            }
            connection->SendMessage(jsonResponse.asString());
        }
        connections_mutex_.unlock();
    }
    void JsonRpcService::SendIntentMediaSeekWallclock(
            int connectionId,
            const std::string& id,
            const std::string& origin,
            const std::string& dateTime
    ){
        LOG(LOG_INFO, "JSON-RPC-EXAMPLE #9: Service called with response. Send response to client...");
        //Case c9
        connections_mutex_.lock();
        WebSocketConnection *connection = GetConnection(connectionId);
        if (connection != nullptr)
        {
            Json::Value jsonResponse;
            jsonResponse["jsonrpc"] = "2.0";
            jsonResponse["method"] = "org.hbbtv.app.intent.media.seek-wallclock";
            Json::Value params;
            params["origin"] = origin;
            params["date-time"] = dateTime;
            jsonResponse["params"] = params;
            if (id.substr(0,3)=="STR"){
                jsonResponse["id"] = id.substr(3, id.length()-3);
            }
            else{
                jsonResponse["id"] = std::stoll(id.substr(3, id.length()-3));
            }
            connection->SendMessage(jsonResponse.asString());
        }
        connections_mutex_.unlock();
    }
    void JsonRpcService::SendIntentSearch(
            int connectionId,
            const std::string& id,
            const std::string& origin,
            const std::string& query
    ){
        LOG(LOG_INFO, "JSON-RPC-EXAMPLE #9: Service called with response. Send response to client...");
        //Case c10
        connections_mutex_.lock();
        WebSocketConnection *connection = GetConnection(connectionId);
        if (connection != nullptr)
        {
            Json::Value jsonResponse;
            jsonResponse["jsonrpc"] = "2.0";
            jsonResponse["method"] = "org.hbbtv.app.intent.search";
            Json::Value params;
            params["origin"] = origin;
            params["query"] = query;
            jsonResponse["params"] = params;
            if (id.substr(0,3)=="STR"){
                jsonResponse["id"] = id.substr(3, id.length()-3);
            }
            else{
                jsonResponse["id"] = std::stoll(id.substr(3, id.length()-3));
            }
            connection->SendMessage(jsonResponse.asString());
        }
        connections_mutex_.unlock();
    }
    void JsonRpcService::SendIntentDisplay(
            int connectionId,
            const std::string& id,
            const std::string& origin,
            const std::string& mediaId
    ){
        LOG(LOG_INFO, "JSON-RPC-EXAMPLE #9: Service called with response. Send response to client...");
        //Case c11
        connections_mutex_.lock();
        WebSocketConnection *connection = GetConnection(connectionId);
        if (connection != nullptr)
        {
            Json::Value jsonResponse;
            jsonResponse["jsonrpc"] = "2.0";
            jsonResponse["method"] = "org.hbbtv.app.intent.display";
            Json::Value params;
            params["origin"] = origin;
            params["mediaId"] = mediaId;
            jsonResponse["params"] = params;
            if (id.substr(0,3)=="STR"){
                jsonResponse["id"] = id.substr(3, id.length()-3);
            }
            else{
                jsonResponse["id"] = std::stoll(id.substr(3, id.length()-3));
            }
            connection->SendMessage(jsonResponse.asString());
        }
        connections_mutex_.unlock();
    }

    void JsonRpcService::SendIntentPlayback(
            int connectionId,
            const std::string& id,
            const std::string& origin,
            const std::string& mediaId,
            const std::string& anchor,
            int offset
    ){
        LOG(LOG_INFO, "JSON-RPC-EXAMPLE #9: Service called with response. Send response to client...");
        //Case c12
        connections_mutex_.lock();
        WebSocketConnection *connection = GetConnection(connectionId);
        if (connection != nullptr)
        {
            Json::Value jsonResponse;
            jsonResponse["jsonrpc"] = "2.0";
            jsonResponse["method"] = "org.hbbtv.app.intent.playback";
            Json::Value params;
            params["origin"] = origin;
            params["mediaId"] = mediaId;
            if (anchor != NullStrValue) params["anchor"] = anchor;
            if (offset != NullIntValue) params["offset"] = offset;

            jsonResponse["params"] = params;
            if (id.substr(0,3)=="STR"){
                jsonResponse["id"] = id.substr(3, id.length()-3);
            }
            else{
                jsonResponse["id"] = std::stoll(id.substr(3, id.length()-3));
            }
            connection->SendMessage(jsonResponse.asString());
        }
        connections_mutex_.unlock();
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
        //case D 1
        connections_mutex_.lock();
        WebSocketConnection *connection = GetConnection(connectionId);
        if (connection != nullptr)
        {
            Json::Value jsonResponse;
            jsonResponse["jsonrpc"] = "2.0";
            jsonResponse["method"] = "org.hbbtv.notify";

            Json::Value params;

            params["msgType"] = "subtitlesPrefChange";

            Json::Value value;
            value["enabled"] = enabled;
            if (size != NullIntValue) value["size"] = size;
            if (fontFamily != NullStrValue) value["fontFamily"] = fontFamily;
            if (textColour != NullStrValue) value["textColour"] = textColour;
            if (textOpacity != NullIntValue) value["textOpacity"] = textOpacity;
            if (textColour != NullStrValue) value["edgeType"] = edgeType;
            if (edgeColour != NullStrValue) value["edgeColour"] = edgeColour;
            if (backgroundColour != NullStrValue) value["backgroundColour"] = backgroundColour;
            if (backgroundOpacity != NullIntValue) value["backgroundOpacity"] = backgroundOpacity;
            if (windowColour != NullStrValue) value["windowColour"] = windowColour;
            if (windowOpacity != NullIntValue) value["windowOpacity"] = windowOpacity;
            if (language != NullStrValue) value["language"] = language;


            params["value"] = value;
            jsonResponse["params"] = params;

            connection->SendMessage(jsonResponse.asString());
        }
        connections_mutex_.unlock();
    }

    void JsonRpcService::NotifyDialogueEnhancement(
            int connectionId,
            int dialogueEnhancementGainPreference,
            int dialogueEnhancementGain,
            int dialogueEnhancementLimitMin,
            int dialogueEnhancementLimitMax
    ){
        //case D 2
        connections_mutex_.lock();
        WebSocketConnection *connection = GetConnection(connectionId);
        if (connection != nullptr)
        {
            Json::Value jsonResponse;
            jsonResponse["jsonrpc"] = "2.0";
            jsonResponse["method"] = "org.hbbtv.notify";

            Json::Value params;

            params["msgType"] = "dialogueEnhancementPrefChange";

            Json::Value value;
            value["dialogueEnhancementGainPreference"] = dialogueEnhancementGainPreference;
            value["dialogueEnhancementGain"] = dialogueEnhancementGain;
            Json::Value dialogueEnhancementLimit;
            dialogueEnhancementLimit["min"] = dialogueEnhancementLimitMin;
            dialogueEnhancementLimit["max"] = dialogueEnhancementLimitMax;

            value["dialogueEnhancementLimit"] = dialogueEnhancementLimit;
            params["value"] = value;
            jsonResponse["params"] = params;

            connection->SendMessage(jsonResponse.asString());
        }
        connections_mutex_.unlock();
    }

    void JsonRpcService::NotifyUIMagnifier(
            int connectionId,
            bool enabled,
            const std::string& magType
    ){
        //case D 3
        LOG(LOG_INFO, "JSON-RPC-EXAMPLE #9: Service called with response. Send response to client...");
        connections_mutex_.lock();
        WebSocketConnection *connection = GetConnection(connectionId);
        if (connection != nullptr)
        {
            Json::Value jsonResponse;
            jsonResponse["jsonrpc"] = "2.0";
            jsonResponse["method"] = "org.hbbtv.notify";

            Json::Value params;

            params["msgType"] = "uiMagnifierPrefChange";

            Json::Value value;
            value["enabled"] = enabled;
            if (magType != NullStrValue) value["magType"] = magType;

            params["value"] = value;
            jsonResponse["params"] = params;

            connection->SendMessage(jsonResponse.asString());
        }
        connections_mutex_.unlock();
    }

    void JsonRpcService::NotifyHighContrastUI(
            int connectionId,
            bool enabled,
            const std::string& hcType
    ){
        //case D 4
        LOG(LOG_INFO, "JSON-RPC-EXAMPLE #9: Service called with response. Send response to client...");
        connections_mutex_.lock();
        WebSocketConnection *connection = GetConnection(connectionId);
        if (connection != nullptr)
        {
            Json::Value jsonResponse;
            jsonResponse["jsonrpc"] = "2.0";
            jsonResponse["method"] = "org.hbbtv.notify";

            Json::Value params;
            params["msgType"] = "highContrastUIPrefChange";

            Json::Value value;
            value["enabled"] = enabled;
            if (hcType != NullStrValue) value["hcType"] = hcType;

            params["value"] = value;
            jsonResponse["params"] = params;
            connection->SendMessage(jsonResponse.asString());
        }
        connections_mutex_.unlock();
    }

    void JsonRpcService::NotifyScreenReader(
            int connectionId,
            bool enabled,
            int speed,
            const std::string& voice,
            const std::string& language
    ){
        //case D 5
        LOG(LOG_INFO, "JSON-RPC-EXAMPLE #9: Service called with response. Send response to client...");
        connections_mutex_.lock();
        WebSocketConnection *connection = GetConnection(connectionId);
        if (connection != nullptr)
        {
            Json::Value jsonResponse;
            jsonResponse["jsonrpc"] = "2.0";
            jsonResponse["method"] = "org.hbbtv.notify";

            Json::Value params;
            params["msgType"] = "screenReaderPrefChange";

            Json::Value value;
            value["enabled"] = enabled;
            if (speed != NullIntValue) value["speed"] = speed;
            if (voice != NullStrValue) value["voice"] = voice;
            if (language != NullStrValue) value["language"] = language;

            params["value"] = value;
            jsonResponse["params"] = params;

            connection->SendMessage(jsonResponse.asString());
        }
        connections_mutex_.unlock();
    }
    void JsonRpcService::NotifyResponseToUserAction(
            int connectionId,
            bool enabled,
            const std::string& type
    ){
        //case D 6
        LOG(LOG_INFO, "JSON-RPC-EXAMPLE #9: Service called with response. Send response to client...");
        connections_mutex_.lock();
        WebSocketConnection *connection = GetConnection(connectionId);
        if (connection != nullptr)
        {
            Json::Value jsonResponse;
            jsonResponse["jsonrpc"] = "2.0";
            jsonResponse["method"] = "org.hbbtv.notify";

            Json::Value params;
            params["msgType"] = "responseToUserActionPrefChange";

            Json::Value value;
            value["enabled"] = enabled;
            if (type != NullStrValue) value["hcType"] = type;

            params["value"] = value;
            jsonResponse["params"] = params;


            connection->SendMessage(jsonResponse.asString());
        }
        connections_mutex_.unlock();
    }

    void JsonRpcService::NotifyAudioDescription(
            int connectionId,
            bool enabled,
            int gainPreference,
            int panAzimuthPreference
    ){
        //case D 7
        LOG(LOG_INFO, "JSON-RPC-EXAMPLE #9: Service called with response. Send response to client...");
        connections_mutex_.lock();
        WebSocketConnection *connection = GetConnection(connectionId);
        if (connection != nullptr)
        {
            Json::Value jsonResponse;
            jsonResponse["jsonrpc"] = "2.0";
            jsonResponse["method"] = "org.hbbtv.notify";

            Json::Value params;
            params["msgType"] = "audioDescriptionPrefChange";

            Json::Value value;
            value["enabled"] = enabled;
            if (gainPreference != NullIntValue) value["gainPreference"] = gainPreference;
            if (panAzimuthPreference != NullIntValue) value["panAzimuthPreference"] = panAzimuthPreference;

            params["value"] = value;
            jsonResponse["params"] = params;

            connection->SendMessage(jsonResponse.asString());
        }
        connections_mutex_.unlock();
    }

    void JsonRpcService::NotifyInVisionSigning(
            int connectionId,
            bool enabled
    ){
        //case D 8
        connections_mutex_.lock();
        WebSocketConnection *connection = GetConnection(connectionId);
        if (connection != nullptr)
        {
            Json::Value jsonResponse;
            jsonResponse["jsonrpc"] = "2.0";
            jsonResponse["method"] = "org.hbbtv.notify";

            Json::Value params;
            params["msgType"] = "inVisionSigningPrefChange";

            Json::Value value;
            value["enabled"] = enabled;

            params["value"] = value;
            jsonResponse["params"] = params;

            connection->SendMessage(jsonResponse.asString());
        }
        connections_mutex_.unlock();
    }


    void JsonRpcService::RespondDialogueEnhancementOverride(
            int connectionId,
            const std::string& id,
            int dialogueEnhancementGain
    ){
        //case Fr 1
        connections_mutex_.lock();
        WebSocketConnection *connection = GetConnection(connectionId);
        if (connection != nullptr)
        {
            Json::Value jsonResponse;
            jsonResponse["jsonrpc"] = "2.0";

            Json::Value result;
            result["method"] = "org.hbbtv.af.dialogueEnhancementOverride";
            result["dialogueEnhancementGain"] = dialogueEnhancementGain;

            jsonResponse["result"] = result;

            if (id.substr(0,3)=="STR"){
                jsonResponse["id"] = id.substr(3, id.length()-3);
            }
            else{
                jsonResponse["id"] =  std::stoll(id.substr(3, id.length()-3));
            }
            connection->SendMessage(jsonResponse.asString());
        }
        connections_mutex_.unlock();
    }
    void JsonRpcService::RespondTriggerResponseToUserAction(
            int connectionId,
            const std::string& id,
            bool actioned
    ){
        //case Fr 2
        connections_mutex_.lock();
        WebSocketConnection *connection = GetConnection(connectionId);
        if (connection != nullptr)
        {
            Json::Value jsonResponse;
            jsonResponse["jsonrpc"] = "2.0";

            Json::Value result;
            result["method"] = "org.hbbtv.af.triggerResponseToUserAction";
            result["actioned"] = actioned;

            jsonResponse["result"] = result;

            if (id.substr(0,3)=="STR"){
                jsonResponse["id"] = id.substr(3, id.length()-3);
            }
            else{
                jsonResponse["id"] =  std::stoll(id.substr(3, id.length()-3));
            }
            connection->SendMessage(jsonResponse.asString());
        }
        connections_mutex_.unlock();
    }
} // namespace NetworkServices
