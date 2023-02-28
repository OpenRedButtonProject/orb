/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 * <p>
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
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
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Vector;

// For building in source. Replaced with com.squareup... by build.gradle during Gradle build:
import com.android.okhttp.*;

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

    private final HtmlBuilder mHtmlBuilder;
    OkHttpClient mHttpClient;
    OkHttpClient mHttpSandboxClient;

    WebResourceClient(HtmlBuilder htmlBuilder) {
        mHtmlBuilder = htmlBuilder;
        mHttpClient = new OkHttpClient();
        mHttpSandboxClient = new OkHttpClient();
    }

    public WebResourceResponse shouldInterceptRequest(WebResourceRequest request, int appId) {
        Log.d(TAG, "Should intercept?: " + request.getUrl());
        Uri url = request.getUrl();
        String scheme = url.getScheme();
        if (request.getMethod().equalsIgnoreCase("GET")) {
            if (url.toString().startsWith(ORB_PLAYER_URI)) {
                return createPlayerPageResponse(request, appId);
            } else if (scheme.equals("http") || scheme.equals("https")) {
                return shouldInterceptHttpRequest(request, appId);
            } else if (scheme.equals("dvb")) {
                return shouldInterceptDsmccRequest(request, appId);
            }
        }
        return null;
    }

    abstract void onRequestFailed(WebResourceRequest request, int appId);

    abstract void onRequestSucceeded(WebResourceRequest request, int appId);

    private WebResourceResponse shouldInterceptHttpRequest(WebResourceRequest request, int appId) {
        WebResourceResponse response = null;
        if (request.isForMainFrame()) {
            try {
                response = handleHttpRequest(request, appId);
            } catch (IOException e) {
                e.printStackTrace();
            }
            if (response == null) {
                onRequestFailed(request, appId);
            } else {
                onRequestSucceeded(request, appId);
            }
        }
        return response;
    }

    private WebResourceResponse handleHttpRequest(WebResourceRequest request, int appId)
            throws IOException {
        // Request
        String url = request.getUrl().toString();
        Map<String, String> requestHeaders = request.getRequestHeaders();
        requestHeaders.put("Accept", String.join(",", HBBTV_MIME_TYPES));

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
                .method(request.getMethod(), null)
                .headers(Headers.of(requestHeaders))
                .build()).execute();

        // Response
        if (!httpResponse.isSuccessful()) {
            return null;
        }
        Charset charset = StandardCharsets.UTF_8;
        String mimeType = getMimeType(httpResponse.header("Content-Type", "text/plain"));
        Map<String, List<String>> httpResponseHeaders = httpResponse.headers().toMultimap();

        if (HTTP_COOKIES_ENABLED) {
            List<String> setCookies = httpResponseHeaders.get("Set-Cookie");
            if (setCookies != null) {
                for (String setCookie : setCookies) {
                    cookieManager.setCookie(url, setCookie);
                }
            }
        }

        if (HTTP_REDIRECTION_ENABLED) {
            String location = httpResponse.header("Location");
            if (location != null) {
                return new WebResourceResponse("text/html", charset.name(),
                        new ByteArrayInputStream(mHtmlBuilder.getRedirectPage(charset, Uri.parse(location))));
            }
        }

        ResponseBody body = httpResponse.body();
        if (body == null) {
            return null;
        }
        InputStream responseStream;
        if (HBBTV_MIME_TYPES.contains(mimeType.toLowerCase())) {
            responseStream = createInjectionResponseStream(body.byteStream(), body, charset, request.getUrl(), appId);
        } else {
            responseStream = createResponseStream(body.byteStream(), body);
        }
        WebResourceResponse response = new WebResourceResponse(mimeType, charset.name(), responseStream);
        Map<String, String> responseHeaders = new HashMap<>();
        httpResponseHeaders.forEach((k, v) -> responseHeaders.put(k, String.join(",", v)));
        response.setResponseHeaders(responseHeaders);
        return response;
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
        DsmccCallback.DvbInputStream body = DsmccCallback.createDvbInput(url);

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
        ;
        String extension = MimeTypeMap.getFileExtensionFromUrl(url);
        if (extension != null && !extension.equals("")) {
            String fromExtension = MimeTypeMap.getSingleton().getMimeTypeFromExtension(extension);
            if (fromExtension != null) {
                type = fromExtension;
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

    WebResourceResponse createTextResponse(int statusCode, String reasonPhrase, String text,
                                           String allowOrigin) {
        Charset charset = StandardCharsets.UTF_8;
        ByteArrayInputStream data = new ByteArrayInputStream(text.getBytes(charset));
        WebResourceResponse response = new WebResourceResponse("text/plain",
                charset.toString(), data);
        if (allowOrigin != null) {
            Map<String, String> headers = new HashMap<>();
            headers.put("Access-Control-Allow-Origin", allowOrigin);
            response.setResponseHeaders(headers);
        }
        if (statusCode != 200) {
            response.setStatusCodeAndReasonPhrase(statusCode, reasonPhrase);
        }
        return response;
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
}

