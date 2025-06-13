package org.orbtv.orbsession;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.pm.ResolveInfo;
import android.content.ServiceConnection;
import android.util.Log;

import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Looper;
import android.os.RemoteException;
import android.view.SurfaceControlViewHost.SurfacePackage;
import android.view.SurfaceView;

import android.view.Display;
import android.view.View;
import android.view.KeyEvent;

import java.util.concurrent.Executor;
import java.util.concurrent.Executors;
import java.util.List;
import java.lang.UnsupportedOperationException;

import org.orbtv.orbservice.ApplicationType;
import org.orbtv.orbservice.IOrbService;
import org.orbtv.orbservice.IOrbClient;


public class OrbSession extends IOrbClient.Stub {

    private static final String TAG = "OrbSession";

    private static final String SERVICE_PACKAGE = "org.chromium.content_shell_apk";
    private static final String SERVICE_CLASS = "org.chromium.content_shell_apk.ContentShellService";


    private SurfaceView mSurfaceView;
    private IOrbService mRemoteService;
    private boolean isBound = false;

    private SurfacePackage mSurfacePackage;
    private Context mVisualContext;

    private interface SafeAidlCallbackDispatcher {
        public void runOnMain(Runnable task);
        public void runDeferred(Runnable task);
    }

    private final SafeAidlCallbackDispatcher dispatcher = new SafeAidlCallbackDispatcher() {

        private final Executor backgroundExecutor = Executors.newSingleThreadExecutor();
        private final Handler mainHandler = new Handler(Looper.getMainLooper());

        @Override
        public void runOnMain(Runnable task) {
            mainHandler.post(task);
        }

        @Override
        public void runDeferred(Runnable task) {
            backgroundExecutor.execute(() -> {
                try {
                    Thread.sleep(50); // Let the Binder stack unwind
                    task.run();
                } catch (InterruptedException e) {
                    Thread.currentThread().interrupt();
                }
            });
        }
    };

    private final ServiceConnection serviceConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName componentName, IBinder iBinder) {
            Log.d(TAG, "Service connected");
            isBound = true;
            mRemoteService = IOrbService.Stub.asInterface(iBinder);

            try {
                mRemoteService.setCallback(OrbSession.this, ApplicationType.APP_TYPE_HBBTV);
            } catch (RemoteException e) {
                Log.e(TAG, "Remote Service exception: " + e.getMessage());
            }
        }

        @Override
        public void onServiceDisconnected(ComponentName componentName) {
            Log.d(TAG, "Service disconnected");
            isBound = false;
        }
    };

    public interface Callback {
        void onOrbSessionReady(String result);
    }

    private final Callback mCallback;

    public OrbSession(Context visualContext, Callback callback, SurfaceView surfaceView) {
        mVisualContext = visualContext;
        mSurfaceView = surfaceView;
        mCallback = callback;
    }

    public void close() {
        if (isBound)
        {
            mVisualContext.unbindService(serviceConnection);
        }

        if (mSurfacePackage != null)
        {
            mSurfacePackage.release();
            mSurfacePackage = null;
        }
    }

    public void connectToService()
    {
        /* Connect to the service */
        Intent intent = new Intent();
        intent.setComponent(new ComponentName(SERVICE_PACKAGE, SERVICE_CLASS));

        List<ResolveInfo> resolveInfos = mVisualContext.getPackageManager().queryIntentServices(intent, 0);
        if (resolveInfos.isEmpty())
        {
            Log.e(TAG, "Failed to bind to service "
                    + SERVICE_CLASS + ". Not found (have you installed it?)");
            return;
        }
        mVisualContext.bindService(intent, serviceConnection, Context.BIND_AUTO_CREATE);
    }

    private void requestRemoteSurface() {

        if (!isBound) {
            Log.e(TAG, "Service not bound!");
            return;
        }

        if (mSurfacePackage != null) {
            mSurfacePackage.release();
            mSurfacePackage = null;
        }

        try {
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
                Display display = mVisualContext.getDisplay();
                if (display != null) {
                    int displayId = display.getDisplayId();
                    Bundle data = new Bundle();
                    data.putBinder(OrbSessionConstants.KEY_HOST_TOKEN, mSurfaceView.getHostToken());
                    data.putInt(OrbSessionConstants.KEY_DISPLAY_ID, displayId);
                    data.putInt(OrbSessionConstants.KEY_WIDTH, mSurfaceView.getWidth());
                    data.putInt(OrbSessionConstants.KEY_HEIGHT, mSurfaceView.getHeight());
                    // On successful connection calls IOrbClient.onSurfaceReady and attachSurfaceToView
                    mRemoteService.requestSurfacePackage(data);
                } else {
                    Log.e(TAG, "A valid Display reference was not found.");
                    return;
                }
            }
        } catch (RemoteException e) {
            Log.e(TAG, "Remote Service exception: " + e.getMessage());
        } catch (UnsupportedOperationException e) {
            Log.e(TAG, "UnsupportedOperationException: " + e.getMessage());
        }
    }

    private void attachSurfaceToView(SurfacePackage surfacePackage) {
        mSurfacePackage = surfacePackage;
        mSurfaceView.setChildSurfacePackage(mSurfacePackage);
        mSurfaceView.setVisibility(View.VISIBLE);
        Log.d(TAG, "Surface Package attached. Dimensions: ["
            + mSurfaceView.getWidth() + "x" + mSurfaceView.getHeight()  + "]");
    }

    public void loadUrl(String url) {
        if (url == null) return;
        try {
            /* Send the URL to the service to handle it */
            mRemoteService.loadUrl(url);
        } catch (RemoteException e)
        {
            Log.e(TAG, "Remote Service exception: " + e.getMessage());
        }
    }

    public boolean sendKeyEvent(int keyCode, KeyEvent event) {
        try {
            mRemoteService.sendKeyEvent(event);
        } catch (RemoteException e) {
            Log.e(TAG, "Remote Service exception: " + e.getMessage());
        }
        return true;
    }

    /**
     * Execute the given request.
     * The request is a string representation of a JSON object with the following form:
     * {
     *    "method": <method>
     *    "params": <params>
     * }
     * The response is also a string representation of a JSON object containing the results, if any.
     * @param jsonRequest String representation of the JSON request
     * @return String representation of the JSON response
     */
    @Override
    public String executeRequest(String jsonRequest)
    {
        // TODO: handle or pass request on
        Log.d(TAG, "Received JSON-RPC request " + jsonRequest);
        return "";
    }

    /**
     * Load DSM-CC file with specified DVB URL.
     * Content is returned IBrowserSession.receiveDsmccContent
     *
     * @param requestId The distinctive request id
     * @param url The DVB URL
     */
    @Override
    public void LoadDsmccDvbUrl(int requestId, String url)
    {
        Log.d(TAG, "Request DSM-CC content: rqst " + requestId + ", url " + url);
        // TODO: pass request to
    }

    @Override
    public void onSurfaceReady(SurfacePackage surfacePackage) {

        Log.d(TAG, "Received SurfacePackage!");

        // While we're probably already running on the main UI thread, this makes sure
        dispatcher.runOnMain(() -> {
            attachSurfaceToView(surfacePackage);

            // We want to avoid calling another AIDL method on an AIDL callback.
            dispatcher.runDeferred(() -> {
                mCallback.onOrbSessionReady(null);
            });
        });
    }

    @Override
    public void onServiceStarted() {
        // Once the service has fully started with some shells connect to
        // the remote UI.
        Log.d(TAG, "onServiceStarted callback. Requesting remote service....");
        requestRemoteSurface();
    }

    @Override
    public void onError(String message) {

    }
}
