/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

package org.orbtv.tvbrowser;

import android.content.res.AssetManager;
import android.net.Uri;

import org.json.JSONException;
import org.json.JSONObject;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.nio.charset.Charset;
import java.nio.charset.StandardCharsets;
import java.util.UUID;

public class HtmlBuilder {
   AssetManager mAssetManager;
   String mHbbtvInjection;
   byte[] mHbbtvInjectionUtf8;
   String mMediaManagerInjection;
   byte[] mMediaManagerInjectionUtf8;
   String mPlayerPage;
   byte[] mPlayerPageUtf8;

   HtmlBuilder(AssetManager assetManager) {
      mAssetManager = assetManager;
      mHbbtvInjection = getHbbtvInjection();
      mHbbtvInjectionUtf8 = mHbbtvInjection.getBytes(StandardCharsets.UTF_8);
      mMediaManagerInjection = getMediaManagerInjection();
      mMediaManagerInjectionUtf8 = mMediaManagerInjection.getBytes(StandardCharsets.UTF_8);
      mPlayerPage = getPlayerPage();
      mPlayerPageUtf8 = mPlayerPage.getBytes(StandardCharsets.UTF_8);
   }

   public byte[] getRedirectPage(Charset charset, Uri uri) {
      String script = "";
      JSONObject json = new JSONObject();
      try {
         json.put("uri", uri.toString());
         script = "<script type=\"text/javascript\">\n//<![CDATA[\n" +
            "const redirect = " + json.toString() + ";" +
            "document.location.href = redirect.uri;" +
            "\n//]]>\n</script>";
      } catch (JSONException e) {
         e.printStackTrace();
      }
      return script.getBytes(charset);
   }

   public byte[] getTokenInjection(Charset charset, Uri uri, int appId) {
      String script = "";
      Token token = new Token(appId, uri.toString(), getOrigin(uri));
      try {
         script = "<script type=\"text/javascript\">\n//<![CDATA[\n" +
            "document.token = " + token.getJSONObject().toString() + ";" +
            "\n//]]>\n</script>";
      } catch (Exception e) {
         e.printStackTrace();
      }
      return script.getBytes(charset);
   }

   public byte[] getHbbtvInjection(Charset charset) {
      if (charset == StandardCharsets.UTF_8) {
         return mHbbtvInjectionUtf8;
      } else {
         return mHbbtvInjection.getBytes(charset);
      }
   }

   public byte[] getMediaManagerInjection(Charset charset) {
      if (charset == StandardCharsets.UTF_8) {
         return mMediaManagerInjectionUtf8;
      } else {
         return mMediaManagerInjection.getBytes(charset);
      }
   }

   public byte[] getPlayerPage(Charset charset) {
      if (charset == StandardCharsets.UTF_8) {
         return mPlayerPageUtf8;
      } else {
         return mPlayerPage.getBytes(charset);
      }
   }

   private String getHbbtvInjection() {
      StringBuilder builder = new StringBuilder();
      builder.append("<script type=\"text/javascript\">\n//<![CDATA[\n");
      try {
         appendAsset(builder, "polyfill/hbbtv.js");
      } catch (IOException e) {
         e.printStackTrace();
         return "";
      }
      builder.append("\n//]]>\n</script>");
      return builder.toString();
   }

   private String getMediaManagerInjection() {
      StringBuilder builder = new StringBuilder();
      builder.append("<script type=\"text/javascript\">\n//<![CDATA[\n");
      try {
         appendAsset(builder, "polyfill/mediamanager.js");
         appendAsset(builder, "polyfill/dash.all.min.js");
      } catch (IOException e) {
         e.printStackTrace();
         return "";
      }
      builder.append("\n//]]>\n</script>");
      return builder.toString();
   }

   private String getPlayerPage() {
      StringBuilder builder = new StringBuilder();
      try {
         appendAsset(builder, "playerpage.html");
      } catch (IOException e) {
         e.printStackTrace();
         return "";
      }
      return builder.toString();
   }

   private void appendAsset(StringBuilder builder, String fileName) throws IOException {
      InputStream is = mAssetManager.open(fileName);
      BufferedReader reader = new BufferedReader(new InputStreamReader(is));
      String line;
      while ((line = reader.readLine()) != null) {
         builder.append(line).append("\n");
      }
      reader.close();
   }

   private String getOrigin(Uri uri) {
      String scheme = uri.getScheme();
      if (!uri.isHierarchical() || (!scheme.equals("http") && !scheme.equals("https") && !scheme.equals("dvb"))) {
         return "uuid-" + UUID.randomUUID().toString();
      }
      int port = uri.getPort();
      return scheme + "://" + uri.getHost() + ((port == -1) ? "" : ":" + port);
   }
}
