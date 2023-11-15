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
