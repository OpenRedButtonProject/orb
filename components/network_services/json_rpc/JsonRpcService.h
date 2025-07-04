/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

#ifndef OBS_NS_JSON_RPC_SERVICE_H
#define OBS_NS_JSON_RPC_SERVICE_H

#include "websocket_service.h"

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <mutex>
#include <memory> 
#include <json/json.h>
#include <functional>

namespace orb {
namespace networkServices {
class JsonRpcService : public WebSocketService {
public:
    enum class ConnectionDataType
    {
        ActionPause,
        ActionPlay,
        ActionFastForward,
        ActionFastReverse,
        ActionStop,
        ActionSeekContent,
        ActionSeekRelative,
        ActionSeekLive,
        ActionSeekWallclock,
        MediaId,
        Title,
        SecondTitle,
        Synopsis,
        BiasToSystemTime,
        StartTime,
        EndTime,
        State,
        NegotiateMethodsAppToTerminal,
        NegotiateMethodsTerminalToApp,
        SubscribedMethods,
        UnsubscribedMethods,
        IntentIdCount,
        VoiceReady
    };

    struct ConnectionData
    {
        bool actionPause = false;
        bool actionPlay = false;
        bool actionFastForward = false;
        bool actionFastReverse = false;
        bool actionStop = false;
        bool actionSeekContent = false;
        bool actionSeekRelative = false;
        bool actionSeekLive = false;
        bool actionSeekWallclock = false;
        std::string mediaId = "";
        std::string title = "";
        std::string secondTitle = "";
        std::string synopsis = "";
        int biasToSystemTime = 0;
        int startTime = -1;
        int endTime = -1;
        std::string state = "";
        std::unordered_set<std::string> negotiateMethodsAppToTerminal;
        std::unordered_set<std::string> negotiateMethodsTerminalToApp;
        std::unordered_set<std::string> subscribedMethods;
        int intentIdCount;
        bool voiceReady = false;
        bool opAppEnabled = false;
    };

    enum class JsonRpcStatus
    {
        SUCCESS = 0,
        PARSE_ERROR = -32700,
        INVALID_REQUEST = -32600,
        METHOD_NOT_FOUND = -32601,
        INVALID_PARAMS = -32602,
        NOTIFICATION_ERROR = -99999,
        UNKNOWN = 1,
    };

    typedef JsonRpcStatus (JsonRpcService::*JsonRpcMethod)(int, const Json::Value&);

    struct SubscribeOptions
    {
        bool subtitles = false;
        bool dialogueEnhancement = false;
        bool uiMagnifier = false;
        bool highContrastUI = false;
        bool screenReader = false;
        bool responseToUserAction = false;
        bool audioDescription = false;
        bool inVisionSigning = false;
    };

    class ISessionCallback {
public:
        virtual void RequestNegotiateMethods() = 0;

        virtual void RequestSubscribe(const SubscribeOptions& options) = 0;

        virtual void RequestUnsubscribe(const SubscribeOptions& options) = 0;

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
            bool isReady) = 0;

        virtual void NotifyStateMedia(
            std::string state) = 0;

        virtual void RespondMessage(
            std::string info) = 0;

        virtual void ReceiveConfirm(
            int connectionId,
            std::string id,
            std::string method) = 0;

        virtual void ReceiveConfirmForSelectChannel(
            int connectionId,
            std::string id,
            std::string method,
            int sessionId) = 0;    

        virtual void ReceiveError(
            int code,
            std::string message) = 0;

        virtual void ReceiveError(
            int code,
            std::string message,
            std::string method,
            std::string data) = 0;

        virtual void RequestIPPlaybackStatusUpdate(
            const Json::Value &params) = 0;

        virtual void RequestIPPlaybackMediaPositionUpdate(
            const Json::Value &params) = 0;
        
        virtual void RequestIPPlaybackSetComponents(
            const Json::Value &params) = 0;

        virtual void RequestIPPlaybackSetPresentFollowing(
            const Json::Value &params) = 0;

        virtual void RequestIPPlaybackSetTimelineMapping(
            const Json::Value &params) = 0;   

        virtual ~ISessionCallback() = default;
    };

    JsonRpcService(int port, const std::string &endpoint,
        std::unique_ptr<ISessionCallback> sessionCallback);

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

    JsonRpcStatus ReceiveConfirm(int connectionId, const Json::Value &obj);

    JsonRpcStatus ReceiveConfirmForSelectChannel(int connectionId, const Json::Value &obj);

    JsonRpcService::JsonRpcStatus ReceiveError(int connectionId, const Json::Value &obj);

    void RespondFeatureSupportInfo(int connectionId, const std::string &id, int featureId,
        const std::string &value);

    void RespondFeatureSettingsSubtitles(int connectionId, const std::string &id, bool enabled,
        int size, const std::string &fontFamily, const std::string &textColour, int textOpacity,
        const std::string &edgeType, const std::string &edgeColour,
        const std::string &backgroundColour, int backgroundOpacity,
        const std::string &windowColour, int windowOpacity, const std::string &language);

    void RespondFeatureSettingsDialogueEnhancement(int connectionId, const std::string &id,
        int dialogueEnhancementGainPreference, int dialogueEnhancementGain,
        int dialogueEnhancementLimitMin, int dialogueEnhancementLimitMax);

    void RespondFeatureSettingsUIMagnifier(int connectionId, const std::string &id,
        bool enabled, const std::string &magType);

    void RespondFeatureSettingsHighContrastUI(int connectionId, const std::string &id,
        bool enabled, const std::string &hcType);

    void RespondFeatureSettingsScreenReader(int connectionId, const std::string &id,
        bool enabled, int speed, const std::string &voice, const std::string &language);

    void RespondFeatureSettingsResponseToUserAction(int connectionId, const std::string &id,
        bool enabled, const std::string &type);

    void RespondFeatureSettingsAudioDescription(int connectionId, const std::string &id,
        bool enabled, int gainPreference, int panAzimuthPreference);

    void RespondFeatureSettingsInVisionSigning(int connectionId, const std::string &id,
        bool enabled);

    void RespondFeatureSuppress(int connectionId, const std::string &id, int featureId,
        const std::string &value);

    void RespondSubscribe(int connectionId, const std::string &id,
        const Json::Value &msgTypeList);
    void RespondUnsubscribe(int connectionId, const std::string &id,
        const Json::Value &msgTypeList);

    void RespondNegotiateMethods(int connectionId, const std::string &id,
        const Json::Value& terminalToApp, const Json::Value& appToTerminal);

    void RespondError(int connectionId, const std::string &id, int code,
        const std::string &message);

    void RespondError(int connectionId, const std::string &id, int code,
        const std::string &message, const std::string &data);

    void VoiceRequestDescription();

    void SendIntentMediaPause();

    void SendIntentMediaPlay();

    void SendIntentMediaFastForward();

    void SendIntentMediaFastReverse();

    void SendIntentMediaStop();

    void SendIntentMediaSeekContent(const std::string &anchor, int offset);

    void SendIntentMediaSeekRelative(int offset);

    void SendIntentMediaSeekLive(int offset);

    void SendIntentMediaSeekWallclock(const std::string &dateTime);

    void SendIntentSearch(const std::string &query);

    void SendIntentDisplay(const std::string &mediaId);

    void SendIntentPlayback(const std::string &mediaId, const std::string &anchor, int offset);

    void NotifySubtitles(bool enabled, int size, const std::string &fontFamily,
        const std::string &textColour, int textOpacity, const std::string &edgeType,
        const std::string &edgeColour, const std::string &backgroundColour, int backgroundOpacity,
        const std::string &windowColour, int windowOpacity, const std::string &language);

    void NotifyDialogueEnhancement(int dialogueEnhancementGainPreference,
        int dialogueEnhancementGain, int dialogueEnhancementLimitMin,
        int dialogueEnhancementLimitMax);

    void NotifyUIMagnifier(bool enabled, const std::string &magType);

    void NotifyHighContrastUI(bool enabled, const std::string &hcType);

    void NotifyScreenReader(bool enabled, int speed, const std::string &voice,
        const std::string &language);

    void NotifyResponseToUserAction(bool enabled, const std::string &type);

    void NotifyAudioDescription(bool enabled, int gainPreference,
        int panAzimuthPreference);

    void NotifyInVisionSigning(bool enabled);

    void RespondDialogueEnhancementOverride(int connectionId, const std::string &id,
        int dialogueEnhancementGain);

    void RespondTriggerResponseToUserAction(int connectionId, const std::string &id,
        bool actioned);

    // OpApp Video Window to Terminal Request methods
    JsonRpcStatus RequestIPPlaybackStatusUpdate(int connectionId, const Json::Value &obj);
    JsonRpcStatus RequestIPPlaybackMediaPositionUpdate(int connectionId, const Json::Value &obj);
    JsonRpcStatus RequestIPPlaybackSetComponents(int connectionId, const Json::Value &obj);
    JsonRpcStatus RequestIPPlaybackSetTimelineMapping(int connectionId, const Json::Value &obj);
    JsonRpcStatus RequestIPPlaybackSetPresentFollowing(int connectionId, const Json::Value &obj);
    
    // Terminal to OpApp Video Window Request methods
    void SendIPPlayerSelectChannel(int channelType, int idType, const std::string& ipBroadcastId);
    void SendIPPlayerPlay(int sessionId);
    void SendIPPlayerPause(int sessionId);
    void SendIPPlayerStop(int sessionId);
    void SendIPPlayerResume(int sessionId);
    void SendIPPlayerSeek(int sessionId, int offset, int reference);
    void SendIPPlayerSetVideoWindow(int sessionId, int x, int y, int width, int height);
    void SendIPPlayerSetRelativeVolume(int sessionId, int volume);
    void SendIPPlayerSelectComponents(int sessionId, const std::vector<int>& videoComponents,
        const std::vector<int>& audioComponents, const std::vector<int>& subtitleComponents);
    void SendIPPlayerResolveTimeline(int sessionId, const std::string& timelineSelector);

private:
    // Setters and getters for variables
    void InitialConnectionData(int connectionId);
    void SetConnectionData(int connectionId, ConnectionDataType type, const Json::Value& value);
    void SetStateMediaToConnectionData(int connectionId, const ConnectionData& mediaData);
    Json::Value GetConnectionData(int connectionId, ConnectionDataType type);

    std::string m_endpoint;
    std::unique_ptr<ISessionCallback> m_sessionCallback;

    // Map to hold JSON-RPC methods
    std::map<std::string, std::function<JsonRpcStatus(int connectionId, const
        Json::Value&)> > m_json_rpc_methods;

    // Sets to hold supported methods for both directions between the regular app and the terminal   
    std::unordered_set<std::string> m_supported_methods_app_to_terminal;
    std::unordered_set<std::string> m_supported_methods_terminal_to_app;

    // Sets to hold supported methods for both directions between the terminal and the operator app
    std::unordered_set<std::string> m_supported_methods_opapp_to_terminal;
    std::unordered_set<std::string> m_supported_methods_terminal_to_opapp;

    // Map to hold connection data for each connection   
    std::unordered_map<int, ConnectionData> m_connectionData;

    // Helper functions
    std::vector<int> GetAllConnectionIds();

    void RegisterMethod(const std::string& name, JsonRpcMethod method);

    void SendJsonMessageToClient(int connectionId, const Json::Value &jsonResponse);

    void GetNotifyConnectionIds(std::vector<int> &availableConnectionIds, const int msgType);

    void RegisterSupportedMethods();

    void SendIntentMessage(const std::string &method, Json::Value &params);

    void AdjustTimeRange(int id, const std::string& method, Json::Value &params);

    void SendNotifyMessage(int msgTypeIndex, const Json::Value &response);

    Json::Value FilterMethods(int connectionId, const Json::Value& methodsList, bool
        isAppToTerminal);

    void CheckIntentMethod(std::vector<int> &connectionIds, const std::string& method);

    std::string GenerateId(int connectionId);

    bool IsOpApp (int connectionId);

    void handleError(int connectionId, JsonRpcStatus status, const Json::Value& obj);

    JsonRpcStatus HandleFeatureRequest(int connectionId, const Json::Value &obj, 
        const std::string &feature);

    void GetIPPlayerConnectionIdsForMethod(std::vector<int> &result, const std::string& method);   
    
    void SendIPPlayerMessageToClients(const std::string& method, const Json::Value &params);

    void SendIPPlayerMessageToClients(const std::string& method, const std::string &sessionId);

    JsonRpcStatus HandleIPPlaybackRequest(int connectionId, const Json::Value &obj, const std::string &method);
    
    JsonRpcStatus ResponseIPPlaybackRequest(int connectionId, const std::string &id, const std::string &method);

    void RegisterJsonRPCMethods();

    bool GetActionValue(const Json::Value &actions, const std::string &actionName);

    void SetSubscribeOptions(SubscribeOptions &options, int featureId);

};
} // namespace networkServices
} // namespace orb

#endif //OBS_NS_JSON_RPC_SERVICE_H 
