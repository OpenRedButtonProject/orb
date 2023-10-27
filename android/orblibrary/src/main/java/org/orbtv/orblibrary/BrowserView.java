/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 * <p>
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

package org.orbtv.orblibrary;

import android.content.Context;
import android.graphics.Color;
import android.util.Log;
import android.view.KeyEvent;
import android.view.View;
import android.webkit.JavascriptInterface;
import android.webkit.PermissionRequest;
import android.webkit.WebChromeClient;
import android.webkit.WebResourceRequest;
import android.webkit.WebResourceResponse;
import android.webkit.WebView;
import android.webkit.WebSettings;
import android.webkit.WebViewClient;

import org.json.JSONObject;
import org.orbtv.orbpolyfill.BridgeToken;

import java.util.Arrays;

class BrowserView extends WebView {
    private static final String TAG = BrowserView.class.getSimpleName();

    private static final int VIEW_HIDDEN_FLAG = 1;
    private static final int PAGE_HIDDEN_FLAG = 2;
    private static final int APP_HIDDEN_FLAG = 4;

    private final Context mContext;
    private int mAppId = -1;
    private int mLoadAppId = -1;
    private int mHiddenMask = 0;
    private WebResourceClient mWebResourceClient;
    private SessionCallback mSessionCallback;

    public BrowserView(Context context, Bridge bridge,
                       OrbSessionFactory.Configuration configuration, DsmccClient dsmccClient) {
        super(context);
        mContext = context;

        setHiddenFlag(PAGE_HIDDEN_FLAG);
        setBackgroundColor(Color.TRANSPARENT);
        getSettings().setJavaScriptEnabled(true);
        getSettings().setDomStorageEnabled(true);
        getSettings().setMediaPlaybackRequiresUserGesture(false);
        getSettings().setUseWideViewPort(true);
        getSettings().setLoadWithOverviewMode(true);
        getSettings().setUserAgentString(configuration.userAgent);
        getSettings().setStandardFontFamily(configuration.sansSerifFontFamily);
        getSettings().setSansSerifFontFamily(configuration.sansSerifFontFamily);
        getSettings().setFixedFontFamily(configuration.fixedFontFamily);
        getSettings().setMixedContentMode(WebSettings.MIXED_CONTENT_ALWAYS_ALLOW);
        addJavascriptInterface(new JavaScriptBridgeInterface(bridge), "androidBridge");

        mWebResourceClient = new WebResourceClient(dsmccClient, new HtmlBuilder(mContext.getAssets()),
                configuration.doNotTrackPreference) {
            @Override
            public void onRequestFailed(WebResourceRequest request, int appId) {
                if (request.isForMainFrame() && appId == mLoadAppId) {
                    mSessionCallback.notifyLoadApplicationFailed(appId);
                    mLoadAppId = -1;
                }
            }

            @Override
            public void onRequestSucceeded(WebResourceRequest request, int appId) {
                if (request.isForMainFrame() && appId == mLoadAppId) {
                    mLoadAppId = -1;
                }
            }
        };

        setWebViewClient(new WebViewClient() {
            @Override
            public WebResourceResponse shouldInterceptRequest(WebView view, WebResourceRequest request) {
                if (request.isForMainFrame()) {
                    mSessionCallback.notifyApplicationPageChanged(mAppId, request.getUrl().toString());
                }
                return mWebResourceClient.shouldInterceptRequest(request, mAppId);
            }

            @Override
            public void onScaleChanged(WebView view, float oldScale, float newScale) {
                BrowserView.this.setInitialScale((int) (BrowserView.this.getHeight() / 720.0 * 100.0));
            }
        });

        setWebChromeClient(new WebChromeClient() {
            @Override
            public void onPermissionRequest(PermissionRequest request) {
                Log.d(TAG, "Received permission request for resources: "
                        + Arrays.toString(request.getResources()));
                String[] resources = request.getResources();
                // Grant access to protected media ID if requested
                if (resources != null) {
                    for (String resource : resources) {
                        if (resource.equals(PermissionRequest.RESOURCE_PROTECTED_MEDIA_ID)) {
                            request.grant(new String[]{resource});
                            return;
                        }
                    }
                }
                request.deny();
            }
        });
    }

    public void setSessionCallback(SessionCallback sessionCallback) {
        mSessionCallback = sessionCallback;
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        if (isKeyAlreadyDispatchedToOverlay(keyCode)) {
            return false;
        }
        return dispatchKeyEvent(event);
    }

    @Override
    public boolean onKeyUp(int keyCode, KeyEvent event) {
        if (isKeyAlreadyDispatchedToOverlay(keyCode)) {
            return false;
        }
        return dispatchKeyEvent(event);
    }

    @Override
    public boolean dispatchKeyEvent(KeyEvent event) {
        int androidKeyCode = event.getKeyCode();
        int keyCode = mSessionCallback.getTvBrowserKeyCode(androidKeyCode);
        if (!mSessionCallback.inApplicationKeySet(mAppId, keyCode)) {
            return false;
        }
        int action = event.getAction();
        if (action == KeyEvent.ACTION_DOWN) {
            if (isKeyMappedByWebView(androidKeyCode)) {
                return super.dispatchKeyEvent(event);
            }
            dispatchJavaScriptKeyEvent("keydown", keyCode);
        } else if (action == KeyEvent.ACTION_UP) {
            dispatchJavaScriptKeyEvent("keypress", keyCode);
            if (isKeyMappedByWebView(androidKeyCode)) {
                return super.dispatchKeyEvent(event);
            }
            dispatchJavaScriptKeyEvent("keyup", keyCode);
        }
        return true;
    }

    @Override
    public void setVisibility(int visibility) {
        if (visibility == View.VISIBLE) {
            unsetHiddenFlag(VIEW_HIDDEN_FLAG);
        } else {
            setHiddenFlag(VIEW_HIDDEN_FLAG);
        }
    }

    public void loadApplication(int appId, String entryUrl) {
        mLoadAppId = appId;
        mContext.getMainExecutor().execute(() -> {
            mAppId = appId;
            loadUrl(entryUrl);
            if (entryUrl.equals("about:blank")) {
                setHiddenFlag(PAGE_HIDDEN_FLAG);
            } else {
                unsetHiddenFlag(PAGE_HIDDEN_FLAG);
            }
        });
    }

    public void showApplication() {
        mContext.getMainExecutor().execute(() -> unsetHiddenFlag(APP_HIDDEN_FLAG));
    }

    public void hideApplication() {
        mContext.getMainExecutor().execute(() -> setHiddenFlag(APP_HIDDEN_FLAG));
    }

    public void dispatchEvent(String type, JSONObject properties) {
        mContext.getMainExecutor().execute(() -> dispatchJavaScriptBridgeEvent(type, properties));
    }

    private void setHiddenFlag(int flag) {
        mHiddenMask |= flag;
        if (mHiddenMask == 0) {
            super.setVisibility(View.VISIBLE);
        } else {
            super.setVisibility(View.INVISIBLE);
        }
    }

    private void unsetHiddenFlag(int flag) {
        mHiddenMask &= ~flag;
        if (mHiddenMask == 0) {
            super.setVisibility(View.VISIBLE);
        } else {
            super.setVisibility(View.INVISIBLE);
        }
    }

    private boolean isKeyMappedByWebView(int keyCode) {
        return keyCode == KeyEvent.KEYCODE_DPAD_UP ||
                keyCode == KeyEvent.KEYCODE_DPAD_DOWN ||
                keyCode == KeyEvent.KEYCODE_DPAD_LEFT ||
                keyCode == KeyEvent.KEYCODE_DPAD_RIGHT ||
                keyCode == KeyEvent.KEYCODE_ENTER ||
                keyCode == KeyEvent.KEYCODE_0 ||
                keyCode == KeyEvent.KEYCODE_1 ||
                keyCode == KeyEvent.KEYCODE_2 ||
                keyCode == KeyEvent.KEYCODE_3 ||
                keyCode == KeyEvent.KEYCODE_4 ||
                keyCode == KeyEvent.KEYCODE_5 ||
                keyCode == KeyEvent.KEYCODE_6 ||
                keyCode == KeyEvent.KEYCODE_7 ||
                keyCode == KeyEvent.KEYCODE_8 ||
                keyCode == KeyEvent.KEYCODE_9;
    }

    private boolean isKeyAlreadyDispatchedToOverlay(int keyCode) {
        return keyCode != KeyEvent.KEYCODE_MEDIA_PLAY &&
                keyCode != KeyEvent.KEYCODE_MEDIA_STOP &&
                keyCode != KeyEvent.KEYCODE_MEDIA_PAUSE &&
                keyCode != KeyEvent.KEYCODE_MEDIA_PLAY_PAUSE &&
                keyCode != KeyEvent.KEYCODE_MEDIA_REWIND &&
                keyCode != KeyEvent.KEYCODE_MEDIA_FAST_FORWARD;
    }

    private void dispatchJavaScriptBridgeEvent(String type, JSONObject properties) {
        Log.i(TAG, "Bridge event: " + type + "(" + properties.toString() + ")");
        evaluateJavascript("document.dispatchBridgeEvent(\"" + type + "\", " +
                properties.toString() + ")", null);
    }

    private void dispatchJavaScriptKeyEvent(String type, int keyCode) {
        evaluateJavascript("document.activeElement.dispatchEvent(new KeyboardEvent('" + type
                + "', " + "{'keyCode': " + keyCode + ", 'bubbles': true}))", null);
    }

    private static class JavaScriptBridgeInterface {
        private final Bridge mBridge;

        JavaScriptBridgeInterface(Bridge bridge) {
            mBridge = bridge;
        }

        @JavascriptInterface
        public String request(String json) {
            try {
                JSONObject object = new JSONObject(json);
                String method = object.getString("method");
                JSONObject params = object.getJSONObject("params");
                BridgeToken token = new BridgeToken(object.getJSONObject("token"));
                synchronized (mBridge) {
                    Log.i(TAG, "Bridge request: " + method + "(" + params.toString() + ")");
                    return mBridge.request(method, token, params).toString();
                }
            } catch (Exception e) {
                e.printStackTrace();
                return "{\"error\": \"Request error\"}";
            }
        }
    }

    public interface SessionCallback {
        /**
         * Get the TvBrowser key code for the Android key code.
         *
         * @param androidKeyCode The Android key code (KeyEvent.KEYCODE_*)
         * @return A TvBrowserTypes.VK_* key code.
         */
        int getTvBrowserKeyCode(int androidKeyCode);

        /**
         * Check whether the key code is in the key set of the application.
         *
         * @param appId The application ID.
         * @param keyCode A TvBrowserTypes.VK_* key code.
         * @return true if it is in the key set, otherwise false
         */
        boolean inApplicationKeySet(int appId, int keyCode);

        /**
         * Notify the application manager that a call to loadApplication failed.
         *
         * @param appId The application ID of the application that failed to load.
         */
        void notifyLoadApplicationFailed(int appId);

        /**
         * Notify the application manager of application page changed, before the new page is
         * loaded. For example, when the user follows a link.
         *
         * @param appId The application ID.
         * @param url The URL of the new page.
         */
        void notifyApplicationPageChanged(int appId, String url);
    }

    static {
        WebView.setWebContentsDebuggingEnabled(true);
    }
}
