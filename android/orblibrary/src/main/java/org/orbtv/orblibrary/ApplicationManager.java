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

import okhttp3.*;

class ApplicationManager {
    private static final String TAG = ApplicationManager.class.getSimpleName();

    private final static int KEY_SET_RED = 0x1;
    private final static int KEY_SET_GREEN = 0x2;
    private final static int KEY_SET_YELLOW = 0x4;
    private final static int KET_SET_BLUE = 0x8;
    private final static int KEY_SET_NAVIGATION = 0x10;
    private final static int KEY_SET_VCR = 0x20;
    private final static int KEY_SET_NUMERIC = 0x100;

    private final Object mLock = new Object();
    private SessionCallback mSessionCallback;
    private final IOrbSessionCallback mOrbLibraryCallback;

    interface SessionCallback {
        /**
         * Tell the browser to load an application. If the entry page fails to load, the browser
         * should call ApplicationManager.onLoadApplicationFailed.
         *
         * @param appId The application ID.
         * @param entryUrl The entry page URL.
         */
        void loadApplication(int appId, String entryUrl);

        /**
         * Tell the browser to show the application.
         */
        void showApplication();

        /**
         * Tell the browser to hide the application.
         */
        void hideApplication();

        /**
         * Tell the broadcast-integration to stop presenting any broadcast component, equivalent to
         * selecting a null service.
         */
        void stopBroadcast();

        /**
         * Tell the broadcast-integration to reset any calls by HbbTV to suspend presentation, set
         * the video rectangle or set the presented components.
         */
        void resetBroadcastPresentation();

        /**
         * Tell the bridge to dispatch a ApplicationLoadError event.
         */
        void dispatchApplicationLoadErrorEvent();

        /**
         * Tell the bridge to dispatch a TransitionedToBroadcastRelated event.
         */
        void dispatchTransitionedToBroadcastRelatedEvent();
    }

    ApplicationManager(final IOrbSessionCallback orbLibraryCallback) {
        jniInitialize(this);
        mOrbLibraryCallback = orbLibraryCallback;
    }

    public void setSessionCallback(SessionCallback sessionCallback) {
        synchronized (mLock) {
            mSessionCallback = sessionCallback;
        }
    }

    public boolean createApplication(String url) {
        return jniCreateApplication(0, url);
    }

    public boolean createApplication(int callingAppId, String url) {
        return jniCreateApplication(callingAppId, url);
    }

    public void destroyApplication(int callingAppId) {
        jniDestroyApplication(callingAppId);
    }

    public boolean processAitSection(int aitPid, int serviceId, byte[] data) {
        jniProcessAitSection(aitPid, serviceId, data);
        return true;
    }

    public boolean processXmlAit(String str) {
        boolean result = jniProcessXmlAit(str);
        return result;
    }

    public boolean isTeletextApplicationSignalled() {
        return jniIsTeletextApplicationSignalled();
    }

    public boolean runTeletextApplication() {
        return jniRunTeletextApplication();
    }

    public void showApplication(int callingAppId) {
        jniShowApplication(callingAppId);
    }

    public void hideApplication(int callingAppId) {
        jniHideApplication(callingAppId);
    }

    public boolean isRequestAllowed(int callingAppId, String callingPageUrl, int methodRequirement) {
        return jniIsRequestAllowed(callingAppId, callingPageUrl, methodRequirement);
    }

    public int getKeyValues(int appId) {
        return jniGetKeySetMask(appId);
    }

    public int getKeyMaximumValue() {
        return KEY_SET_RED |
                KEY_SET_GREEN |
                KEY_SET_YELLOW |
                KET_SET_BLUE |
                KEY_SET_NAVIGATION |
                KEY_SET_VCR |
                KEY_SET_NUMERIC;
    }

    public boolean inKeySet(int appId, int keyCode) {
        return jniInKeySet(appId, keyCode);
    }

    public int setKeyValue(int appId, int value) {
        return jniSetKeySetMask(appId, value);
    }

    public void onNetworkAvailabilityChanged(boolean available) {
        jniOnNetworkAvailabilityChanged(available);
    }

    public void onLoadApplicationFailed(int appId) {
        jniOnLoadApplicationFailed(appId);
    }

    public void onApplicationPageChanged(int appId, String url) {
        jniOnApplicationPageChanged(appId, url);
    }

    public void onBroadcastStopped() {
        jniOnBroadcastStopped();
    }

    public void onChannelChanged(int onetId, int transId, int servId) {
        jniOnChannelChange(onetId, transId, servId);
    }

    public void close() {
        jniFinalize();
    }

    // Native interface
    private long mJniManagerPointerField; // Reserved for native library

    private native void jniInitialize(ApplicationManager applicationManager);

    private native void jniFinalize();

    private native boolean jniCreateApplication(int callingAppId, String url);

    private native void jniDestroyApplication(int callingAppId);

    private native void jniShowApplication(int callingAppId);

    private native void jniHideApplication(int callingAppId);

    private native int jniSetKeySetMask(int appId, int keySetMask);

    private native int jniGetKeySetMask(int appId);

    private native boolean jniInKeySet(int appId, int keyCode);

    private native void jniProcessAitSection(int aitPid, int serviceId, byte[] data);

    private native boolean jniProcessXmlAit(String data);

    private native boolean jniIsTeletextApplicationSignalled();

    private native boolean jniRunTeletextApplication();

    private native void jniOnNetworkAvailabilityChanged(boolean available);

    private native void jniOnLoadApplicationFailed(int appId);

    private native void jniOnApplicationPageChanged(int appId, String url);

    private native void jniOnBroadcastStopped();

    private native void jniOnChannelChange(int onetId, int transId, int servId);

    private native boolean jniIsRequestAllowed(int callingAppId, String callingPageUrl, int methodRequirement);

    private void jniCbStopBroadcast() {
        synchronized (mLock) {
            if (mSessionCallback != null) {
                mSessionCallback.stopBroadcast();
            } else {
                Log.e(TAG, "Presentation listener not set.");
            }
        }
    }

    private void jniCbResetBroadcastPresentation() {
        synchronized (mLock) {
            if (mSessionCallback != null) {
                mSessionCallback.resetBroadcastPresentation();
            } else {
                Log.e(TAG, "Presentation listener not set.");
            }
        }
    }

    private void jniCbLoadApplication(int appId, String entryUrl) {
        synchronized (mLock) {
            if (mSessionCallback != null) {
                mSessionCallback.resetBroadcastPresentation();
                mSessionCallback.loadApplication(appId, entryUrl);
            } else {
                Log.e(TAG, "Presentation listener not set.");
            }
        }
    }

    private void jniCbShowApplication() {
        synchronized (mLock) {
            if (mSessionCallback != null) {
                mSessionCallback.showApplication();
            } else {
                Log.e(TAG, "Presentation listener not set.");
            }
        }
    }

    private void jniCbHideApplication() {
        synchronized (mLock) {
            if (mSessionCallback != null) {
                mSessionCallback.hideApplication();
            } else {
                Log.e(TAG, "Presentation listener not set.");
            }
        }
    }

    private String jniCbGetXmlAitContents(String url) {
        OkHttpClient okClient = new OkHttpClient();
        Request okRequest = new Request.Builder()
                .url(url)
                .build();
        Response okResponse;
        try {
            okResponse = okClient.newCall(okRequest).execute();
            String contentType = okResponse.header("Content-Type");
            if (contentType == null || !contentType.startsWith("application/vnd.dvb.ait+xml")) {
                return "";
            }
            if (!okResponse.isSuccessful()) {
                return "";
            }
            String contents = okResponse.body().string();
            return contents;
        } catch (Exception e) {
            e.printStackTrace();
            return "";
        }
    }

    private void jniCbOnApplicationLoadError() {
        synchronized (mLock) {
            if (mSessionCallback != null) {
                mSessionCallback.dispatchApplicationLoadErrorEvent();
            } else {
                Log.e(TAG, "Presentation listener not set.");
            }
        }
    }

    private void jniCbOnTransitionedToBroadcastRelated() {
        synchronized (mLock) {
            if (mSessionCallback != null) {
                mSessionCallback.dispatchTransitionedToBroadcastRelatedEvent();
            } else {
                Log.e(TAG, "Presentation listener not set.");
            }
        }
    }

    private int jniCbonNativeGetParentalControlAge() {
        return (mOrbLibraryCallback != null) ? mOrbLibraryCallback.getParentalControlAge() : 0;
    }

    private String jniCbonNativeGetParentalControlRegion() {
        return (mOrbLibraryCallback != null) ? mOrbLibraryCallback.getParentalControlRegion() : "";
    }

    private String jniCbonNativeGetParentalControlRegion3() {
        return (mOrbLibraryCallback != null) ? mOrbLibraryCallback.getCountryId() : "";
    }
}

