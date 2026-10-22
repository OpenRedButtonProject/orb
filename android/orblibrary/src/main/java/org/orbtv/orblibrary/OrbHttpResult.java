package org.orbtv.orblibrary;

import java.io.Closeable;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Map;

/** Status, headers and body for one fetch, whether it came from Cronet or OkHttp. */
final class OrbHttpResult implements Closeable {
    private final int mCode;
    private final String mMessage;
    private final Map<String, List<String>> mHeaders;
    private final InputStream mBody;
    private final long mContentLength;
    private final Closeable mCloser;

    OrbHttpResult(int code, String message, Map<String, List<String>> headers,
            InputStream body, long contentLength, Closeable closer) {
        mCode = code;
        mMessage = message;
        mHeaders = headers != null ? headers : Collections.emptyMap();
        mBody = body;
        mContentLength = contentLength;
        mCloser = closer;
    }

    int code() {
        return mCode;
    }

    String message() {
        return mMessage;
    }

    boolean isSuccessful() {
        return mCode >= 200 && mCode < 300;
    }

    Map<String, List<String>> headers() {
        return mHeaders;
    }

    String header(String name) {
        List<String> values = headersNamed(name);
        if (values.isEmpty()) {
            return null;
        }
        return values.get(0);
    }

    List<String> headersNamed(String name) {
        for (Map.Entry<String, List<String>> entry : mHeaders.entrySet()) {
            if (entry.getKey() != null && entry.getKey().equalsIgnoreCase(name)) {
                return entry.getValue() != null ? entry.getValue() : Collections.emptyList();
            }
        }
        return Collections.emptyList();
    }

    List<String> headerNames() {
        List<String> names = new ArrayList<>();
        for (String name : mHeaders.keySet()) {
            if (name != null) {
                names.add(name);
            }
        }
        return names;
    }

    InputStream byteStream() {
        return mBody != null ? mBody : new java.io.ByteArrayInputStream(new byte[0]);
    }

    long contentLength() {
        return mContentLength;
    }

    @Override
    public void close() throws IOException {
        if (mCloser != null) {
            mCloser.close();
        }
    }
}
