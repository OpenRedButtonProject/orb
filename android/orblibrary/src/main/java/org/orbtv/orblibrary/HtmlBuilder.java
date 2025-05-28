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

import android.content.res.AssetManager;
import android.net.Uri;
import android.util.Log;

import org.json.JSONException;
import org.json.JSONObject;
import org.orbtv.orbpolyfill.BridgeToken;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.nio.charset.Charset;
import java.nio.charset.StandardCharsets;
import java.util.UUID;
import java.util.Arrays;

class HtmlBuilder {
    private static final String TAG = HtmlBuilder.class.getSimpleName();

    AssetManager mAssetManager;
    String mHbbtvInjection;
    byte[] mHbbtvInjectionUtf8;

    HtmlBuilder(AssetManager assetManager) {
        mAssetManager = assetManager;
        mHbbtvInjection = getHbbtvInjection();
        mHbbtvInjectionUtf8 = mHbbtvInjection.getBytes(StandardCharsets.UTF_8);
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
        BridgeToken token = new BridgeToken(appId, uri.toString(), getOrigin(uri));
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
