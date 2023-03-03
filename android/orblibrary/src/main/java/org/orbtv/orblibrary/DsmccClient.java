/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at the top level of this
 * repository.
 */

package org.orbtv.orblibrary;

import android.util.Log;

import java.nio.ByteBuffer;

import java.util.HashMap;

class DsmccClient {
   private static final String TAG = DsmccClient.class.getSimpleName();

   private final HashMap<Integer, DvbInputStream> mRequestMap = new HashMap<>();
   private final IOrbSessionCallback mSessionCallback;
   private int mNextRequestId = 501;

   DsmccClient(IOrbSessionCallback sessionCallback) {
      mSessionCallback = sessionCallback;
   }

   public DvbInputStream requestContent(String url) {
      Log.d(TAG, "Request DSMCC content: " + url);
      int requestId = mNextRequestId++;
      DvbInputStream stream = new DvbInputStream(requestId, mSessionCallback);
      mRequestMap.put(requestId, stream);
      boolean valid = mSessionCallback.requestDsmccDvbContent(url, requestId);
      if (!valid) {
         Log.w(TAG, "Request failed " + requestId + " url: " + url);
         mRequestMap.remove(requestId);
         stream.mStatusOK = false;
         stream.mReceived = true;
         stream.mStatus = 404;
      }
      return stream;
   }

   public void onDsmccReceiveContent(int requestId, ByteBuffer buffer) {
      DvbInputStream dvbRequest = mRequestMap.remove(requestId);
      if (dvbRequest == null) {
         Log.w(TAG, "Request ID not found " + requestId);
      } else {
         dvbRequest.receiveContent(buffer);
      }
   }
}
