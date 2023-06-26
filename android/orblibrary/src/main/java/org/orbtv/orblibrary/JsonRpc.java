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


    public void onRespondNegotiateMethods(int connection, int id,
                                          String terminalToApp, String appToTerminal) {
        Log.d(TAG, "JSON-RPC-EXAMPLE #7: onRespondNegotiateMethods...");
        nativeOnRespondNegotiateMethods(connection, id, terminalToApp, appToTerminal);
    }

    public void onRespondSubscribe(
        boolean isSubscribe,
        int connection,
        int id,
        boolean subtitles, boolean dialogueEnhancement,
        boolean uiMagnifier, boolean highContrastUI,
        boolean screenReader, boolean responseToUserAction,
        boolean audioDescription, boolean inVisionSigning) {
        Log.d(TAG, "JSON-RPC-EXAMPLE #7: onRespondSubscribe...");
        nativeOnRespondSubscribe(isSubscribe, connection, id,
                subtitles, dialogueEnhancement, uiMagnifier, highContrastUI,
                screenReader, responseToUserAction, audioDescription, inVisionSigning);
    }

    public void onRespondDialogueEnhancementOverride(
        int connection,
        int id,
        int dialogueEnhancementGain) {
        Log.d(TAG, "JSON-RPC-EXAMPLE #7: JsonRpc Java called with response. Call native...");
        nativeOnRespondDialogueEnhancementOverride(connection, id, dialogueEnhancementGain);
    }

    public void onRespondFeatureSupportInfo(
        int connection,
        int id,
        int feature,
        String value) {
        Log.d(TAG, "JSON-RPC-EXAMPLE #7a: JsonRpc Java called with response. Call native...");
        nativeOnRespondFeatureSupportInfo(connection, id, feature, value);
    }

    public void onRespondFeatureSettingsQuery(
        int connection,
        int id,
        int feature) {
        Log.d(TAG, "JSON-RPC-EXAMPLE #7a: JsonRpc Java called with response. Call native...");
        nativeOnRespondFeatureSettingsQuery(connection, id, feature);
    }

    public void onRespondFeatureSuppress(
        int connection,
        int id,
        int feature,
        String value) {
        Log.d(TAG, "JSON-RPC-EXAMPLE #7a: JsonRpc Java called with response. Call native...");
        nativeOnRespondFeatureSuppress(connection, id, feature, value);
    }

    public void onRespondError(
        int connection,
        int id,
        int code,
        String message) {
        Log.d(TAG, "JSON-RPC-EXAMPLE #7a: onRespondError...");
        nativeOnRespondError(connection, id, code, message);
    }

    public void onRespondError(
        int connection,
        int id,
        int code,
        String message,
        String method) {
        Log.d(TAG, "JSON-RPC-EXAMPLE #7a: onRespondError...");
        nativeOnRespondErrorWithMethod(connection, id, code, message, method);
    }

    // Called by native

    private void onRequestNegotiateMethods(
            int connection,
            int id,
            String terminalToApp,
            String appToTerminal) {
        Log.d(TAG, "JSON-RPC-EXAMPLE #4a: onRequestNegotiateMethods...");
        mOrbSessionCallback.onRequestNegotiateMethods(connection, id, terminalToApp, appToTerminal);
    }

    private void onRequestSubscribe(
        boolean isSubscribe,
        int connection,
        int id,
        boolean subtitles, boolean dialogueEnhancement,
        boolean uiMagnifier, boolean highContrastUI,
        boolean screenReader, boolean responseToUserAction,
        boolean audioDescription, boolean inVisionSigning) {
        Log.d(TAG, "JSON-RPC-EXAMPLE #4: Java called with request. Call ORB session callback...");
        mOrbSessionCallback.onRequestSubscribe(isSubscribe, connection, id,
                subtitles, dialogueEnhancement, uiMagnifier, highContrastUI,
                screenReader, responseToUserAction, audioDescription, inVisionSigning);
    }

    private void onRequestDialogueEnhancementOverride(
        int connection,
        int id,
        int dialogueEnhancementGain) {
        Log.d(TAG, "JSON-RPC-EXAMPLE #4: Java called with request. Call ORB session callback...");
        mOrbSessionCallback.onRequestDialogueEnhancementOverride(connection, id, dialogueEnhancementGain);
    }

    private void onRequestFeatureSupportInfo(
            int connection,
            int id,
            int feature) {
        Log.d(TAG, "JSON-RPC-EXAMPLE #4a: onRequestFeatureSupportInfo...");
        mOrbSessionCallback.onRequestFeatureSupportInfo(connection, id, feature);
    }

    private void onRequestFeatureSettingsQuery(
            int connection,
            int id,
            int feature) {
        Log.d(TAG, "JSON-RPC-EXAMPLE #4a: onRequestFeatureSettingsQuery...");
        mOrbSessionCallback.onRequestFeatureSettingsQuery(connection, id, feature);
    }

    private void onRequestFeatureSuppress(
            int connection,
            int id,
            int feature) {
        Log.d(TAG, "JSON-RPC-EXAMPLE #4a: onRequestFeatureSuppress...");
        mOrbSessionCallback.onRequestFeatureSuppress(connection, id, feature);
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
            int id,
            int code,
            String message) {
        Log.d(TAG, "JSON-RPC-EXAMPLE #4a: onReceiveError...");
        mOrbSessionCallback.onReceiveError(connection, id, code, message);
    }

    // Native

    private native void nativeOpen(int port, String endpoint);

    private native void nativeClose();

    private native void nativeOnRespondNegotiateMethods(
        int connection,
        int id,
        String terminalToApp,
        String appToTerminal);

    private native void nativeOnRespondSubscribe(
        boolean isSubscribe,
        int connection,
        int id,
        boolean subtitles, boolean dialogueEnhancement,
        boolean uiMagnifier, boolean highContrastUI,
        boolean screenReader, boolean responseToUserAction,
        boolean audioDescription, boolean inVisionSigning);

    private native void nativeOnRespondDialogueEnhancementOverride(
        int connection,
        int id,
        int dialogueEnhancementGain);

    private native void nativeOnRespondFeatureSupportInfo(
        int connection,
        int id,
        int feature,
        String value);

    private native void nativeOnRespondFeatureSettingsQuery(
        int connection,
        int id,
        int feature);

    private native void nativeOnRespondFeatureSuppress(
        int connection,
        int id,
        int feature,
        String value);

    private native void nativeOnRespondError(
            int connection,
            int id,
            int code,
            String message);

    private native void nativeOnRespondErrorWithMethod(
            int connection,
            int id,
            int code,
            String message,
            String method);
}
