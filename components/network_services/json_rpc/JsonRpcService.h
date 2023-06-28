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
    class SessionCallback {
public:
        virtual void RequestNegotiateMethods(
            int connectionId,
            std::string,
            std::string terminalToApp,
            std::string appToTerminal) = 0;

        virtual void RequestSubscribe(
            int connectionId,
            std::string,
            bool subtitles, bool dialogueEnhancement,
            bool uiMagnifier, bool highContrastUI,
            bool screenReader, bool responseToUserAction,
            bool audioDescription, bool inVisionSigning) = 0;

        virtual void RequestUnsubscribe(
            int connectionId,
            std::string,
            bool subtitles, bool dialogueEnhancement,
            bool uiMagnifier, bool highContrastUI,
            bool screenReader, bool responseToUserAction,
            bool audioDescription, bool inVisionSigning) = 0;

        virtual void RequestDialogueEnhancementOverride(
            int connectionId,
            std::string,
            int dialogueEnhancementGain) = 0;

        virtual void RequestTriggerResponseToUserAction(
            int connectionId,
            std::string id,
            bool actioned) = 0;

        virtual void RequestFeatureSupportInfo(
            int connectionId,
            std::string,
            int feature) = 0;

        virtual void RequestFeatureSettingsQuery(
            int connectionId,
            std::string,
            int feature) = 0;

        virtual void RequestFeatureSuppress(
            int connectionId,
            std::string,
            int feature) = 0;

        virtual void NotifyVoiceReady(
            int connectionId,
            bool isReady) = 0;

        virtual void ReceiveError(
            int connectionId,
            std::string,
            int code,
            std::string message) = 0;

        virtual ~SessionCallback() = default;
    };

        JsonRpcService(int port, const std::string &endpoint,
            std::unique_ptr<SessionCallback> m_sessionCallback);

        bool OnConnection(WebSocketConnection *connection) override;

        void OnMessageReceived(WebSocketConnection *connection, const std::string &text) override;

        void OnDisconnected(WebSocketConnection *connection) override;

        void OnServiceStopped() override;

        void RespondError(int connectionId, const std::string &id, int code, const std::string &message,
                          const std::string &method);

        void RespondError(int connectionId, const std::string &id, int code, const std::string &message);

        void RespondNegotiateMethods(int connectionId, const std::string &id,
                                const std::string &terminalToApp,
                                const std::string &appToTerminal);

        void RespondSubscribe(int connectionId, const std::string &id, bool msgType0, bool msgType1,
                              bool msgType2, bool msgType3, bool msgType4, bool msgType5, bool msgType6,
                              bool msgType7);

        void RespondFeatureSuppress(int connectionId, const std::string &id, const std::string &feature,
                                    const std::string &value);

        void RespondFeatureSettingsInVisionSigning(int connectionId, const std::string &id, bool enabled);

        void RespondFeatureSettingsAudioDescription(int connectionId, const std::string &id, bool enabled,
                                               int gainPreference, int panAzimuthPreference);

        void RespondFeatureSettingsResponseToUserAction(int connectionId, const std::string &id,
                                                   bool enabled,
                                                   const std::string &type);

        void RespondFeatureSettingsScreenReader(int connectionId, const std::string &id, bool enabled,
                                           int speed,
                                           const std::string &voice, const std::string &language);

        void RespondFeatureSettingsHighContrastUI(int connectionId, const std::string &id, bool enabled,
                                                  const std::string &hcType);

        void RespondFeatureSettingsUiMagnifier(int connectionId, const std::string &id, bool enabled,
                                               const std::string &magType);

        void RespondFeatureSettingsQueryDialogueEnhancement(int connectionId, const std::string &id,
                                                            int dialogueEnhancementGainPreference,
                                                            int dialogueEnhancementGain,
                                                            int dialogueEnhancementLimitMin,
                                                            int dialogueEnhancementLimitMax);

        void RespondFeatureSettingsQuerySubtitles(int connectionId, const std::string &id, bool enabled,
                                                  int size, const std::string &fontFamily,
                                                  const std::string &textColour, int textOpacity,
                                                  const std::string &edgeType,
                                                  const std::string &edgeColour,
                                                  const std::string &backgroundColour,
                                                  int backgroundOpacity,
                                                  const std::string &windowColour,
                                                  int windowOpacity, const std::string &language);

        void RespondFeatureSupportInfo(int connectionId, const std::string &id, const std::string &feature,
                                  const std::string &value);

        void
        SendIntentMediaPause(int connectionId, const std::string &id, const std::string &origin);

        void
        SendIntentMediaPlay(int connectionId, const std::string &id, const std::string &origin);

        void SendIntentMediaFastForward(int connectionId, const std::string &id,
                                        const std::string &origin);

        void SendIntentMediaFastReverse(int connectionId, const std::string &id,
                                        const std::string &origin);

        void
        SendIntentMediaStop(int connectionId, const std::string &id, const std::string &origin);

        void SendIntentMediaSeekContent(int connectionId, const std::string &id,
                                        const std::string &origin,
                                        const std::string &anchor, int offset);

        void SendIntentMediaSeekRelative(int connectionId, const std::string &id,
                                         const std::string &origin,
                                         const std::string &anchor, int offset);

        void
        SendIntentMediaSeekLive(int connectionId, const std::string &id, const std::string &origin,
                                int offset);

        void
        SendIntentMediaSeekWallclock(int connectionId, const std::string &id,
                                     const std::string &origin,
                                     const std::string &dateTime);

        void SendIntentSearch(int connectionId, const std::string &id, const std::string &origin,
                              const std::string &query);

        void SendIntentDisplay(int connectionId, const std::string &id, const std::string &origin,
                               const std::string &mediaId);

        void SendIntentPlayback(int connectionId, const std::string &id, const std::string &origin,
                                const std::string &mediaId, const std::string &anchor, int offset);

        void NotifyInVisionSigning(int connectionId, const std::string &id, bool enabled);

        void
        NotifyAudioDescription(int connectionId, const std::string &id, bool enabled,
                               int gainPreference,
                               int panAzimuthPreference);

        void NotifyResponseToUserAction(int connectionId, const std::string &id, bool enabled,
                                        const std::string &type);

        void NotifyScreenReader(int connectionId, const std::string &id, bool enabled, int speed,
                                const std::string &voice, const std::string &language);

        void NotifyHighContrastUI(int connectionId, const std::string &id, bool enabled,
                                  const std::string &hcType);

        void NotifyUIMagnifier(int connectionId, const std::string &id, bool enabled,
                               const std::string &magType);

        void NotifyDialogueEnhancement(int connectionId, const std::string &id,
                                       int dialogueEnhancementGainPreference,
                                       int dialogueEnhancementGain,
                                       int dialogueEnhancementLimitMin,
                                       int dialogueEnhancementLimitMax);

        void NotifySubtitles(int connectionId, const std::string &id, bool enabled, int size,
                             const std::string &fontFamily, const std::string &textColour,
                             int textOpacity,
                             const std::string &edgeType, const std::string &edgeColour,
                             const std::string &backgroundColour, int backgroundOpacity,
                             const std::string &windowColour, int windowOpacity,
                             const std::string &language);
        void RespondDialogueEnhancementOverride(int connectionId, const std::string &id,
                                                int dialogueEnhancementGain);

        void
        RespondTriggerResponseToUserAction(int connectionId, const std::string &id, bool actioned);
    private:
        std::string m_endpoint;
        std::unique_ptr<SessionCallback> m_sessionCallback;

    };
} // namespace NetworkServices

#endif // OBS_NS_JSONRPCSERVICE_
