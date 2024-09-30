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
import java.util.List;
import java.util.Map;
import java.util.HashMap;

import okhttp3.*;

class ApplicationManager {
    private static final String TAG = ApplicationManager.class.getSimpleName();

    private final static String NOT_STARTED_URL = "about:blank";

    private final static int KEY_SET_RED = 0x1;
    private final static int KEY_SET_GREEN = 0x2;
    private final static int KEY_SET_YELLOW = 0x4;
    private final static int KET_SET_BLUE = 0x8;
    private final static int KEY_SET_NAVIGATION = 0x10;
    private final static int KEY_SET_VCR = 0x20;
    private final static int KEY_SET_NUMERIC = 0x100;
    private final static Map<String, Integer> KEY_OTHERS = new HashMap<String, Integer>() {{
        put("VK_RECORD", 0x416);
    }};

    private final Object mLock = new Object();
    private SessionCallback mSessionCallback;
    private final IOrbSessionCallback mOrbLibraryCallback;

    private String m_entryUrl = NOT_STARTED_URL;

    interface SessionCallback {
        /**
         * Tell the browser to load an application. If the entry page fails to load, the browser
         * should call ApplicationManager.onLoadApplicationFailed.
         *
         * @param appId The application ID.
         * @param entryUrl The entry page URL.
         *
         * @since 204
         * @param graphics The list of the co-ordinate graphics supported by the application
         */
        void loadApplication(int appId, String entryUrl, int[] graphics);

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

        void dispatchApplicationSchemeUpdatedEvent(String scheme);

        /**
         * Notify that the active key set and optional other keys are changed.
         *
         * @param keySet Key set (a bitwise mask of constants, as defined by HbbTV).
         * @param otherKeys Optional other keys.
         */
        void notifyKeySetChanged(int keySet, int[] otherKeys);

        /**
         * Notify that the application status is changed.
         *
         * @param status The application status.
         */
        void notifyApplicationStatusChanged(IOrbSessionCallback.ApplicationStatus status);
    }

    ApplicationManager(final IOrbSessionCallback orbLibraryCallback) {
        jniInitialize(this);
        mOrbLibraryCallback = orbLibraryCallback;
    }

    int getOrbHbbTVVersion() {
        return jniGetOrbHbbTVVersion();
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

    public boolean processXmlAit(String str, boolean isDvbi, String scheme) {
        boolean result = jniProcessXmlAit(str, isDvbi, scheme);
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

    public int[] getOtherKeyValues(int appId) {
        return jniGetOtherKeyValues(appId);
    }

    public int getKeyValues(int appId) {
        return jniGetKeySetMask(appId);
    }

    public int getKeyMaximumOtherKeys() {
        int result = 0;
        for (Integer value : KEY_OTHERS.values()) {
            result |= value;
        }
        return result;
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

    public int setKeyValue(int appId, int value, List<String> otherKeysList) {
        synchronized (mLock) {
            int[] otherKeysArray;
            int kMask = 0;
            if (mSessionCallback != null) {
                if (otherKeysList == null || otherKeysList.isEmpty()) {
                    otherKeysArray = new int[0];
                } else {
                    otherKeysArray = new int[otherKeysList.size()];
                    for (int i = 0; i < otherKeysList.size(); i++) {
                        try {
                            otherKeysArray[i] = Integer.parseInt(otherKeysList.get(i));
                        } catch (NumberFormatException e) {
                            continue;
                        }
                    }
                }
                kMask = jniSetKeySetMask(appId, value, otherKeysArray);
                if (kMask > 0)
                {
                    mSessionCallback.notifyKeySetChanged(value, otherKeysArray);
                }
            }
            return kMask;
        }
    }

    public String getApplicationScheme(int appId) {
        return jniGetApplicationScheme(appId);
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

    private native int jniGetOrbHbbTVVersion();

    private native void jniFinalize();

    private native boolean jniCreateApplication(int callingAppId, String url);

    private native void jniDestroyApplication(int callingAppId);

    private native void jniShowApplication(int callingAppId);

    private native void jniHideApplication(int callingAppId);

    private native int jniSetKeySetMask(int appId, int keySetMask, int[] otherKeys);

    private native int jniGetKeySetMask(int appId);

    private native int[] jniGetOtherKeyValues(int appId);

    private native String jniGetApplicationScheme(int appId);

    private native boolean jniInKeySet(int appId, int keyCode);

    private native void jniProcessAitSection(int aitPid, int serviceId, byte[] data);

    private native boolean jniProcessXmlAit(String data, boolean isDvbi, String scheme);

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
                int[] otherKeys = new int[0];
                mSessionCallback.notifyKeySetChanged(0, otherKeys);
                mSessionCallback.resetBroadcastPresentation();
            } else {
                Log.e(TAG, "Presentation listener not set.");
            }
        }
    }

    private void jniCbLoadApplication(int appId, String entryUrl, int[] graphics) {
        synchronized (mLock) {
            m_entryUrl = entryUrl;
            if (mSessionCallback != null) {
                mSessionCallback.resetBroadcastPresentation();
                mSessionCallback.loadApplication(appId, entryUrl, graphics);
                if (m_entryUrl.equals(NOT_STARTED_URL)) {
                    mSessionCallback.notifyApplicationStatusChanged(
                            IOrbSessionCallback.ApplicationStatus.NOT_STARTED);
                } else {
                    // jniCbLoadApplication is called before jniCbShowApplication
                    mSessionCallback.notifyApplicationStatusChanged(
                            IOrbSessionCallback.ApplicationStatus.INVISIBLE);
                }
            } else {
                Log.e(TAG, "Presentation listener not set.");
            }
        }
    }

    private void jniCbShowApplication() {
        synchronized (mLock) {
            if (mSessionCallback != null) {
                mSessionCallback.showApplication();
                if (!m_entryUrl.equals(NOT_STARTED_URL)) {
                    mSessionCallback.notifyApplicationStatusChanged(
                            IOrbSessionCallback.ApplicationStatus.VISIBLE);
                }
            } else {
                Log.e(TAG, "Presentation listener not set.");
            }
        }
    }

    private void jniCbHideApplication() {
        synchronized (mLock) {
            if (mSessionCallback != null) {
                mSessionCallback.hideApplication();
                if (!m_entryUrl.equals(NOT_STARTED_URL)) {
                    mSessionCallback.notifyApplicationStatusChanged(
                            IOrbSessionCallback.ApplicationStatus.INVISIBLE);
                }
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

    private void jniCbonApplicationSchemeUpdated(String scheme) {
        synchronized (mLock) {
            if (mSessionCallback != null) {
                mSessionCallback.dispatchApplicationSchemeUpdatedEvent(scheme);
            } else {
                Log.e(TAG, "Presentation listener not set.");
            }
        }
    }
}
