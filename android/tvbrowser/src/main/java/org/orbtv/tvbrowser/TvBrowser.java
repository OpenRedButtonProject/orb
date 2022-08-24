/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 * <p>
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

package org.orbtv.tvbrowser;

import android.content.Context;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;
import android.view.View;

import org.json.JSONObject;

import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.List;
import java.util.UUID;

public class TvBrowser {

    /**
     * Create a TV browser session.
     *
     * @param context The context of the application.
     * @param callback The implementation of TvBrowserCallback to call.
     * @param configuration Configuration for this session
     * @return A TV browser session.
     */
    public static Session createSession(Context context, TvBrowserCallback callback,
                                        IDsmccEngine dsmcc, Configuration configuration) {
        Session session = new Session(context, callback, configuration);
        Handler handler = new Handler(context.getMainLooper());
        handler.post(() -> callback.onSessionReady(session));
        DsmccClient dsmClient = new DsmccClient(dsmcc, session);
        return session;
    }

    /**
     * Create a distinctive identifier. See HbbTV clause 12.1.5 for requirements.
     *
     * @param deviceUniqueValue A value unique to the device (e.g. serial number).
     * @param commonSecretValue A secret value common to the manufacturer, model or product family.
     * @param origin The origin of the HTML document (see HbbTV clause 6.3.2).
     * @return A distinctive identifier on success, null otherwise.
     */
    public static String createDistinctiveIdentifier(String deviceUniqueValue,
                                                     String commonSecretValue, String origin) {
        MessageDigest md;
        try {
            md = MessageDigest.getInstance("SHA-256");
        } catch (NoSuchAlgorithmException e) {
            e.printStackTrace();
            return null;
        }
        String changingValue = UUID.randomUUID().toString();
        String message = deviceUniqueValue + commonSecretValue + origin + changingValue;
        StringBuilder sb = new StringBuilder();
        for (byte b : md.digest(message.getBytes())) {
            sb.append(Integer.toHexString((b & 0xFF) | 0x100).substring(1, 3));
        }
        return sb.toString();
    }

    public static class Configuration {
        public final int mediaSyncWcPort;
        public final int mediaSyncCiiPort;
        public final int mediaSyncTsPort;
        public final int app2appLocalPort;
        public final int app2appRemotePort;
        public final String mainActivityUuid;
        public final String userAgent;
        public final String sansSerifFontFamily;
        public final String fixedFontFamily;

        public Configuration(int mediaSyncWcPort, int mediaSyncCiiPort, int mediaSyncTsPort,
                             int app2appLocalPort, int app2appRemotePort, String mainActivityUuid, String userAgent,
                             String sansSerifFontFamily, String fixedFontFamily) {
            this.mediaSyncWcPort = mediaSyncWcPort;
            this.mediaSyncCiiPort = mediaSyncCiiPort;
            this.mediaSyncTsPort = mediaSyncTsPort;
            this.app2appLocalPort = app2appLocalPort;
            this.app2appRemotePort = app2appRemotePort;
            this.mainActivityUuid = mainActivityUuid;
            this.userAgent = userAgent;
            this.sansSerifFontFamily = sansSerifFontFamily;
            this.fixedFontFamily = fixedFontFamily;
        }
    }

    public static class Session {
        private static final String TAG = "Orb/Session";

        private final TvBrowserCallback mTvBrowserCallback;
        private ApplicationManager mApplicationManager;
        private Configuration mConfiguration;
        private MediaSynchroniserManager mMediaSynchroniserManager;
        private Bridge mBridge;
        private BrowserView mBrowserView;

        /**
         * TV browser session.
         *
         * @param context The context of the application.
         * @param tvBrowserCallback The implementation of TvBrowserCallback to call.
         */
        public Session(Context context, final TvBrowserCallback tvBrowserCallback, Configuration configuration) {

            mTvBrowserCallback = tvBrowserCallback;
            mConfiguration = configuration;
            mApplicationManager = new ApplicationManager(mTvBrowserCallback);
            mMediaSynchroniserManager = new MediaSynchroniserManager(configuration);
            mBridge = new Bridge(this, tvBrowserCallback, configuration, mApplicationManager, mMediaSynchroniserManager);
            mBrowserView = new BrowserView(context, mBridge, configuration);

            mApplicationManager.setSessionCallback(new ApplicationManager.SessionCallback() {
                /**
                 * Tell the browser to load an application. If the entry page fails to load, the browser
                 * should call ApplicationManager.onLoadApplicationFailed.
                 *
                 * @param appId The application ID.
                 * @param entryUrl The entry page URL.
                 */
                @Override
                public void loadApplication(int appId, String entryUrl) {
                    mBrowserView.loadApplication(appId, entryUrl);
                }

                /**
                 * Tell the browser to show the application.
                 */
                @Override
                public void showApplication() {
                    mBrowserView.showApplication();
                }

                /**
                 * Tell the browser to hide the application.
                 */
                @Override
                public void hideApplication() {
                    mBrowserView.hideApplication();
                }

                /**
                 * Tell the broadcast-integration to stop presenting any broadcast component, equivalent to
                 * selecting a null service.
                 */
                @Override
                public void stopBroadcast() {
                    // TODO Make this an explicit method
                    mTvBrowserCallback.setChannelByTriplet(-1, -1, -1, -1, -1, null, false, "", 0);
                }

                /**
                 * Tell the broadcast-integration to reset any calls by HbbTV to suspend presentation,
                 * set the video rectangle or set the presented components.
                 */
                public void resetBroadcastPresentation() {
                    // TODO Make this one call in the API
                    mTvBrowserCallback.setDvbVideoRectangle(0, 0, 1280, 720, false);
                    mTvBrowserCallback.setPresentationSuspended(false);
                }

                /**
                 * Tell the bridge to dispatch a ApplicationLoadError event.
                 */
                @Override
                public void dispatchApplicationLoadErrorEvent() {
                    mBridge.dispatchApplicationLoadErrorEvent();
                }

                /**
                 * Tell the bridge to dispatch a TransitionedToBroadcastRelated event to the polyfill.
                 */
                @Override
                public void dispatchTransitionedToBroadcastRelatedEvent() {
                    mBridge.dispatchTransitionedToBroadcastRelatedEvent();
                }
            });

            mBrowserView.setSessionCallback(new BrowserView.SessionCallback() {
                /**
                 * Get the TvBrowser key code for the Android key code.
                 *
                 * @param androidKeyCode The Android key code (KeyEvent.KEYCODE_*)
                 * @return A TvBrowserTypes.VK_* key code.
                 */
                @Override
                public int getTvBrowserKeyCode(int androidKeyCode) {
                    return mTvBrowserCallback.getTvBrowserKeyCode(androidKeyCode);
                }

                /**
                 * Check whether the key code is in the key set of the application.
                 *
                 * @param appId The application ID.
                 * @param keyCode A TvBrowserTypes.VK_* key code.
                 * @return true if it is in the key set, otherwise false
                 */
                @Override
                public boolean inApplicationKeySet(int appId, int keyCode) {
                    return mApplicationManager.inKeySet(appId, keyCode);
                }

                /**
                 * Notify the application manager that a call to loadApplication failed.
                 *
                 * @param appId The application ID of the application that failed to load.
                 */
                @Override
                public void notifyLoadApplicationFailed(int appId) {
                    mApplicationManager.onLoadApplicationFailed(appId);
                }

                /**
                 * Notify the application manager of application page changed, before the new page is
                 * loaded. For example, when the user follows a link.
                 *
                 * @param appId The application ID.
                 * @param url The URL of the new page.
                 */
                @Override
                public void notifyApplicationPageChanged(int appId, String url) {
                    mApplicationManager.onApplicationPageChanged(appId, url);
                }
            });

            mBridge.setSessionCallback(new Bridge.SessionCallback() {
                @Override
                public boolean isRequestAllowed(int callingAppId, String callingPageUrl, int methodRequirement) {
                    return mApplicationManager.isRequestAllowed(callingAppId, callingPageUrl, methodRequirement);
                }

                /**
                 * Tell the browser to dispatch an event to the polyfill.
                 *
                 * @param type The event type.
                 * @param properties A name/value map of event properties in a JSON object.
                 */
                @Override
                public void dispatchEvent(String type, JSONObject properties) {
                    mBrowserView.dispatchEvent(type, properties);
                }
            });

            mMediaSynchroniserManager.setSessionCallback(new MediaSynchroniserManager.SessionCallback() {
                /**
                 * Tell the bridge
                 *
                 * @param
                 * @param
                 */
                @Override
                public void dispatchTimelineAvailableEvent(TvBrowserTypes.Timeline timeline) { //arguments
                    mBridge.dispatchTimelineAvailableEvent(timeline);
                }

                /**
                 * Tell the bridge
                 *
                 * @param
                 * @param
                 */
                @Override
                public void dispatchTimelineUnavailableEvent(String timelineSelector) { //arguments
                    mBridge.dispatchTimelineUnavailableEvent(timelineSelector);
                }

                /**
                 * Tell the bridge
                 *
                 * @param
                 * @param
                 */
                @Override
                public void dispatchInterDeviceSyncEnabled(int mediaSyncId) { //arguments
                    mBridge.dispatchInterDeviceSyncEnabled(mediaSyncId);
                }

                /**
                 * Tell the bridge
                 *
                 * @param
                 * @param
                 */
                @Override
                public void dispatchInterDeviceSyncDisabled(int mediaSyncId) { //arguments
                    mBridge.dispatchInterDeviceSyncDisabled(mediaSyncId);
                }

                /**
                 * Tell the tv browser callback
                 *
                 * @param
                 * @param
                 */
                @Override
                public int startTEMITimelineMonitoring(int componentTag, int timelineId) {
                    return mTvBrowserCallback.startTEMITimelineMonitoring(componentTag, timelineId);
                }

                /**
                 * Tell the tv browser callback
                 *
                 * @param
                 * @param
                 */
                @Override
                public boolean stopTEMITimelineMonitoring(int filterId) {
                    return mTvBrowserCallback.stopTEMITimelineMonitoring(filterId);
                }

                /**
                 * Tell the tv browser callback
                 *
                 * @param
                 * @param
                 */
                @Override
                public long getCurrentPtsTime() {
                    return mTvBrowserCallback.getCurrentPtsTime();
                }

                /**
                 * Tell the tv browser callback
                 *
                 * @param
                 * @param
                 */
                @Override
                public long getCurrentTemiTime(int filterId) {
                    return mTvBrowserCallback.getCurrentTemiTime(filterId);
                }
            });
        }

        /**
         * Get the View of the TV browser session. This should be added to the content view of the
         * application.
         *
         * @return The View of the TV browser session.
         */
        public View getView() {
            return mBrowserView;
        }

        /**
         * Get the URL of the inter-device sync service
         * @return The URL of the inter-device sync service.
         */
        public String getInterDevSyncUrl() {
            return "ws://" +
                    mTvBrowserCallback.getHostAddress() +
                    ":" +
                    mConfiguration.mediaSyncCiiPort;
        }

        /**
         * Get the base URL of the app2app local service.
         * @return The base URL of the app2app remote service.
         */
        public String getApp2AppLocalBaseUrl() {
            return "ws://127.0.0.1:" +
                    mConfiguration.app2appLocalPort +
                    "/hbbtv/" +
                    mConfiguration.mainActivityUuid +
                    "/";
        }

        /**
         * Get the base URL of the app2app remote service.
         * @return The base URL of the app2app remote service.
         */
        public String getApp2AppRemoteBaseUrl() {
            return "ws://" +
                    mTvBrowserCallback.getHostAddress() +
                    ":" +
                    mConfiguration.app2appRemotePort +
                    "/hbbtv/" +
                    mConfiguration.mainActivityUuid + "/";
        }

        /**
         * Launches a "Broadcast-INDEPENDENT" application, the url could be an XML-AIT file.
         *
         * @param url URL where application is to be found
         * @return true if the application might be launched, false otherwise
         */
        public boolean launchApplication(String url) {
            Log.d(TAG, "Set URL " + url);
            boolean launch = true;
            if (Looper.myLooper() == Looper.getMainLooper()) {
                // Shouldn't execute network request on main thread
                new Thread(() -> {
                    mApplicationManager.createApplication(url);
                }).start();
            } else {
                launch = mApplicationManager.createApplication(url);
            }
            return launch;
        }

        /**
         * Requests the HbbTV engine to process the specified AIT. The HbbTV engine expects the
         * relevant AITs only (the first one after HBBTV_Start and when the version/PID changes).
         * If more than one stream is signalled in the PMT for a service with an
         * application_signalling_descriptor, then the application_signalling_descriptor for the
         * stream containing the AIT for the HbbTV application shall include the
         * HbbTV application_type (0x0010).
         *
         * @param aitPid PID of the AIT
         * @param serviceId Service ID the AIT refers to
         * @param data The buffer containing the AIT row data
         */
        public void processAitSection(int aitPid, int serviceId, byte[] data) {
            if (data == null) {
                Log.e(TAG, "AIT section data is null.");
                return;
            }
            mApplicationManager.processAitSection(aitPid, serviceId, data);
        }

        /**
         * TODO(comment)
         *
         * @param xmlAit
         */
        public void processXmlAit(String xmlAit) {
            if (xmlAit == null) {
                Log.e(TAG, "XML AIT is null.");
                return;
            }
            mApplicationManager.processXmlAit(xmlAit);
        }

        /**
         * Called when the service list has changed.
         */
        public void onServiceListChanged() {
            mBridge.dispatchServiceListChangedEvent();
        }

        /**
         * Called when the parental rating of the currently playing service has changed.
         *
         * @param blocked TRUE if the current service is blocked by the parental control system.
         */
        public void onParentalRatingChanged(boolean blocked) {
            mBridge.dispatchParentalRatingChangeEvent(blocked);
        }

        /**
         * TODO(comment)
         */
        public void onParentalRatingError() {
            mBridge.dispatchParentalRatingErrorEvent();
        }

        /**
         * Called when there is a change in the set of components being presented.
         *
         * @param componentType Type of component whose presentation has changed.
         */
        public void onSelectedComponentChanged(int componentType) {
            mBridge.dispatchSelectedComponentChangedEvent(componentType);
        }

        /**
         * Called when there is a change in the set of components being presented.
         *
         * @param componentType Type of component whose presentation has changed.
         */
        public void onComponentChanged(int componentType) {
            mBridge.dispatchComponentChangedEvent(componentType);
        }

        /**
         * Called when there is a change in status of the service identified by the DVB triplet.
         *
         * @param onetId     Original Network ID
         * @param transId    Transport Stream ID
         * @param servId     Service ID
         * @param statusCode
         * @param permanentError
         */
        public void onChannelStatusChanged(int onetId, int transId, int servId, int statusCode,
                                           boolean permanentError) {
            mBridge.dispatchChannelStatusChangedEvent(onetId, transId, servId, statusCode, permanentError);
            if (statusCode == TvBrowserTypes.CHANNEL_STATUS_CONNECTING) {
                mApplicationManager.onChannelChanged(onetId, transId, servId);
            }

            String ccid = mTvBrowserCallback.getCurrentCcid();
            TvBrowserTypes.Channel channel = mTvBrowserCallback.getChannel(ccid);
            List<TvBrowserTypes.Programme> programmes = mTvBrowserCallback.getPresentFollowingProgrammes(ccid);
            mMediaSynchroniserManager.updateBroadcastContentStatus(onetId, transId, servId, statusCode, permanentError, programmes);
        }

        /**
         * Called when the present/following events have changed on the current service.
         */
        public void onProgrammesChanged() {
            mBridge.dispatchProgrammesChangedEvent();

            String ccid = mTvBrowserCallback.getCurrentCcid();
            TvBrowserTypes.Channel channel = mTvBrowserCallback.getChannel(ccid);
            List<TvBrowserTypes.Programme> programmes = mTvBrowserCallback.getPresentFollowingProgrammes(ccid);
            mMediaSynchroniserManager.updateBroadcastContentStatus(channel.onid, channel.tsid, channel.sid, TvBrowserTypes.CHANNEL_STATUS_PRESENTING, false, programmes);
        }

        /**
         * Called when the video aspect ratio has changed.
         *
         * @param aspectRatioCode Code as defined by TvBrowserTypes.ASPECT_RATIO_*
         */
        public void onVideoAspectRatioChanged(int aspectRatioCode) {
            // TODO
        }

        /**
         * TODO(comment)
         */
        public void onLowMemoryEvent() {
            mBridge.dispatchLowMemoryEventEvent();
        }

        /**
         * TODO(comment)
         */
        public void onTimelineUnavailableEvent(int filterId) {
            mMediaSynchroniserManager.setTEMITimelineAvailability(filterId, false, 0, 0, 0);
        }

        /**
         * TODO(comment)
         */
        public void onTimelineAvailableEvent(int filterId, long currentTime, long timescale, double speed) {
            mMediaSynchroniserManager.setTEMITimelineAvailability(filterId, true, currentTime, timescale, speed);
        }

        /**
         * TODO(comment)
         *
         * @param connected
         */
        public void onNetworkStatusEvent(boolean connected) {
            mApplicationManager.onNetworkAvailabilityChanged(connected);
        }

        /**
         * Called when the user has decided whether the application at origin should be allowed access
         * to a distinctive identifier.
         *
         * @param origin The origin of the requesting application
         * @param accessAllowed true if access allowed, otherwise false
         */
        public void onAccessToDistinctiveIdentifierDecided(String origin, boolean accessAllowed) {
            mBridge.dispatchAccessToDistinctiveIdentifierEvent(origin, accessAllowed);
        }

        /**
         * TODO(comment)
         *
         * @param search
         * @param status
         * @param programmes
         * @param offset
         * @param totalSize
         */
        public void onMetadataSearchCompleted(int search, int status, List<TvBrowserTypes.Programme> programmes, int offset, int totalSize) {
            mBridge.dispatchMetadataSearchCompleted(search, status, programmes, offset, totalSize);
        }

        public void onReceiveStreamEvent(int id, String name, String data, String text, String status) {
            mBridge.dispatchStreamEvent(id, name, data, text, status);
        }

        /**
         * TODO(library) What makes sense here?
         */
        public void close() {
            mApplicationManager.close();
            mBridge.releaseResources();
            //mTvBrowserCallback.finaliseTEMITimelineMonitoring();
        }
    }

    static {
        System.loadLibrary("org.orbtv.tvbrowser.networkservices-jni");
        System.loadLibrary("org.orbtv.tvbrowser.applicationmanager-jni");
    }
}
