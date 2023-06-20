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

    public void onRespondDialogueEnhancementOverride(
        int connection,
        int id,
        int dialogueEnhancementGain) {
        Log.d(TAG, "JSON-RPC-EXAMPLE #7: JsonRpc Java called with response. Call native...");
        nativeOnRespondDialogueEnhancementOverride(connection, id, dialogueEnhancementGain);
    }

    // Called by native

    private void onRequestDialogueEnhancementOverride(
        int connection,
        int id,
        int dialogueEnhancementGain) {
        Log.d(TAG, "JSON-RPC-EXAMPLE #4: Java called with request. Call ORB session callback...");
        mOrbSessionCallback.onRequestDialogueEnhancementOverride(connection, id, dialogueEnhancementGain);
    }

    // Native

    private native void nativeOpen(int port, String endpoint);

    private native void nativeClose();

    private native void nativeOnRespondDialogueEnhancementOverride(
        int connection,
        int id,
        int dialogueEnhancementGain);
}
