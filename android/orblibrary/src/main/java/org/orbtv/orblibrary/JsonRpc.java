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
        Log.d(TAG, "JSON-RPC-EXAMPLE #7a: onRespondNegotiateMethods...");
        nativeOnRespondNegotiateMethods(connection, id, terminalToApp, appToTerminal);
    }

    public void onRespondSubscribe(
        boolean isSubscribe,
        int connection,
        String id,
        boolean subtitles, boolean dialogueEnhancement,
        boolean uiMagnifier, boolean highContrastUI,
        boolean screenReader, boolean responseToUserAction,
        boolean audioDescription, boolean inVisionSigning) {
        Log.d(TAG, "JSON-RPC-EXAMPLE #7a: onRespondSubscribe...");
        nativeOnRespondSubscribe(isSubscribe, connection, id,
                subtitles, dialogueEnhancement, uiMagnifier, highContrastUI,
                screenReader, responseToUserAction, audioDescription, inVisionSigning);
    }

    public void onRespondDialogueEnhancementOverride(
        int connection,
        String id,
        int dialogueEnhancementGain) {
        Log.d(TAG, "JSON-RPC-EXAMPLE #7a: onRespondDialogueEnhancementOverride...");
        nativeOnRespondDialogueEnhancementOverride(connection, id, dialogueEnhancementGain);
    }

    public void onRespondTriggerResponseToUserAction(
            int connection,
            String id,
            boolean actioned) {
        Log.d(TAG, "JSON-RPC-EXAMPLE #7a: onRespondTriggerResponseToUserAction...");
        nativeOnRespondTriggerResponseToUserAction(connection, id, actioned);
    }

    public void onRespondFeatureSupportInfo(
        int connection,
        String id,
        int feature,
        String value) {
        Log.d(TAG, "JSON-RPC-EXAMPLE #7a: onRespondFeatureSupportInfo...");
        nativeOnRespondFeatureSupportInfo(connection, id, feature, value);
    }

    public void onRespondFeatureSuppress(
        int connection,
        String id,
        int feature,
        String value) {
        Log.d(TAG, "JSON-RPC-EXAMPLE #7a: onRespondFeatureSuppress...");
        nativeOnRespondFeatureSuppress(connection, id, feature, value);
    }

    public void onRespondError(
        int connection,
        String id,
        int code,
        String message) {
        Log.d(TAG, "JSON-RPC-EXAMPLE #7a: onRespondError...");
        nativeOnRespondError(connection, id, code, message);
    }

    public void onRespondError(
        int connection,
        String id,
        int code,
        String message,
        String data) {
        Log.d(TAG, "JSON-RPC-EXAMPLE #7a: onRespondError...");
        nativeOnRespondErrorWithData(connection, id, code, message, data);
    }

    public void onQuerySubtitles(
        int connection,
        String id,
        boolean enabled,
        int size,
        String fontFamily,
        String textColour,
        int textOpacity,
        String edgeType,
        String edgeColour,
        String backgroundColour,
        int backgroundOpacity,
        String windowColour,
        int windowOpacity,
        String language) {
        Log.d(TAG, "JSON-RPC-EXAMPLE #7a: onQuerySubtitles...");
        nativeOnQuerySubtitles(connection, id,
                enabled, size, fontFamily, textColour, textOpacity,
                edgeType, edgeColour, backgroundColour, backgroundOpacity,
                windowColour, windowOpacity, language);
    }

    public void onQueryDialogueEnhancement(
        int connection,
        String id,
        int gainPreference,
        int gain,
        int limitMin,
        int limitMax) {
        Log.d(TAG, "JSON-RPC-EXAMPLE #7a: onQueryDialogueEnhancement...");
        nativeOnQueryDialogueEnhancement(connection, id,
                gainPreference, gain, limitMin, limitMax);
    }

    public void onQueryUIMagnifier(
        int connection,
        String id,
        boolean enabled,
        String magType) {
        Log.d(TAG, "JSON-RPC-EXAMPLE #7a: onQueryUIMagnifier...");
        nativeOnQueryUIMagnifier(connection, id, enabled, magType);
    }

    public void onQueryHighContrastUI(
        int connection,
        String id,
        boolean enabled,
        String hcType) {
        Log.d(TAG, "JSON-RPC-EXAMPLE #7a: onQueryHighContrastUI...");
        nativeOnQueryHighContrastUI(connection, id, enabled, hcType);
    }

    public void onQueryScreenReader(
        int connection,
        String id,
        boolean enabled,
        int speed,
        String voice,
        String language) {
        Log.d(TAG, "JSON-RPC-EXAMPLE #7a: onQueryScreenReader...");
        nativeOnQueryScreenReader(connection, id,
                enabled, speed, voice, language);
    }

    public void onQueryResponseToUserAction(
        int connection,
        String id,
        boolean enabled,
        String type) {
        Log.d(TAG, "JSON-RPC-EXAMPLE #7a: onQueryResponseToUserAction...");
        nativeOnQueryResponseToUserAction(connection, id, enabled, type);
    }

    public void onQueryAudioDescription(
        int connection,
        String id,
        boolean enabled,
        int gainPreference,
        int panAzimuthPreference) {
        Log.d(TAG, "JSON-RPC-EXAMPLE #7a: onQueryAudioDescription...");
        nativeOnQueryAudioDescription(connection, id,
                enabled, gainPreference, panAzimuthPreference);
    }

    public void onQueryInVisionSigning(
        int connection,
        String id,
        boolean enabled) {
        Log.d(TAG, "JSON-RPC-EXAMPLE #7a: onQueryInVisionSigning...");
        nativeOnQueryInVisionSigning(connection, id, enabled);
    }

    public void onSendIntentMediaBasics(
        int cmd,
        int connection,
        String id,
        String origin) {
        Log.d(TAG, "JSON-RPC-EXAMPLE #7a: onSendIntentMediaBasics...");
        nativeOnSendIntentMediaBasics(cmd, connection, id, origin);
    }

    public void onSendIntentMediaSeekContent(
        int connection,
        String id,
        String origin,
        String anchor,
        int offset) {
        Log.d(TAG, "JSON-RPC-EXAMPLE #7a: onSendIntentMediaSeekContent...");
        nativeOnSendIntentMediaSeekContent(connection, id, origin, anchor, offset);
    }

    public void onSendIntentMediaSeekRelative(
        int connection,
        String id,
        String origin,
        int offset) {
        Log.d(TAG, "JSON-RPC-EXAMPLE #7a: onSendIntentMediaSeekRelative...");
        nativeOnSendIntentMediaSeekRelative(connection, id, origin, offset);
    }

    public void onSendIntentMediaSeekLive(
        int connection,
        String id,
        String origin,
        int offset) {
        Log.d(TAG, "JSON-RPC-EXAMPLE #7a: onSendIntentMediaSeekLive...");
        nativeOnSendIntentMediaSeekLive(connection, id, origin, offset);
    }

    public void onSendIntentMediaSeekWallclock(
        int connection,
        String id,
        String origin,
        String dateTime) {
        Log.d(TAG, "JSON-RPC-EXAMPLE #7a: onSendIntentMediaSeekWallclock...");
        nativeOnSendIntentMediaSeekWallclock(connection, id, origin, dateTime);
    }

    public void onSendIntentSearch(
        int connection,
        String id,
        String origin,
        String query) {
        Log.d(TAG, "JSON-RPC-EXAMPLE #7a: onSendIntentSearch...");
        nativeOnSendIntentSearch(connection, id, origin, query);
    }

    public void onSendIntentDisplay(
        int connection,
        String id,
        String origin,
        String mediaId) {
        Log.d(TAG, "JSON-RPC-EXAMPLE #7a: onSendIntentDisplay...");
        nativeOnSendIntentDisplay(connection, id, origin, mediaId);
    }

    public void onSendIntentPlayback(
        int connection,
        String id,
        String origin,
        String mediaId,
        String anchor,
        int offset) {
        Log.d(TAG, "JSON-RPC-EXAMPLE #7a: onSendIntentPlayback...");
        nativeOnSendIntentPlayback(connection, id, origin, mediaId, anchor, offset);
    }

    // Called by native

    private void onRequestNegotiateMethods(
        int connection,
        String id,
        String terminalToApp,
        String appToTerminal) {
        Log.d(TAG, "JSON-RPC-EXAMPLE #4a: onRequestNegotiateMethods...");
        mOrbSessionCallback.onRequestNegotiateMethods(connection, id, terminalToApp, appToTerminal);
    }

    private void onRequestSubscribe(
        boolean isSubscribe,
        int connection,
        String id,
        boolean subtitles, boolean dialogueEnhancement,
        boolean uiMagnifier, boolean highContrastUI,
        boolean screenReader, boolean responseToUserAction,
        boolean audioDescription, boolean inVisionSigning) {
        Log.d(TAG, "JSON-RPC-EXAMPLE #4: onRequestSubscribe...");
        mOrbSessionCallback.onRequestSubscribe(isSubscribe, connection, id,
                subtitles, dialogueEnhancement, uiMagnifier, highContrastUI,
                screenReader, responseToUserAction, audioDescription, inVisionSigning);
    }

    private void onRequestDialogueEnhancementOverride(
        int connection,
        String id,
        int dialogueEnhancementGain) {
        Log.d(TAG, "JSON-RPC-EXAMPLE #4: onRequestDialogueEnhancementOverride...");
        mOrbSessionCallback.onRequestDialogueEnhancementOverride(connection, id, dialogueEnhancementGain);
    }

    private void onRequestTriggerResponseToUserAction(
            int connection,
            String id,
            String magnitude){
        Log.d(TAG, "JSON-RPC-EXAMPLE #4: onRequestTriggerResponseToUserAction...");
        mOrbSessionCallback.onRequestTriggerResponseToUserAction(connection, id, magnitude);
    }

    private void onRequestFeatureSupportInfo(
            int connection,
            String id,
            int feature) {
        Log.d(TAG, "JSON-RPC-EXAMPLE #4a: onRequestFeatureSupportInfo...");
        mOrbSessionCallback.onRequestFeatureSupportInfo(connection, id, feature);
    }

    private void onRequestFeatureSettingsQuery(
            int connection,
            String id,
            int feature) {
        Log.d(TAG, "JSON-RPC-EXAMPLE #4a: onRequestFeatureSettingsQuery...");
        mOrbSessionCallback.onRequestFeatureSettingsQuery(connection, id, feature);
    }

    private void onRequestFeatureSuppress(
            int connection,
            String id,
            int feature) {
        Log.d(TAG, "JSON-RPC-EXAMPLE #4a: onRequestFeatureSuppress...");
        mOrbSessionCallback.onRequestFeatureSuppress(connection, id, feature);
    }

    private void onReceiveIntentConfirm(
            int connection,
            String id,
            String method) {
        Log.d(TAG, "JSON-RPC-EXAMPLE #4a: onReceiveIntentConfirm...");
        mOrbSessionCallback.onReceiveIntentConfirm(connection, id, method);
    }

    private void onNotifyVoiceReady(
            int connection,
            boolean isReady) {
        Log.d(TAG, "JSON-RPC-EXAMPLE #4a: onNotifyVoiceReady...");
        mOrbSessionCallback.onNotifyVoiceReady(connection, isReady);
    }

    private void onNotifyStateMedia(
            int connection,
            String state,
            boolean actPause, boolean actPlay, boolean actFastForward, boolean actFastReverse, boolean actStop,
            boolean actSeekContent, boolean actSeekRelative, boolean actSeekLive, boolean actWallclock) {
        Log.d(TAG, "JSON-RPC-EXAMPLE #4a: onNotifyStateMedia...");
        mOrbSessionCallback.onNotifyStateMedia(connection, state,
                actPause, actPlay, actFastForward, actFastReverse, actStop,
                actSeekContent, actSeekRelative, actSeekLive, actWallclock);
    }

    private void onNotifyStateMediaAllValues(
            int connection,
            String state, String kind, String type, String currentTime,
            String rangeStart, String rangeEnd,
            boolean actPause, boolean actPlay, boolean actFastForward, boolean actFastReverse, boolean actStop,
            boolean actSeekContent, boolean actSeekRelative, boolean actSeekLive, boolean actWallclock,
            String mediaId, String title, String secTitle, String synopsis,
            boolean subtitlesEnabled, boolean subtitlesAvailable,
            boolean audioDescripEnabled, boolean audioDescripAvailable,
            boolean signLangEnabled, boolean signLangAvailable) {
        Log.d(TAG, "JSON-RPC-EXAMPLE #4a: onNotifyStateMedia...");
        mOrbSessionCallback.onNotifyStateMedia(connection,
                state, kind, type, currentTime,
                rangeStart, rangeEnd,
                actPause, actPlay, actFastForward, actFastReverse, actStop,
                actSeekContent, actSeekRelative, actSeekLive, actWallclock,
                mediaId, title, secTitle, synopsis,
                subtitlesEnabled, subtitlesAvailable,
                audioDescripEnabled, audioDescripAvailable,
                signLangEnabled, signLangAvailable);
    }

    private void onReceiveError(
            int connection,
            String id,
            int code,
            String message) {
        Log.d(TAG, "JSON-RPC-EXAMPLE #4a: onReceiveError...");
        mOrbSessionCallback.onReceiveError(connection, id, code, message);
    }

    private void onReceiveError(
            int connection,
            String id,
            int code,
            String message,
            String method,
            String data) {
        Log.d(TAG, "JSON-RPC-EXAMPLE #4a: onReceiveError with all params...");
        mOrbSessionCallback.onReceiveError(connection, id, code, message, method, data);
    }

    // Native

    private native void nativeOpen(int port, String endpoint);

    private native void nativeClose();

    private native void nativeOnRespondNegotiateMethods(
        int connection,
        String id,
        String terminalToApp,
        String appToTerminal);

    private native void nativeOnRespondSubscribe(
        boolean isSubscribe,
        int connection,
        String id,
        boolean subtitles, boolean dialogueEnhancement,
        boolean uiMagnifier, boolean highContrastUI,
        boolean screenReader, boolean responseToUserAction,
        boolean audioDescription, boolean inVisionSigning);

    private native void nativeOnRespondDialogueEnhancementOverride(
        int connection,
        String id,
        int dialogueEnhancementGain);

    private native void nativeOnRespondTriggerResponseToUserAction(
        int connection,
        String id,
        boolean actioned);

    private native void nativeOnRespondFeatureSupportInfo(
        int connection,
        String id,
        int feature,
        String value);

    private native void nativeOnRespondFeatureSuppress(
        int connection,
        String id,
        int feature,
        String value);

    private native void nativeOnRespondError(
        int connection,
        String id,
        int code,
        String message);

    private native void nativeOnRespondErrorWithData(
        int connection,
        String id,
        int code,
        String message,
        String data);


    private native void nativeOnQuerySubtitles(
        int connection,
        String id,
        boolean enabled,
        int size,
        String fontFamily,
        String textColour,
        int textOpacity,
        String edgeType,
        String edgeColour,
        String backgroundColour,
        int backgroundOpacity,
        String windowColour,
        int windowOpacity,
        String language);

    private native void nativeOnQueryDialogueEnhancement(
        int connection,
        String id,
        int gainPreference,
        int gain,
        int limitMin,
        int limitMax);

    private native void nativeOnQueryUIMagnifier(
        int connection,
        String id,
        boolean enabled,
        String magType);

    private native void nativeOnQueryHighContrastUI(
        int connection,
        String id,
        boolean enabled,
        String hcType);

    private native void nativeOnQueryScreenReader(
        int connection,
        String id,
        boolean enabled,
        int speed,
        String voice,
        String language);

    private native void nativeOnQueryResponseToUserAction(
        int connection,
        String id,
        boolean enabled,
        String type);

    private native void nativeOnQueryAudioDescription(
        int connection,
        String id,
        boolean enabled,
        int gainPreference,
        int panAzimuthPreference);

    private native void nativeOnQueryInVisionSigning(
        int connection,
        String id,
        boolean enabled);

    private native void nativeOnSendIntentMediaBasics(
        int cmd,
        int connection,
        String id,
        String origin);

    private native void nativeOnSendIntentMediaSeekContent(
        int connection,
        String id,
        String origin,
        String anchor,
        int offset);

    private native void nativeOnSendIntentMediaSeekRelative(
        int connection,
        String id,
        String origin,
        int offset);

    private native void nativeOnSendIntentMediaSeekLive(
        int connection,
        String id,
        String origin,
        int offset);

    private native void nativeOnSendIntentMediaSeekWallclock(
        int connection,
        String id,
        String origin,
        String dateTime);

    private native void nativeOnSendIntentSearch(
        int connection,
        String id,
        String origin,
        String query);

    private native void nativeOnSendIntentDisplay(
        int connection,
        String id,
        String origin,
        String mediaId);

    private native void nativeOnSendIntentPlayback(
        int connection,
        String id,
        String origin,
        String mediaId,
        String anchor,
        int offset);
}
