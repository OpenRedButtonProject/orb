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

import android.net.Uri;
import android.util.Log;
import android.webkit.CookieManager;
import android.webkit.MimeTypeMap;
import android.webkit.WebResourceRequest;
import android.webkit.WebResourceResponse;

import java.io.ByteArrayInputStream;
import java.io.Closeable;
import java.io.IOException;
import java.io.InputStream;
import java.io.SequenceInputStream;
import java.nio.charset.Charset;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Vector;
import java.util.concurrent.ConcurrentHashMap;
import okhttp3.*;

abstract class WebResourceClient {
    private static final String TAG = WebResourceClient.class.getSimpleName();

    private static final boolean HTTP_COOKIES_ENABLED = true;
    private static final boolean HTTP_REDIRECTION_ENABLED = true;
    private static final List<String> HBBTV_MIME_TYPES = Arrays.asList(
            "text/html",
            "application/xhtml+xml",
            "application/xml",
            "application/vnd.hbbtv.xhtml+xml"
    );
    private static final String ORB_PLAYER_URI = "orb://player";

    private final DsmccClient mDsmccClient;
    private final HtmlBuilder mHtmlBuilder;
    private final boolean mDoNotTrackEnabled;
    private final HtmlUaFetcher mHtmlUaFetcher;
    private final ConcurrentHashMap<String, HtmlUaFetcher.Result> mHtmlUaDocuments =
            new ConcurrentHashMap<>();
    OkHttpClient mHttpClient;
    OkHttpClient mHttpSandboxClient;
    private String mAcceptValue;

    WebResourceClient(DsmccClient dsmccClient, HtmlBuilder htmlBuilder,
                      boolean doNotTrackEnabled, HtmlUaFetcher htmlUaFetcher) {
        mDsmccClient = dsmccClient;
        mHtmlBuilder = htmlBuilder;
        mHtmlUaFetcher = htmlUaFetcher;
        List<Protocol> protocols = new ArrayList<>();
        protocols.add(Protocol.HTTP_2);
        protocols.add(Protocol.HTTP_1_1);
        mHttpClient = new OkHttpClient.Builder()
                .followRedirects(false)
                .followSslRedirects(false)
                .protocols(protocols)
                .connectTimeout(10, java.util.concurrent.TimeUnit.SECONDS)
                .readTimeout(30, java.util.concurrent.TimeUnit.SECONDS)
                .writeTimeout(10, java.util.concurrent.TimeUnit.SECONDS)
                .build();
        mHttpSandboxClient = new OkHttpClient();
        mDoNotTrackEnabled = doNotTrackEnabled;
        ArrayList<String> accept = new ArrayList<>(HBBTV_MIME_TYPES);
        // A wildcard MIME type is necessary for some servers when optional parameters are specified
        accept.add("*/*;q=0.8");
        mAcceptValue = String.join(",", accept);
    }

    static String documentCacheKey(String url) {
        if (url == null) {
            return "";
        }
        Uri uri = Uri.parse(url);
        return uri.buildUpon().fragment(null).build().toString();
    }

    static boolean isHttpOrHttps(String url) {
        if (url == null) {
            return false;
        }
        return url.startsWith("http://") || url.startsWith("https://");
    }

    /**
     * Download an http(s) document with the HTML UA, then run {@code onDone}.
     * Call {@link android.webkit.WebView#loadUrl} from {@code onDone} so intercept
     * can wrap the cached body (do not block shouldInterceptRequest).
     */
    void prefetchHttpDocument(String url, Runnable onDone) {
        Runnable done = onDone != null ? onDone : () -> {};
        if (mHtmlUaFetcher == null || !isHttpOrHttps(url)) {
            done.run();
            return;
        }
        Map<String, String> extra = null;
        if (mDoNotTrackEnabled) {
            extra = new HashMap<>();
            extra.put("DNT", "1");
        }
        mHtmlUaFetcher.fetchAsync(url, mAcceptValue, "include", extra, result -> {
            if (result != null && result.networkOk) {
                mHtmlUaDocuments.put(documentCacheKey(url), result);
                Log.i(TAG, "HTML UA document cached type=" + result.contentType
                        + " bytes=" + result.body.length() + " url=" + url);
            } else {
                Log.w(TAG, "HTML UA document GET failed, intercept will use OkHttp: " + url);
            }
            done.run();
        });
    }

    boolean hasHtmlUaDocument(String url) {
        return mHtmlUaDocuments.containsKey(documentCacheKey(url));
    }

    public WebResourceResponse shouldInterceptRequest(WebResourceRequest request, int appId) {
        //Log.d(TAG, "Should intercept?: " + request.getUrl());
        Uri url = request.getUrl();
        String scheme = url.getScheme();
        String method = request.getMethod();
        boolean isGet = method.equalsIgnoreCase("GET");
        boolean isOptions = method.equalsIgnoreCase("OPTIONS");
        if (!isGet && !isOptions) {
            return null;
        }
        if (url.toString().startsWith(ORB_PLAYER_URI)) {
            if (!isGet) {
                return null;
            }
            return createPlayerPageResponse(request, appId);
        }
        if (scheme.equals("http") || scheme.equals("https")) {
            if (isGet) {
                return shouldInterceptHttpRequest(request, appId);
            }
            return shouldInterceptHttpOptionsRequest(request, appId);
        }
        if (scheme.equals("dvb")) {
            if (!isGet) {
                return null;
            }
            return shouldInterceptDsmccRequest(request, appId);
        }
        return null;
    }

    abstract void onRequestFailed(WebResourceRequest request, int appId);

    abstract void onRequestSucceeded(WebResourceRequest request, int appId);

    private WebResourceResponse shouldInterceptHttpRequest(WebResourceRequest request, int appId) {
        WebResourceResponse response = null;
        try {
            response = handleHttpRequest(request, appId);
        } catch (IOException e) {
            Log.e(TAG, "IOException handling HTTP request: " + request.getUrl(), e);
            e.printStackTrace();
        } catch (Exception e) {
            Log.e(TAG, "Unexpected exception handling HTTP request: " + request.getUrl(), e);
            e.printStackTrace();
        }
        if (response == null) {
            Log.w(TAG, "Calling onRequestFailed for: " + request.getUrl());
            onRequestFailed(request, appId);
        } else {
            onRequestSucceeded(request, appId);
        }
        return response;
    }

    /**
     * CORS preflight and other non-GET HTTP(s) requests must be intercepted when the document uses a
     * non-http(s) scheme (e.g. dvb://): returning null defers to the default loader which often
     * cannot complete cross-scheme fetches, surfacing as XHR status 0 in the page.
     */
    private WebResourceResponse shouldInterceptHttpOptionsRequest(WebResourceRequest request, int appId) {
        WebResourceResponse response = null;
        try {
            response = handleHttpOptionsRequest(request, appId);
        } catch (IOException e) {
            Log.e(TAG, "IOException handling HTTP OPTIONS: " + request.getUrl(), e);
            e.printStackTrace();
        } catch (Exception e) {
            Log.e(TAG, "Unexpected exception handling HTTP OPTIONS: " + request.getUrl(), e);
            e.printStackTrace();
        }
        if (response == null) {
            Log.w(TAG, "Calling onRequestFailed for OPTIONS: " + request.getUrl());
            onRequestFailed(request, appId);
        } else {
            onRequestSucceeded(request, appId);
        }
        return response;
    }

    private static Map<String, String> mutableRequestHeaders(WebResourceRequest request) {
        Map<String, String> out = new HashMap<>();
        Map<String, String> in = request.getRequestHeaders();
        if (in != null) {
            out.putAll(in);
        }
        return out;
    }

    private static String getHeaderIgnoreCase(Map<String, String> headers, String name) {
        for (Map.Entry<String, String> e : headers.entrySet()) {
            if (e.getKey() != null && e.getKey().equalsIgnoreCase(name)) {
                return e.getValue();
            }
        }
        return null;
    }

    private static void removeHeaderIgnoreCase(Map<String, String> headers, String headerName) {
        if (headers == null || headerName == null) {
            return;
        }
        headers.entrySet().removeIf(entry ->
                entry.getKey() != null && entry.getKey().equalsIgnoreCase(headerName));
    }

    /**
     * Echo {@code Origin} on the response like {@link #handleDsmccRequest} does, so fetches from
     * dvb:// (or other non-http) documents to http(s) APIs satisfy the embedder CORS check.
     */
    private static void applyOriginReflectionCors(Map<String, String> responseHeaders,
            Map<String, String> requestHeaders) {
        String origin = getHeaderIgnoreCase(requestHeaders, "Origin");
        if (origin == null || origin.isEmpty()) {
            return;
        }
        /*
         * Upstream often sends Access-Control-Allow-Origin: * (sometimes duplicated → "*,*").
         * If we only put() our reflected origin, some stacks still surface multiple values
         * ("dvb://, *,*") and Chromium rejects CORS. Drop any existing ACAO first.
         */
        removeHeaderIgnoreCase(responseHeaders, "Access-Control-Allow-Origin");
        responseHeaders.put("Access-Control-Allow-Origin", origin);
        String vary = getHeaderIgnoreCase(responseHeaders, "Vary");
        if (vary == null || vary.isEmpty()) {
            responseHeaders.put("Vary", "Origin");
        } else if (!vary.toLowerCase().contains("origin")) {
            responseHeaders.put("Vary", vary + ", Origin");
        }
    }

    private WebResourceResponse handleHttpOptionsRequest(WebResourceRequest request, int appId)
            throws IOException {
        String url = request.getUrl().toString();
        Map<String, String> requestHeaders = mutableRequestHeaders(request);

        CookieManager cookieManager;
        if (HTTP_COOKIES_ENABLED) {
            cookieManager = CookieManager.getInstance();
            String cookie = cookieManager.getCookie(url);
            if (cookie != null) {
                requestHeaders.put("Cookie", cookie);
            }
        }

        Response httpResponse = mHttpClient.newCall(new Request.Builder()
                .url(url)
                .method("OPTIONS", null)
                .headers(Headers.of(requestHeaders))
                .build()).execute();

        Log.d(TAG, "HTTP OPTIONS response code: " + httpResponse.code() + ", for URL: " + url);

        Charset charset = StandardCharsets.UTF_8;
        Map<String, String> responseHeaders = new HashMap<>();
        String optionsRequestOrigin = getHeaderIgnoreCase(requestHeaders, "Origin");
        for (String name : httpResponse.headers().names()) {
            if (name != null && optionsRequestOrigin != null && !optionsRequestOrigin.isEmpty()
                    && name.equalsIgnoreCase("Access-Control-Allow-Origin")) {
                continue;
            }
            responseHeaders.put(name, String.join(",", httpResponse.headers(name)));
        }
        applyOriginReflectionCors(responseHeaders, requestHeaders);

        ResponseBody body = httpResponse.body();
        InputStream stream = (body != null) ? body.byteStream() : new ByteArrayInputStream(new byte[0]);
        String reasonPhrase = httpResponse.message();
        if (reasonPhrase == null || reasonPhrase.trim().isEmpty()) {
            reasonPhrase = httpResponse.isSuccessful() ? "OK" : "Error";
        }
        return new WebResourceResponse(
                "text/plain", charset.name(), httpResponse.code(), reasonPhrase, responseHeaders, stream);
    }

    private WebResourceResponse handleHtmlUaDocument(WebResourceRequest request,
            HtmlUaFetcher.Result ua, int appId) {
        String url = request.getUrl().toString();
        boolean isRedirect = ua.redirected && ua.finalUrl != null && !ua.finalUrl.isEmpty()
                && !documentCacheKey(ua.finalUrl).equals(documentCacheKey(url));
        boolean isError = ua.status != 0 && (ua.status < 200 || ua.status >= 300) && !isRedirect
                && !(ua.status >= 301 && ua.status <= 308);
        if (!ua.networkOk || (isError && request.isForMainFrame())) {
            Log.w(TAG, "HTML UA main-frame error " + ua.status
                    + ", deferring to default loader: " + url);
            return null;
        }

        Charset charset = charsetFromContentType(ua.contentType);
        String mimeType = getMimeType(ua.contentType.isEmpty() ? "text/html" : ua.contentType);
        String[] parts = mimeType.split(";", 2);
        mimeType = parts[0];

        if (isRedirect) {
            Log.i(TAG, "HTML UA redirect " + url + " -> " + ua.finalUrl);
            return new WebResourceResponse("text/html", charset.name(),
                    new ByteArrayInputStream(mHtmlBuilder.getRedirectPage(charset,
                            Uri.parse(ua.finalUrl))));
        }

        byte[] bodyBytes = ua.body.getBytes(charset);
        InputStream bodyStream = new ByteArrayInputStream(bodyBytes);
        InputStream responseStream;
        boolean injectHbbtv = ua.networkOk && HBBTV_MIME_TYPES.contains(mimeType.toLowerCase());
        if (injectHbbtv) {
            responseStream = createInjectionResponseStream(bodyStream, bodyStream, charset,
                    request.getUrl(), appId);
        } else {
            responseStream = createResponseStream(bodyStream, bodyStream);
        }

        Map<String, String> requestHeaders = mutableRequestHeaders(request);
        Map<String, String> responseHeaders = new HashMap<>();
        String requestOriginForCors = getHeaderIgnoreCase(requestHeaders, "Origin");
        for (Map.Entry<String, String> e : ua.headers.entrySet()) {
            String k = e.getKey();
            if (k == null) {
                continue;
            }
            if (k.equalsIgnoreCase("Content-Encoding")
                    || k.equalsIgnoreCase("Content-Length")
                    || k.equalsIgnoreCase("Transfer-Encoding")) {
                continue;
            }
            if (requestOriginForCors != null && !requestOriginForCors.isEmpty()
                    && k.equalsIgnoreCase("Access-Control-Allow-Origin")) {
                continue;
            }
            String header = e.getValue() != null ? e.getValue() : "";
            if (k.equalsIgnoreCase("Content-Security-Policy")) {
                header = updateCspHeader(header);
            }
            responseHeaders.put(k, header);
        }
        applyOriginReflectionCors(responseHeaders, requestHeaders);

        int status = ua.status > 0 ? ua.status : 200;
        String reasonPhrase = (status >= 200 && status < 300) ? "OK" : "Error";
        return new WebResourceResponse(mimeType, charset.name(), status, reasonPhrase,
                responseHeaders, responseStream);
    }

    private static Charset charsetFromContentType(String contentType) {
        if (contentType != null) {
            String[] parts = contentType.split(";");
            for (String part : parts) {
                String p = part.trim();
                if (p.toLowerCase().startsWith("charset=")) {
                    String name = p.substring(8).trim().replace("\"", "");
                    try {
                        return Charset.forName(name);
                    } catch (Exception ignored) {
                    }
                }
            }
        }
        return StandardCharsets.UTF_8;
    }

    private WebResourceResponse handleHttpRequest(WebResourceRequest request, int appId)
            throws IOException {
        String url = request.getUrl().toString();
        HtmlUaFetcher.Result uaDocument = mHtmlUaDocuments.remove(documentCacheKey(url));
        if (uaDocument != null) {
            Log.i(TAG, "HTTP main-frame from HTML UA status=" + uaDocument.status
                    + " type=" + uaDocument.contentType + " " + url);
            return handleHtmlUaDocument(request, uaDocument, appId);
        }

        // Request
        Map<String, String> requestHeaders = mutableRequestHeaders(request);

        CookieManager cookieManager;
        if (HTTP_COOKIES_ENABLED) {
            cookieManager = CookieManager.getInstance();
            String cookie = cookieManager.getCookie(url);
            if (cookie != null) {
                requestHeaders.put("Cookie", cookie);
            }
        }

        if (mDoNotTrackEnabled) {
            requestHeaders.put("DNT", "1");
        }

        Response httpResponse = mHttpClient.newCall(new Request.Builder()
                .url(url)
                .method(request.getMethod(), null)
                .headers(Headers.of(requestHeaders))
                .build()).execute();

        Log.d(TAG, "HTTP response code: " + httpResponse.code() + ", for URL: " + url);
        boolean isRedirect = (httpResponse.code() >= 301 && httpResponse.code() <= 308);
        boolean isError = !httpResponse.isSuccessful() && !isRedirect;

        /*
         * Returning null hands the request back to WebView's default loader. That works for
         * http(s) top-level documents, but subresource loads from non-http(s) documents (e.g.
         * dvb://) then fail with XHR status 0 and no response headers. Forward OkHttp's status and
         * body for subresource errors instead of null.
         */
        if (isError && request.isForMainFrame()) {
            Log.w(TAG, "HTTP main-frame error " + httpResponse.code() + ", deferring to default loader: " + url);
            return null;
        }
        if (isError) {
            Log.w(TAG, "HTTP subresource error " + httpResponse.code() + ", forwarding response to WebView: " + url);
        }

        Charset charset = StandardCharsets.UTF_8;
        String mimeType = getMimeType(httpResponse.header("Content-Type", "text/plain"));

        // Strip optional parameters for the comparison
        String[] parts = mimeType.split(";", 2);
        mimeType = parts[0];

        Map<String, List<String>> httpResponseHeaders = httpResponse.headers().toMultimap();

        if (HTTP_COOKIES_ENABLED) {
            List<String> setCookies = httpResponseHeaders.get("Set-Cookie");
            if (setCookies != null) {
                for (String setCookie : setCookies) {
                    cookieManager.setCookie(url, setCookie);
                }
            }
        }

        if (isRedirect) {
            if (HTTP_REDIRECTION_ENABLED) {
                String location = httpResponse.header("Location");
                if (location != null) {
                    return new WebResourceResponse("text/html", charset.name(),
                            new ByteArrayInputStream(mHtmlBuilder.getRedirectPage(charset, Uri.parse(location))));
                }
            }
        }

        ResponseBody body = httpResponse.body();
        InputStream responseStream;
        if (body == null) {
            Log.w(TAG, "HTTP response body is null for: " + url);
            responseStream = new ByteArrayInputStream(new byte[0]);
        } else {
            long contentLength = body.contentLength();
            Log.d(TAG, "Response body size for " + url + ": " + contentLength + " bytes, MIME type: " + mimeType);
            boolean injectHbbtv = httpResponse.isSuccessful()
                    && HBBTV_MIME_TYPES.contains(mimeType.toLowerCase());
            if (injectHbbtv) {
                //Log.d(TAG, "Creating injection response stream for HBBTV MIME type: " + url);
                responseStream = createInjectionResponseStream(body.byteStream(), body, charset, request.getUrl(), appId);
            } else {
                responseStream = createResponseStream(body.byteStream(), body);
            }
        }

        Map<String, String> responseHeaders = new HashMap<>();
        String requestOriginForCors = getHeaderIgnoreCase(requestHeaders, "Origin");

        httpResponseHeaders.forEach((k, v) -> {
            if (k == null) {
                return;
            }
            if (requestOriginForCors != null && !requestOriginForCors.isEmpty()
                    && k.equalsIgnoreCase("Access-Control-Allow-Origin")) {
                return;
            }
            String header = String.join(",", v);
            if (k.equalsIgnoreCase("Content-Security-Policy")) {
                header = updateCspHeader(header);
            }
            responseHeaders.put(k, header);
        });

        applyOriginReflectionCors(responseHeaders, requestHeaders);

        String reasonPhrase = httpResponse.message();
        if (reasonPhrase == null || reasonPhrase.trim().isEmpty()) {
            reasonPhrase = httpResponse.isSuccessful() ? "OK" : "Error";
        }
        return new WebResourceResponse(mimeType, charset.name(), httpResponse.code(), reasonPhrase,
                responseHeaders, responseStream);
    }

    private WebResourceResponse shouldInterceptDsmccRequest(WebResourceRequest request, int appId) {
        WebResourceResponse response = null;
        response = handleDsmccRequest(request, appId);
        if (response.getStatusCode() == 404) {
            onRequestFailed(request, appId);
        } else {
            onRequestSucceeded(request, appId);
        }
        return response;
    }

    private WebResourceResponse handleDsmccRequest(WebResourceRequest request, int appId) {
        String url = request.getUrl().toString();
        Charset charset = StandardCharsets.UTF_8;
        String mimeType = getMimeTypeFromUrl(url);
        DvbInputStream body = mDsmccClient.requestContent(url);

        InputStream responseStream;
        if (request.isForMainFrame() && HBBTV_MIME_TYPES.contains(mimeType.toLowerCase())) {
            responseStream = createInjectionResponseStream(body, body, charset, request.getUrl(), appId);
        } else {
            responseStream = createResponseStream(body, body);
        }
        WebResourceResponse response = new WebResourceResponse(mimeType, charset.name(), responseStream);
        String origin = request.getRequestHeaders().get("Origin");
        if (origin != null) {
            Map<String, String> headerMap = new HashMap<>();
            headerMap.put("Access-Control-Allow-Origin", origin);
            headerMap.put("Content-Length", String.valueOf(body.getDataLength()));
            response.setResponseHeaders(headerMap);
        }
        response.setStatusCodeAndReasonPhrase(body.getStatusCode(), body.reasonPhrase());

        return response;
    }

    private static String getMimeType(String contentType) {
        String mimeType = "text/plain";
        String[] parts = contentType.split(";");
        if (parts.length > 0) {
            mimeType = parts[0];
        }
        if (mimeType.equalsIgnoreCase("application/vnd.hbbtv.xhtml+xml")) {
            mimeType = "application/xhtml+xml";
        }
        return mimeType;
    }

    private String getMimeTypeFromUrl(String url) {
        String type = "*/*";
        String extension = MimeTypeMap.getFileExtensionFromUrl(url);
        if (extension != null && !extension.equals("")) {
            String fromExtension = MimeTypeMap.getSingleton().getMimeTypeFromExtension(extension);
            if (fromExtension != null) {
                type = fromExtension;
            } else if (extension.equals("html5")) {
                type = "text/html";
            } else if (extension.equals("cehtml")) {
                type = "application/xhtml+xml";
            }
        }
        return type;
    }

    private InputStream createResponseStream(InputStream body, Closeable closeable) {
        Vector<InputStream> wrapper = new Vector<>();
        wrapper.add(body);
        return new SequenceInputStream(wrapper.elements()) {
            @Override
            public void close() {
                try {
                    closeable.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        };
    }

    private InputStream createInjectionResponseStream(InputStream body, Closeable closeable,
                                                      Charset charset, Uri uri, int appId) {
        Vector<InputStream> payload = new Vector<>();
        int payloadLength = 0;

        byte[] tokenInjection = mHtmlBuilder.getTokenInjection(charset, uri, appId);
        payload.add(new ByteArrayInputStream(tokenInjection));
        payloadLength += tokenInjection.length;

        byte[] hbbtvInjection = mHtmlBuilder.getHbbtvInjection(charset);
        payload.add(new ByteArrayInputStream(hbbtvInjection));
        payloadLength += hbbtvInjection.length;

        return new InjectionInputStream(body, charset, payload, payloadLength) {
            @Override
            void onClose() {
                try {
                    closeable.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        };
    }

    WebResourceResponse createPlayerPageResponse(WebResourceRequest request, int appId) {
        Charset charset = StandardCharsets.UTF_8;
        ByteArrayInputStream data = new ByteArrayInputStream(mHtmlBuilder.getPlayerPage(charset));
        Vector<InputStream> payload = new Vector<>();
        int payloadLength = 0;

        byte[] tokenInjection = mHtmlBuilder.getTokenInjection(charset, request.getUrl(), appId);
        payload.add(new ByteArrayInputStream(tokenInjection));
        payloadLength += tokenInjection.length;

        byte[] hbbtvInjection = mHtmlBuilder.getMediaManagerInjection(charset);
        payload.add(new ByteArrayInputStream(hbbtvInjection));
        payloadLength += hbbtvInjection.length;

        InputStream inputStream = new InjectionInputStream(data, charset, payload, payloadLength) {
            @Override
            void onClose() {
                try {
                    data.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        };
        WebResourceResponse response = new WebResourceResponse("text/html",
                charset.toString(), inputStream);
        return response;
    }

    private static String updateCspHeader(String header) {
        if (header == null || header.trim().isEmpty()) {
            return header;
        }
        String[] directives = header.split(";\\s*");
        boolean directiveFound = false;
        StringBuilder updatedHeader = new StringBuilder();
        for (String directive : directives) {
            if (directive.startsWith("frame-src ") || directive.startsWith("default-src ")) {
                if (!directive.contains(" orb:")) {
                    directive += " orb:";
                }
                directiveFound = true;
            }
            updatedHeader.append(directive).append("; ");
        }
        if (!directiveFound) {
            updatedHeader.append("frame-src orb:; ");
        }
        return updatedHeader.toString().trim();
    }
}

