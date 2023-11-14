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

import java.io.ByteArrayInputStream;
import java.io.FilterInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.nio.charset.Charset;
import java.util.Vector;

abstract class InjectionInputStream extends FilterInputStream {
    private static final String TAG = InjectionInputStream.class.getSimpleName();

    InputStream mResponseBody;
    Charset mCharset;
    Vector<InputStream> mPayload;
    int mPayloadLength;
    byte[] mSearchBuffer;
    int mSearchBufferDataLength;
    boolean mSearchCompleted;

    protected InjectionInputStream(InputStream responseBody, Charset charset,
                                   Vector<InputStream> payload, int payloadLength) {
        super(responseBody);
        mResponseBody = responseBody;
        mCharset = charset;
        mPayload = payload;
        mPayloadLength = payloadLength;
        mSearchBuffer = new byte[8192];
        mSearchBufferDataLength = 0;
        mSearchCompleted = false;
    }

    @Override
    public int read(byte[] b) throws IOException {
        return read(b, 0, b.length);
    }

    @Override
    public int read(byte[] b, int off, int len) throws IOException {
        int n = 0;
        if (!mSearchCompleted) {
            // Append data to mSearchBuffer
            n = super.read(b, off, len);
            if (n == -1) {
                return -1;
            }
            if (mSearchBuffer.length < mSearchBufferDataLength + n) {
                // Unlikely case
                byte[] resized = new byte[(mSearchBufferDataLength + n) * 2];
                System.arraycopy(mSearchBuffer, 0, resized, 0, mSearchBuffer.length);
                mSearchBuffer = resized;
            }
            System.arraycopy(b, off, mSearchBuffer, mSearchBufferDataLength, n);
            mSearchBufferDataLength += n;
            // Search mSearchBuffer
            String searchBuffer = new String(mSearchBuffer, 0, mSearchBufferDataLength, mCharset);
            int index = findInjectionIndex(searchBuffer);
            if (index > -1) {
                // Add remainder after injection point to mPayload
                byte[] remainder = searchBuffer.substring(index).getBytes(mCharset);
                mPayload.add(new ByteArrayInputStream(remainder));
                n -= remainder.length;
                mSearchCompleted = true;
            }
        }
        if (mSearchCompleted) {
            while (!mPayload.isEmpty() && (len - n) > 0) {
                int nbytes = mPayload.firstElement().read(b, off + n, len - n);
                if (nbytes == -1) {
                    mPayload.remove(0);
                } else {
                    mPayloadLength -= nbytes;
                    n += nbytes;
                }
            }
        }
        if ((len - n) > 0) {
            int nbytes = super.read(b, off + n, len - n);
            if (n == 0 && nbytes == -1) {
                return -1;
            }
            n += nbytes;
        }
        return n;
    }

    @Override
    public int available() throws IOException {
        return super.available() + mPayloadLength;
    }

    @Override
    public void close() throws IOException {
        super.close();
        onClose();
    }

    void onClose() {
    }

    private int findInjectionIndex(String searchBuffer) {
        int index = searchBuffer.indexOf("<html");
        if (index == -1) {
            index = searchBuffer.indexOf("<head");
        }
        if (index > -1) {
            char quote = '\0';
            while (index < searchBuffer.length()) {
                char ch = searchBuffer.charAt(index);
                if (quote == ch) {
                    quote = '\0';
                } else if (ch == '"' || ch == '\'') {
                    quote = ch;
                }
                index++;
                if (quote == '\0' && ch == '>') {
                    return index;
                }
            }
        }
        return -1;
    }
}

