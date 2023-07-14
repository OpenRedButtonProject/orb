/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

#ifndef OBS_NS_JSONRPCSERVICE_
#define OBS_NS_JSONRPCSERVICE_

#include "websocket_service.h"

#include <string>
#include <unordered_map>
#include <mutex>
#include <json/json.h>

namespace NetworkServices {
class JsonRpcService : public WebSocketService {
public:
    enum class JsonRpcStatus
    {
        SUCCESS = 0,
        PARSE_ERROR = -32700,
        INVALID_REQUEST = -32600,
        METHOD_NOT_FOUND = -32601,
        INVALID_PARAMS = -32602,
    };
    typedef JsonRpcStatus (JsonRpcService::*JsonRpcMethod)(int, const Json::Value&);

    class SessionCallback {
public:
        virtual void RequestNegotiateMethods(
            int connectionId,
            std::string id,
            std::string terminalToApp,
            std::string appToTerminal) = 0;

        virtual void RequestSubscribe(
            int connectionId,
            std::string id,
            bool subtitles, bool dialogueEnhancement,
            bool uiMagnifier, bool highContrastUI,
            bool screenReader, bool responseToUserAction,
            bool audioDescription, bool inVisionSigning) = 0;

        virtual void RequestUnsubscribe(
            int connectionId,
            std::string id,
            bool subtitles, bool dialogueEnhancement,
            bool uiMagnifier, bool highContrastUI,
            bool screenReader, bool responseToUserAction,
            bool audioDescription, bool inVisionSigning) = 0;

        virtual void RequestDialogueEnhancementOverride(
            int connectionId,
            std::string id,
            int dialogueEnhancementGain) = 0;

        virtual void RequestTriggerResponseToUserAction(
            int connectionId,
            std::string id,
            std::string magnitude) = 0;

        virtual void RequestFeatureSupportInfo(
            int connectionId,
            std::string id,
            int feature) = 0;

        virtual void RequestFeatureSettingsQuery(
            int connectionId,
            std::string id,
            int feature) = 0;

        virtual void RequestFeatureSuppress(
            int connectionId,
            std::string id,
            int feature) = 0;

        virtual void NotifyVoiceReady(
            int connectionId,
            bool isReady) = 0;

        virtual void NotifyStateMedia(
            int connectionId,
            std::string state,
            bool actPause, bool actPlay, bool actFastForward, bool actFastReverse, bool actStop,
            bool actSeekContent, bool actSeekRelative, bool actSeekLive, bool actWallclock) = 0;

        virtual void NotifyStateMedia(
            int connectionId,
            std::string state, std::string kind, std::string type, std::string currentTime,
            std::string rangeStart, std::string rangeEnd,
            bool actPause, bool actPlay, bool actFastForward, bool actFastReverse, bool actStop,
            bool actSeekContent, bool actSeekRelative, bool actSeekLive, bool actWallclock,
            std::string mediaId, std::string title, std::string secTitle, std::string synopsis,
            bool subtitlesEnabled, bool subtitlesAvailable,
            bool audioDescripEnabled, bool audioDescripAvailable,
            bool signLangEnabled, bool signLangAvailable) = 0;

        virtual void ReceiveIntentConfirm(
            int connectionId,
            std::string id,
            std::string method) = 0;

        virtual void ReceiveError(
            int connectionId,
            std::string id,
            int code,
            std::string message) = 0;

        virtual void ReceiveError(
            int connectionId,
            std::string id,
            int code,
            std::string message,
            std::string method,
            std::string data) = 0;

        virtual ~SessionCallback() = default;
    };

    JsonRpcService(int port, const std::string &endpoint,
        std::unique_ptr<SessionCallback> m_sessionCallback);

    bool OnConnection(WebSocketConnection *connection) override;

    void OnMessageReceived(WebSocketConnection *connection, const std::string &text) override;

    void OnDisconnected(WebSocketConnection *connection) override;

    void OnServiceStopped() override;

    JsonRpcStatus RequestNegotiateMethods(int connectionId, const Json::Value &obj);

    JsonRpcStatus RequestSubscribe(int connectionId, const Json::Value &obj);

    JsonRpcStatus RequestUnsubscribe(int connectionId, const Json::Value &obj);

    JsonRpcStatus RequestFeatureSupportInfo(int connectionId, const Json::Value &obj);

    JsonRpcStatus RequestFeatureSettingsQuery(int connectionId, const Json::Value &obj);

    JsonRpcStatus RequestFeatureSuppress(int connectionId, const Json::Value &obj);

    JsonRpcStatus RequestDialogueEnhancementOverride(int connectionId, const Json::Value &obj);

    JsonRpcStatus RequestTriggerResponseToUserAction(int connectionId, const Json::Value &obj);

    JsonRpcStatus NotifyVoiceReady(int connectionId, const Json::Value &obj);

    JsonRpcStatus NotifyStateMedia(int connectionId, const Json::Value &obj);

    JsonRpcStatus ReceiveIntentConfirm(int connectionId, const Json::Value &obj);

    void RespondFeatureSupportInfo(int connectionId, const std::string id, int featureId,
        const std::string value);

    void RespondFeatureSettingsSubtitles(int connectionId, const std::string id, bool enabled,
        int size, const std::string fontFamily, const std::string textColour, int textOpacity,
        const std::string edgeType, const std::string edgeColour,
        const std::string backgroundColour, int backgroundOpacity,
        const std::string windowColour, int windowOpacity, const std::string language);

    void RespondFeatureSettingsDialogueEnhancement(int connectionId, const std::string id,
        int dialogueEnhancementGainPreference, int dialogueEnhancementGain,
        int dialogueEnhancementLimitMin, int dialogueEnhancementLimitMax);

    void RespondFeatureSettingsUIMagnifier(int connectionId, const std::string id,
        bool enabled, const std::string magType);

    void RespondFeatureSettingsHighContrastUI(int connectionId, const std::string id,
        bool enabled, const std::string hcType);

    void RespondFeatureSettingsScreenReader(int connectionId, const std::string id,
        bool enabled, int speed, const std::string voice, const std::string language);

    void RespondFeatureSettingsResponseToUserAction(int connectionId, const std::string id,
        bool enabled, const std::string type);

    void RespondFeatureSettingsAudioDescription(int connectionId, const std::string id,
        bool enabled, int gainPreference, int panAzimuthPreference);

    void RespondFeatureSettingsInVisionSigning(int connectionId, const std::string id,
        bool enabled);

    void RespondFeatureSuppress(int connectionId, const std::string id, int featureId,
        const std::string value);

    void RespondSubscribe(int connectionId, const std::string id,
        bool subtitles, bool dialogueEnhancement, bool uiMagnifier, bool highContrastUI,
        bool screenReader, bool responseToUserAction, bool audioDescription, bool inVisionSigning);

    void RespondUnsubscribe(int connectionId, const std::string id,
        bool subtitles, bool dialogueEnhancement, bool uiMagnifier, bool highContrastUI,
        bool screenReader, bool responseToUserAction, bool audioDescription, bool inVisionSigning);

    void RespondNegotiateMethods(int connectionId, const std::string id,
        const std::string terminalToApp, const std::string appToTerminal);

    void RespondError(int connectionId, const std::string id, int code,
        const std::string message);

    void RespondError(int connectionId, const std::string id, int code,
        const std::string message, const std::string data);

    void SendIntentMediaPause(int connectionId, const std::string id, const std::string origin);

    void SendIntentMediaPlay(int connectionId, const std::string id, const std::string origin);

    void SendIntentMediaFastForward(int connectionId, const std::string id,
        const std::string origin);

    void SendIntentMediaFastReverse(int connectionId, const std::string id,
        const std::string origin);

    void SendIntentMediaStop(int connectionId, const std::string id, const std::string origin);

    void SendIntentMediaSeekContent(int connectionId, const std::string id,
        const std::string origin, const std::string anchor, int offset);

    void SendIntentMediaSeekRelative(int connectionId, const std::string id,
        const std::string origin, int offset);

    void SendIntentMediaSeekLive(int connectionId, const std::string id,
        const std::string origin, int offset);

    void SendIntentMediaSeekWallclock(int connectionId, const std::string id,
        const std::string origin, const std::string dateTime);

    void SendIntentSearch(int connectionId, const std::string id, const std::string origin,
        const std::string query);

    void SendIntentDisplay(int connectionId, const std::string id, const std::string origin,
        const std::string mediaId);

    void SendIntentPlayback(int connectionId, const std::string id, const std::string origin,
        const std::string mediaId, const std::string anchor, int offset);

    void NotifySubtitles(int connectionId, bool enabled, int size, const std::string fontFamily,
        const std::string textColour, int textOpacity, const std::string edgeType,
        const std::string edgeColour, const std::string backgroundColour, int backgroundOpacity,
        const std::string windowColour, int windowOpacity, const std::string language);

    void NotifyDialogueEnhancement(int connectionId, int dialogueEnhancementGainPreference,
        int dialogueEnhancementGain, int dialogueEnhancementLimitMin,
        int dialogueEnhancementLimitMax);

    void NotifyUIMagnifier(int connectionId, bool enabled, const std::string magType);

    void NotifyHighContrastUI(int connectionId, bool enabled, const std::string hcType);

    void NotifyScreenReader(int connectionId, bool enabled, int speed, const std::string voice,
        const std::string language);

    void NotifyResponseToUserAction(int connectionId, bool enabled, const std::string type);

    void NotifyAudioDescription(int connectionId, bool enabled, int gainPreference,
        int panAzimuthPreference);

    void NotifyInVisionSigning(int connectionId, bool enabled);

    void RespondDialogueEnhancementOverride(int connectionId, const std::string id,
        int dialogueEnhancementGain);

    void RespondTriggerResponseToUserAction(int connectionId, const std::string id,
        bool actioned);


private:
    std::string m_endpoint;
    std::unique_ptr<SessionCallback> m_sessionCallback;
    std::map<std::string, std::function<JsonRpcStatus(int connectionId, const
        Json::Value&)> > m_json_rpc_methods;
    std::map<std::string, int> m_mapOfFeatures;

    static const std::string accessibilityFeatures[8];


    // Helper functions
    std::string AddDataTypeIdentify(Json::Value value);

    void registerMethod(const std::string& name, JsonRpcMethod method);


    JsonRpcService::JsonRpcStatus ReceiveError(int connectionId, const Json::Value &obj);

    bool HasParam(const Json::Value &object, const std::string &name, Json::ValueType type);

    bool HasJsonParam(const Json::Value &json, const std::string &param);


    Json::Value CreateFeatureSettingsQuery(const std::string feature, Json::Value value);

    Json::Value CreateNotifyRequest(Json::Value params);

    Json::Value CreateJsonResponse(const std::string id, const std::string method,
        Json::Value params);

    Json::Value CreateJsonResponse(const std::string id, Json::Value result);

    Json::Value CreateJsonErrorResponse(const std::string id, Json::Value error);

    void SendJsonMessageToClient(int connectionId, const std::string responseName,
        Json::Value jsonResponse);

    void CreateJsonRpcErrorObject(int connectionId, const Json::Value &obj, JsonRpcStatus status);

    Json::Value CreateNegotiatedMethods(const std::string stringList);
};
} // namespace NetworkServices

#endif // OBS_NS_JSONRPCSERVICE_