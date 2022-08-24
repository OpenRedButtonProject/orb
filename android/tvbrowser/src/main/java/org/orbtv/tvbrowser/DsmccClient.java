/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 * <p>
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

package org.orbtv.tvbrowser;

import android.os.Build;
import android.util.Log;

import java.io.IOException;
import java.io.InputStream;
import java.nio.ByteBuffer;

import java.util.HashMap;
import java.util.Objects;

public class DsmccClient implements IDsmccClient {
    private static final String TAG = "Orb/DsmccClient";

    private final TvBrowser.Session mSession;
    private static IDsmccEngine mDsmccEngine;

    private static int mNextRequestId = 501;
    private static int mNextListenId = 1;
    private static final HashMap<Integer, DvbInputStream> mRequestMap = new HashMap<>();
    private static final HashMap<Integer, String> mListenMap = new HashMap<>();

    DsmccClient(IDsmccEngine dsmcc, TvBrowser.Session session) {
        mDsmccEngine = dsmcc;
        mDsmccEngine.setDsmccClient(this);
        mSession = session;
    }

    /**
     * Called by DsmccEngine on receiving content
     *
     * @param requestId ID of request
     * @param buffer ByteBuffer with content for DSMCC file
     */
    public void onReceiveContent(int requestId, ByteBuffer buffer) {
        //Log.d(TAG,"receiveContent rqstId=" + requestId);
        DvbInputStream dvbRequest = mRequestMap.remove(requestId);
        if (dvbRequest == null) {
            Log.w(TAG, "Request ID not found " + requestId);
        } else {
            dvbRequest.receiveContent(buffer);
        }
    }

    /**
     * Called by DsmccEngine on receiving Stream Event
     *
     * @param listenId ID of listener
     * @param name Name of Stream event
     * @param data Data associated with stream event
     */
    public void onReceiveStreamEvent(int listenId, String name, String data, String text, String status) {
        Log.d(TAG, "receiveStreamEvent listenId=" + listenId + " " + name + " " + data);
        // TODO
        // data needs to be placed in two fields -
        // 'data' which is 'encoded in hexadecimal', and
        // 'text' which is 'encoded in UTF-8'
        // status is 'trigger'
        // (see TS 102796 sec 8.2.1.2)
        // and map listenId to URL, name, and listener object ?
        // then dispatch event to JS
        mSession.onReceiveStreamEvent(listenId, name, data, text, status);
    }

    static DvbInputStream createDvbInput(String url) {
        int requestId = mNextRequestId++;
        DvbInputStream stream = new DvbInputStream(requestId);
        mRequestMap.put(requestId, stream);
        Log.d(TAG, "DvbInput url=" + url + ", requestId=" + requestId);
        boolean valid = mDsmccEngine.requestDvbContent(url, requestId);
        if (!valid) {
            Log.w(TAG, "Request failed " + requestId + " url: " + url);
            mRequestMap.remove(requestId);
            stream.mValid = false;
            stream.mReceived = true;
            stream.mStatus = 404;
        }
        return stream;
    }

    static int subscribeStreamEventName(String url, String name) {
        String id = "U=" + url + ",N=" + name;// is this string the best way?
        int listenId = mNextListenId++;
        mListenMap.put(listenId, id);
        if (!mDsmccEngine.subscribeStreamEventName(url, name, listenId)) {
            return -1;
        }
        return listenId;
    }

    static int subscribeStreamEventId(String name, int componentTag, int eventId) {
        String id = "C=" + componentTag + ",E=" + eventId;// is this string the best way?
        int listenId = mNextListenId++;
        if (!mDsmccEngine.subscribeStreamEventId(name, componentTag, eventId, listenId)) {
            return -1;
        }
        return listenId;
    }

    static void unsubscribeStreamEvent(int id) {
        mDsmccEngine.unsubscribeStreamEvent(id);
        mListenMap.remove(id);
    }

    static class DvbInputStream extends InputStream {
        private static final String TAG = "DvbInput";

        private final Object mLock = new Object();
        private volatile boolean mValid;
        private volatile int mStatus;
        private boolean mReceived;
        private final int mRequestId;
        private ByteBuffer mBuffer;
        private int mLength;

        DvbInputStream(int requestId) {
            mRequestId = requestId;
            mBuffer = null;
            mReceived = false;
            mValid = true;
            mStatus = 202;
            mLength = 0;
        }

        @Override
        public int available() throws IOException {
            int avail;
            if (!mValid) {
                throw new IOException("no dvb content");
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
            if (!mValid) {
                throw new IOException("no dvb content");
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
            if (!mValid) {
                throw new IOException("no dvb content");
            }
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
                Objects.checkFromIndexSize(offset, length, dst.length);
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
                Log.d(TAG, "close " + mRequestId);
                mDsmccEngine.closeDvbContent(mRequestId);
                mValid = false;
                mBuffer = null;
            }
        }

        public void receiveContent(ByteBuffer buffer) {
            synchronized (mLock) {
                Log.i(TAG, "receiveContent " + buffer);
                mReceived = true;
                if (buffer != null) {
                    mBuffer = buffer;
                    mStatus = 200;
                    mLength = buffer.limit();
                } else {
                    mValid = false;
                    mStatus = 404;
                }
                mLock.notifyAll();
            }
        }

        private void waitForContent() {
            Log.d(TAG, "waiting For Content " + mReceived);
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
}

