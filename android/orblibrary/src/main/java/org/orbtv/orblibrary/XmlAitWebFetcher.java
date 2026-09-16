/**
 * ORB Software. Copyright (c) 2026 Ocean Blue Software Limited
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
import android.os.Handler;
import android.os.Looper;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup;
import android.webkit.ConsoleMessage;
import android.webkit.JavascriptInterface;
import android.webkit.WebChromeClient;
import android.webkit.WebResourceRequest;
import android.webkit.WebResourceResponse;
import android.webkit.WebSettings;
import android.webkit.WebView;
import android.webkit.WebViewClient;

import org.json.JSONObject;

import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.atomic.AtomicReference;

/**
 * Fetches an XML AIT with the HTML user agent (Trichrome / Chromium), including HTTP/3.
 * A hidden WebView is used so the GET is not OkHttp and does not deadlock the app
 * WebView's JavaScript bridge thread.
 */
class XmlAitWebFetcher {
    private static final String TAG = XmlAitWebFetcher.class.getSimpleName();
    private static final String JS_BRIDGE = "XmlAitBridge";
    private static final long TIMEOUT_MS = 20_000;

    static final class Result {
        final boolean networkOk;
        final String contentType;
        final String body;

        Result(boolean networkOk, String contentType, String body) {
            this.networkOk = networkOk;
            this.contentType = contentType != null ? contentType : "";
            this.body = body != null ? body : "";
        }
    }

    private interface Pending {
        void onResult(boolean ok, String contentType, String body);
    }

    private final Context mContext;
    private final String mUserAgent;
    private final Handler mMainHandler = new Handler(Looper.getMainLooper());
    private final AtomicInteger mGeneration = new AtomicInteger();
    private ViewGroup mHost;
    private WebView mWebView;
    private volatile Pending mPending;

    XmlAitWebFetcher(Context context, String userAgent) {
        mContext = context;
        mUserAgent = userAgent;
    }

    void attachHost(ViewGroup host) {
        mHost = host;
    }

    void fetchAsync(String url, java.util.function.Consumer<Result> callback) {
        if (url == null || url.isEmpty() || callback == null) {
            if (callback != null) {
                callback.accept(null);
            }
            return;
        }
        final int generation = mGeneration.incrementAndGet();
        mMainHandler.post(() -> startLoad(url, generation, (ok, contentType, body) -> {
            callback.accept(new Result(ok, contentType, body));
        }));
        mMainHandler.postDelayed(() -> {
            if (generation != mGeneration.get()) {
                return;
            }
            Log.w(TAG, "XML AIT WebView fetch timed out: " + url);
            mGeneration.incrementAndGet();
            mPending = null;
            resetWebView();
            callback.accept(null);
        }, TIMEOUT_MS);
    }

    /**
     * Blocking GET. Must not run on the main thread (WebView callbacks need it).
     *
     * @return null if the WebView fetch could not complete
     */
    Result fetch(String url) {
        if (Looper.myLooper() == Looper.getMainLooper()) {
            Log.w(TAG, "XML AIT WebView fetch skipped on main thread: " + url);
            return null;
        }
        final CountDownLatch latch = new CountDownLatch(1);
        final AtomicReference<Result> result = new AtomicReference<>();
        fetchAsync(url, r -> {
            result.set(r);
            latch.countDown();
        });
        try {
            if (!latch.await(TIMEOUT_MS + 1000, TimeUnit.MILLISECONDS)) {
                return null;
            }
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
            return null;
        }
        return result.get();
    }

    void close() {
        mGeneration.incrementAndGet();
        mMainHandler.post(this::resetWebView);
    }

    private void startLoad(String url, int generation, Pending pending) {
        try {
            ensureWebView();
            mPending = (ok, contentType, body) -> {
                if (generation != mGeneration.get()) {
                    return;
                }
                mGeneration.incrementAndGet();
                mPending = null;
                pending.onResult(ok, contentType, body);
            };
            Log.i(TAG, "XML AIT HTML UA GET " + url);
            mWebView.loadDataWithBaseURL(url, bootstrapHtml(url), "text/html", "UTF-8", null);
        } catch (Exception e) {
            Log.e(TAG, "XML AIT WebView fetch failed to start: " + url, e);
            mPending = null;
            pending.onResult(false, "", "");
        }
    }

    private void ensureWebView() {
        if (mWebView != null) {
            attachToHost();
            return;
        }
        WebView webView = new WebView(mContext);
        webView.setLayerType(View.LAYER_TYPE_SOFTWARE, null);
        webView.setVisibility(View.INVISIBLE);
        webView.setWebViewClient(new WebViewClient() {
            @Override
            public WebResourceResponse shouldInterceptRequest(WebView view, WebResourceRequest request) {
                // Pass every request to Trichrome so https: can use HTTP/3.
                return null;
            }

            @Override
            public void onPageFinished(WebView view, String loadedUrl) {
                Log.i(TAG, "XML AIT fetcher page finished: " + loadedUrl);
            }
        });
        webView.setWebChromeClient(new WebChromeClient() {
            @Override
            public boolean onConsoleMessage(ConsoleMessage consoleMessage) {
                Log.i(TAG, "XML AIT fetcher console: " + consoleMessage.message());
                return true;
            }
        });
        WebSettings settings = webView.getSettings();
        settings.setJavaScriptEnabled(true);
        settings.setDomStorageEnabled(false);
        settings.setBlockNetworkLoads(false);
        if (mUserAgent != null && !mUserAgent.isEmpty()) {
            settings.setUserAgentString(mUserAgent);
        }
        webView.addJavascriptInterface(new Bridge(), JS_BRIDGE);
        mWebView = webView;
        attachToHost();
        webView.onResume();
    }

    private void attachToHost() {
        if (mWebView == null || mHost == null || mWebView.getParent() != null) {
            return;
        }
        // Unattached WebViews do not run script or network; keep 1px in the session overlay.
        mHost.addView(mWebView, new ViewGroup.LayoutParams(1, 1));
    }

    private void resetWebView() {
        mPending = null;
        if (mWebView == null) {
            return;
        }
        mWebView.stopLoading();
        if (mWebView.getParent() instanceof ViewGroup) {
            ((ViewGroup) mWebView.getParent()).removeView(mWebView);
        }
        mWebView.destroy();
        mWebView = null;
    }

    private static String bootstrapHtml(String url) {
        String quoted = JSONObject.quote(url);
        return "<!DOCTYPE html><html><head><meta charset=\"utf-8\"></head><body><script>"
                + "(function(){"
                + "function fail(e){try{" + JS_BRIDGE + ".onResult(false,'',String(e||''));}catch(x){}}"
                + "try{"
                + "fetch(" + quoted + ",{method:'GET',credentials:'omit',cache:'no-store',"
                + "headers:{'Accept':'application/vnd.dvb.ait+xml,application/xml,*/*'}})"
                + ".then(function(r){var ct=r.headers.get('Content-Type')||'';"
                + "return r.text().then(function(t){" + JS_BRIDGE + ".onResult(!!r.ok,ct,t);});})"
                + ".catch(fail);"
                + "}catch(e){fail(e);}"
                + "})();"
                + "</script></body></html>";
    }

    private class Bridge {
        @JavascriptInterface
        public void onResult(boolean ok, String contentType, String body) {
            Log.i(TAG, "XML AIT HTML UA result ok=" + ok + " type=" + contentType
                    + " bytes=" + (body != null ? body.length() : 0));
            Pending pending = mPending;
            if (pending != null) {
                pending.onResult(ok, contentType, body);
            }
        }
    }
}
