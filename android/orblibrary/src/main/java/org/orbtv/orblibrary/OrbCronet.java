package org.orbtv.orblibrary;

import android.content.Context;
import android.util.Log;

import org.chromium.net.CronetEngine;
import org.chromium.net.CronetException;
import org.chromium.net.ExperimentalCronetEngine;
import org.chromium.net.UrlRequest;
import org.chromium.net.UrlResponseInfo;

import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.nio.ByteBuffer;
import java.util.ArrayDeque;
import java.util.Collections;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.ScheduledFuture;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.atomic.AtomicReference;

/**
 * One Cronet engine for every http and https fetch {@link WebResourceClient} makes.
 * Alt-Svc learned on one request is reused by the next, and HTTPS DNS
 * records (RFC 9460) are enabled so an HTTP/3-only origin can be reached
 * on the first request.
 */
final class OrbCronet {
    private static final String TAG = "OrbCronet";
    /** Matches the OkHttp read timeout. Covers the wait for headers and each body read. */
    private static final long READ_TIMEOUT_SEC = 30;

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
    private static final ScheduledExecutorService sReadTimeouts =
            Executors.newSingleThreadScheduledExecutor(r -> {
                Thread thread = new Thread(r, "orb-cronet-timeout");
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

    /**
     * Returned at header time. {@link InputStream#read()} blocks until the next
     * bytes arrive or the body really ends, so a large file can start playing
     * while the rest downloads. At most {@link #QUEUE_HIGH_BYTES} are held, and
     * the Cronet callback thread is never blocked on the reader.
     *
     * <p>WebView's first {@link InputStream#available()} call is the whole
     * object. A prefix of this queue must not be that size, and reporting that
     * prefix as the Content-Length lets the decoder open on the codec-config
     * buffer and never submit the first picture. That first call returns 0
     * unless every byte is already queued. WebView then publishes
     * {@code Content-Length: 0} and reads until the real end. Later calls only
     * choose the next read size, so they return the bytes already held.
     * Otherwise each read is capped at 2 KB. A finished 1-byte body is reported
     * as 1: only a later {@code available()} of 1 means "more may exist".
     */
    private static final class BodyStream extends InputStream {
        private static final int QUEUE_HIGH_BYTES = 1024 * 1024;
        private static final int QUEUE_LOW_BYTES = 256 * 1024;

        private final String mUrl;
        private final ArrayDeque<byte[]> mChunks = new ArrayDeque<>();
        private int mChunkOffset;
        private int mQueued;
        private long mReceived;
        private boolean mFinished;
        private boolean mClosed;
        private boolean mSizeSampled;
        private IOException mFailure;

        private Runnable mOnDrain;

        BodyStream(String url) {
            mUrl = url;
        }

        void setOnDrain(Runnable onDrain) {
            mOnDrain = onDrain;
        }

        synchronized int queuedBytes() {
            return mQueued;
        }

        synchronized void offer(byte[] chunk) {
            if (mClosed || mFinished || mFailure != null || chunk.length == 0) {
                return;
            }
            mChunks.add(chunk);
            mQueued += chunk.length;
            mReceived += chunk.length;
            notifyAll();
        }

        synchronized long received() {
            return mReceived;
        }

        synchronized void finish() {
            mFinished = true;
            notifyAll();
        }

        synchronized void fail(IOException failure) {
            if (mFailure == null) {
                mFailure = failure;
            }
            mFinished = true;
            // Drop bytes still queued so the next read fails at once.
            // Returning them first would play the truncated prefix before the error.
            mChunks.clear();
            mChunkOffset = 0;
            mQueued = 0;
            notifyAll();
        }

        @Override
        public synchronized int available() {
            if (!mSizeSampled) {
                mSizeSampled = true;
                if (mFinished && mFailure == null && mReceived >= 1
                        && mReceived <= Integer.MAX_VALUE && mQueued == mReceived) {
                    return mQueued;
                }
                return 0;
            }
            // 1 means "more may exist" to WebView, the same as 0.
            if (mQueued <= 1) {
                return 0;
            }
            return mQueued;
        }

        @Override
        public int read() throws IOException {
            byte[] one = new byte[1];
            int n = read(one, 0, 1);
            return n < 0 ? -1 : (one[0] & 0xff);
        }

        @Override
        public int read(byte[] buffer, int offset, int length) throws IOException {
            if (offset < 0 || length < 0 || length > buffer.length - offset) {
                throw new IndexOutOfBoundsException();
            }
            if (length == 0) {
                return 0;
            }
            int copied;
            boolean resume;
            synchronized (this) {
                waitForData();
                if (mQueued == 0) {
                    if (mFailure != null) {
                        throw mFailure;
                    }
                    return -1;
                }
                copied = 0;
                while (copied < length && mQueued > 0) {
                    byte[] chunk = mChunks.peek();
                    int n = Math.min(length - copied, chunk.length - mChunkOffset);
                    System.arraycopy(chunk, mChunkOffset, buffer, offset + copied, n);
                    mChunkOffset += n;
                    mQueued -= n;
                    copied += n;
                    if (mChunkOffset == chunk.length) {
                        mChunks.remove();
                        mChunkOffset = 0;
                    }
                }
                resume = mQueued < QUEUE_LOW_BYTES;
                notifyAll();
            }
            if (resume && mOnDrain != null) {
                mOnDrain.run();
            }
            return copied;
        }

        private void waitForData() throws IOException {
            while (mQueued == 0 && mFailure == null && !mFinished && !mClosed) {
                try {
                    wait();
                } catch (InterruptedException e) {
                    Thread.currentThread().interrupt();
                    throw new IOException("Interrupted reading body from " + mUrl, e);
                }
            }
        }

        @Override
        public synchronized void close() {
            mClosed = true;
            notifyAll();
        }
    }

    private static final class Call extends UrlRequest.Callback {
        private final CronetEngine mEngine;
        private final String mUrl;
        private final String mMethod;
        private final Map<String, String> mHeaders;
        private final CountDownLatch mHeadersReady = new CountDownLatch(1);
        private final AtomicReference<UrlResponseInfo> mInfo = new AtomicReference<>();
        private final AtomicReference<Throwable> mError = new AtomicReference<>();
        private final BodyStream mStream;
        private final Object mTimeoutLock = new Object();
        private final AtomicInteger mState = new AtomicInteger(STATE_RUNNING);
        private final AtomicReference<ScheduledFuture<?>> mReadTimeout = new AtomicReference<>();
        private final ByteBuffer mReadBuffer = ByteBuffer.allocateDirect(16 * 1024);
        private int mReadGeneration;
        private int mReadState;
        private boolean mResumePosted;
        private long mContentLength = -1;
        private boolean mEncodedBody;
        private volatile boolean mRedirect;

        private static final int STATE_RUNNING = 0;
        private static final int STATE_DONE = 1;
        private static final int STATE_TIMED_OUT = 2;
        private static final int READ_IDLE = 0;
        private static final int READ_RUNNING = 1;
        private static final int READ_PAUSED = 2;

        Call(CronetEngine engine, String url, String method, Map<String, String> headers) {
            mEngine = engine;
            mUrl = url;
            mMethod = method;
            mHeaders = headers;
            mStream = new BodyStream(url);
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
            mStream.setOnDrain(() -> resumeIfDrained(request));
            request.start();
            try {
                if (!mHeadersReady.await(READ_TIMEOUT_SEC, TimeUnit.SECONDS)) {
                    finish();
                    request.cancel();
                    throw new IOException("Timed out waiting for headers from " + mUrl);
                }
            } catch (InterruptedException e) {
                finish();
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
            return new OrbHttpResult(info.getHttpStatusCode(), info.getHttpStatusText(),
                    headers, mRedirect ? new ByteArrayInputStream(new byte[0]) : mStream,
                    mContentLength, () -> {
                mStream.close();
                finish();
                request.cancel();
            });
        }

        /** Time out a Cronet read that the server has stopped feeding. A paused reader does not. */
        private void armReadTimeout(UrlRequest request) {
            final int generation;
            synchronized (mTimeoutLock) {
                disarmReadTimeoutLocked();
                if (mState.get() != STATE_RUNNING) {
                    return;
                }
                generation = ++mReadGeneration;
                mReadTimeout.set(sReadTimeouts.schedule(() -> {
                    synchronized (mTimeoutLock) {
                        if (mReadGeneration != generation
                                || !mState.compareAndSet(STATE_RUNNING, STATE_TIMED_OUT)) {
                            return;
                        }
                    }
                    Log.w(TAG, "Timed out reading body from " + mUrl);
                    request.cancel();
                }, READ_TIMEOUT_SEC, TimeUnit.SECONDS));
            }
        }

        private void disarmReadTimeout() {
            synchronized (mTimeoutLock) {
                disarmReadTimeoutLocked();
            }
        }

        private void disarmReadTimeoutLocked() {
            mReadGeneration++;
            ScheduledFuture<?> pending = mReadTimeout.getAndSet(null);
            if (pending != null) {
                pending.cancel(false);
            }
        }

        private void finish() {
            mState.compareAndSet(STATE_RUNNING, STATE_DONE);
            disarmReadTimeout();
        }

        private void signalHeaders() {
            mHeadersReady.countDown();
        }

        private boolean continueRead(UrlRequest request) {
            boolean readAgain;
            synchronized (mStream) {
                if (mState.get() != STATE_RUNNING || mStream.mClosed
                        || mReadState != READ_RUNNING) {
                    readAgain = false;
                } else if (mStream.queuedBytes() >= BodyStream.QUEUE_HIGH_BYTES) {
                    mReadState = READ_PAUSED;
                    disarmReadTimeout();
                    readAgain = false;
                } else {
                    readAgain = true;
                }
            }
            if (!readAgain) {
                return false;
            }
            startBodyRead(request);
            return true;
        }

        /**
         * Called from the WebView reader. {@link UrlRequest#read} has to run on
         * the callback executor, as the deferred reply to {@code onReadCompleted}.
         */
        private void resumeIfDrained(UrlRequest request) {
            synchronized (mStream) {
                if (mResumePosted || mReadState != READ_PAUSED
                        || mState.get() != STATE_RUNNING || mStream.mClosed
                        || mStream.queuedBytes() >= BodyStream.QUEUE_LOW_BYTES) {
                    return;
                }
                mResumePosted = true;
            }
            sCallbackExecutor.execute(() -> resumeIfDrainedOnCallback(request));
        }

        private void resumeIfDrainedOnCallback(UrlRequest request) {
            boolean resume;
            synchronized (mStream) {
                mResumePosted = false;
                resume = mState.get() == STATE_RUNNING
                        && mReadState == READ_PAUSED
                        && mStream.queuedBytes() < BodyStream.QUEUE_LOW_BYTES
                        && !mStream.mClosed;
                if (resume) {
                    mReadState = READ_RUNNING;
                }
            }
            if (resume) {
                startBodyRead(request);
            }
        }

        /**
         * Cronet throws {@link IllegalStateException} when the request has already
         * finished or a read is already outstanding. {@code close()} can cancel in
         * the gap after the state check, and that exception must not fail the load.
         */
        private void startBodyRead(UrlRequest request) {
            synchronized (mStream) {
                if (mState.get() != STATE_RUNNING || mStream.mClosed
                        || mReadState != READ_RUNNING) {
                    return;
                }
            }
            armReadTimeout(request);
            synchronized (mStream) {
                if (mState.get() != STATE_RUNNING || mStream.mClosed
                        || mReadState != READ_RUNNING) {
                    disarmReadTimeout();
                    return;
                }
            }
            try {
                request.read(mReadBuffer);
            } catch (IllegalStateException e) {
                disarmReadTimeout();
            }
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
            Map<String, List<String>> headers = info.getAllHeaders();
            if (headers == null) {
                headers = Collections.emptyMap();
            }
            mContentLength = contentLength(headers);
            mEncodedBody = encodedBody(headers);
            mReadState = READ_RUNNING;
            startBodyRead(request);
            signalHeaders();
        }

        @Override
        public void onReadCompleted(UrlRequest request, UrlResponseInfo info,
                ByteBuffer buffer) {
            disarmReadTimeout();
            buffer.flip();
            if (buffer.hasRemaining()) {
                byte[] copy = new byte[buffer.remaining()];
                buffer.get(copy);
                mStream.offer(copy);
            }
            buffer.clear();
            continueRead(request);
        }

        @Override
        public void onSucceeded(UrlRequest request, UrlResponseInfo info) {
            finish();
            mInfo.compareAndSet(null, info);
            long received = mStream.received();
            int status = info == null ? -1 : info.getHttpStatusCode();
            if (bodyLengthMismatch(status, received)) {
                Log.w(TAG, "Body is " + received + " bytes, Content-Length is "
                        + mContentLength + " for " + mUrl);
                mStream.fail(new IOException("Body is " + received
                        + " bytes, Content-Length is " + mContentLength + " for " + mUrl));
            } else {
                Log.d(TAG, "Body " + received + " bytes " + mUrl);
                mStream.finish();
            }
            signalHeaders();
        }

        @Override
        public void onFailed(UrlRequest request, UrlResponseInfo info, CronetException error) {
            finish();
            mError.compareAndSet(null, error);
            mStream.fail(new IOException("Cronet request failed for " + mUrl, error));
            signalHeaders();
        }

        /**
         * An uncompressed body with a Content-Length has to arrive whole.
         * A short read used to look like a normal end, so the player kept a
         * truncated file. HEAD, redirects and empty statuses have no body to
         * compare, and a decoded body is not the encoded length.
         */
        private boolean bodyLengthMismatch(int status, long received) {
            if (mRedirect || mEncodedBody || mContentLength < 0) {
                return false;
            }
            if (mMethod != null && mMethod.equalsIgnoreCase("HEAD")) {
                return false;
            }
            if (status == 204 || status == 205 || status == 304) {
                return false;
            }
            return received != mContentLength;
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
            int previous = mState.get();
            boolean timedOut = previous == STATE_TIMED_OUT;
            finish();
            synchronized (mStream) {
                if (mStream.mClosed || mRedirect || previous == STATE_DONE) {
                    signalHeaders();
                    return;
                }
            }
            IOException failure = timedOut
                    ? new IOException("Timed out reading body from " + mUrl)
                    : new IOException("Cronet request canceled");
            mError.compareAndSet(null, failure);
            mStream.fail(failure);
            signalHeaders();
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

    private static boolean encodedBody(Map<String, List<String>> headers) {
        for (Map.Entry<String, List<String>> entry : headers.entrySet()) {
            if (entry.getKey() == null || entry.getValue() == null
                    || !entry.getKey().equalsIgnoreCase("Content-Encoding")) {
                continue;
            }
            for (String value : entry.getValue()) {
                if (value == null) {
                    continue;
                }
                for (String token : value.split(",")) {
                    String coding = token.trim();
                    if (coding.equalsIgnoreCase("gzip")
                            || coding.equalsIgnoreCase("x-gzip")
                            || coding.equalsIgnoreCase("deflate")
                            || coding.equalsIgnoreCase("br")) {
                        return true;
                    }
                }
            }
        }
        return false;
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

}
