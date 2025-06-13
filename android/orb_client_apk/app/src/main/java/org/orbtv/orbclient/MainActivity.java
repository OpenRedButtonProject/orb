package org.orbtv.orbclient;


import android.app.Activity;
import android.os.Bundle;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.KeyEvent;
import android.widget.FrameLayout;

import org.orbtv.orbsession.OrbSession;


public class MainActivity extends Activity implements OrbSession.Callback {

    private static final String TAG = "MainActivity";

    // This is for test purposes only. The mock20xapp uses a TestSuiteRunner class to load URLs.
    public static final String TEST_URL = "http://itv.mit-xperts.com/hbbtvtest/index.php";

    private SurfaceView mSurfaceView;

    private OrbSession mOrbSession;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);  // Links to the layout

        FrameLayout layout = findViewById(R.id.contentview_holder);
        mSurfaceView = new SurfaceView(this);
        layout.addView(mSurfaceView, new FrameLayout.LayoutParams(
                FrameLayout.LayoutParams.MATCH_PARENT,
                FrameLayout.LayoutParams.MATCH_PARENT));

        mOrbSession = new OrbSession(this, this, mSurfaceView);

        mSurfaceView.getHolder().addCallback(new SurfaceHolder.Callback() {
            @Override
            public void surfaceCreated(SurfaceHolder holder) {
                Log.d(TAG, "Surface created");
                mOrbSession.connectToService();
            }

            @Override
            public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
                // Handle changes if necessary
                Log.d(TAG, "Surface changed");
            }

            @Override
            public void surfaceDestroyed(SurfaceHolder holder) {
                Log.d(TAG, "Surface destroyed");
            }
        });
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        mOrbSession.close();
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        return mOrbSession.sendKeyEvent(keyCode, event);
    }

    @Override
    public void onOrbSessionReady(String result) {
        mOrbSession.loadUrl(TEST_URL);
    }
}
