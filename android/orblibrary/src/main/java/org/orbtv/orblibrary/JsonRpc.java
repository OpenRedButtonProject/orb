package org.orbtv.orblibrary;

import android.util.Log;

public class JsonRpc {
    final static String TAG = JsonRpc.class.getSimpleName();

    private long mServicePointerField; // Pointer field used by native code

    JsonRpc() {
        nativeInit();
    }

    public void uninit() {
        nativeUninit();
    }

    private void onHelloTerminalCallback(String message) {
        Log.d(TAG, "Hello Terminal! Message: " + message);
    }

    private native void nativeInit();
    private native void nativeUninit();
    private native void nativeHelloApp();
}
