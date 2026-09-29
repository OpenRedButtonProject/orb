package org.orbtv.orblibrary;

import android.content.Context;
import android.util.Log;

import org.chromium.net.CronetEngine;
import org.chromium.net.CronetException;
import org.chromium.net.ExperimentalCronetEngine;
import org.chromium.net.UrlRequest;
import org.chromium.net.UrlResponseInfo;

import java.io.IOException;
import java.io.PipedInputStream;
import java.io.PipedOutputStream;
import java.nio.ByteBuffer;
import java.util.Collections;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicReference;

/**
 * One Cronet engine for every http and https fetch {@link WebResourceClient} makes.
 * Alt-Svc learned on one request is reused by the next, and HTTPS DNS
 * records (RFC 9460) are enabled so an HTTP/3-only origin can be reached
 * on the first request.
 */
final class OrbCronet {
    private static final String TAG = "OrbCronet";
    private static final long HEADER_TIMEOUT_SEC = 30;

    private static final String EXPERIMENTAL_OPTIONS =
            "{\"AsyncDNS\":{\"enable\":true},"
                    + "\"UseDnsHttpsSvcb\":{\"enable\":true,\"use_alpn\":true}}";

    private static CronetEngine sEngine;
    private static boolean sUnavailable;
    private static final ExecutorService sCallbackExecutor = Executors.newSingleThreadExecutor(r -> {
        Thread thread = new Thread(r, "orb-cronet");
        thread.setDaemon(true);
        return thread;
    });

    private OrbCronet() {
    }

    /**
     * @return a response, or null when Cronet could not be started so the caller can use OkHttp
     */
    static OrbHttpResult execute(Context context, String url, String method,
            Map<String, String> headers) throws IOException {
        CronetEngine engine = engine(context);
        if (engine == null) {
            return null;
        }
        return new Call(engine, url, method, headers).execute();
    }

    private static synchronized CronetEngine engine(Context context) {
        if (sEngine != null || sUnavailable) {
            return sEngine;
        }
        try {
            Context app = context.getApplicationContext();
            java.io.File storage = new java.io.File(app.getCacheDir(), "cronet");
            if (!storage.isDirectory() && !storage.mkdirs()) {
                throw new IOException("Cannot create " + storage);
            }
            ExperimentalCronetEngine.Builder builder =
                    new ExperimentalCronetEngine.Builder(app)
                            .setExperimentalOptions(EXPERIMENTAL_OPTIONS)
                            .enableQuic(true)
                            .enableHttp2(true);
            sEngine = builder
                    .enableBrotli(true)
                    .setStoragePath(storage.getAbsolutePath())
                    // Honor Cache-Control. Caching every body would replay a cleartext
                    // probe after this engine started carrying http as well as https.
                    // The storage directory still keeps QUIC and Alt-Svc state.
                    .enableHttpCache(CronetEngine.Builder.HTTP_CACHE_DISK, 1024 * 1024)
                    .build();
            Log.i(TAG, "Cronet engine started, QUIC and HTTPS DNS records enabled");
        } catch (Throwable t) {
            Log.e(TAG, "Cronet engine failed to start; HTTP stays on OkHttp", t);
            sUnavailable = true;
            sEngine = null;
        }
        return sEngine;
    }

    private static final class Call extends UrlRequest.Callback {
        private final CronetEngine mEngine;
        private final String mUrl;
        private final String mMethod;
        private final Map<String, String> mHeaders;
        private final CountDownLatch mHeadersReady = new CountDownLatch(1);
        private final AtomicReference<UrlResponseInfo> mInfo = new AtomicReference<>();
        private final AtomicReference<Throwable> mError = new AtomicReference<>();
        private final PipedOutputStream mBodyOut;
        private final PipedInputStream mBodyIn;
        private volatile boolean mRedirect;

        Call(CronetEngine engine, String url, String method, Map<String, String> headers)
                throws IOException {
            mEngine = engine;
            mUrl = url;
            mMethod = method;
            mHeaders = headers;
            mBodyOut = new PipedOutputStream();
            mBodyIn = new PipedInputStream(mBodyOut, 64 * 1024);
        }

        OrbHttpResult execute() throws IOException {
            UrlRequest.Builder builder = mEngine.newUrlRequestBuilder(
                    mUrl, this, sCallbackExecutor);
            builder.setHttpMethod(mMethod);
            if (mHeaders != null) {
                for (Map.Entry<String, String> header : mHeaders.entrySet()) {
                    if (header.getKey() == null || header.getValue() == null
                            || skipHeader(header.getKey())) {
                        continue;
                    }
                    try {
                        builder.addHeader(header.getKey(), header.getValue());
                    } catch (IllegalArgumentException e) {
                        Log.w(TAG, "Dropping header " + header.getKey() + " for " + mUrl);
                    }
                }
            }
            UrlRequest request = builder.build();
            request.start();
            try {
                if (!mHeadersReady.await(HEADER_TIMEOUT_SEC, TimeUnit.SECONDS)) {
                    request.cancel();
                    throw new IOException("Timed out waiting for headers from " + mUrl);
                }
            } catch (InterruptedException e) {
                request.cancel();
                Thread.currentThread().interrupt();
                throw new IOException("Interrupted waiting for headers from " + mUrl, e);
            }
            Throwable error = mError.get();
            if (error != null) {
                throw new IOException("Cronet request failed for " + mUrl, error);
            }
            UrlResponseInfo info = mInfo.get();
            if (info == null) {
                throw new IOException("No response info for " + mUrl);
            }
            Log.d(TAG, info.getNegotiatedProtocol() + " " + info.getHttpStatusCode()
                    + " " + mUrl);
            Map<String, List<String>> headers = info.getAllHeaders();
            if (headers == null) {
                headers = Collections.emptyMap();
            }
            long length = contentLength(headers);
            return new OrbHttpResult(info.getHttpStatusCode(), info.getHttpStatusText(),
                    headers, mRedirect ? empty() : mBodyIn, length, () -> {
                request.cancel();
                closeQuietly(mBodyOut);
                closeQuietly(mBodyIn);
            });
        }

        @Override
        public void onRedirectReceived(UrlRequest request, UrlResponseInfo info,
                String newLocationUrl) {
            // Cronet reports an HSTS scheme upgrade as an internal redirect.
            // A real 3xx is cancelled so ORB can turn it into an HTML redirect page.
            if (isHstsInternalRedirect(info)) {
                request.followRedirect();
                return;
            }
            mRedirect = true;
            mInfo.set(info);
            request.cancel();
        }

        @Override
        public void onResponseStarted(UrlRequest request, UrlResponseInfo info) {
            mInfo.set(info);
            ByteBuffer buffer = ByteBuffer.allocateDirect(16 * 1024);
            request.read(buffer);
            mHeadersReady.countDown();
        }

        @Override
        public void onReadCompleted(UrlRequest request, UrlResponseInfo info,
                ByteBuffer buffer) {
            try {
                buffer.flip();
                if (buffer.hasRemaining()) {
                    byte[] copy = new byte[buffer.remaining()];
                    buffer.get(copy);
                    mBodyOut.write(copy);
                }
                buffer.clear();
                request.read(buffer);
            } catch (IOException e) {
                mError.compareAndSet(null, e);
                request.cancel();
            }
        }

        @Override
        public void onSucceeded(UrlRequest request, UrlResponseInfo info) {
            mInfo.compareAndSet(null, info);
            closeQuietly(mBodyOut);
            mHeadersReady.countDown();
        }

        @Override
        public void onFailed(UrlRequest request, UrlResponseInfo info, CronetException error) {
            mError.compareAndSet(null, error);
            closeQuietly(mBodyOut);
            mHeadersReady.countDown();
        }

        private static boolean isHstsInternalRedirect(UrlResponseInfo info) {
            if (info == null || info.getAllHeaders() == null) {
                return false;
            }
            for (Map.Entry<String, List<String>> header : info.getAllHeaders().entrySet()) {
                if (header.getKey() == null || !header.getKey().equalsIgnoreCase(
                        "Non-Authoritative-Reason") || header.getValue() == null) {
                    continue;
                }
                for (String value : header.getValue()) {
                    if (value != null && value.equalsIgnoreCase("HSTS")) {
                        return true;
                    }
                }
            }
            return false;
        }

        @Override
        public void onCanceled(UrlRequest request, UrlResponseInfo info) {
            if (mRedirect) {
                closeQuietly(mBodyOut);
                mHeadersReady.countDown();
                return;
            }
            mError.compareAndSet(null, new IOException("Cronet request canceled"));
            closeQuietly(mBodyOut);
            mHeadersReady.countDown();
        }
    }

    private static boolean skipHeader(String name) {
        String lower = name.toLowerCase(Locale.US);
        return lower.equals("accept-encoding")
                || lower.equals("connection")
                || lower.equals("content-length")
                || lower.equals("host")
                || lower.equals("keep-alive")
                || lower.equals("transfer-encoding")
                || lower.equals("upgrade");
    }

    private static long contentLength(Map<String, List<String>> headers) {
        for (Map.Entry<String, List<String>> entry : headers.entrySet()) {
            if (entry.getKey() != null && entry.getKey().equalsIgnoreCase("Content-Length")
                    && entry.getValue() != null && !entry.getValue().isEmpty()) {
                try {
                    return Long.parseLong(entry.getValue().get(0));
                } catch (NumberFormatException ignored) {
                    return -1;
                }
            }
        }
        return -1;
    }

    private static java.io.InputStream empty() {
        return new java.io.ByteArrayInputStream(new byte[0]);
    }

    private static void closeQuietly(java.io.Closeable closeable) {
        try {
            closeable.close();
        } catch (IOException ignored) {
        }
    }
}
