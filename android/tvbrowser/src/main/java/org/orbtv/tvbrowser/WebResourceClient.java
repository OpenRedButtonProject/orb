/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

package org.orbtv.tvbrowser;

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
import java.net.InetAddress;
import java.net.UnknownHostException;
import java.nio.charset.Charset;
import java.nio.charset.StandardCharsets;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Vector;

// For building in source. Replaced with com.squareup... by build.gradle during Gradle build:
import com.android.okhttp.*;

public abstract class WebResourceClient {
   private static final String TAG = WebResourceClient.class.getSimpleName();

   private static final boolean HTTP_COOKIES_ENABLED = true;
   private static final boolean HTTP_REDIRECTION_ENABLED = true;
   private static final List<String> HBBTV_MIME_TYPES = Arrays.asList(
      "text/html",
      "application/xhtml+xml",
      "application/xml",
      "application/vnd.hbbtv.xhtml+xml"
   );
   private static final String ORB_PLAYER_MAGIC_SUFFIX = "orb_player_magic_suffix";

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
         if (url.toString().endsWith(ORB_PLAYER_MAGIC_SUFFIX)) {
            return createPlayerPageResponse();
         }  else if (scheme.equals("http") || scheme.equals("https")) {
            return shouldInterceptHttpRequest(request, appId);
         } else if (scheme.equals("dvb")) {
            return shouldInterceptDsmccRequest(request, appId);
         } else if (scheme.equals("dash") || scheme.equals("dashs")) {
            return shouldInterceptDashRequest(request, appId);
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
      DsmccClient.DvbInputStream body = DsmccClient.createDvbInput(url);

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

   private WebResourceResponse shouldInterceptDashRequest(WebResourceRequest request, int appId) {
      // IMPORTANT: Ordinarily <video> is backed by a player that executes outside of the page and
      // is not affected by same-origin policy, other than the embedded content being inaccessible.
      //
      // To enable dash.js to access resources on different origins, requests purporting to be from
      // dash.js are allowed and handled by a "sandbox HTTP client" that must:
      //
      // (1) Only handle requests with a GET method
      // (2) Only handle requests with a publicly routable host or (c.)hbbtvn.test for testing
      // (3) Be isolated from the browser and not send any site data such as cookies

      // Handle special dash://resolve.host/{hostname} requests
      if (request.getUrl().getHost().equals("resolve.host")) {
         String hostname = request.getUrl().getLastPathSegment();
         Log.d(TAG, "Resolve hostname" + hostname);
         InetAddress addr = resolvePublicHostname(hostname);
         String text = "";
         if (addr != null) {
            text = addr.getHostAddress();
         }
         return createTextResponse(200, "", text,
            request.getRequestHeaders().getOrDefault("Origin", null));
      }

      // Handle other dash:// requests
      String url = "http" + request.getUrl().toString().substring(4); // Replace dash with http
      // Must only handle requests with a GET method
      if (!request.getMethod().equalsIgnoreCase("GET")) {
         return null;
      }
      // Must only handle requests with a publicly routable host or (c.)hbbtvn.test for testing
      String hostname = Uri.parse(url).getHost();
      if (resolvePublicHostname(hostname) == null) {
         // Not public host or HbbTV test URL, fail unless NA URL:
         if (!hostname.equals("server-na.hbbtv1.test")) {
            return null;
         }
      }
      WebResourceResponse response;
      try {
         response = handleDashRequest(request, appId);
      } catch (IOException e) {
         return createTextResponse(504, "Gateway Timeout", "",
            request.getRequestHeaders().getOrDefault("Origin", null));
      }
      return response;
   }

   private WebResourceResponse handleDashRequest(WebResourceRequest request, int appId)
         throws IOException {
      Map<String, String> requestHeaders = request.getRequestHeaders();
      String url = "http" + request.getUrl().toString().substring(4); // Replace dash with http

      // Must be isolated from the browser and not send any site data such as cookies
      Response httpResponse = mHttpSandboxClient.newCall(new Request.Builder()
         .url(url)
         .method(request.getMethod(), null)
         .headers(Headers.of(requestHeaders))
         .build()).execute();

      Charset charset = StandardCharsets.UTF_8;
      if (!httpResponse.isSuccessful()) {
         return createTextResponse(httpResponse.code(), httpResponse.message(), "",
            request.getRequestHeaders().getOrDefault("Origin", null));
      }
      String mimeType = getMimeType(httpResponse.header("Content-Type", "text/plain"));
      Map<String, List<String>> httpResponseHeaders = httpResponse.headers().toMultimap();
      String origin = request.getRequestHeaders().get("Origin");
      if (origin != null) {
         httpResponseHeaders.put("Access-Control-Allow-Origin", Collections.singletonList(origin));
      }
      ResponseBody body = httpResponse.body();
      if (body == null) {
         return null;
      }
      InputStream responseStream = createResponseStream(body.byteStream(), body);
      WebResourceResponse response = new WebResourceResponse(mimeType, charset.name(), responseStream);
      Map<String, String> responseHeaders = new HashMap<>();
      httpResponseHeaders.forEach((k, v) -> responseHeaders.put(k, String.join(",", v)));
      response.setResponseHeaders(responseHeaders);
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
      String type = "*/*";;
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

   private InetAddress resolvePublicHostname(String hostname) {
      // Must only handle requests with a publicly routable host or HbbTV test URL
      InetAddress addr;
      try {
         addr = InetAddress.getByName(hostname);
         if (addr.isSiteLocalAddress() || addr.isLoopbackAddress() || addr.isLinkLocalAddress()) {
            // Is private (10/8, 172.16/12, 192.168/16), loopback (127/8) or link local (169.254/16)
            if (!hostname.matches("^([a-c]\\.)?hbbtv[1-3].test$")) {
               throw new UnknownHostException();
            }
         }
      } catch (UnknownHostException e) {
         Log.d(TAG, "Unknown hostname: " + hostname);
         return null;
      }
      return addr;
   }

   WebResourceResponse createPlayerPageResponse() {
      Charset charset = StandardCharsets.UTF_8;
      ByteArrayInputStream data = new ByteArrayInputStream(mHtmlBuilder.getPlayerPage(charset));
      WebResourceResponse response = new WebResourceResponse("text/html",
         charset.toString(), data);
      return response;
   }
}
