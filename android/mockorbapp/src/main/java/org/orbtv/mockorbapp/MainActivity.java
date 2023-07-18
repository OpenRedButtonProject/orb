/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 * <p>
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
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
import android.util.Log;
import android.util.Pair;
import android.widget.FrameLayout;
import android.widget.TableLayout;
import android.widget.TableRow;
import android.widget.TextView;

import org.orbtv.orblibrary.OrbSessionFactory;
import org.orbtv.orblibrary.IOrbSession;

import org.orbtv.mockdialservice.IMockDialService;
import org.orbtv.mockdialservice.IMockDialServiceCallback;

import java.sql.Time;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;
import java.util.Locale;
import java.util.Vector;

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

    private IOrbSession mTvBrowserSession = null;
    private MockOrbSessionCallback mMockCallback;
    private IMockDialService mDialService = null;
    private int mDialAppId = -1;

    private List<Pair<String, String>> mLogLines = new ArrayList<>();
    private void log(String message) {
        SimpleDateFormat formatter = new SimpleDateFormat("HH:mm:ss:SSS",
                Locale.getDefault());
        mLogLines.add(new Pair<>(formatter.format(new Date()), message));
        if (mLogLines.size() > 8) {
            mLogLines.remove(0);
        }
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                TableLayout table = findViewById(R.id.log_table);
                for (int i = 7; i >= 0; i--) {
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
                FIXED_FONT_FAMILY
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
        //frameLayout.addView(mTvBrowserSession.getView());
        mTvBrowserSession.onNetworkStatusEvent(true); // TODO(library) Is this good?
        mMockCallback.setOnEventListener(new MockOrbSessionCallback.OnEventListener() {
            public void onShowMessage(String message) {
                log(message);
            }
        });
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        unbindDialService();
    }

    private final IMockDialServiceCallback.Stub mDialServiceCallback =
            new IMockDialServiceCallback.Stub() {
                @Override
                public int startApp(String payload) throws RemoteException {
                    Log.d(TAG, "Start HbbTV DIAL app with payload: " + payload);
                    if (mTvBrowserSession != null) {
                        mTvBrowserSession.processXmlAit(payload);
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

    private void bindDialService() {
        ComponentName component = new ComponentName(DIAL_PACKAGE_NAME, DIAL_CLASS_NAME);
        Intent intent = new Intent().setComponent(component);
        Log.d(TAG, "Try to resolve DIAL service intent: " +
                getApplicationContext().getPackageManager().queryIntentServices(intent, 0));
        getApplicationContext().bindService(intent, mDialServiceConnection, Context.BIND_AUTO_CREATE);
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
