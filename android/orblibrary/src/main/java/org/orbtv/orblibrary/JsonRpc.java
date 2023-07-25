package org.orbtv.orblibrary;

import android.util.Log;

import java.util.UUID;

public class JsonRpc {
    final static String TAG = JsonRpc.class.getSimpleName();

    final static String JSON_RPC_SERVER_BASE_URL = "ws://127.0.0.1:"; // TODO wss?
    final static String JSON_RPC_SERVER_VERSION = "1.7.1";

    private final int mPort;
    private final String mEndpoint;
    private final IOrbSessionCallback mOrbSessionCallback;
    private long mServicePointerField; // Pointer field used by native code

    JsonRpc(int port, IOrbSessionCallback orbSessionCallback) {
        mPort = port;
        mOrbSessionCallback = orbSessionCallback;
        mEndpoint = "/hbbtv/" + UUID.randomUUID().toString() + "/";
        nativeOpen(mPort, mEndpoint);
    }

    public void close() {
        nativeClose();
    }

    public String getUrl() {
        return JSON_RPC_SERVER_BASE_URL + mPort + mEndpoint;
    }

    public String getVersion() {
        return JSON_RPC_SERVER_VERSION;
    }


    public void onRespondNegotiateMethods(int connection, String id,
                                          String terminalToApp, String appToTerminal) {
        nativeOnRespondNegotiateMethods(connection, id, terminalToApp, appToTerminal);
    }

    public void onRespondSubscribe(boolean isSubscribe, int connection, String id,
                                   boolean subtitles, boolean dialogueEnhancement,
                                   boolean uiMagnifier, boolean highContrastUI,
                                   boolean screenReader, boolean responseToUserAction,
                                   boolean audioDescription, boolean inVisionSigning) {
        nativeOnRespondSubscribe(isSubscribe, connection, id,
                subtitles, dialogueEnhancement, uiMagnifier, highContrastUI,
                screenReader, responseToUserAction, audioDescription, inVisionSigning);
    }

    public void onRespondDialogueEnhancementOverride(int connection, String id,
                                                     int dialogueEnhancementGain) {
        nativeOnRespondDialogueEnhancementOverride(connection, id, dialogueEnhancementGain);
    }

    public void onRespondTriggerResponseToUserAction(int connection, String id, boolean actioned) {
        nativeOnRespondTriggerResponseToUserAction(connection, id, actioned);
    }

    public void onRespondFeatureSupportInfo(int connection, String id, int feature, String value) {
        nativeOnRespondFeatureSupportInfo(connection, id, feature, value);
    }

    public void onRespondFeatureSuppress(int connection, String id, int feature, String value) {
        nativeOnRespondFeatureSuppress(connection, id, feature, value);
    }

    public void onRespondError(int connection, String id, int code, String message) {
        nativeOnRespondError(connection, id, code, message);
    }

    public void onRespondError(int connection, String id, int code, String message, String data) {
        nativeOnRespondErrorWithData(connection, id, code, message, data);
    }

    public void onQuerySubtitles(int connection, String id, boolean enabled,
                                 int size, String fontFamily, String textColour, int textOpacity,
                                 String edgeType, String edgeColour,
                                 String backgroundColour, int backgroundOpacity,
                                 String windowColour, int windowOpacity, String language) {
        nativeOnQuerySubtitles(connection, id,
                enabled, size, fontFamily, textColour, textOpacity,
                edgeType, edgeColour, backgroundColour, backgroundOpacity,
                windowColour, windowOpacity, language);
    }

    public void onQueryDialogueEnhancement(int connection, String id, int gainPreference, int gain,
                                           int limitMin, int limitMax) {
        nativeOnQueryDialogueEnhancement(connection, id,
                gainPreference, gain, limitMin, limitMax);
    }

    public void onQueryUIMagnifier(int connection, String id, boolean enabled, String magType) {
        nativeOnQueryUIMagnifier(connection, id, enabled, magType);
    }

    public void onQueryHighContrastUI(int connection, String id, boolean enabled, String hcType) {
        nativeOnQueryHighContrastUI(connection, id, enabled, hcType);
    }

    public void onQueryScreenReader(int connection, String id,
                                    boolean enabled, int speed, String voice, String language) {
        nativeOnQueryScreenReader(connection, id, enabled, speed, voice, language);
    }

    public void onQueryResponseToUserAction(int connection, String id, boolean enabled, String type) {
        nativeOnQueryResponseToUserAction(connection, id, enabled, type);
    }

    public void onQueryAudioDescription(int connection, String id, boolean enabled,
                                        int gainPreference, int panAzimuthPreference) {
        nativeOnQueryAudioDescription(connection, id,
                enabled, gainPreference, panAzimuthPreference);
    }

    public void onQueryInVisionSigning(int connection, String id, boolean enabled) {
        nativeOnQueryInVisionSigning(connection, id, enabled);
    }

    public void onSendIntentMediaBasics(int cmd, int connection, String id, String origin) {
        nativeOnSendIntentMediaBasics(cmd, connection, id, origin);
    }

    public void onSendIntentMediaSeekContent(int connection, String id, String origin,
                                             String anchor, int offset) {
        nativeOnSendIntentMediaSeekContent(connection, id, origin, anchor, offset);
    }

    public void onSendIntentMediaSeekRelative(int connection, String id, String origin,
                                              int offset) {
        nativeOnSendIntentMediaSeekRelative(connection, id, origin, offset);
    }

    public void onSendIntentMediaSeekLive(int connection, String id, String origin,
                                          int offset) {
        nativeOnSendIntentMediaSeekLive(connection, id, origin, offset);
    }

    public void onSendIntentMediaSeekWallclock(int connection, String id, String origin,
                                               String dateTime) {
        nativeOnSendIntentMediaSeekWallclock(connection, id, origin, dateTime);
    }

    public void onSendIntentSearch(int connection, String id, String origin, String query) {
        nativeOnSendIntentSearch(connection, id, origin, query);
    }

    public void onSendIntentDisplay(int connection, String id, String origin, String mediaId) {
        nativeOnSendIntentDisplay(connection, id, origin, mediaId);
    }

    public void onSendIntentPlayback(int connection, String id, String origin, String mediaId,
                                     String anchor, int offset) {
        nativeOnSendIntentPlayback(connection, id, origin, mediaId, anchor, offset);
    }

    // Called by native

    private void onRequestNegotiateMethods(int connection, String id,
                                           String terminalToApp, String appToTerminal) {
        mOrbSessionCallback.onRequestNegotiateMethods(connection, id, terminalToApp, appToTerminal);
    }

    private void onRequestSubscribe(boolean isSubscribe, int connection, String id,
                                    boolean subtitles, boolean dialogueEnhancement,
                                    boolean uiMagnifier, boolean highContrastUI,
                                    boolean screenReader, boolean responseToUserAction,
                                    boolean audioDescription, boolean inVisionSigning) {
        mOrbSessionCallback.onRequestSubscribe(isSubscribe, connection, id,
                subtitles, dialogueEnhancement, uiMagnifier, highContrastUI,
                screenReader, responseToUserAction, audioDescription, inVisionSigning);
    }

    private void onRequestDialogueEnhancementOverride(int connection, String id,
                                                      int dialogueEnhancementGain) {
        mOrbSessionCallback.onRequestDialogueEnhancementOverride(connection, id, dialogueEnhancementGain);
    }

    private void onRequestTriggerResponseToUserAction(int connection, String id, String magnitude) {
        mOrbSessionCallback.onRequestTriggerResponseToUserAction(connection, id, magnitude);
    }

    private void onRequestFeatureSupportInfo(int connection, String id, int feature) {
        mOrbSessionCallback.onRequestFeatureSupportInfo(connection, id, feature);
    }

    private void onRequestFeatureSettingsQuery(int connection, String id, int feature) {
        mOrbSessionCallback.onRequestFeatureSettingsQuery(connection, id, feature);
    }

    private void onRequestFeatureSuppress(int connection, String id, int feature) {
        mOrbSessionCallback.onRequestFeatureSuppress(connection, id, feature);
    }

    private void onReceiveIntentConfirm(int connection, String id, String method) {
        mOrbSessionCallback.onReceiveIntentConfirm(connection, id, method);
    }

    private void onNotifyVoiceReady(int connection, boolean isReady) {
        mOrbSessionCallback.onNotifyVoiceReady(connection, isReady);
    }

    private void onNotifyStateMedia(int connection, String state,
                                    boolean actPause, boolean actPlay, boolean actFastForward,
                                    boolean actFastReverse, boolean actStop,
                                    boolean actSeekContent, boolean actSeekRelative,
                                    boolean actSeekLive, boolean actWallclock) {
        mOrbSessionCallback.onNotifyStateMedia(connection, state,
                actPause, actPlay, actFastForward, actFastReverse, actStop,
                actSeekContent, actSeekRelative, actSeekLive, actWallclock);
    }

    private void onNotifyStateMediaAllValues(int connection, String state, String kind, String type,
                                             String currentTime, String rangeStart, String rangeEnd,
                                             boolean actPause, boolean actPlay, boolean actFastForward,
                                             boolean actFastReverse, boolean actStop,
                                             boolean actSeekContent, boolean actSeekRelative,
                                             boolean actSeekLive, boolean actWallclock,
                                             String mediaId, String title, String secTitle, String synopsis,
                                             boolean subtitlesEnabled, boolean subtitlesAvailable,
                                             boolean audioDescripEnabled, boolean audioDescripAvailable,
                                             boolean signLangEnabled, boolean signLangAvailable) {
        mOrbSessionCallback.onNotifyStateMedia(connection, state, kind, type,
                currentTime, rangeStart, rangeEnd,
                actPause, actPlay, actFastForward, actFastReverse, actStop,
                actSeekContent, actSeekRelative, actSeekLive, actWallclock,
                mediaId, title, secTitle, synopsis,
                subtitlesEnabled, subtitlesAvailable,
                audioDescripEnabled, audioDescripAvailable,
                signLangEnabled, signLangAvailable);
    }

    private void onReceiveError(int connection, String id, int code, String message) {
        mOrbSessionCallback.onReceiveError(connection, id, code, message);
    }

    private void onReceiveError(int connection, String id, int code, String message,
                                String method, String data) {
        mOrbSessionCallback.onReceiveError(connection, id, code, message, method, data);
    }

    // Native

    private native void nativeOpen(int port, String endpoint);

    private native void nativeClose();

    private native void nativeOnRespondNegotiateMethods(int connection, String id,
                                                        String terminalToApp, String appToTerminal);

    private native void nativeOnRespondSubscribe(boolean isSubscribe, int connection, String id,
                                                 boolean subtitles, boolean dialogueEnhancement,
                                                 boolean uiMagnifier, boolean highContrastUI,
                                                 boolean screenReader, boolean responseToUserAction,
                                                 boolean audioDescription, boolean inVisionSigning);

    private native void nativeOnRespondDialogueEnhancementOverride(int connection, String id,
                                                                   int dialogueEnhancementGain);

    private native void nativeOnRespondTriggerResponseToUserAction(int connection, String id,
                                                                   boolean actioned);

    private native void nativeOnRespondFeatureSupportInfo(int connection, String id,
                                                          int feature, String value);

    private native void nativeOnRespondFeatureSuppress(int connection, String id,
                                                       int feature, String value);

    private native void nativeOnRespondError(int connection, String id, int code, String message);

    private native void nativeOnRespondErrorWithData(int connection, String id, int code,
                                                     String message, String data);

    private native void nativeOnQuerySubtitles(int connection, String id, boolean enabled,
                                               int size, String fontFamily,
                                               String textColour, int textOpacity,
                                               String edgeType, String edgeColour,
                                               String backgroundColour, int backgroundOpacity,
                                               String windowColour, int windowOpacity,
                                               String language);

    private native void nativeOnQueryDialogueEnhancement(int connection, String id,
                                                         int gainPreference, int gain,
                                                         int limitMin, int limitMax);

    private native void nativeOnQueryUIMagnifier(int connection, String id, boolean enabled,
                                                 String magType);

    private native void nativeOnQueryHighContrastUI(int connection, String id, boolean enabled,
                                                    String hcType);

    private native void nativeOnQueryScreenReader(int connection, String id, boolean enabled,
                                                  int speed, String voice, String language);

    private native void nativeOnQueryResponseToUserAction(int connection, String id, boolean enabled,
                                                          String type);

    private native void nativeOnQueryAudioDescription(int connection, String id, boolean enabled,
                                                      int gainPreference, int panAzimuthPreference);

    private native void nativeOnQueryInVisionSigning(int connection, String id, boolean enabled);

    private native void nativeOnSendIntentMediaBasics(int cmd, int connection, String id,
                                                      String origin);

    private native void nativeOnSendIntentMediaSeekContent(int connection, String id, String origin,
                                                           String anchor, int offset);

    private native void nativeOnSendIntentMediaSeekRelative(int connection, String id, String origin,
                                                            int offset);

    private native void nativeOnSendIntentMediaSeekLive(int connection, String id, String origin,
                                                        int offset);

    private native void nativeOnSendIntentMediaSeekWallclock(int connection, String id, String origin,
                                                             String dateTime);

    private native void nativeOnSendIntentSearch(int connection, String id, String origin,
                                                 String query);

    private native void nativeOnSendIntentDisplay(int connection, String id, String origin,
                                                  String mediaId);

    private native void nativeOnSendIntentPlayback(int connection, String id, String origin,
                                                   String mediaId, String anchor, int offset);
}
