/**
 * ORB Software. Copyright (c) 2023 Ocean Blue Software Limited
 * <p>
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 * <p>
 * PermissionRequestActivity is a simple activity used to request runtime permissions.
 * It requests the RECORD_AUDIO and WRITE_EXTERNAL_STORAGE permissions for speech to text.
 * Once the permission request is completed, the activity finishes.
 */

package org.orbtv.voicerecognitionservice;

import android.Manifest;
import android.app.Activity;
import android.os.Bundle;

public class PermissionRequestActivity extends Activity {
    private static final int REQUEST_PERMISSION_CODE = 1001;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        requestPermissions(new String[]{Manifest.permission.RECORD_AUDIO, Manifest.permission.WRITE_EXTERNAL_STORAGE}, REQUEST_PERMISSION_CODE);
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
        finish();
    }
}