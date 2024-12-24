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

package org.orbtv.mock204app;

import android.app.Activity;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
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
import java.util.HashMap;
import java.util.List;
import java.util.Locale;
import java.util.Map;

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

    private static final String ORB_PACKAGE_NAME = "org.orbtv.orbservice";
    private static final String ORB_CLASS_NAME = "org.orbtv.orbservice.OrbService";

    private static final String VOICE_RECOGNITION_PACKAGE_NAME = "org.orbtv.voicerecognitionservice";
    private static final String VOICE_RECOGNITION_CLASS_NAME = "org.orbtv.voicerecognitionservice.VoiceRecognitionService";

    private final String MOCK_MEDIA_ID = "urn:broadcaster:programme:1249863457643";
    private final int INTENT_MEDIA_PAUSE = 0;
    private final int INTENT_MEDIA_PLAY = 1;
    private final int INTENT_MEDIA_FAST_FORWARD = 2;
    private final int INTENT_MEDIA_FAST_REVERSE = 3;
    private final int INTENT_MEDIA_STOP = 4;
    private final int INTENT_MEDIA_SEEK_CONTENT = 5;
    private final int INTENT_MEDIA_SEEK_RELATIVE = 6;
    private final int INTENT_MEDIA_SEEK_LIVE = 7;
    private final int INTENT_MEDIA_SEEK_WALLCLOCK = 8;
    private final int INTENT_SEARCH = 9;
    private final int INTENT_DISPLAY = 10;
    private final int INTENT_PLAYBACK = 11;
    private final int ACT_REQUEST_MEDIA_DESCRIPTION = 18;
    private final int ACT_REQUEST_TEXT_INPUT = 19;
    private final int LOG_MESSAGE = 99;
    private final int LOG_ERROR_NONE_ACTION = 100;
    private final int LOG_ERROR_MULTI_ACTIONS = 101;
    private final int LOG_ERROR_INTENT_SEND = 102;

    private final int ACT_PRESS_BUTTON_NUMB_ZERO = 20;
    private final int ACT_PRESS_BUTTON_NUMB_ONE = 21;
    private final int ACT_PRESS_BUTTON_NUMB_TWO = 22;
    private final int ACT_PRESS_BUTTON_NUMB_THREE = 23;
    private final int ACT_PRESS_BUTTON_NUMB_FOUR = 24;
    private final int ACT_PRESS_BUTTON_NUMB_FIVE = 25;
    private final int ACT_PRESS_BUTTON_NUMB_SIX = 26;
    private final int ACT_PRESS_BUTTON_NUMB_SEVEN = 27;
    private final int ACT_PRESS_BUTTON_NUMB_EIGHT = 28;
    private final int ACT_PRESS_BUTTON_NUMB_NINE = 29;
    private final int ACT_PRESS_BUTTON_RED = 30;
    private final int ACT_PRESS_BUTTON_GREEN = 31;
    private final int ACT_PRESS_BUTTON_YELLOW = 32;
    private final int ACT_PRESS_BUTTON_BLUE = 33;
    private final int ACT_PRESS_BUTTON_UP = 34;
    private final int ACT_PRESS_BUTTON_DOWN = 35;
    private final int ACT_PRESS_BUTTON_LEFT = 36;
    private final int ACT_PRESS_BUTTON_RIGHT = 37;
    private final int ACT_PRESS_BUTTON_ENTER = 38;
    private final int ACT_PRESS_BUTTON_BACK = 39;
    private final Map<Integer, String> ACT_BUTTON_NAMES = new HashMap<Integer, String>() {
        {
            put(ACT_PRESS_BUTTON_NUMB_ZERO, "0");
            put(ACT_PRESS_BUTTON_NUMB_ONE, "1");
            put(ACT_PRESS_BUTTON_NUMB_TWO, "2");
            put(ACT_PRESS_BUTTON_NUMB_THREE, "3");
            put(ACT_PRESS_BUTTON_NUMB_FOUR, "4");
            put(ACT_PRESS_BUTTON_NUMB_FIVE, "5");
            put(ACT_PRESS_BUTTON_NUMB_SIX, "6");
            put(ACT_PRESS_BUTTON_NUMB_SEVEN, "7");
            put(ACT_PRESS_BUTTON_NUMB_EIGHT, "8");
            put(ACT_PRESS_BUTTON_NUMB_NINE, "9");
            put(ACT_PRESS_BUTTON_RED, "RED");
            put(ACT_PRESS_BUTTON_GREEN, "GREEN");
            put(ACT_PRESS_BUTTON_YELLOW, "YELLOW");
            put(ACT_PRESS_BUTTON_BLUE, "BLUE");
            put(ACT_PRESS_BUTTON_UP, "UP");
            put(ACT_PRESS_BUTTON_DOWN, "DOWN");
            put(ACT_PRESS_BUTTON_LEFT, "LEFT");
            put(ACT_PRESS_BUTTON_RIGHT, "RIGHT");
            put(ACT_PRESS_BUTTON_ENTER, "ENTER");
            put(ACT_PRESS_BUTTON_BACK, "BACK");
        }
    };

    public native void nativeServiceConnected(IBinder binder);
    public native void nativeServiceDisconnected();
    public native void nativeTest();

    private IOrbSession mTvBrowserSession = null;
    private MockOrbSessionCallback mMockCallback;
    private IMockDialService mDialService = null;
    private VoiceServiceReceiver mVoiceReceiver;
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
        bindOrbService();
        bindDialService();
        bindVoiceRecognitionService();
        frameLayout.addView(mTvBrowserSession.getView());
        TableLayout table = findViewById(R.id.log_table);
        table.bringToFront();
        mTvBrowserSession.onNetworkStatusEvent(true); // TODO(library) Is this good?
        mMockCallback.setConsoleCallback(new MockOrbSessionCallback.ConsoleCallback() {
            public void log(String message) {
                consoleLog(message);
            }
        });
        registerVoiceReceiver();
    }

    private void registerVoiceReceiver() {
        mVoiceReceiver = new VoiceServiceReceiver();
        registerReceiver(mVoiceReceiver, new IntentFilter("org.orbtv.voiceservice.message"));
        mVoiceReceiver.setVoiceCallback(new VoiceServiceReceiver.ResultCallback() {
            @Override
            public void onReceive(Integer action, String info, String anchor, int offset) {
                if (!sendVoiceCommand(action, info, anchor, offset)) {
                    consoleLog("Error in voice recognition");
                }
            }
        });
    }

    private void unregisterVoiceReceiver() {
        unregisterReceiver(mVoiceReceiver);
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
        unregisterVoiceReceiver();
        unbindOrbService();
    }

    private final ServiceConnection mOrbServiceConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName componentName, IBinder binder) {
            Log.d(TAG, "ORB service connected");
            nativeServiceConnected(binder);
            runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    nativeTest();
                }
            });
        }

        @Override
        public void onServiceDisconnected(ComponentName componentName) {
            Log.e(TAG, "ORB service disconnected");
            nativeServiceDisconnected();
        }
    };

    private void bindOrbService() {
        ComponentName component = new ComponentName(ORB_PACKAGE_NAME, ORB_CLASS_NAME);
        Intent intent = new Intent().setComponent(component);
        Log.d(TAG, "Try to resolve ORB service intent: " +
                getApplicationContext().getPackageManager().queryIntentServices(intent, 0));
        getApplicationContext().bindService(intent, mOrbServiceConnection, Context.BIND_AUTO_CREATE);
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

    private void unbindOrbService() {
         getApplicationContext().unbindService(mOrbServiceConnection);
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

    /**
     * @since 204
     *
     * Sends voice commands based on provided actions, messages, anchors, and offsets, where some of the parameters are optional.
     *
     * @param action The predefined index number of the intent, from intent.media.pause to intent.playback
     * @param info   The value uniquely identifying a piece of content:
     *               - INTENT_MEDIA_SEEK_WALLCLOCK: a wall clock time
     *               - INTENT_DISPLAY: a URI
     *               - INTENT_SEARCH: a search term specified by the user.
     *               - INTENT_PLAYBACK: a URI
     * @param anchor The value indicates an anchor point of the content...
     * @param offset The number value for the time position, a number of seconds
     * @return True if the command is successfully executed; otherwise, handles appropriately.
     */
    private boolean sendVoiceCommand(Integer action, String info, String anchor, int offset) {

        switch (action) {
            case INTENT_MEDIA_PAUSE:
            case INTENT_MEDIA_PLAY:
            case INTENT_MEDIA_FAST_FORWARD:
            case INTENT_MEDIA_FAST_REVERSE:
            case INTENT_MEDIA_STOP:
            case INTENT_MEDIA_SEEK_CONTENT:
            case INTENT_MEDIA_SEEK_RELATIVE:
            case INTENT_MEDIA_SEEK_LIVE:
            case INTENT_MEDIA_SEEK_WALLCLOCK:
            case INTENT_SEARCH:
            case INTENT_DISPLAY:
            case INTENT_PLAYBACK:
                return onVoiceSendIntent(action, info, anchor, offset);
            case ACT_REQUEST_MEDIA_DESCRIPTION:
                consoleLog("Request for the information of media playing...");
                return mTvBrowserSession.onVoiceRequestDescription();
            case ACT_REQUEST_TEXT_INPUT:
                consoleLog("Enter text {" + info + "}");
                return mTvBrowserSession.onVoiceRequestTextInput(info);
            case LOG_MESSAGE:
            case LOG_ERROR_NONE_ACTION:
            case LOG_ERROR_MULTI_ACTIONS:
            case LOG_ERROR_INTENT_SEND:
                consoleLog(info);
                return true;
            default:
                return onVoiceSendKeyAction(action);
        }
    }

    /**
     * @since 204
     *
     * Called to send a send a keyUp event, from a voice command, to the application, potentially dispatching the event and show a message on window log.
     *
     * @param action The index number of the intent, either pressing a button or showing a log
     */
    private boolean onVoiceSendKeyAction(Integer action) {

        String buttonName = ACT_BUTTON_NAMES.getOrDefault(action, "invalid");
        if (buttonName.equals("invalid")) {
            return false;
        }
        consoleLog("Press " + buttonName + " button");
        int keyCode;
        switch (action) {
            case ACT_PRESS_BUTTON_NUMB_ZERO:
            case ACT_PRESS_BUTTON_NUMB_ONE:
            case ACT_PRESS_BUTTON_NUMB_TWO:
            case ACT_PRESS_BUTTON_NUMB_THREE:
            case ACT_PRESS_BUTTON_NUMB_FOUR:
            case ACT_PRESS_BUTTON_NUMB_FIVE:
            case ACT_PRESS_BUTTON_NUMB_SIX:
            case ACT_PRESS_BUTTON_NUMB_SEVEN:
            case ACT_PRESS_BUTTON_NUMB_EIGHT:
            case ACT_PRESS_BUTTON_NUMB_NINE:
                keyCode = KeyEvent.KEYCODE_0 + action - ACT_PRESS_BUTTON_NUMB_ZERO;
                break;
            case ACT_PRESS_BUTTON_RED:
                keyCode = KeyEvent.KEYCODE_PROG_RED;
                break;
            case ACT_PRESS_BUTTON_GREEN:
                keyCode = KeyEvent.KEYCODE_PROG_GREEN;
                break;
            case ACT_PRESS_BUTTON_YELLOW:
                keyCode = KeyEvent.KEYCODE_PROG_YELLOW;
                break;
            case ACT_PRESS_BUTTON_BLUE:
                keyCode = KeyEvent.KEYCODE_PROG_BLUE;
                break;
            case ACT_PRESS_BUTTON_UP:
                keyCode = KeyEvent.KEYCODE_DPAD_UP;
                break;
            case ACT_PRESS_BUTTON_DOWN:
                keyCode = KeyEvent.KEYCODE_DPAD_DOWN;
                break;
            case ACT_PRESS_BUTTON_LEFT:
                keyCode = KeyEvent.KEYCODE_DPAD_LEFT;
                break;
            case ACT_PRESS_BUTTON_RIGHT:
                keyCode = KeyEvent.KEYCODE_DPAD_RIGHT;
                break;
            case ACT_PRESS_BUTTON_ENTER:
                keyCode = KeyEvent.KEYCODE_ENTER;
                break;
            case ACT_PRESS_BUTTON_BACK:
                keyCode = KeyEvent.KEYCODE_DEL;
                break;
            default:
                return false;
        }
        KeyEvent event = new KeyEvent(KeyEvent.ACTION_UP, keyCode);
        return mTvBrowserSession.dispatchKeyEvent(event);
    }

    /**
     * @since 204
     *
     * Called to send an intent, from a voice command, to applications
     *
     * @param action The index number of the intent, from intent.media.pause to intent.playback
     * @param info   The value uniquely identifying a piece of content:
     *               - INTENT_MEDIA_SEEK_WALLCLOCK: a wall clock time
     *               - INTENT_DISPLAY: a media name
     *               - INTENT_SEARCH: a search term specified by the user.
     *               - INTENT_PLAYBACK: a media name
     * @param anchor The value indicates an anchor point of the content, which is either "start" or "end"
     * @param offset The number value for the time position, a number of seconds
     * @return true if this event has been handled, and false if not
     */
    private boolean onVoiceSendIntent(Integer action, String info, String anchor,
                                      int offset) {

        switch (action) {
            case INTENT_MEDIA_PAUSE:
                mTvBrowserSession.onSendIntentMediaBasics(INTENT_MEDIA_PAUSE);
                consoleLog("Send an intent, action: pause");
                return true;
            case INTENT_MEDIA_PLAY:
                mTvBrowserSession.onSendIntentMediaBasics(INTENT_MEDIA_PLAY);
                consoleLog("Send an intent, action: play");
                return true;
            case INTENT_MEDIA_FAST_FORWARD:
                mTvBrowserSession.onSendIntentMediaBasics(INTENT_MEDIA_FAST_FORWARD);
                consoleLog("Send an intent, action: fast-forward");
                return true;
            case INTENT_MEDIA_FAST_REVERSE:
                mTvBrowserSession.onSendIntentMediaBasics(INTENT_MEDIA_FAST_REVERSE);
                consoleLog("Send an intent, action: fast-reverse");
                return true;
            case INTENT_MEDIA_STOP:
                mTvBrowserSession.onSendIntentMediaBasics(INTENT_MEDIA_STOP);
                consoleLog("Send an intent, action: stop");
                return true;
            case INTENT_MEDIA_SEEK_CONTENT:
                if (anchor.equals("start") || anchor.equals("end")) {
                    mTvBrowserSession.onSendIntentMediaSeekContent(anchor, offset);
                    consoleLog("Send an intent, action: seek-content");
                    return true;
                }
                break;
            case INTENT_MEDIA_SEEK_RELATIVE:
                mTvBrowserSession.onSendIntentMediaSeekRelative(offset);
                consoleLog("Send an intent, action: seek-relative");
                return true;
            case INTENT_MEDIA_SEEK_LIVE:
                mTvBrowserSession.onSendIntentMediaSeekLive(offset);
                consoleLog("Send an intent, action: seek-live");
                return true;
            case INTENT_DISPLAY:
                mTvBrowserSession.onSendIntentDisplay(MOCK_MEDIA_ID);
                consoleLog("Send an intent, action: display");
                return true;
            case INTENT_MEDIA_SEEK_WALLCLOCK:
                mTvBrowserSession.onSendIntentMediaSeekWallclock(info);
                consoleLog("Send an intent, action: seek-wallclock");
                return true;
            case INTENT_SEARCH:
                mTvBrowserSession.onSendIntentSearch(info);
                consoleLog("Send an intent, action: search");
                return true;
            case INTENT_PLAYBACK:
                mTvBrowserSession.onSendIntentPlayback(MOCK_MEDIA_ID, anchor, offset);
                consoleLog("Send an intent, action: playback");
                return true;
        }
        return false;
    }

    static {
        System.loadLibrary("org.orbtv.mock204app.native");
    }
}
