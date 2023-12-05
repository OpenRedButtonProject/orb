/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 * <p>
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package org.orbtv.mockorbapp;

import android.app.Activity;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.IBinder;
import android.os.RemoteException;
import android.provider.Settings;
import android.util.Log;
import android.util.Pair;
import android.view.KeyEvent;
import android.widget.FrameLayout;
import android.widget.TableLayout;
import android.widget.TableRow;
import android.widget.TextView;

import org.orbtv.mockdialservice.IMockDialService;
import org.orbtv.mockdialservice.IMockDialServiceCallback;
import org.orbtv.orblibrary.IOrbSession;
import org.orbtv.orblibrary.OrbSessionFactory;

import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;
import java.util.Locale;

public class MainActivity extends Activity {
    private static final String TAG = "MainActivity";

    private static final int MEDIA_SYNC_WC_PORT = 8182;
    private static final int MEDIA_SYNC_CII_PORT = 8183;
    private static final int MEDIA_SYNC_TS_PORT = 8184;
    private static final int APP2APP_LOCAL_PORT = 8185;
    private static final int APP2APP_REMOTE_PORT = 8186;
    private static final int JSON_RPC_PORT = 8187;
    private static final String MAIN_ACTIVITY_UUID = "abcd-efgh-hijk-lmno";
    private static final String USER_AGENT = "HbbTV/1.6.1 (+DRM; OBS; Android; v1.0.0-alpha; ; OBS;)";
    private static final String SANS_SERIF_FONT_FAMILY = "Tiresias Screenfont";
    private static final String FIXED_FONT_FAMILY = "Letter Gothic 12 Pitch";

    private static final String DIAL_PACKAGE_NAME = "org.orbtv.mockdialservice";
    private static final String DIAL_CLASS_NAME = "org.orbtv.mockdialservice.MockDialService";

    private static final String VOICE_RECOGNITION_PACKAGE_NAME = "org.orbtv.voicerecognitionservice";
    private static final String VOICE_RECOGNITION_CLASS_NAME = "org.orbtv.voicerecognitionservice.VoiceRecognitionService";

    private IOrbSession mTvBrowserSession = null;
    private MockOrbSessionCallback mMockCallback;
    private IMockDialService mDialService = null;
    private Intent mVoiceIntent = null;
    private int mDialAppId = -1;
    private int mMaxLogSize = 8;
    private List<Pair<String, String>> mLogLines = new ArrayList<>();

    private void consoleLog(String message) {
        SimpleDateFormat formatter = new SimpleDateFormat("HH:mm:ss:SSS",
                Locale.getDefault());
        mLogLines.add(new Pair<>(formatter.format(new Date()), message));
        if (mLogLines.size() > mMaxLogSize) {
            mLogLines.remove(0);
        }
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                TableLayout table = findViewById(R.id.log_table);
                for (int i = mMaxLogSize - 1; i >= 0; i--) {
                    TableRow row = (TableRow) table.getChildAt(i);
                    TextView time = (TextView) row.getChildAt(0);
                    TextView log = (TextView) row.getChildAt(1);
                    if (mLogLines.size() > i) {
                        Pair<String, String> pair = mLogLines.get(i);
                        time.setText(pair.first);
                        log.setText(pair.second);
                    } else {
                        time.setText("");
                        log.setText("");
                    }
                }
            }
        });
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        OrbSessionFactory.Configuration configuration = new OrbSessionFactory.Configuration(
                MEDIA_SYNC_WC_PORT,
                MEDIA_SYNC_CII_PORT,
                MEDIA_SYNC_TS_PORT,
                APP2APP_LOCAL_PORT,
                APP2APP_REMOTE_PORT,
                JSON_RPC_PORT,
                MAIN_ACTIVITY_UUID,
                USER_AGENT,
                SANS_SERIF_FONT_FAMILY,
                FIXED_FONT_FAMILY,
                getDoNotTrackEnabled(getApplicationContext()),
                getCleanTracksEnabled(getApplicationContext())
      );

        setContentView(R.layout.activity_main);
        Bundle extras = getIntent().getExtras();
        try {
            mMockCallback = new MockOrbSessionCallback(this, extras);
        } catch (Exception e) {
            e.printStackTrace();
            Log.d("orb_automation_msg", "finished");
            // Don't continue with invalid state
            return;
        }
        FrameLayout frameLayout = findViewById(R.id.frameLayout);
        mTvBrowserSession = OrbSessionFactory.createSession(getApplicationContext(), mMockCallback,
                configuration);
        bindDialService();
        bindVoiceRecognitionService();
        //frameLayout.addView(mTvBrowserSession.getView());
        mTvBrowserSession.onNetworkStatusEvent(true); // TODO(library) Is this good?
        mMockCallback.setConsoleCallback(new MockOrbSessionCallback.ConsoleCallback() {
            public void log(String message) {
                consoleLog(message);
            }
        });
        mMockCallback.registerVoiceReceiver();
    }

    @Override
    public boolean onKeyUp(int keyCode, KeyEvent event) {
        if (keyCode == KeyEvent.KEYCODE_U) {
            runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    startService(mVoiceIntent);
                }
            });
        }
        if (mMockCallback.onKeyUp(keyCode, event)) {
            return true;
        }
        return super.onKeyUp(keyCode, event);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        unbindDialService();
        unbindVoiceRecognitionService();
        mMockCallback.unregisterVoiceReceiver();
    }

    private final IMockDialServiceCallback.Stub mDialServiceCallback =
            new IMockDialServiceCallback.Stub() {
                @Override
                public int startApp(String payload) throws RemoteException {
                    Log.d(TAG, "Start HbbTV DIAL app with payload: " + payload);
                    if (mTvBrowserSession != null) {
                        mTvBrowserSession.processXmlAit(payload, false, "urn:dvb:metadata:cs:LinkedApplicationCS:2019:1.1");
                    }
                    return 2; // Running
                }

                @Override
                public int hideApp() throws RemoteException {
                    Log.d(TAG, "Hide HbbTV DIAL app");
                    return 2; // Running
                }

                @Override
                public void stopApp() throws RemoteException {
                    Log.d(TAG, "Stop HbbTV DIAL app");
                }

                @Override
                public int getAppStatus() throws RemoteException {
                    Log.d(TAG, "Get HbbTV DIAL app status");
                    return 2; // Running
                }
            };

    private final ServiceConnection mDialServiceConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName componentName, IBinder binder) {
            Log.d(TAG, "DIAL service connected");
            mDialService = IMockDialService.Stub.asInterface(binder);
            String data1 = "X_HbbTV_App2AppURL=" + mTvBrowserSession.getApp2AppRemoteBaseUrl();
            String data2 = "X_HbbTV_InterDevSyncURL=" + mTvBrowserSession.getInterDevSyncUrl();
            try {
                mDialAppId = mDialService.registerApp(mDialServiceCallback, "HbbTV", data1, data2);
            } catch (RemoteException e) {
                e.printStackTrace();
            }
            if (mDialAppId == -1) {
                Log.e(TAG, "Failed to register app with the DIAL server. This might happen " +
                        "because a 'HbbTV' app is already registered with the server.");
            }
        }

        @Override
        public void onServiceDisconnected(ComponentName componentName) {
            Log.e(TAG, "DIAL service disconnected");
            mDialService = null;
            mDialAppId = -1;
        }
    };

    private final ServiceConnection mVoiceRecognitionServiceConnection = new ServiceConnection() {
        public void onServiceConnected(ComponentName className, IBinder binder) {
            Log.d("VoiceRecognition ServiceConnection", "connected");
        }
        public void onServiceDisconnected(ComponentName className) {
            Log.d("VoiceRecognition ServiceConnection", "disconnected");
        }
    };

    private void bindDialService() {
        ComponentName component =
                new ComponentName(DIAL_PACKAGE_NAME, DIAL_CLASS_NAME);
        Intent mIntent = new Intent().setComponent(component);
        Log.d(TAG, "Try to resolve DIAL service intent: " +
                getApplicationContext().getPackageManager().queryIntentServices(mIntent, 0));
        getApplicationContext().bindService(mIntent,
                mDialServiceConnection,
                Context.BIND_AUTO_CREATE);
    }

    private void bindVoiceRecognitionService() {
        ComponentName component =
                new ComponentName(VOICE_RECOGNITION_PACKAGE_NAME, VOICE_RECOGNITION_CLASS_NAME);
        mVoiceIntent = new Intent().setComponent(component);
        Log.d(TAG, "Try to resolve Voice Recognition service intent: " +
                getApplicationContext().getPackageManager().queryIntentServices(mVoiceIntent, 0));
        getApplicationContext().bindService(mVoiceIntent,
                mVoiceRecognitionServiceConnection,
                Context.BIND_AUTO_CREATE);
    }

    private void unbindDialService() {
        if (mDialService != null) {
            try {
                mDialService.unregisterApp(mDialAppId);
            } catch (RemoteException e) {
                e.printStackTrace();
            }
            getApplicationContext().unbindService(mDialServiceConnection);
        }
    }

    private void unbindVoiceRecognitionService() {
        getApplicationContext().unbindService(mVoiceRecognitionServiceConnection);
    }

    private boolean getDoNotTrackEnabled(Context context) {
        String setting = Settings.Global.getString(context.getContentResolver(), "do_not_track_enabled");
        if (setting != null && setting.equals("1")) {
            Log.d(TAG, "do_not_track_enabled=1");
            return true;
        } else {
            Log.d(TAG, "do_not_track_enabled=unset|0");
            return false;
        }
    }

    private boolean getCleanTracksEnabled(Context context) {
      String setting = Settings.Global.getString(context.getContentResolver(), "clean_tracks_enabled");
      if (setting != null && setting.equals("1")) {
          Log.d(TAG, "clean_tracks_enabled=1");
          return true;
      } else {
          Log.d(TAG, "clean_tracks_enabled=unset|0");
          return false;
      }
    }

    public String getHostAddress() {
        // Use the same host address as the DIAL service
        String ip = null;
        if (mDialService != null) {
            try {
                ip = mDialService.getHostAddress();
            } catch (RemoteException ignored) {
            }
        }
        if (ip == null) {
            ip = "127.0.0.1";
        }
        return ip;
    }
}
