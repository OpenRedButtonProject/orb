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

import org.orbtv.orbpolyfill.BridgeToken;

import org.orbtv.orbservice.IDvbBrokerSession;
import org.orbtv.orbservice.IDvbClientSession;

class OrbDvbBroker
{
    private static final String TAG = "OrbDvbBroker";

    private static final String ORB_PACKAGE_NAME = "org.orbtv.orbservice";
    private static final String MOD_CLASS_NAME = "org.orbtv.orbservice.DvbBrokerService";

    private IDvbBrokerSession mDvbBroker= null;
    private Bridge mBridge;

    public OrbDvbBroker(Context context, Bridge bridge)
    {
        mBridge = bridge;
        ComponentName component = new ComponentName(ORB_PACKAGE_NAME, MOD_CLASS_NAME);
        Intent intent = new Intent().setComponent(component);
        Log.d(TAG, "Try to resolve Orb Moderator service intent: " +
                context.getPackageManager().queryIntentServices(intent, 0));
        context.bindService(intent, mOrbConnection, Context.BIND_AUTO_CREATE);
    }

    private final ServiceConnection mOrbConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName componentName, IBinder binder) {
            Log.d(TAG, "Orb service moderator connected");
            mDvbBroker = IDvbBrokerSession.Stub.asInterface(binder);
            try {
                mDvbBroker.initialise(new DvbClientSession());
            } catch (RemoteException e) {
                Log.e(TAG, "IDvbBrokerSession.initialise failed: " + e.getMessage());
            }
        }

        @Override
        public void onServiceDisconnected(ComponentName componentName) {
            Log.e(TAG, "ORB service moderator disconnected");
        }
    };

    /**
     * Requests the HbbTV engine to process the specified AIT. The HbbTV engine expects the relevant
     * AITs only (the first one after HBBTV_Start and when the version/PID changes). If more than one
     * stream is signalled in the PMT for a service with an application_signalling_descriptor, then
     * the application_signalling_descriptor for the stream containing the AIT for the HbbTV
     * application shall include the HbbTV application_type (0x0010).
     *
     * @param aitPid    PID of the AIT
     * @param serviceId Service ID the AIT refers to
     * @param data      The buffer containing the AIT row data
     */
    void processAitSection(int aitPid, int serviceId, byte[] data)
    {
        if (mDvbBroker != null)
        {
            try {
                mDvbBroker.processAitSection(aitPid, serviceId, data);
            } catch (RemoteException e) {
                Log.e(TAG, "mDvbBroker.processAitSection failed: " + e.getMessage());
            }
        }
    }


    private class DvbClientSession extends IDvbClientSession.Stub
    {
        DvbClientSession()
        {
        }

        /**
         * Execute the given bridge request.
         * The request is a string representation of a JSON object with the following form:
         * {
         *    "token": <token>
         *    "method": <method>
         *    "params": <params>
         * }
         * The response is also a string representation of a JSON object containing the results, if any.
         * @param jsonRequest byte array as a string representation of the JSON request
         * @return A byte array as a string representation of the JSON response
         */
        @Override
        public byte[] executeRequest(byte[] jsonRequest)
        {
            String json = new String(jsonRequest, java.nio.charset.StandardCharsets.ISO_8859_1);
            String result;
            try {
                JSONObject object = new JSONObject(json);
                String method = object.getString("method");
                JSONObject params = object.getJSONObject("params");
                BridgeToken token = new BridgeToken(object.getJSONObject("token"));

                Log.i(TAG, "Bridge request: " + method + "(" + params.toString() + ")");

                // TODO: uncomment next line when ready to fully
                // route bridge requests through ORB Broker/Moderator
                //result = mBridge.request(method, token, params).toString();
                result = "{\"error\": \"Request unsupported\"}";
            } catch (Exception e) {
                e.printStackTrace();
                result = "{\"error\": \"Request error\"}";
            }
            return result.getBytes(java.nio.charset.StandardCharsets.ISO_8859_1);
        }
    }

}
