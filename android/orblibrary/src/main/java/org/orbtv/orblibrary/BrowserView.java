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
    private JavaScriptBridgeInterface mJavaScriptBridgeInterface;

    private int mViewWidth = 0; // Await onLayoutChange to calculate View width
    private int mAppWidth = 1280; // Apps are 1280 by default

    public BrowserView(Context context, Bridge bridge,
                       OrbSessionFactory.Configuration configuration, DsmccClient dsmccClient) {
        super(context);
        mContext = context;
        mJavaScriptBridgeInterface = new JavaScriptBridgeInterface(bridge, new OrbcBridge(context, this));

        setHiddenFlag(PAGE_HIDDEN_FLAG);
        setBackgroundColor(Color.TRANSPARENT);
        getSettings().setJavaScriptEnabled(true);
        getSettings().setDomStorageEnabled(true);
        getSettings().setMediaPlaybackRequiresUserGesture(false);
        getSettings().setUserAgentString(configuration.userAgent);
        getSettings().setStandardFontFamily(configuration.sansSerifFontFamily);
        getSettings().setSansSerifFontFamily(configuration.sansSerifFontFamily);
        getSettings().setFixedFontFamily(configuration.fixedFontFamily);
        getSettings().setMixedContentMode(WebSettings.MIXED_CONTENT_ALWAYS_ALLOW);
        addJavascriptInterface(mJavaScriptBridgeInterface, "androidBridge");

        /**
         * Disable support for the 'viewport' HTML meta tag to ensure that the layout width is
         * always equal to the WebView View's width. The initial scale is determined based on this
         * width, to scale the 1280x720 app to fit the WebView.
         */
        getSettings().setUseWideViewPort(false);
        addOnLayoutChangeListener(new OnLayoutChangeListener() {
            @Override
            public void onLayoutChange(View v, int left, int top, int right, int bottom,
                    int oldLeft, int oldTop, int oldRight, int oldBottom) {
                int width = right - left;
                if (width != mViewWidth) {
                    mViewWidth = width;
                    updateScale();
                }
            }
        });

        mWebResourceClient = new WebResourceClient(dsmccClient, new HtmlBuilder(mContext.getAssets()),
                configuration.doNotTrackEnabled) {
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
            Log.d(TAG, "Key press event {" + androidKeyCode + "} is not supported in app");
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

    public void loadApplication(int appId, String entryUrl, int[] graphicsConfigIds) {
        mLoadAppId = appId;
        mContext.getMainExecutor().execute(() -> {
            mAppId = appId;
            loadUrl(entryUrl);
            updateAppResolution(graphicsConfigIds);
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

    public void dispatchTextInput(String text) {
        dispatchJavaScriptTextInput(text);
    }

    public void close() {
        synchronized (mJavaScriptBridgeInterface.mBridge) {
            mJavaScriptBridgeInterface.mQuitting = true;
        }
        removeJavascriptInterface("androidBridge");
        getSettings().setJavaScriptEnabled(false);
        destroy();
    }

    private void updateScale() {
        mContext.getMainExecutor().execute(() -> {
            int scale = 100;
            if (mViewWidth != 0) {
                scale = (mViewWidth * 100) / mAppWidth;
            }
            Log.d(TAG, "Set scale to " + scale);
            setInitialScale(scale);
        });
    }

    private void updateAppResolution(int[] graphicsConfigIds) {
        int resolution = 720;
        if (graphicsConfigIds != null && graphicsConfigIds.length != 0) {
            Arrays.sort(graphicsConfigIds);
            resolution = graphicsConfigIds[graphicsConfigIds.length -1];
        }
        Log.d(TAG, "Rendering resolution is set to " + resolution + "p");
        if (resolution <= 720) {
            mAppWidth = 1280;
        } else if (resolution <= 1080) {
            mAppWidth = 1920;
        } else {
            mAppWidth = 3840;
        }
        updateScale();
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
                keyCode != KeyEvent.KEYCODE_MEDIA_FAST_FORWARD &&
                keyCode != KeyEvent.KEYCODE_MEDIA_RECORD;
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

    private void dispatchJavaScriptTextInput(String text) {
        String escapedText = JSONObject.quote(text);
        evaluateJavascript("const tagName = document.activeElement.tagName.toLowerCase();" +
                "if (tagName === 'input' || tagName === 'textarea')" +
                "    document.activeElement.value = " + escapedText + ";" +
                "document.activeElement.dispatchEvent(new Event('input'));" +
                "document.activeElement.dispatchEvent(new Event('change'));", null);
    }

    private static class JavaScriptBridgeInterface {
        public final Bridge mBridge;
        public final OrbcBridge mOrbcBridge;
        public boolean mQuitting = false;

        JavaScriptBridgeInterface(Bridge bridge, OrbcBridge obridge) {
            mBridge = bridge;
            mOrbcBridge = obridge;
        }

        @JavascriptInterface
        public String request(String json) {

            // Call new moderator library to handle application and network requests
            String result = mOrbcBridge.executeRequest(json);
            Log.i(TAG, "New interface would return: " + result);

            try {
                JSONObject object = new JSONObject(json);
                String method = object.getString("method");
                JSONObject params = object.getJSONObject("params");
                BridgeToken token = new BridgeToken(object.getJSONObject("token"));
                synchronized (mBridge) {
                    if (mQuitting) {
                        throw new Exception("Quitting");
                    }
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
         * @param appId   The application ID.
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
         * @param url   The URL of the new page.
         */
        void notifyApplicationPageChanged(int appId, String url);
    }

    static {
        WebView.setWebContentsDebuggingEnabled(true);
    }
}
