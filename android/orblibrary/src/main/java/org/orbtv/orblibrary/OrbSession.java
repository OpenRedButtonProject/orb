package org.orbtv.orblibrary;

import android.content.Context;
import android.os.Looper;
import android.util.Log;
import android.view.View;

import org.json.JSONObject;
import org.orbtv.orbpolyfill.BridgeTypes;

import java.nio.ByteBuffer;
import java.util.List;

class OrbSession implements IOrbSession {
    private static final String TAG = OrbSession.class.getSimpleName();

    private final IOrbSessionCallback mOrbSessionCallback;
    private ApplicationManager mApplicationManager;
    private OrbSessionFactory.Configuration mConfiguration;
    private MediaSynchroniserManager mMediaSynchroniserManager;
    private Bridge mBridge;
    private BrowserView mBrowserView;
    private DsmccClient mDsmccClient;

    /**
     * TV browser session.
     *
     * @param context  The context of the application.
     * @param callback The implementation of TvBrowserCallback to call.
     */
    public OrbSession(Context context, final IOrbSessionCallback callback,
                      OrbSessionFactory.Configuration configuration) {
        mOrbSessionCallback = callback;
        mConfiguration = configuration;
        mApplicationManager = new ApplicationManager(mOrbSessionCallback);
        mMediaSynchroniserManager = new MediaSynchroniserManager(configuration);
        mBridge = new Bridge(this, callback, configuration, mApplicationManager,
                mMediaSynchroniserManager);
        mDsmccClient = new DsmccClient(callback);
        mBrowserView = new BrowserView(context, mBridge, configuration, mDsmccClient);

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
                mOrbSessionCallback.setChannelByTriplet(-1, -1, -1, -1, -1,
                        null, false, "", 0);
            }

            /**
             * Tell the broadcast-integration to reset any calls by HbbTV to suspend presentation,
             * set the video rectangle or set the presented components.
             */
            public void resetBroadcastPresentation() {
                // TODO Make this one call in the API
                mOrbSessionCallback.setDvbVideoRectangle(0, 0, 1280, 720);
                mOrbSessionCallback.setPresentationSuspended(false);
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
             * @return A BridgeTypes.VK_* key code.
             */
            @Override
            public int getTvBrowserKeyCode(int androidKeyCode) {
                return mOrbSessionCallback.getTvBrowserKeyCode(androidKeyCode);
            }

            /**
             * Check whether the key code is in the key set of the application.
             *
             * @param appId The application ID.
             * @param keyCode A BridgeTypes.VK_* key code.
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
            public boolean isRequestAllowed(int callingAppId, String callingPageUrl,
                                            int methodRequirement) {
                return mApplicationManager.isRequestAllowed(callingAppId, callingPageUrl,
                        methodRequirement);
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
            public void dispatchTimelineAvailableEvent(BridgeTypes.Timeline timeline) { //arguments
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
                return mOrbSessionCallback.startTEMITimelineMonitoring(componentTag, timelineId);
            }

            /**
             * Tell the tv browser callback
             *
             * @param
             * @param
             */
            @Override
            public boolean stopTEMITimelineMonitoring(int filterId) {
                return mOrbSessionCallback.stopTEMITimelineMonitoring(filterId);
            }

            /**
             * Tell the tv browser callback
             *
             * @param
             * @param
             */
            @Override
            public long getCurrentPtsTime() {
                return mOrbSessionCallback.getCurrentPtsTime();
            }

            /**
             * Tell the tv browser callback
             *
             * @param
             * @param
             */
            @Override
            public long getCurrentTemiTime(int filterId) {
                return mOrbSessionCallback.getCurrentTemiTime(filterId);
            }
        });
    }

    /**
     * Get the View of the TV browser session. This should be added to the content view of the
     * application.
     *
     * @return The View of the TV browser session.
     */
    @Override
    public View getView() {
        return mBrowserView;
    }

    /**
     * Get the URL of the inter-device sync service
     *
     * @return The URL of the inter-device sync service.
     */
    @Override
    public String getInterDevSyncUrl() {
        return "ws://" +
                mOrbSessionCallback.getHostAddress() +
                ":" +
                mConfiguration.mediaSyncCiiPort;
    }

    /**
     * Get the base URL of the app2app local service.
     *
     * @return The base URL of the app2app remote service.
     */
    @Override
    public String getApp2AppLocalBaseUrl() {
        return "ws://127.0.0.1:" +
                mConfiguration.app2appLocalPort +
                "/hbbtv/" +
                mConfiguration.mainActivityUuid +
                "/";
    }

    /**
     * Get the base URL of the app2app remote service.
     *
     * @return The base URL of the app2app remote service.
     */
    @Override
    public String getApp2AppRemoteBaseUrl() {
        return "ws://" +
                mOrbSessionCallback.getHostAddress() +
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
    @Override
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
     * @param aitPid    PID of the AIT
     * @param serviceId Service ID the AIT refers to
     * @param data      The buffer containing the AIT row data
     */
    @Override
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
    @Override
    public void processXmlAit(String xmlAit) {
        if (xmlAit == null) {
            Log.e(TAG, "XML AIT is null.");
            return;
        }
        mApplicationManager.processXmlAit(xmlAit);
    }

    /**
     * Returns whether a Teletext application is signalled in the current AIT.
     *
     * @return True if a Teletext application is signalled, false otherwise.
     */
    @Override
    public boolean isTeletextApplicationSignalled() {
        return mApplicationManager.isTeletextApplicationSignalled();
    }

    /**
     * Launch the Teletext application signalled in the current AIT (e.g., when the user presses the
     * TEXT key).
     *
     * @return True if the application is launched, false otherwise.
     */
    @Override
    public boolean launchTeletextApplication() {
        return mApplicationManager.runTeletextApplication();
    }

    /**
     * Called when the service list has changed.
     */
    @Override
    public void onServiceListChanged() {
        mBridge.dispatchServiceListChangedEvent();
    }

    /**
     * Called when the parental rating of the currently playing service has changed.
     *
     * @param blocked TRUE if the current service is blocked by the parental control system.
     */
    @Override
    public void onParentalRatingChanged(boolean blocked) {
        mBridge.dispatchParentalRatingChangeEvent(blocked);
    }

    /**
     * Called when there is a parental rating error.
     */
    @Override
    public void onParentalRatingError(String contentID, List<BridgeTypes.ParentalRating> ratings,
                                      String DRMSystemID) {
        mBridge.dispatchParentalRatingErrorEvent(contentID, ratings, DRMSystemID);
    }

    /**
     * Called when there is a change in the set of components being presented.
     *
     * @param componentType Type of component whose presentation has changed.
     */
    @Override
    public void onSelectedComponentChanged(int componentType) {
        mBridge.dispatchSelectedComponentChangedEvent(componentType);
    }

    /**
     * Called when there is a change in the set of components being presented.
     *
     * @param componentType Type of component whose presentation has changed.
     */
    @Override
    public void onComponentChanged(int componentType) {
        mBridge.dispatchComponentChangedEvent(componentType);
    }

    /**
     * Called when there is a change in status of the service identified by the DVB triplet.
     *
     * @param onetId         Original Network ID
     * @param transId        Transport Stream ID
     * @param servId         Service ID
     * @param statusCode
     * @param permanentError
     */
    @Override
    public void onChannelStatusChanged(int onetId, int transId, int servId, int statusCode,
                                       boolean permanentError) {
        mBridge.dispatchChannelStatusChangedEvent(onetId, transId, servId, statusCode, permanentError);
        if (statusCode == BridgeTypes.CHANNEL_STATUS_CONNECTING) {
            mApplicationManager.onChannelChanged(onetId, transId, servId);
        }

        String ccid = mOrbSessionCallback.getCurrentCcid();
        if (!ccid.isEmpty()) {
            BridgeTypes.Channel channel = mOrbSessionCallback.getChannel(ccid);
            List<BridgeTypes.Programme> programmes = mOrbSessionCallback.getProgrammeList(ccid);
            mMediaSynchroniserManager.updateBroadcastContentStatus(onetId, transId, servId, statusCode, permanentError, programmes);
        }
    }

    /**
     * Called when the present/following events have changed on the current service.
     */
    @Override
    public void onProgrammesChanged() {
        mBridge.dispatchProgrammesChangedEvent();

        String ccid = mOrbSessionCallback.getCurrentCcid();
        if (!ccid.isEmpty()) {
            BridgeTypes.Channel channel = mOrbSessionCallback.getChannel(ccid);
            List<BridgeTypes.Programme> programmes = mOrbSessionCallback.getProgrammeList(ccid);
            mMediaSynchroniserManager.updateBroadcastContentStatus(channel.onid, channel.tsid, channel.sid, BridgeTypes.CHANNEL_STATUS_PRESENTING, false, programmes);
        }
    }

    /**
     * Called when the video aspect ratio has changed.
     *
     * @param aspectRatioCode Code as defined by BridgeTypes.ASPECT_RATIO_*
     */
    @Override
    public void onVideoAspectRatioChanged(int aspectRatioCode) {
        // TODO
    }

    /**
     * TODO(comment)
     */
    @Override
    public void onLowMemoryEvent() {
        mBridge.dispatchLowMemoryEventEvent();
    }

    /**
     * TODO(comment)
     */
    @Override
    public void onTimelineUnavailableEvent(int filterId) {
        mMediaSynchroniserManager.setTEMITimelineAvailability(filterId, false, 0, 0, 0);
    }

    /**
     * TODO(comment)
     */
    @Override
    public void onTimelineAvailableEvent(int filterId, long currentTime, long timescale, double speed) {
        mMediaSynchroniserManager.setTEMITimelineAvailability(filterId, true, currentTime, timescale, speed);
    }

    /**
     * TODO(comment)
     *
     * @param connected
     */
    @Override
    public void onNetworkStatusEvent(boolean connected) {
        mApplicationManager.onNetworkAvailabilityChanged(connected);
    }

    /**
     * Called when the user has decided whether the application at origin should be allowed access
     * to a distinctive identifier.
     *
     * @param origin        The origin of the requesting application
     * @param accessAllowed true if access allowed, otherwise false
     */
    @Override
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
    public void onMetadataSearchCompleted(int search, int status, List<BridgeTypes.Programme> programmes, int offset, int totalSize) {
        mBridge.dispatchMetadataSearchCompleted(search, status, programmes, offset, totalSize);
    }

    /**
     * Notify about DRM licensing errors during playback of DRM protected A/V content.
     *
     * @param errorState      details the type of error:
     *                        - 0: no license, consumption of the content is blocked.
     *                        - 1: invalid license, consumption of the content is blocked.
     *                        - 2: valid license, consumption of the content is unblocked.
     * @param contentID       unique identifier of the protected content
     * @param DRMSystemID     ID of the DRM System
     * @param rightsIssuerURL indicates the value of the rightsIssuerURL that can
     *                        be used to non-silently obtain the rights for the content item
     */
    @Override
    public void onDRMRightsError(int errorState, String contentID, String DRMSystemID, String rightsIssuerURL) {
        mBridge.dispatchDRMRightsError(errorState, contentID, DRMSystemID, rightsIssuerURL);
    }

    /**
     * Called when the status of a DRM system changes.
     *
     * @param drmSystem          ID of the DRM System
     * @param drmSystemIds       List of the DRM System IDs handled by the DRM System
     * @param status             status of the indicated DRM system. Possible states:
     *                           - 0 READY, fully initialised and ready
     *                           - 1 UNKNOWN, no longer available
     *                           - 2 INITIALISING, initialising and not ready to communicate
     *                           - 3 ERROR, in error state
     * @param protectionGateways space separated list of zero or more CSP Gateway
     *                           types that are capable of supporting the DRM system
     * @param supportedFormats   space separated list of zero or more supported
     *                           file and/or container formats by the DRM system
     */
    @Override
    public void onDRMSystemStatusChange(String drmSystem, List<String> drmSystemIds, int status,
                                        String protectionGateways, String supportedFormats) {
        mBridge.dispatchDRMSystemStatusChange(drmSystem, drmSystemIds, status, protectionGateways, supportedFormats);
    }

    /**
     * Called when the underlying DRM system has a result message as a consequence
     * of a call to sendDRMMessage.
     *
     * @param msgID      identifies the original message which has led to this resulting message
     * @param resultMsg  DRM system specific result message
     * @param resultCode result code. Valid values include:
     *                   - 0 Successful
     *                   - 1 Unknown error
     *                   - 2 Cannot process request
     *                   - 3 Unknown MIME type
     *                   - 4 User consent needed
     *                   - 5 Unknown DRM system
     *                   - 6 Wrong format
     */
    @Override
    public void onDRMMessageResult(String msgID, String resultMsg, int resultCode) {
        mBridge.dispatchDRMMessageResult(msgID, resultMsg, resultCode);
    }

    /**
     * Called when the underlying DRM system has a message to report.
     *
     * @param msg         DRM system specific message
     * @param DRMSystemID ID of the DRM System
     */
    @Override
    public void onDRMSystemMessage(String msg, String DRMSystemID) {
        mBridge.dispatchDRMSystemMessage(msg, DRMSystemID);
    }

    /**
     * Called by IDsmcc on receiving content
     *
     * @param requestId ID of request
     * @param buffer    ByteBuffer with content for DSMCC file
     */
    @Override
    public void onDsmccReceiveContent(int requestId, ByteBuffer buffer) {
        mDsmccClient.onDsmccReceiveContent(requestId, buffer);
    }

    /**
     * Called by IDsmcc on receiving Stream Event
     *
     * @param listenId ID of listener
     * @param name     Name of Stream event
     * @param data     Data asssociated with stream event
     */
    @Override
    public void onDsmccReceiveStreamEvent(int listenId, String name, String data, String text, String status) {
        mBridge.dispatchStreamEvent(listenId, name, data, text, status);
    }

    /**
     * TODO(library) What makes sense here?
     */
    @Override
    public void close() {
        mApplicationManager.close();
        mBridge.releaseResources();
        //mOrbSessionCallback.finaliseTEMITimelineMonitoring();
    }
}
