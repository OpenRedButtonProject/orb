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

import android.os.Build;

import java.io.IOException;
import java.io.InputStream;
import java.nio.ByteBuffer;
import java.util.Objects;

class DvbInputStream extends InputStream {
    private final Object mLock = new Object();
    private final int mRequestId;
    private final IOrbSessionCallback mSessionCallback;
    public volatile boolean mStatusOK;
    public volatile int mStatus;
    public volatile boolean mReceived;
    private ByteBuffer mBuffer;
    private int mLength;

    DvbInputStream(int requestId, IOrbSessionCallback sessionCallback) {
        mRequestId = requestId;
        mSessionCallback = sessionCallback;
        mBuffer = null;
        mReceived = false;
        mStatusOK = true;
        mStatus = 202;
        mLength = 0;
    }

    @Override
    public int available() throws IOException {
        int avail;
        if (!mStatusOK) {
            return 0;
        }
        synchronized (mLock) {
            if (!mReceived || mBuffer == null) {
                avail = 0;
            } else {
                avail = mBuffer.remaining();
            }
        }
        return avail;
    }

    @Override
    public int read() throws IOException {
        if (!mStatusOK) {
            return -1;
        }
        waitForContent();
        if (mBuffer == null) {
            throw new IOException("Invalid dvb content");
        }
        if (!mBuffer.hasRemaining()) {
            return -1;
        }
        return mBuffer.get();
    }

    @Override
    public int read(byte[] dst, int offset, int length) throws IOException {
        if (!mStatusOK) {
            return -1;
        }
        if (length == 0) {
            return 0;
        }
        waitForContent();
        if (mBuffer == null) {
            throw new IOException("Invalid dvb content");
        }
        if (!mBuffer.hasRemaining()) {
            return -1;
        }
        if (length > mBuffer.remaining()) {
            length = mBuffer.remaining();
        }
        mBuffer.get(dst, offset, length);
        return length;
    }

    @Override
    public void close() {
        synchronized (mLock) {
            mSessionCallback.closeDsmccDvbContent(mRequestId);
            mBuffer = null;
        }
    }

    public void receiveContent(ByteBuffer buffer) {
        synchronized (mLock) {
            mReceived = true;
            if (buffer != null) {
                mBuffer = buffer;
                mStatus = 200;
                mLength = buffer.limit();
            } else {
                mStatusOK = false;
                mStatus = 404;
            }
            mLock.notifyAll();
        }
    }

    private void waitForContent() {
        synchronized (mLock) {
            while (!mReceived) {
                try {
                    mLock.wait();
                } catch (InterruptedException ignored) {
                }
            }
        }
    }

    int getStatusCode() {
        waitForContent();
        return mStatus;
    }

    String reasonPhrase() {
        switch (mStatus) {
            case 200:
                return "OK";
            case 202:
                return "Accepted";
            case 404:
                return "Not found";
        }
        return "Unknown"; // never here
    }

    int getDataLength() {
        return mLength;
    }
}
