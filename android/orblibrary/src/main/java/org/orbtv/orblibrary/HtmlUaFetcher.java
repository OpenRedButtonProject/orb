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

import java.util.ArrayDeque;
import java.util.Collections;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.atomic.AtomicReference;

/**
 * GET via the HTML user agent (Trichrome / Chromium), including HTTP/3.
 * A hidden WebView is used so the request is not OkHttp. One fetch at a time;
 * callers must not block {@code shouldInterceptRequest} waiting for this
 * (that thread is needed for the hidden WebView's network).
 */
class HtmlUaFetcher {
    private static final String TAG = HtmlUaFetcher.class.getSimpleName();
    private static final String JS_BRIDGE = "HtmlUaBridge";
    private static final long TIMEOUT_MS = 20_000;

    static final String ACCEPT_XML_AIT =
            "application/vnd.dvb.ait+xml,application/xml,*/*";

    static final class Result {
        final boolean networkOk;
        final int status;
        final String contentType;
        final String body;
        final String finalUrl;
        final boolean redirected;
        final Map<String, String> headers;

        Result(boolean networkOk, int status, String contentType, String body,
                String finalUrl, boolean redirected, Map<String, String> headers) {
            this.networkOk = networkOk;
            this.status = status;
            this.contentType = contentType != null ? contentType : "";
            this.body = body != null ? body : "";
            this.finalUrl = finalUrl != null ? finalUrl : "";
            this.redirected = redirected;
            this.headers = headers != null ? headers : Collections.emptyMap();
        }
    }

    private interface Pending {
        void onResult(boolean ok, int status, String contentType, String body,
                String finalUrl, boolean redirected, String headersJson);
    }

    private static final class Job {
        final String url;
        final String accept;
        final String credentials;
        final Map<String, String> extraHeaders;
        final java.util.function.Consumer<Result> callback;
        int generation;

        Job(String url, String accept, String credentials, Map<String, String> extraHeaders,
                java.util.function.Consumer<Result> callback) {
            this.url = url;
            this.accept = accept;
            this.credentials = credentials;
            this.extraHeaders = extraHeaders;
            this.callback = callback;
        }
    }

    private final Context mContext;
    private final String mUserAgent;
    private final Handler mMainHandler = new Handler(Looper.getMainLooper());
    private final AtomicInteger mGeneration = new AtomicInteger();
    private final ArrayDeque<Job> mQueue = new ArrayDeque<>();
    private ViewGroup mHost;
    private WebView mWebView;
    private volatile Pending mPending;
    private Job mRunning;

    HtmlUaFetcher(Context context, String userAgent) {
        mContext = context;
        mUserAgent = userAgent;
    }

    void attachHost(ViewGroup host) {
        mHost = host;
        mMainHandler.post(this::attachToHost);
    }

    void fetchAsync(String url, String accept, String credentials,
            java.util.function.Consumer<Result> callback) {
        fetchAsync(url, accept, credentials, null, callback);
    }

    void fetchAsync(String url, String accept, String credentials,
            Map<String, String> extraHeaders, java.util.function.Consumer<Result> callback) {
        if (url == null || url.isEmpty() || callback == null) {
            if (callback != null) {
                callback.accept(null);
            }
            return;
        }
        Job job = new Job(url, accept, credentials, extraHeaders, callback);
        mMainHandler.post(() -> {
            mQueue.add(job);
            pump();
        });
    }

    /**
     * Blocking GET. Must not run on the main thread, and must not run on a
     * WebView {@code shouldInterceptRequest} thread (deadlocks the hidden UA).
     *
     * @return null if the WebView fetch could not complete
     */
    Result fetch(String url, String accept, String credentials) {
        if (Looper.myLooper() == Looper.getMainLooper()) {
            Log.w(TAG, "HTML UA fetch skipped on main thread: " + url);
            return null;
        }
        final CountDownLatch latch = new CountDownLatch(1);
        final AtomicReference<Result> result = new AtomicReference<>();
        fetchAsync(url, accept, credentials, r -> {
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
        mMainHandler.post(() -> {
            mQueue.clear();
            mRunning = null;
            resetWebView();
        });
    }

    private void pump() {
        if (mRunning != null) {
            return;
        }
        Job job = mQueue.poll();
        if (job == null) {
            return;
        }
        mRunning = job;
        job.generation = mGeneration.incrementAndGet();
        startLoad(job);
        final int generation = job.generation;
        mMainHandler.postDelayed(() -> {
            if (generation != mGeneration.get() || mRunning != job) {
                return;
            }
            Log.w(TAG, "HTML UA WebView fetch timed out: " + job.url);
            mGeneration.incrementAndGet();
            mPending = null;
            mRunning = null;
            resetWebView();
            job.callback.accept(null);
            pump();
        }, TIMEOUT_MS);
    }

    private void startLoad(Job job) {
        try {
            ensureWebView();
            mPending = (ok, status, contentType, body, finalUrl, redirected, headersJson) -> {
                if (job.generation != mGeneration.get() || mRunning != job) {
                    return;
                }
                mGeneration.incrementAndGet();
                mPending = null;
                mRunning = null;
                try {
                    job.callback.accept(new Result(ok, status, contentType, body, finalUrl,
                            redirected, parseHeaders(headersJson)));
                } finally {
                    pump();
                }
            };
            Log.i(TAG, "HTML UA GET " + job.url);
            mWebView.loadDataWithBaseURL(job.url, bootstrapHtml(job), "text/html", "UTF-8", null);
        } catch (Exception e) {
            Log.e(TAG, "HTML UA WebView fetch failed to start: " + job.url, e);
            mPending = null;
            mRunning = null;
            job.callback.accept(null);
            pump();
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
            public WebResourceResponse shouldInterceptRequest(WebView view,
                    WebResourceRequest request) {
                // Pass every request to Trichrome so https: can use HTTP/3.
                return null;
            }

            @Override
            public void onPageFinished(WebView view, String loadedUrl) {
                Log.i(TAG, "HTML UA fetcher page finished: " + loadedUrl);
            }
        });
        webView.setWebChromeClient(new WebChromeClient() {
            @Override
            public boolean onConsoleMessage(ConsoleMessage consoleMessage) {
                Log.i(TAG, "HTML UA fetcher console: " + consoleMessage.message());
                return true;
            }
        });
        WebSettings settings = webView.getSettings();
        settings.setJavaScriptEnabled(true);
        settings.setDomStorageEnabled(false);
        settings.setBlockNetworkLoads(false);
        settings.setMixedContentMode(WebSettings.MIXED_CONTENT_ALWAYS_ALLOW);
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

    private static String bootstrapHtml(Job job) {
        JSONObject hdrs = new JSONObject();
        try {
            hdrs.put("Accept", job.accept != null ? job.accept : "*/*");
            if (job.extraHeaders != null) {
                for (Map.Entry<String, String> e : job.extraHeaders.entrySet()) {
                    if (e.getKey() != null && e.getValue() != null) {
                        hdrs.put(e.getKey(), e.getValue());
                    }
                }
            }
        } catch (Exception ignored) {
        }
        String credentials = "include".equals(job.credentials) ? "include" : "omit";
        String quotedUrl = JSONObject.quote(job.url);
        String quotedCred = JSONObject.quote(credentials);
        int generation = job.generation;
        return "<!DOCTYPE html><html><head><meta charset=\"utf-8\"></head><body><script>"
                + "(function(){"
                + "function fail(e){try{" + JS_BRIDGE
                + ".onResult(" + generation + ",false,0,'',String(e||''),'',false,'{}');}catch(x){}}"
                + "try{"
                + "fetch(" + quotedUrl + ",{method:'GET',credentials:" + quotedCred
                + ",cache:'no-store',headers:" + hdrs.toString() + "})"
                + ".then(function(r){var ct=r.headers.get('Content-Type')||'';"
                + "var hd={};try{r.headers.forEach(function(v,k){hd[k]=v;});}catch(h){}"
                + "return r.text().then(function(t){" + JS_BRIDGE
                + ".onResult(" + generation + ",!!r.ok,r.status,ct,t,r.url||'',!!r.redirected,"
                + "JSON.stringify(hd));});})"
                + ".catch(fail);"
                + "}catch(e){fail(e);}"
                + "})();"
                + "</script></body></html>";
    }

    private static Map<String, String> parseHeaders(String headersJson) {
        if (headersJson == null || headersJson.isEmpty()) {
            return Collections.emptyMap();
        }
        try {
            JSONObject obj = new JSONObject(headersJson);
            Map<String, String> out = new HashMap<>();
            Iterator<String> keys = obj.keys();
            while (keys.hasNext()) {
                String k = keys.next();
                out.put(k, obj.optString(k, ""));
            }
            return out;
        } catch (Exception e) {
            return Collections.emptyMap();
        }
    }

    private class Bridge {
        @JavascriptInterface
        public void onResult(int generation, boolean ok, int status, String contentType,
                String body, String finalUrl, boolean redirected, String headersJson) {
            Log.i(TAG, "HTML UA result ok=" + ok + " status=" + status + " type=" + contentType
                    + " bytes=" + (body != null ? body.length() : 0)
                    + " redirected=" + redirected);
            mMainHandler.post(() -> {
                if (generation != mGeneration.get()) {
                    return;
                }
                Pending pending = mPending;
                if (pending != null) {
                    pending.onResult(ok, status, contentType, body, finalUrl, redirected,
                            headersJson);
                }
            });
        }
    }
}
