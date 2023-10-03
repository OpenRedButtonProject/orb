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