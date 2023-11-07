/**
 * ORB Software. Copyright (c) 2023 Ocean Blue Software Limited
 * <p>
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 * <p>
 * The NotificationReceiverActivity is essential to keep a background service
 * (audio recording on Android) running continuously. If there is no continuous
 * notification, Android limit and terminate the service to
 * conserve resources.
 */

package org.orbtv.voicerecognitionservice;

import android.app.Activity;
import android.os.Bundle;

public class NotificationReceiverActivity extends Activity {
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.result);
    }
}
