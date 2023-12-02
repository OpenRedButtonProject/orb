package org.orbtv.mockorbapp;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

public class VoiceServiceReceiver extends BroadcastReceiver {
    private static final String TAG = VoiceServiceReceiver.class.getSimpleName();

    public interface ResultCallback {
        void onReceive(Integer action, String info, String anchor, int offset);
    }

    private VoiceServiceReceiver.ResultCallback mResultCallback;

    public void setVoiceCallback(VoiceServiceReceiver.ResultCallback callback) {
        mResultCallback = callback;
    }

    @Override
    public void onReceive(Context context, Intent intent) {
        int actId = intent.getIntExtra("actId", 1);
        String info = intent.getStringExtra("info");
        String anchor = intent.getStringExtra("anchor");
        int offset = intent.getIntExtra("offset", 0);
        Log.d(TAG, "Received msg : " + actId + ", " + info + ", " + anchor + ", " + offset);

        mResultCallback.onReceive(actId, info, anchor, offset);
    }
}
