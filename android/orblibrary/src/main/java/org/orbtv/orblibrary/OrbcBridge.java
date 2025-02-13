package org.orbtv.orblibrary;


import android.content.Context;
import android.content.ComponentName;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.IBinder;
import android.os.RemoteException;
import android.provider.Settings;
import android.util.Log;

import org.json.JSONException;
import org.json.JSONObject;

import org.orbtv.orbservice.IBridgeSession;
import org.orbtv.orbservice.IBrowserSession;

class OrbcBridge
{
    private static final String TAG = "OrbcBridge";

    private static final String ORB_PACKAGE_NAME = "org.orbtv.orbservice";
    private static final String BRIDGE_CLASS_NAME = "org.orbtv.orbservice.BridgeService";

    private IBridgeSession mBridgeSession = null;
    private final BrowserView mBrowserView;

    public OrbcBridge(Context context, BrowserView view)
    {
        mBrowserView = view;
        ComponentName component = new ComponentName(ORB_PACKAGE_NAME, BRIDGE_CLASS_NAME);
        Intent intent = new Intent().setComponent(component);
        Log.d(TAG, "Try to resolve Orb Bridge service intent: " +
                context.getPackageManager().queryIntentServices(intent, 0));
        context.bindService(intent, mBridgeConnection, Context.BIND_AUTO_CREATE);
    }

    private final ServiceConnection mBridgeConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName componentName, IBinder binder) {
            Log.d(TAG, "Bridge service connected");
            mBridgeSession = IBridgeSession.Stub.asInterface(binder);
            try {
                mBridgeSession.initialise(new OrbcBrowserSession());
            } catch (RemoteException e) {
                Log.e(TAG, "IBridgeSession.initialise failed: " + e.getMessage());
            }
        }

        @Override
        public void onServiceDisconnected(ComponentName componentName) {
            Log.e(TAG, "ORB service disconnected");
        }
    };

    public String executeRequest(String request)
    {
        if (mBridgeSession != null)
        {
            try {
                byte[] result = mBridgeSession.executeRequest(request.getBytes(java.nio.charset.StandardCharsets.ISO_8859_1));
                return new String(result, java.nio.charset.StandardCharsets.ISO_8859_1);
            } catch (RemoteException e) {
                Log.e(TAG, "IBridgeSession.executeRequest failed: " + e.getMessage());
            }
        }
        return "{\"error\": \"Request error\"}";
    }

    private class OrbcBrowserSession extends IBrowserSession.Stub
    {
        OrbcBrowserSession()
        {
        }

        @Override
        public boolean dispatchKeyEvent(int action, int android_code, int tv_code)
        {
            Log.d(TAG, "dispatchKeyEvent; action: " + action + ", acode: " + android_code + ", tcode: " + tv_code);
            return false;
        }

        @Override
        public void loadApplication(int app_id, byte[] url, int[] graphic_ids)
        {
            String urlstr = new String(url, java.nio.charset.StandardCharsets.US_ASCII);
            Log.d(TAG, "loadApplication; app_id: " + app_id + ", url: " + urlstr);
            mBrowserView.loadApplication(app_id, urlstr, graphic_ids);
        }

        @Override
        public void showApplication()
        {
            Log.d(TAG, "showApplication()");
            mBrowserView.showApplication();
        }

        @Override
        public void hideApplication()
        {
            Log.d(TAG, "hideApplication()");
            mBrowserView.hideApplication();
        }

        @Override
        public void dispatchEvent(byte[] type, byte[] properties)
        {
            String stype = new String(type, java.nio.charset.StandardCharsets.ISO_8859_1);
            String sprop = new String(properties, java.nio.charset.StandardCharsets.ISO_8859_1);
            Log.d(TAG, "");
            try {
                mBrowserView.dispatchEvent(stype, new JSONObject(sprop));
            } catch (JSONException e) {
                Log.e(TAG, "dispatchEvent failed " + e.getMessage());
            }
        }

        @Override
        public void dispatchTextInput(byte[] text)
        {
            String txt = new String(text, java.nio.charset.StandardCharsets.ISO_8859_1);
            Log.d(TAG, "");
        }

        @Override
        public void receiveDsmccContent(int requestId, byte[] content)
        {

        }
    }

}
