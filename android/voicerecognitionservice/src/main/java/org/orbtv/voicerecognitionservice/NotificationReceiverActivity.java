package org.orbtv.voicerecognitionservice;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;

public class NotificationReceiverActivity extends Activity {
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.result);
    }
}
