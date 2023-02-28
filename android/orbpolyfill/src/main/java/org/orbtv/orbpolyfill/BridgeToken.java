/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

package org.orbtv.orbpolyfill;

import android.util.Base64;

import org.json.JSONException;
import org.json.JSONObject;

import java.security.InvalidKeyException;
import java.security.NoSuchAlgorithmException;
import java.util.UUID;

import javax.crypto.Mac;
import javax.crypto.spec.SecretKeySpec;

public class BridgeToken {
   private int mAppId;
   private String mUri;
   private String mOrigin;

   public BridgeToken(int appId, String uri, String origin)
   {
      mAppId = appId;
      mUri = uri;
      mOrigin = origin;
   }

   public BridgeToken(JSONObject object) throws Exception {
      JSONObject payload = getObjectPayload(object);
      if (payload == null) {
         throw new Exception("Cannot get object payload");
      }
      mAppId = payload.getInt("appId");
      mUri = payload.getString("uri");
      mOrigin = payload.getString("origin");
   }

   public int getAppId() {
      return mAppId;
   }

   public String getUri() {
      return mUri;
   }

   public String getOrigin() {
      return mOrigin;
   }

   public JSONObject getJSONObject() throws Exception {
      JSONObject payload = new JSONObject();
      payload.put("appId", mAppId);
      payload.put("uri", mUri);
      payload.put("origin", mOrigin);
      return getObject(payload);
   }

   private final static byte[] gSecretKey;

   private static JSONObject getObject(JSONObject payload) throws JSONException {
      String json = payload.toString();
      String signature = getHash(json);
      if (signature == null) {
         return null;
      }
      JSONObject object = new JSONObject();
      object.put("payload", json);
      object.put("signature", signature);
      return object;
   }

   private static JSONObject getObjectPayload(JSONObject object) {
      String json;
      String claimedSignature;
      try {
         json = object.getString("payload");
         claimedSignature = object.getString("signature");
      } catch (JSONException e) {
         return null;
      }
      String signature = getHash(json);
      JSONObject payload = null;
      if (signature != null && signature.equals(claimedSignature)) {
         try {
            payload = new JSONObject(json);
         } catch (JSONException e) {
            e.printStackTrace();
            payload = null;
         }
      }
      return payload;
   }

   private static String getHash(String message) {
      Mac mac;
      try {
         mac = Mac.getInstance("HmacSHA256");
         mac.init(new SecretKeySpec(gSecretKey, "HmacSHA256"));
      } catch (NoSuchAlgorithmException | InvalidKeyException e) {
         e.printStackTrace();
         return null;
      }
      return Base64.encodeToString(mac.doFinal(message.getBytes()), Base64.DEFAULT).trim();
   }

   static {
      gSecretKey = UUID.randomUUID().toString().getBytes();
   }
}
