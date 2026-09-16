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

import android.content.Context;
import android.os.Handler;
import android.os.Looper;
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

    private static final String XML_AIT_MIME = "application/vnd.dvb.ait+xml";
    /** Must match ApplicationManager::XML_AIT_FETCH_ASYNC. */
    private static final String XML_AIT_FETCH_ASYNC = "\u001eORB_XML_AIT_ASYNC";

    private final Object mLock = new Object();
    private SessionCallback mSessionCallback;
    private final IOrbSessionCallback mOrbLibraryCallback;
    private final XmlAitWebFetcher mXmlAitFetcher;
    private final Handler mMainHandler = new Handler(Looper.getMainLooper());

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

        /**
         * Returns true if the provided triplet is in an instance within the
         * currently playing service, otherwise false.
         */
        boolean isInstanceInCurrentService(int onid, int tsid, int sid);
    }

    ApplicationManager(final IOrbSessionCallback orbLibraryCallback, Context context,
            String userAgent) {
        jniInitialize(this);
        mOrbLibraryCallback = orbLibraryCallback;
        mXmlAitFetcher = new XmlAitWebFetcher(context, userAgent);
    }

    int getOrbHbbTVVersion() {
        return jniGetOrbHbbTVVersion();
    }

    void attachXmlAitFetcher(android.view.ViewGroup host) {
        mXmlAitFetcher.attachHost(host);
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
        // Notify only when JNI killed a running type 1.2 app (skip-autostart).
        // Early-out or EXIT (callingAppId 0) must not discard the next instance.
        if (jniDestroyApplication(callingAppId)) {
            Log.i(TAG, "LA 1.2 destroyApplication(); requesting DVB-I instance discard");
            mOrbLibraryCallback.onLinkedApplication12ExplicitlyExited();
        }
    }

    /**
     * Kill the running application without starting the broadcast autostart app.
     * Used for HbbTV O.3 parental block of linked application 1.2.
     */
    public void killForParentalControl() {
        jniKillForParentalControl();
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

    public String getApplicationHowRelatedHref(int appId) {
        return jniGetApplicationHowRelatedHref(appId);
    }

    public void setApplicationHowRelatedHref(String href) {
        jniSetApplicationHowRelatedHref(href);
    }

    public void onNetworkAvailabilityChanged(boolean available) {
        jniOnNetworkAvailabilityChanged(available);
    }

    public void onLoadApplicationFailed(int appId) {
        jniOnLoadApplicationFailed(appId);
    }

    /**
     * @return true if a DVB-I linked app was killed and not re-started (limit reached).
     */
    public boolean onApplicationIrrecoverableError(int appId) {
        return jniOnApplicationIrrecoverableError(appId);
    }

    public void onApplicationPageChanged(int appId, String url) {
        jniOnApplicationPageChanged(appId, url);
    }

    public void onApplicationPresented(int appId) {
        jniOnApplicationPresented(appId);
    }

    public void onBroadcastStopped() {
        jniOnBroadcastStopped();
    }

    public void onChannelChanged(int onetId, int transId, int servId, boolean isDvbi) {
        onChannelChanged(onetId, transId, servId, isDvbi, false);
    }

    public void onChannelChanged(int onetId, int transId, int servId, boolean isDvbi,
            boolean useBroadcastAit) {
        jniOnChannelChange(onetId, transId, servId, isDvbi, useBroadcastAit);
    }

    public void close() {
        mXmlAitFetcher.close();
        jniFinalize();
    }

    // Native interface
    private long mJniManagerPointerField; // Reserved for native library

    private native void jniInitialize(ApplicationManager applicationManager);

    private native int jniGetOrbHbbTVVersion();

    private native void jniFinalize();

    private native boolean jniCreateApplication(int callingAppId, String url);

    private native void jniContinueCreateFromHttpLocator(String url, String xmlAit);

    private native boolean jniDestroyApplication(int callingAppId);

    private native void jniKillForParentalControl();

    private native void jniShowApplication(int callingAppId);

    private native void jniHideApplication(int callingAppId);

    private native int jniSetKeySetMask(int appId, int keySetMask, int[] otherKeys);

    private native int jniGetKeySetMask(int appId);

    private native int[] jniGetOtherKeyValues(int appId);

    private native String jniGetApplicationScheme(int appId);

    private native String jniGetApplicationHowRelatedHref(int appId);

    private native void jniSetApplicationHowRelatedHref(String href);

    private native boolean jniInKeySet(int appId, int keyCode);

    private native void jniProcessAitSection(int aitPid, int serviceId, byte[] data);

    private native boolean jniProcessXmlAit(String data, boolean isDvbi, String scheme);

    private native boolean jniIsTeletextApplicationSignalled();

    private native boolean jniRunTeletextApplication();

    private native void jniOnNetworkAvailabilityChanged(boolean available);

    private native void jniOnLoadApplicationFailed(int appId);

    private native boolean jniOnApplicationIrrecoverableError(int appId);

    private native void jniOnApplicationPageChanged(int appId, String url);

    private native void jniOnApplicationPresented(int appId);

    private native void jniOnBroadcastStopped();

    private native void jniOnChannelChange(int onetId, int transId, int servId, boolean isDvbi,
            boolean useBroadcastAit);

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
        mXmlAitFetcher.fetchAsync(url, result -> onXmlAitFetchDone(url, result));
        return XML_AIT_FETCH_ASYNC;
    }

    private void onXmlAitFetchDone(String url, XmlAitWebFetcher.Result uaResult) {
        if (uaResult != null && uaResult.networkOk) {
            Log.i(TAG, "XML AIT fetched via HTML UA type=" + uaResult.contentType
                    + " bytes=" + uaResult.body.length() + " url=" + url);
            jniContinueCreateFromHttpLocator(url,
                    xmlAitBodyOrEmpty(uaResult.contentType, uaResult.body));
            return;
        }
        Log.w(TAG, "XML AIT HTML UA fetch failed, falling back to OkHttp: " + url);
        new Thread(() -> {
            String body = fetchXmlAitWithOkHttp(url);
            mMainHandler.post(() -> jniContinueCreateFromHttpLocator(url, body));
        }, "xml-ait-okhttp").start();
    }

    private static String xmlAitBodyOrEmpty(String contentType, String body) {
        if (contentType == null || !contentType.startsWith(XML_AIT_MIME)) {
            return "";
        }
        return body != null ? body : "";
    }

    private String fetchXmlAitWithOkHttp(String url) {
        OkHttpClient okClient = new OkHttpClient();
        Request okRequest = new Request.Builder()
                .url(url)
                .build();
        try {
            Response okResponse = okClient.newCall(okRequest).execute();
            String contentType = okResponse.header("Content-Type");
            if (!okResponse.isSuccessful()) {
                return "";
            }
            return xmlAitBodyOrEmpty(contentType, okResponse.body() != null
                    ? okResponse.body().string() : "");
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

    private boolean jniCbisInstanceInCurrentService(int onid, int tsid, int sid) {
        synchronized (mLock) {
            if (mSessionCallback != null) {
                return mSessionCallback.isInstanceInCurrentService(onid, tsid, sid);
            } else {
                Log.e(TAG, "Presentation listener not set.");
            }
        }
        return false;
    }
}
