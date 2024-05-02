package org.orbtv.orblibrary;

import android.content.Context;
import android.content.IntentFilter;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;
import android.view.KeyEvent;
import android.view.View;

import org.json.JSONObject;
import org.orbtv.orbpolyfill.BridgeTypes;

import java.nio.ByteBuffer;
import java.util.List;

class OrbSession implements IOrbSession {
    private static final String TAG = OrbSession.class.getSimpleName();
    private final IOrbSessionCallback mOrbSessionCallback;
    private final int mOrbHbbTVVersion;
    private ApplicationManager mApplicationManager;
    private OrbSessionFactory.Configuration mConfiguration;
    private MediaSynchroniserManager mMediaSynchroniserManager;
    private JsonRpc mJsonRpc;
    private Bridge mBridge;
    private BrowserView mBrowserView;
    private DsmccClient mDsmccClient;
    private final int EMPTY_INTEGER = -999999;
    private final String EMPTY_STRING = "";

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
        mOrbHbbTVVersion = mApplicationManager.getOrbHbbTVVersion();
        Log.d(TAG, "ORB HbbTV Version: " + mOrbHbbTVVersion);

        mMediaSynchroniserManager = new MediaSynchroniserManager(configuration);

        if (mOrbHbbTVVersion >= 204) {
            mJsonRpc = new JsonRpc(configuration.jsonRpcPort, mOrbSessionCallback);
        } else {
            mJsonRpc = null;
        }

        mBridge = new Bridge(this, callback, configuration, mApplicationManager,
                mMediaSynchroniserManager, mJsonRpc);
        mDsmccClient = new DsmccClient(callback);
        mBrowserView = new BrowserView(context, mBridge, configuration, mDsmccClient);

        mApplicationManager.setSessionCallback(new ApplicationManager.SessionCallback() {
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
            @Override
            public void loadApplication(int appId, String entryUrl, int[] graphics) {
                if (mOrbHbbTVVersion >= 204) {
                    mBrowserView.loadApplication(appId, entryUrl, graphics);
                } else {
                    mBrowserView.loadApplication(appId, entryUrl, null);
                }
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

            /**
             * Notify that the key set has changed.
             *
             * @param keySet The application key set.
             */
            public void notifyKeySetChanged(int keySet, int[] otherKeys) {
                Handler handler = new Handler(Looper.getMainLooper());
                Runnable runnable = () -> {
                    if (mOrbSessionCallback != null) {
                        mOrbSessionCallback.onKeySetChanged(keySet, otherKeys);
                    }
                };
                handler.post(runnable);
            }

            /**
             * Notify that the application status is changed.
             *
             * @param status The application status.
             */
            @Override
            public void notifyApplicationStatusChanged(IOrbSessionCallback.ApplicationStatus status) {
                Handler handler = new Handler(Looper.getMainLooper());
                Runnable runnable = () -> {
                    if (mOrbSessionCallback != null) {
                        mOrbSessionCallback.onApplicationStatusChanged(status);
                    }
                };
                handler.post(runnable);
            }

            @Override
            public void dispatchApplicationSchemeUpdatedEvent(String scheme) {
                mBridge.dispatchApplicationSchemeUpdatedEvent(scheme);
            }

            /**
             * Returns true if the provided triplet is in an instance within the
             * currently playing service, otherwise false.
             */
            @Override
            public boolean isInstanceInCurrentService(int onid, int tsid, int sid) {
                return mOrbSessionCallback.isInstanceInCurrentService(onid, tsid, sid);
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
     * When the user presses the exit key, destroy the calling application.
     */
    @Override
    public void onExitKeyPress() {
        mApplicationManager.destroyApplication(0);
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
     * For portals (not DVB-I).
     *
     * @param xmlAit
     */
    @Override
    public void processXmlAit(String xmlAit) {
        processXmlAit(xmlAit, false, "urn:dvb:metadata:cs:LinkedApplicationCS:2019:1.1");
    }

    /**
     * TODO(comment)
     *
     * @param xmlAit
     */
    @Override
    public void processXmlAit(String xmlAit, boolean isDvbi, String scheme) {
        if (xmlAit == null) {
            Log.e(TAG, "XML AIT is null.");
            return;
        }
        mApplicationManager.processXmlAit(xmlAit, isDvbi, scheme);
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
     * Called to Tell the browser to dispatch an key press event.
     *
     * @param event The KeyEvent, with an action and a KeyCode, to be handled
     */
    @Override
    public boolean dispatchKeyEvent(KeyEvent event) {
        return mBrowserView.dispatchKeyEvent(event);
    }

    /**
     * Called to Tell the browser to dispatch an text input.
     *
     * @param text The content of the text input
     */
    public void dispatchTextInput(String text) {
        mBrowserView.dispatchTextInput(text);
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
     * @param componentType If the presentation has changed for only one component type, this value
     * should be set to BridgeTypes.COMPONENT_TYPE_* for that specific type. If the presentation has
     * changed for more than one component type, this value should be set to
     * BridgeTypes.COMPONENT_TYPE_ANY.
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
     * Called when the dvbi client has tuned to a specific instance
     *
     * @param index     Index of the currently tuned service instance
     */
    @Override
    public void onServiceInstanceChange(int index) {
        mBridge.dispatchServiceInstanceChangedEvent(index);
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
     * Called when the user changes the audio language
     *
     * @param language      The new preferred audio language
     */
    @Override
    public void onPreferredAudioLanguageChanged(String language) {
        mBridge.dispatchPreferredAudioLanguageChanged(language);
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
    public void onDsmccReceiveStreamEvent(int listenId, String name, String data, String text, String status, BridgeTypes.DASHEvent dashEvent) {
        mBridge.dispatchStreamEvent(listenId, name, data, text, status, dashEvent);
    }

    /**
     * TODO(library) What makes sense here?
     */
    @Override
    public void close() {
        mBrowserView.close();
        mApplicationManager.close();
        mBridge.releaseResources();
        if (mOrbHbbTVVersion >= 204) {
            mJsonRpc.close();
        }
        //mOrbSessionCallback.finaliseTEMITimelineMonitoring();
    }

    /**
     * @since 204
     * 
     * Called to send a response message for a result of overriding dialogue enhancement
     *
     * @param connection              The request and response should have the same value
     * @param id                      The request and response should have the same value
     * @param dialogueEnhancementGain The applied gain value in dB of the dialogue enhancement
     */
    @Override
    public void onRespondDialogueEnhancementOverride(int connection, String id,
                                                     int dialogueEnhancementGain) {
        if (mOrbHbbTVVersion < 204) {
            throw new UnsupportedOperationException("Unsupported 204 API.");
        }

        mJsonRpc.onRespondDialogueEnhancementOverride(connection, id,
                dialogueEnhancementGain);
    }

    /**
     * @since 204
     * 
     * Called to send a response message for a result of trigger response to user action
     *
     * @param connection The request and response should have the same value
     * @param id         The request and response should have the same value
     * @param actioned   The result of user action mechanism
     *                   - true: successfully triggered
     *                   - false: unsuccessfully triggered
     */
    @Override
    public void onRespondTriggerResponseToUserAction(int connection, String id, boolean actioned) {
        if (mOrbHbbTVVersion < 204) {
            throw new UnsupportedOperationException("Unsupported 204 API.");
        }

        mJsonRpc.onRespondTriggerResponseToUserAction(connection, id, actioned);
    }

    /**
     * @since 204
     * 
     * Called to send a response message for the support information of a feature
     *
     * @param connection The request and response should have the same value
     * @param id         The request and response should have the same value
     * @param feature    The index of a particular accessibility feature
     * @param value      The result code of the support for the accessibility feature
     */
    @Override
    public void onRespondFeatureSupportInfo(int connection, String id, int feature, String value) {
        if (mOrbHbbTVVersion < 204) {
            throw new UnsupportedOperationException("Unsupported 204 API.");
        }

        mJsonRpc.onRespondFeatureSupportInfo(connection, id, feature, value);
    }

    /**
     * @since 204
     * 
     * Called to send a response message for suppressing the support of a feature
     *
     * @param connection The request and response should have the same value
     * @param id         The request and response should have the same value
     * @param feature    The index of a particular accessibility feature
     * @param value      The result code for suppressing
     */
    @Override
    public void onRespondFeatureSuppress(int connection, String id, int feature, String value) {
        if (mOrbHbbTVVersion < 204) {
            throw new UnsupportedOperationException("Unsupported 204 API.");
        }

        mJsonRpc.onRespondFeatureSuppress(connection, id, feature, value);
    }

    /**
     * @since 204
     * 
     * Called to send an error message
     *
     * @param connection The request and response should have the same value
     * @param id         The request and response should have the same value
     * @param code       The error code
     * @param message    The error message
     */
    @Override
    public void onRespondError(int connection, String id, int code, String message) {
        if (mOrbHbbTVVersion < 204) {
            throw new UnsupportedOperationException("Unsupported 204 API.");
        }

        mJsonRpc.onRespondError(connection, id, code, message);
    }

    /**
     * @since 204
     * 
     * Called to send an error message with some data
     *
     * @param connection The request and response should have the same value
     * @param id         The request and response should have the same value
     * @param code       The error code
     * @param message    The error message
     * @param data       The error data
     */
    public void onRespondError(int connection, String id, int code, String message, String data) {
        if (mOrbHbbTVVersion < 204) {
            throw new UnsupportedOperationException("Unsupported 204 API.");
        }

        mJsonRpc.onRespondError(connection, id, code, message, data);
    }

    /**
     * @since 204
     * 
     * Called to send a message with the user settings of subtitles
     *
     * @param connection        The request and response should have the same value
     * @param id                The request and response should have the same value
     * @param enabled           Enabled subtitles
     * @param size              The font size
     * @param fontFamily        The description of the font family
     * @param textColour        The text colour in RGB24 format
     * @param textOpacity       The test opacity with the percentage from 0 to 100
     * @param edgeType          The description of edge type
     * @param edgeColour        The edge colour in RGB24 format
     * @param backgroundColour  The background colour in RGB24 format
     * @param backgroundOpacity The background opacity with the percentage from 0 to 100
     * @param windowColour      The window colour in RGB24 format
     * @param windowOpacity     The window opacity with the percentage from 0 to 100
     * @param language          The description of language in ISO639-2 3-character code
     */
    @Override
    public void onQuerySubtitles(int connection, String id, boolean enabled,
                                 int size, String fontFamily, String textColour, int textOpacity,
                                 String edgeType, String edgeColour,
                                 String backgroundColour, int backgroundOpacity,
                                 String windowColour, int windowOpacity, String language) {
        if (mOrbHbbTVVersion < 204) {
            throw new UnsupportedOperationException("Unsupported 204 API.");
        }

        mJsonRpc.onQuerySubtitles(connection, id, enabled, size, fontFamily, textColour, textOpacity,
                edgeType, edgeColour, backgroundColour, backgroundOpacity,
                windowColour, windowOpacity, language);
    }

    /**
     * @since 204
     *
     * Called to send a notification message when the subtitles user settings change
     *
     * @param enabled           Enabled subtitles
     * @param size              The font size
     * @param fontFamily        The description of the font family
     * @param textColour        The text colour in RGB24 format
     * @param textOpacity       The test opacity with the percentage from 0 to 100
     * @param edgeType          The description of edge type
     * @param edgeColour        The edge colour in RGB24 format
     * @param backgroundColour  The background colour in RGB24 format
     * @param backgroundOpacity The background opacity with the percentage from 0 to 100
     * @param windowColour      The window colour in RGB24 format
     * @param windowOpacity     The window opacity with the percentage from 0 to 100
     * @param language          The description of language in ISO639-2 3-character code
     */
    public void onNotifySubtitles(boolean enabled, int size, String fontFamily, String textColour, int textOpacity,
                                  String edgeType, String edgeColour,
                                  String backgroundColour, int backgroundOpacity,
                                  String windowColour, int windowOpacity, String language) {
        if (mOrbHbbTVVersion < 204) {
            throw new UnsupportedOperationException("Unsupported 204 API.");
        }

        mJsonRpc.onQuerySubtitles(EMPTY_INTEGER, EMPTY_STRING, enabled, size, fontFamily, textColour, textOpacity,
                edgeType, edgeColour, backgroundColour, backgroundOpacity,
                windowColour, windowOpacity, language);
    }

    /**
     * @since 204
     * 
     * Called to send a message with the settings of dialogue enhancement
     *
     * @param connection     The request and response should have the same value
     * @param id             The request and response should have the same value
     * @param gainPreference The dialogue enhancement gain preference in dB
     * @param gain           The currently-active gain value in dB
     * @param limitMin       The current allowed minimum gain value in dB
     * @param limitMax       The current allowed maximum gain value in dB
     */
    @Override
    public void onQueryDialogueEnhancement(int connection, String id, int gainPreference, int gain,
                                           int limitMin, int limitMax) {
        if (mOrbHbbTVVersion < 204) {
            throw new UnsupportedOperationException("Unsupported 204 API.");
        }

        mJsonRpc.onQueryDialogueEnhancement(connection, id,
                gainPreference, gain, limitMin, limitMax);
    }

    /**
     * @since 204
     *
     * Called to send a notification message when the dialogue enhancement user settings change
     *
     * @param gainPreference The dialogue enhancement gain preference in dB
     * @param gain           The currently-active gain value in dB
     * @param limitMin       The current allowed minimum gain value in dB
     * @param limitMax       The current allowed maximum gain value in dB
     */
    @Override
    public void onNotifyDialogueEnhancement(int gainPreference, int gain, int limitMin, int limitMax) {
        if (mOrbHbbTVVersion < 204) {
            throw new UnsupportedOperationException("Unsupported 204 API.");
        }

        mJsonRpc.onQueryDialogueEnhancement(EMPTY_INTEGER, EMPTY_STRING,
                gainPreference, gain, limitMin, limitMax);
    }

    /**
     * @since 204
     * 
     * Called to send a message with the settings of a user Interface Magnification feature
     *
     * @param connection The request and response should have the same value
     * @param id         The request and response should have the same value
     * @param enabled    Enabled a screen magnification UI setting
     * @param magType    The description of the type of magnification scheme currently set
     */
    @Override
    public void onQueryUIMagnifier(int connection, String id, boolean enabled, String magType) {
        if (mOrbHbbTVVersion < 204) {
            throw new UnsupportedOperationException("Unsupported 204 API.");
        }

        mJsonRpc.onQueryUIMagnifier(connection, id, enabled, magType);
    }

    /**
     * @since 204
     *
     * Called to send a notification message when the High Contrast UI user settings change
     *
     * @param enabled    Enabled a screen magnification UI setting
     * @param magType    The description of the type of magnification scheme currently set
     */
    @Override
    public void onNotifyUIMagnifier(boolean enabled, String magType) {
        if (mOrbHbbTVVersion < 204) {
            throw new UnsupportedOperationException("Unsupported 204 API.");
        }

        mJsonRpc.onQueryUIMagnifier(EMPTY_INTEGER, EMPTY_STRING, enabled, magType);
    }

    /**
     * @since 204
     * 
     * Called to send a message with the settings of a high contrast UI feature
     *
     * @param connection The request and response should have the same value
     * @param id         The request and response should have the same value
     * @param enabled    Enabled a high contrast UI
     * @param hcType     The description of the type of high contrast scheme currently set
     */
    @Override
    public void onQueryHighContrastUI(int connection, String id, boolean enabled, String hcType) {
        if (mOrbHbbTVVersion < 204) {
            throw new UnsupportedOperationException("Unsupported 204 API.");
        }

        mJsonRpc.onQueryHighContrastUI(connection, id, enabled, hcType);
    }

    /**
     * @since 204
     *
     * Called to send a notification message when the High Contrast UI user settings change
     *
     * @param enabled    Enabled a high contrast UI
     * @param hcType     The description of the type of high contrast scheme currently set
     */
    @Override
    public void onNotifyHighContrastUI(boolean enabled, String hcType) {
        if (mOrbHbbTVVersion < 204) {
            throw new UnsupportedOperationException("Unsupported 204 API.");
        }

        mJsonRpc.onQueryHighContrastUI(EMPTY_INTEGER, EMPTY_STRING, enabled, hcType);
    }

    /**
     * @since 204
     * 
     * Called to send a message with the settings of a screen reader feature
     *
     * @param connection The request and response should have the same value
     * @param id         The request and response should have the same value
     * @param enabled    Enabled a screen reader preference
     * @param speed      A percentage scaling factor of the default speech speed, 100% considered normal speed
     * @param voice      The description of the voice
     * @param language   The description of language in ISO639-2 3-character code
     */
    @Override
    public void onQueryScreenReader(int connection, String id,
                                    boolean enabled, int speed, String voice, String language) {
        if (mOrbHbbTVVersion < 204) {
            throw new UnsupportedOperationException("Unsupported 204 API.");
        }

        mJsonRpc.onQueryScreenReader(connection, id, enabled, speed, voice, language);
    }

    /**
     * @since 204
     *
     * Called to send a notification message with the settings of a screen reader feature
     *
     * @param enabled    Enabled a screen reader preference
     * @param speed      A percentage scaling factor of the default speech speed, 100% considered normal speed
     * @param voice      The description of the voice
     * @param language   The description of language in ISO639-2 3-character code
     */
    @Override
    public void onNotifyScreenReader(boolean enabled, int speed, String voice, String language) {
        if (mOrbHbbTVVersion < 204) {
            throw new UnsupportedOperationException("Unsupported 204 API.");
        }

        mJsonRpc.onQueryScreenReader(EMPTY_INTEGER, EMPTY_STRING, enabled, speed, voice, language);
    }

    /**
     * @since 204
     * 
     * Called to send a message with the settings of a "response to a user action" feature
     *
     * @param connection The request and response should have the same value
     * @param id         The request and response should have the same value
     * @param enabled    Enabled a "response to a user action" preference
     * @param type       The description of the mechanism the terminal uses to feedback to the user that the user action has occurred.
     */
    @Override
    public void onQueryResponseToUserAction(int connection, String id, boolean enabled, String type) {
        if (mOrbHbbTVVersion < 204) {
            throw new UnsupportedOperationException("Unsupported 204 API.");
        }

        mJsonRpc.onQueryResponseToUserAction(connection, id, enabled, type);
    }

    /**
     * @since 204
     *
     * Called to send a notification message when ‘Response to a User Action’ user settings change
     *
     * @param enabled    Enabled a "response to a user action" preference
     * @param type       The description of the mechanism the terminal uses to feedback to the user that the user action has occurred.
     */
    @Override
    public void onNotifyResponseToUserAction(boolean enabled, String type) {
        if (mOrbHbbTVVersion < 204) {
            throw new UnsupportedOperationException("Unsupported 204 API.");
        }

        mJsonRpc.onQueryResponseToUserAction(EMPTY_INTEGER, EMPTY_STRING, enabled, type);
    }

    /**
     * @since 204
     * 
     * Called to send a message with the settings of an audio description feature
     *
     * @param connection           The request and response should have the same value
     * @param id                   The request and response should have the same value
     * @param enabled              Enabled audio description
     * @param gainPreference       The audio description gain preference set by the user in dB.
     * @param panAzimuthPreference The degree of the azimuth pan preference set by the user
     */
    @Override
    public void onQueryAudioDescription(int connection, String id, boolean enabled,
                                        int gainPreference, int panAzimuthPreference) {
        if (mOrbHbbTVVersion < 204) {
            throw new UnsupportedOperationException("Unsupported 204 API.");
        }

        mJsonRpc.onQueryAudioDescription(connection, id, enabled, gainPreference, panAzimuthPreference);
    }

    /**
     * @since 204
     *
     * Called to send a notification message when the Audio Description user settings change
     *
     * @param enabled              Enabled audio description
     * @param gainPreference       The audio description gain preference set by the user in dB.
     * @param panAzimuthPreference The degree of the azimuth pan preference set by the user
     */
    @Override
    public void onNotifyAudioDescription(boolean enabled, int gainPreference, int panAzimuthPreference) {
        if (mOrbHbbTVVersion < 204) {
            throw new UnsupportedOperationException("Unsupported 204 API.");
        }

        mJsonRpc.onQueryAudioDescription(EMPTY_INTEGER, EMPTY_STRING, enabled, gainPreference, panAzimuthPreference);
    }

    /**
     * @since 204
     * 
     * Called to send a message with the settings of an in-vision signing feature
     *
     * @param connection The request and response should have the same value
     * @param id         The request and response should have the same value
     * @param enabled    Enabled an in-vision signing preference
     */
    @Override
    public void onQueryInVisionSigning(int connection, String id, boolean enabled) {
        if (mOrbHbbTVVersion < 204) {
            throw new UnsupportedOperationException("Unsupported 204 API.");
        }

        mJsonRpc.onQueryInVisionSigning(connection, id, enabled);
    }

    /**
     * @since 204
     *
     * Called to send a notification message when the In-Vision Signing user settings change
     *
     * @param enabled    Enabled an in-vision signing preference
     */
    @Override
    public void onNotifyInVisionSigning(boolean enabled) {
        if (mOrbHbbTVVersion < 204) {
            throw new UnsupportedOperationException("Unsupported 204 API.");
        }

        mJsonRpc.onQueryInVisionSigning(EMPTY_INTEGER, EMPTY_STRING, enabled);
    }

    /**
     * @since 204
     * 
     * Called to send an intent for a request to operate the media playback
     *
     * @param cmd The index of a basic intent of media playback
     *            - 0: pause
     *            - 1: play
     *            - 2: fast-forward
     *            - 3: fast-reverse
     *            - 4: stop
     */
    @Override
    public void onSendIntentMediaBasics(int cmd) {
        if (mOrbHbbTVVersion < 204) {
            throw new UnsupportedOperationException("Unsupported 204 API.");
        }

        mJsonRpc.onSendIntentMediaBasics(cmd);
    }

    /**
     * @since 204
     * 
     * Called to send an intent for a request to seek a time position relative to the start or end of the media content
     *
     * @param anchor The value indicates an anchor point of the content
     *               - "start": the start or end of the content
     *               - "end": the start or end of the content
     * @param offset The number value for the time position, a positive or negative number of seconds
     */
    @Override
    public void onSendIntentMediaSeekContent(String anchor, int offset) {
        if (mOrbHbbTVVersion < 204) {
            throw new UnsupportedOperationException("Unsupported 204 API.");
        }

        mJsonRpc.onSendIntentMediaSeekContent(anchor, offset);
    }

    /**
     * @since 204
     * 
     * Called to send an intent for a request to seek a time position relative to the current time of the media content
     *
     * @param offset The number value for the current time position, a positive or negative number of seconds
     */
    @Override
    public void onSendIntentMediaSeekRelative(int offset) {
        if (mOrbHbbTVVersion < 204) {
            throw new UnsupportedOperationException("Unsupported 204 API.");
        }

        mJsonRpc.onSendIntentMediaSeekRelative(offset);
    }

    /**
     * @since 204
     * 
     * Called to send an intent for a request to seek a time position relative to the live edge of the media content
     *
     * @param offset The number value for the time position at or before the live edge, zero or negative number of seconds
     */
    @Override
    public void onSendIntentMediaSeekLive(int offset) {
        if (mOrbHbbTVVersion < 204) {
            throw new UnsupportedOperationException("Unsupported 204 API.");
        }

        mJsonRpc.onSendIntentMediaSeekLive(offset);
    }

    /**
     * @since 204
     * 
     * Called to send an intent for a request to seek a time position relating to absolute wall clock time
     *
     * @param dateTime The value conveys the wall clock time, in internet date-time format
     */
    @Override
    public void onSendIntentMediaSeekWallclock(String dateTime) {
        if (mOrbHbbTVVersion < 204) {
            throw new UnsupportedOperationException("Unsupported 204 API.");
        }

        mJsonRpc.onSendIntentMediaSeekWallclock(dateTime);
    }

    /**
     * @since 204
     * 
     * Called to send an intent to request a search of content available
     *
     * @param query The string value is the search term specified by the user.
     */
    @Override
    public void onSendIntentSearch(String query) {
        if (mOrbHbbTVVersion < 204) {
            throw new UnsupportedOperationException("Unsupported 204 API.");
        }

        mJsonRpc.onSendIntentSearch(query);
    }

    /**
     * @since 204
     * 
     * Called to send an intent to request a display (but not playback) of a specific identified piece of content
     *
     * @param mediaId The value for a URI uniquely identifying a piece of content
     */
    @Override
    public void onSendIntentDisplay(String mediaId) {
        if (mOrbHbbTVVersion < 204) {
            throw new UnsupportedOperationException("Unsupported 204 API.");
        }

        mJsonRpc.onSendIntentDisplay(mediaId);
    }

    /**
     * @since 204
     * 
     * Called to send an intent to request immediate playback of a specific identified piece of content
     *
     * @param mediaId The value for a URI uniquely identifying a piece of content
     *                === With meanings as seek-content ===
     * @param anchor  The value indicates an anchor point of the content
     *                - "start": the start or end of the content
     *                - "end": the start or end of the content
     * @param offset  The number value for the time position, a positive or negative number of seconds
     *                === With meaning as seek-live ===
     * @param offset  The number value for the time position at or before the live edge, zero or negative number of seconds
     */
    @Override
    public void onSendIntentPlayback(String mediaId, String anchor, int offset) {
        if (mOrbHbbTVVersion < 204) {
            throw new UnsupportedOperationException("Unsupported 204 API.");
        }

        mJsonRpc.onSendIntentPlayback(mediaId, anchor, offset);
    }

    /**
     * @since 204
     *
     * Request for the Description of the current media playback on the application
     */
    @Override
    public boolean onVoiceRequestDescription() {
        if (mOrbHbbTVVersion < 204) {
            throw new UnsupportedOperationException("Unsupported 204 API.");
        }

        mJsonRpc.onVoiceRequestDescription();
        return true;
    }

    /**
     * @since 204
     *
     * Request to deliver a text input, from voice command, to applications
     *
     * @param input The content of the text
     * @return true if this event has been handled, and false if not
     */
    @Override
    public boolean onVoiceRequestTextInput(String input) {
        if (mOrbHbbTVVersion < 204) {
            throw new UnsupportedOperationException("Unsupported 204 API.");
        }

        // Mock function to display the input text
        dispatchTextInput(input);
        return true;
    }

}
