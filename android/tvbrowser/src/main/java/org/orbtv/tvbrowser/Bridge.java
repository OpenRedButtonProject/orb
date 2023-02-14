/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 * <p>
 * Licensed under the ORB License that can be found in the LICENSE file at the top level of this
 * repository.
 */

package org.orbtv.tvbrowser;

import android.util.Log;

import org.json.JSONException;
import org.json.JSONObject;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

class Bridge extends AbstractBridge {
    private static final String TAG = "Orb/Bridge";

    private final TvBrowser.Session mTvBrowserSession;
    private final TvBrowserCallback mTvBrowserCallback;
    private final ApplicationManager mApplicationManager;
    private final TvBrowser.Configuration mConfiguration;
    private final App2AppService mApp2AppService;
    private final MediaSynchroniserManager mMediaSyncManager;

    Bridge(TvBrowser.Session tvBrowser, TvBrowserCallback tvBrowserCallback,
           TvBrowser.Configuration configuration, ApplicationManager applicationManager, MediaSynchroniserManager mediaSyncManager) {
        mTvBrowserSession = tvBrowser;
        mTvBrowserCallback = tvBrowserCallback;
        mConfiguration = configuration;
        mApplicationManager = applicationManager;
        mMediaSyncManager = mediaSyncManager;
        mApp2AppService = App2AppService.GetInstance();
        if (!mApp2AppService.Start(mConfiguration.app2appLocalPort, mConfiguration.app2appRemotePort)) {
            Log.w(TAG, "Failed to start App2App service.");
        }
    }

    public void releaseResources() {
        mApp2AppService.Stop();
        mMediaSyncManager.releaseResources();
    }

    /**
     * Set the size and position of the broadcast video.
     *
     * Values in pixels are relative to a 1280x720 logical coordinate system and should be scaled to
     * the actual width and height of the browser.
     *
     * @param token The token associated with this request.
     * @param x The X position in pixels.
     * @param y The Y position in pixels.
     * @param width The width in pixels.
     * @param height The height in pixels.
     */
    @Override
    protected void Broadcast_setVideoRectangle(Token token, int x, int y, int width, int height) {
        mTvBrowserCallback.setDvbVideoRectangle(x, y, width, height);
    }

    /**
     * Get the current broadcast channel.
     *
     * Security: FOR_BROADCAST_APP_ONLY.
     *
     * @param token The token associated with this request.
     *
     * @return A Channel object representing the current broadcast channel; or null if not available.
     */
    @Override
    protected TvBrowserTypes.Channel Broadcast_getCurrentChannel(Token token) {
        // TODO Add 1:1 method to callback
        String ccid = mTvBrowserCallback.getCurrentCcid();
        if (ccid != null) {
            return mTvBrowserCallback.getChannel(ccid);
        }
        return null;
    }

    /**
     * Get the current broadcast channel.
     *
     * This method can be called while transitioning to broadcast-related, allowing certain events to
     * be implemented.
     *
     * Security: FOR_BROADCAST_OR_TRANSITIONING_APP_ONLY.
     *
     * @param token The token associated with this request.
     *
     * @return A Channel object representing the current broadcast channel; or null if not available.
     */
    @Override
    protected TvBrowserTypes.Channel Broadcast_getCurrentChannelForEvent(Token token) {
        // TODO Add 1:1 method to callback
        String ccid = mTvBrowserCallback.getCurrentCcid();
        if (ccid != null) {
            return mTvBrowserCallback.getChannel(ccid);
        }
        return null;
    }

    /**
     * Get the broadcast channel list.
     *
     * Security: FOR_BROADCAST_APP_ONLY.
     *
     * @param token The token associated with this request.
     *
     * @return A list of Channel objects representing the broadcast channel list; or an empty list if
     *    not available.
     */
    @Override
    protected List<TvBrowserTypes.Channel> Broadcast_getChannelList(Token token) {
        return mTvBrowserCallback.getChannelList();
    }

    /**
     * Select a logically null broadcast channel (e.g. tune off).
     *
     * When a logically null broadcast channel is selected, the Application Manager must transition
     * the running application to broadcast-independent or kill it, depending on the signalling.
     *
     * Security: FOR_RUNNING_APP_ONLY.
     *
     * @param token The token associated with this request.
     *
     * @return A CHANNEL_STATUS_* code (on success, the code has a value less than 0).
     */
    @Override
    protected int Broadcast_setChannelToNull(Token token) {
        // TODO Add 1:1 method to callback
        mTvBrowserCallback.setChannelByTriplet(
                -1,
                -1,
                -1,
                -1,
                -1,
                null,
                false,
                "",
                0);
        mApplicationManager.onBroadcastStopped();
        return -4;
    }

    /**
     * Select the broadcast channel (e.g. tune) with the given CCID.
     *
     * Security: FOR_RUNNING_APP_ONLY.
     *
     * @param token The token associated with this request.
     * @param ccid The CCID of the channel to set.
     * @param trickplay True if the application has optionally hinted trickplay resources are
     *    required; or false otherwise. Does not affect the success of this operation.
     * @param contentAccessDescriptorURL Optionally, additional information for DRM-protected IPTV
     *    broadcasts; or an empty string otherwise.
     * @param quiet Type of channel change: 0 for normal; 1 for normal, no UI; 2 for quiet (HbbTV
     *    A.2.4.3.2).
     *
     * @return A CHANNEL_STATUS_* code (on success, the code has a value less than 0).
     */
    @Override
    protected int Broadcast_setChannelToCcid(Token token, String ccid, boolean trickplay,
                                             String contentAccessDescriptorURL, int quiet) {
        TvBrowserTypes.Channel ch = mTvBrowserCallback.getChannel(ccid);
        // TODO Add 1:1 method to callback
        return mTvBrowserCallback.setChannelByTriplet(
                ch.idType,
                ch.onid,
                ch.tsid,
                ch.sid,
                ch.sourceId,
                ch.ipBroadcastId,
                trickplay,
                contentAccessDescriptorURL,
                quiet);
    }

    /**
     * Select the given broadcast channel (e.g. tune) with the given triplet and information.
     *
     * Security: FOR_RUNNING_APP_ONLY.
     *
     * @param token The token associated with this request.
     * @param idType The type of the channel to set (ID_* code).
     * @param onid The original network ID of the channel to set.
     * @param tsid The transport stream ID of the channel to set.
     * @param sid The service ID of the channel to set.
     * @param sourceID Optionally, the ATSC source_ID of the channel to set; or -1 otherwise.
     * @param ipBroadcastID Optionally, the DVB textual service ID of the (IP broadcast) channel
     *    to set; or an empty string otherwise.
     * @param trickplay True if the application has optionally hinted trickplay resources are
     *    required; or false otherwise. Does not affect the success of this operation.
     * @param contentAccessDescriptorURL Optionally, additional information for DRM-protected IPTV
     *    broadcasts; or an empty string otherwise.
     * @param quiet Type of channel change: 0 for normal, 1 for normal no UI, 2 for quiet (HbbTV
     *    A.2.4.3.2).
     *
     * @return A CHANNEL_STATUS_* code (on success, the code has a value less than 0).
     */
    @Override
    protected int Broadcast_setChannelToTriplet(Token token, int idType, int onid, int tsid, int sid,
                                                int sourceID, String ipBroadcastID, boolean trickplay, String contentAccessDescriptorURL,
                                                int quiet) {
        return mTvBrowserCallback.setChannelByTriplet(
                idType,
                onid,
                tsid,
                sid,
                sourceID,
                ipBroadcastID,
                trickplay,
                contentAccessDescriptorURL,
                quiet);
    }

    /**
     * Select the broadcast channel with the given DSD.
     *
     * Security: FOR_RUNNING_APP_ONLY.
     *
     * @param token The token associated with this request.
     * @param dsd The DSD of the channel to set.
     * @param sid The service ID of the channel to set.
     * @param trickplay True if the application has optionally hinted trickplay resources are
     *    required; or false otherwise. Does not affect the success of this operation.
     * @param contentAccessDescriptorURL Optionally, additional information for DRM-protected IPTV
     *    broadcasts; or an empty string otherwise.
     * @param quiet Type of channel change: 0 for normal, 1 for normal no UI, 2 for quiet (HbbTV
     *    A.2.4.3.2).
     *
     * @return A CHANNEL_STATUS_* code (on success, the code has a value less than 0).
     */
    @Override
    protected int Broadcast_setChannelToDsd(Token token, String dsd, int sid, boolean trickplay,
                                            String contentAccessDescriptorURL, int quiet) {
        return mTvBrowserCallback.setChannelByDsd(
                dsd,
                sid,
                trickplay,
                contentAccessDescriptorURL,
                quiet);
    }

    /**
     * Get the programme list for the given broadcast channel.
     *
     * Security: FOR_BROADCAST_APP_ONLY.
     *
     * @param token The token associated with this request.
     * @param ccid The CCID of the broadcast channel.
     *
     * @return A list of Programme objects available for the broadcast channel; or an empty list if
     *    not available.
     */
    @Override
    protected List<TvBrowserTypes.Programme> Broadcast_getProgrammes(Token token, String ccid) {
        return mTvBrowserCallback.getProgrammeList(ccid);
    }

    /**
     * Get the component list for the given broadcast channel.
     *
     * Security: FOR_BROADCAST_APP_ONLY.
     *
     * @param token The token associated with this request.
     * @param ccid The CCID of the broadcast channel.
     * @param type Optionally the component type (COMPONENT_TYPE_* code); or -1 otherwise.
     *
     * @return A list of Component objects available for the broadcast channel; or an empty list if
     *    not available.
     */
    @Override
    protected List<TvBrowserTypes.Component> Broadcast_getComponents(Token token, String ccid,
                                                                     int type) {
        return mTvBrowserCallback.getComponentList(ccid, type);
    }

    /**
     * Get a private audio component in the selected channel.
     *
     * Security: FOR_BROADCAST_APP_ONLY.
     *
     * @param token The token associated with this request.
     * @param componentTag The component_tag of the component.
     *
     * @return The private component with the specified component_tag in the PMT of the currently
     *    selected broadcast channel; or null if unavailable or the component is not private (i.e.
     *    the stream type is audio, video or subtitle).
     *
     *    Mandatory properties: id, pid and encrypted. The id property shall be usable with the
     *    overrideComponentSelection method to select the component as an audio track. Other
     *    Component properties are not required.
     */
    @Override
    protected TvBrowserTypes.Component Broadcast_getPrivateAudioComponent(Token token,
                                                                          String componentTag) {
        return mTvBrowserCallback.getPrivateAudioComponent(componentTag);
    }

    /**
     * Get a private video component in the selected channel.
     *
     * Security: FOR_BROADCAST_APP_ONLY.
     *
     * @param token The token associated with this request.
     * @param componentTag The component_tag of the component.
     *
     * @return The private component with the specified component_tag in the PMT of the currently
     *    selected broadcast channel; or null if unavailable or the component is not private (i.e.
     *    the stream type is audio, video or subtitle).
     *
     *    Mandatory properties: id, pid and encrypted. The id property shall be usable with the
     *    overrideComponentSelection method to select the component as an video track. Other
     *    Component properties are not required.
     */
    @Override
    protected TvBrowserTypes.Component Broadcast_getPrivateVideoComponent(Token token,
                                                                          String componentTag) {
        return mTvBrowserCallback.getPrivateVideoComponent(componentTag);
    }

    /**
     * Override the default component selection of the terminal for the specified type.
     *
     * If id is empty, no component shall be selected for presentation (presentation is explicitly
     * disabled). Otherwise, the specified component shall be selected for presentation.
     *
     * If playback has already started, the presented component shall be updated.
     *
     * Default component selection shall be restored (revert back to the control of the terminal)
     * when: (1) the application terminates, (2) the channel is changed, (3) presentation has not
     * been explicitly disabled and the user selects another track in the terminal UI, or (4) the
     * restoreComponentSelection method is called.
     *
     * Security: FOR_BROADCAST_APP_ONLY.
     *
     * @param token The token associated with this request.
     * @param type Type of component selection to override (COMPONENT_TYPE_* code).
     * @param id A platform-defined component id or an empty string to disable presentation.
     */
    protected void Broadcast_overrideComponentSelection(Token token, int type, String id) {
        mTvBrowserCallback.overrideComponentSelection(type, id);
    }

    /**
     * Restore the default component selection of the terminal for the specified type.
     *
     * If playback has already started, the presented component shall be updated.
     *
     * Security: FOR_BROADCAST_APP_ONLY.
     *
     * @param token The token associated with this request.
     * @param type Type of component selection override to clear (COMPONENT_TYPE_* code).
     */
    protected void Broadcast_restoreComponentSelection(Token token, int type) {
        mTvBrowserCallback.restoreComponentSelection(type);
    }

    /**
     * Start a metadata search.
     *
     * When the result is ready, it is dispatched to the bridge as a MetadataSearch event.
     *
     * Security: FOR_BROADCAST_APP_ONLY.
     *
     * @param token The token associated with this request.
     * @param query A string that represents the query the application wants to carry out.
     * @param offset Offset into the total result set.
     * @param count Maximum number of items to include in the result from the offset.
     * @param channelConstraintList Optionally, a list of strings describing constraints the
     *    application wants applied to the query; or an empty list otherwise.
     */
    @Override
    protected void Broadcast_startSearch(Token token, String query, int offset, int count,
                                         List<String> channelConstraintList) {
        mTvBrowserCallback.startSearch(new TvBrowserTypes.Query(query), offset, count,
                channelConstraintList);
    }

    /**
     * Abort a started metadata search.
     *
     * Security: FOR_BROADCAST_APP_ONLY.
     *
     * @param token The token associated with this request.
     * @param queryId The ID from the query string that started the search.
     */
    @Override
    protected void Broadcast_abortSearch(Token token, int queryId) {
        mTvBrowserCallback.abortSearch(queryId);
    }

    /**
     * Add a stream event listener subscribed to a DSM-CC event.
     *
     * When an event is transmitted, it is dispatched to the bridge as a StreamEvent event.
     *
     * @param token The token associated with this request.
     * @param targetURL Optionally, the DSM-CC StreamEvent object URL; or an empty string
     *    otherwise.
     * @param eventName The event name to subscribe to.
     * @param componentTag If no targetURL is provided, the component tag; or -1 otherwise.
     * @param streamEventId If no targetURL is provided, the event ID; or -1 otherwise.
     *
     * @return The listener ID. Associated StreamEvent events shall include this ID.
     */
    @Override
    protected int Broadcast_addStreamEventListener(Token token, String targetURL, String eventName,
                                                   int componentTag, int streamEventId) {
        // TODO Split this method into 2
        if (targetURL.startsWith("dvb:")) {
            return DsmccClient.subscribeStreamEventName(targetURL, eventName);
        } else {
            return DsmccClient.subscribeStreamEventId(eventName, componentTag, streamEventId);
        }
    }

    /**
     * Remove the given stream event listener.
     *
     * @param token The token associated with this request.
     * @param id The listener ID to remove.
     */
    @Override
    protected void Broadcast_removeStreamEventListener(Token token, int id) {
        DsmccClient.unsubscribeStreamEvent(id);
    }

    /**
     * Set whether the broadcast presentation should be suspended.
     *
     * Presentation of broadcast components such as audio, video and text tracks should be suspended
     * or resumed, with the component selections preserved. The channel should remain logically
     * selected (e.g. tuned).
     *
     * Security: FOR_BROADCAST_APP_ONLY.
     *
     * @param token The token associated with this request.
     * @param presentationSuspended True if the broadcast presentation should be suspended; false
     *    otherwise.
     */
    @Override
    protected void Broadcast_setPresentationSuspended(Token token, boolean presentationSuspended) {
        mTvBrowserCallback.setPresentationSuspended(presentationSuspended);
    }

    /**
     * Get the parental rating of the current broadcast programme.
     *
     * @param token The token associated with this request.
     *
     * @return A ParentalRating object representing the parental rating; or null if not available.
     */
    @Override
    protected TvBrowserTypes.ParentalRating Programme_getParentalRating(Token token) {
        // TODO Add 1:1 method to callback
        List<TvBrowserTypes.Programme> programmes =
                mTvBrowserCallback.getPresentFollowingProgrammes(mTvBrowserCallback.getCurrentCcid());
        if (programmes != null && !programmes.isEmpty()) {
            List<TvBrowserTypes.ParentalRating> ratings = programmes.get(0).parentalRatings;
            if (ratings != null && !ratings.isEmpty()) {
                return ratings.get(0); // TODO Why index 0?
            }
        }
        return null;
    }

    /**
     * Get a list of the raw SI descriptor data for a programme.
     *
     * @param ccid The CCID (channel ID) of the programme.
     * @param programmeID The ID of the programme.
     * @param descriptorTag The descriptor tag ID of the data to get.
     * @param descriptorTagExtension Optionally, the extended descriptor tag ID of the data to
     *    get; -1 otherwise.
     * @param privateDataSpecifier Optionally, the private data specifier of the data to be
     *    returned; 0 otherwise.
     *
     * @return A list of SI descriptor data. If there are multiple descriptors with the same tag ID,
     *    they will all be returned.
     */
    @Override
    protected List<String> Programme_getSIDescriptors(String ccid, String programmeID,
                                                      int descriptorTag, int descriptorTagExtension, int privateDataSpecifier) {
        return mTvBrowserCallback.getSiDescriptorData(
                ccid,
                programmeID,
                descriptorTag,
                descriptorTagExtension,
                privateDataSpecifier);
    }

    /**
     * Create and run a new application.
     *
     * The calling application is identified by the token associated with the request, to check
     * whether it is allowed to create and run a new application.
     *
     * @param token The token associated with this request.
     * @param url A HTTP/HTTPS or DVB URL.
     *
     *    A HTTP/HTTPS URL may refer to the entry page or XML AIT of a broadcast-independent app.
     *
     *    A DVB URL may refer to a broadcast-related app signalled in the current service AIT. This
     *    will result in the signalled URL being loaded, which may be HTTP/HTTPS for broadband or DVB
     *    for carousel.
     *
     * @return True if the application can be created; false otherwise.
     */
    @Override
    protected boolean Manager_createApplication(Token token, String url) {
        return mApplicationManager.createApplication(token.getAppId(), url);
    }

    /**
     * Destroy the calling application.
     *
     * The calling application is identified by the token associated with the request.
     *
     * @param token The token associated with this request.
     */
    @Override
    protected void Manager_destroyApplication(Token token) {
        mApplicationManager.destroyApplication(token.getAppId());
    }

    /**
     * Show the calling application.
     *
     * The calling application is identified by the token associated with the request.
     *
     * @param token The token associated with this request.
     */
    @Override
    protected void Manager_showApplication(Token token) {
        mApplicationManager.showApplication(token.getAppId());
    }

    /**
     * Hide the calling application.
     *
     * The calling application is identified by the token associated with the request.
     *
     * @param token The token associated with this request.
     */
    @Override
    protected void Manager_hideApplication(Token token) {
        mApplicationManager.hideApplication(token.getAppId());
    }

    /**
     * Get the free memory available to the application.
     *
     * @param token The token associated with this request.
     */
    @Override
    protected long Manager_getFreeMem(Token token) {
        // TODO Should all ints be longs?
        return mTvBrowserCallback.getFreeMemory();
    }

    /**
     * Get the keyset for this application.
     *
     * The calling application is identified by the token associated with the request.
     *
     * @param token The token associated with this request.
     *
     * @return The keyset for this application.
     */
    @Override
    protected int Manager_getKeyValues(Token token) {
        return mApplicationManager.getKeyValues(token.getAppId());
    }

    /**
     * Get the maximum keyset available to applications.
     *
     * @param token The token associated with this request.
     *
     * @return The maximum keyset available to applications.
     */
    @Override
    protected int Manager_getKeyMaximumValue(Token token) {
        return mApplicationManager.getKeyMaximumValue();
    }

    /**
     * Set the keyset for this application.
     *
     * The calling application is identified by the token associated with the request.
     *
     * @param token The token associated with this request.
     * @param value The keyset to set for this application.
     *
     * @return The keyset for this application.
     */
    @Override
    protected int Manager_setKeyValue(Token token, int value) {
        return mApplicationManager.setKeyValue(token.getAppId(), value);
    }

    /**
     * Get the URL of an icon that represents the given key.
     *
     * @param token The token associated with this request.
     * @param code The code of the key to get an icon for (VK_ code).
     *
     * @return A URL of an icon that represents the given key; or null if not available.
     */
    @Override
    protected String Manager_getKeyIcon(Token token, int code) {
        // TODO Call callback 1:1
        return "";
    }

    /**
     * Get a list of rating schemes supported by this integration.
     *
     * @param token The token associated with this request.
     *
     * @return A list of ParentalRatingScheme objects representing the supported parental rating
     *    schemes; or an empty list of not available.
     */
    @Override
    protected List<TvBrowserTypes.ParentalRatingScheme> ParentalControl_getRatingSchemes(Token token) {
        // TODO Add 1:1 method to callback
        List<TvBrowserTypes.ParentalRatingScheme> schemes = new ArrayList<>();
        List<TvBrowserTypes.ParentalRating> ratings = new ArrayList<>();
        String region = mTvBrowserCallback.getParentalControlRegion();
        for (int i = 4; i <= 18; ++i) {
            ratings.add(new TvBrowserTypes.ParentalRating(String.valueOf(i), "dvb-si", i,
                    0, region));
        }
        schemes.add(new TvBrowserTypes.ParentalRatingScheme("dvb-si", ratings));
        return schemes;
    }

    /**
     * Get the parental rating threshold currently set on the system for the given scheme.
     *
     * @param token The token associated with this request.
     * @param schemeName The name of the scheme to get the threshold of.
     *
     * @return A ParentalRating object representing the threshold.
     */
    @Override
    protected TvBrowserTypes.ParentalRating ParentalControl_getThreshold(Token token,
                                                                         String schemeName) {
        // TODO Add 1:1 method to callback
        if (schemeName.equals("dvb-si")) {
            int thresholdAge = mTvBrowserCallback.getParentalControlAge();
            return new TvBrowserTypes.ParentalRating(
                    String.valueOf(thresholdAge),
                    schemeName,
                    thresholdAge,
                    0,
                    mTvBrowserCallback.getParentalControlRegion()
            );
        }
        return null;
    }

    /**
     * Test whether the rating is blocked for the given scheme, region and value.
     *
     * @param token The token associated with this request.
     * @param scheme The name of the scheme to test.
     * @param region The code of the region to test (ISO 3166-1 alpha-3 codes).
     * @param value The value to test.
     *
     * @return True if the rating is blocked; or false otherwise.
     */
    @Override
    protected boolean ParentalControl_isRatingBlocked(Token token, String scheme, String region,
                                                      int value) {
        // TODO Add 1:1 method to callback
        if (scheme.toLowerCase().equals("dvb-si")) {
            // The value property of the parental rating is equal to
            // the value in DVB-SI rating field + 3 (table A.6 of A.2.28)
            String parentalRegion = mTvBrowserCallback.getParentalControlRegion().toLowerCase();
            int age = mTvBrowserCallback.getParentalControlAge();
            return !parentalRegion.equals(region.toLowerCase()) || age <= value + 3;
        }
        return false;
    }

    /**
     * Get the current capabilities of the terminal.
     *
     * @param token The token associated with this request.
     *
     * @return A Capabilities object.
     */
    @Override
    protected TvBrowserTypes.Capabilities Configuration_getCapabilities(Token token) {
        return mTvBrowserCallback.getCapabilities();
    }

    /**
     * Get a list of audio profiles supported by the terminal, as defined by HBBTV 10.2.4.7 for the
     * audio_profile element.
     *
     * @param token The token associated with this request.
     *
     * @return A list of audio profiles supported by the terminal.
     */
    @Override
    protected List<TvBrowserTypes.AudioProfile> Configuration_getAudioProfiles(Token token) {
        return mTvBrowserCallback.getAudioProfiles();
    }

    /**
     * Get a list of video profiles supported by the terminal, as defined by HBBTV 10.2.4.7 for the
     * video_profile element.
     *
     * @param token The token associated with this request.
     *
     * @return A list of video profiles supported by the terminal.
     */
    @Override
    protected List<TvBrowserTypes.VideoProfile> Configuration_getVideoProfiles(Token token) {
        return mTvBrowserCallback.getVideoProfiles();
    }

    /**
     * If the terminal supports UHD, get a list that describes the highest quality video format the
     * terminal supports, as defined by HBBTV 10.2.4.7 for the video_display_format element;
     * otherwise get an empty list.
     *
     * Note: If the terminal changes its display format based on the content being played, multiple
     * elements may be included in the list when multiple frame rate families are usable or the
     * highest resolution does not support each highest quality parameter.
     *
     * @param token The token associated with this request.
     *
     * @return A list that describes the highest quality video format.
     */
    @Override
    protected List<TvBrowserTypes.VideoDisplayFormat> Configuration_getVideoDisplayFormats(Token token) {
        return mTvBrowserCallback.getVideoDisplayFormats();
    }

    /**
     * Get the current number of additional media streams containing SD video accompanied by audio
     * that can be decoded and presented by an A/V control object or HTML5 media element.
     *
     * @param token The token associated with this request.
     *
     * @return The current number of additional media streams. If the value is non-zero, then a call
     *    to play an A/V control object, HTML5 media element or video/broadcast object shall not fail
     *    due to lack of resources for SD media.
     */
    @Override
    protected int Configuration_getExtraSDVideoDecodes(Token token) {
        return mTvBrowserCallback.getExtraSDVideoDecodes();
    }

    /**
     * Get the current number of additional media streams containing HD video accompanied by audio
     * that can be decoded and presented by an A/V control object or HTML5 media element.
     *
     * @param token The token associated with this request.
     *
     * @return The current number of additional media streams. If the value is non-zero, then a call
     *    to play an A/V control object, HTML5 media element or video/broadcast object shall not fail
     *    due to lack of resources for HD media.
     */
    @Override
    protected int Configuration_getExtraHDVideoDecodes(Token token) {
        return mTvBrowserCallback.getExtraHDVideoDecodes();
    }

    /**
     * Get the current number of additional media streams containing UHD video accompanied by audio
     * that can be decoded and presented by an A/V control object or HTML5 media element.
     *
     * @param token The token associated with this request.
     *
     * @return The current number of additional media streams. If the value is non-zero, then a call
     *    to play an A/V control object, HTML5 media element or video/broadcast object shall not fail
     *    due to lack of resources for UHD media.
     */
    @Override
    protected int Configuration_getExtraUHDVideoDecodes(Token token) {
        return mTvBrowserCallback.getExtraUHDVideoDecodes();
    }

    /**
     * Get certain immutable information about the system.
     *
     * @param token The token associated with this request.
     *
     * @return A SystemInformation object.
     */
    @Override
    protected TvBrowserTypes.SystemInformation Configuration_getLocalSystem(Token token) {
        return mTvBrowserCallback.getSystemInformation();
    }

    /**
     * Get preferred languages to be used for audio playback on this system.
     *
     * @param token The token associated with this request.
     *
     * @return Comma separated string of languages (ISO 639-2 codes), in order of preference.
     */
    @Override
    protected String Configuration_getPreferredAudioLanguage(Token token) {
        return mTvBrowserCallback.getPreferredAudioLanguage();
    }

    /**
     * Get preferred languages to be used for subtitles on this system.
     *
     * @param token The token associated with this request.
     *
     * @return Comma separated string of languages (ISO 639-2 codes), in order of preference.
     */
    @Override
    protected String Configuration_getPreferredSubtitleLanguage(Token token) {
        return mTvBrowserCallback.getPreferredSubtitleLanguage();
    }

    /**
     * Get preferred languages to be used for the user-interface on this system.
     *
     * @param token The token associated with this request.
     *
     * @return Comma separated string of languages (ISO 639-2 codes), in order of preference.
     */
    @Override
    protected String Configuration_getPreferredUILanguage(Token token) {
        return mTvBrowserCallback.getPreferredUILanguage();
    }

    /**
     * Get a string containing the three character country code identifying the country this system
     * is deployed.
     *
     * @param token The token associated with this request.
     *
     * @return Country code the receiver is deployed (ISO 3166-1 alpha-3 code).
     */
    @Override
    protected String Configuration_getCountryId(Token token) {
        return mTvBrowserCallback.getCountryId();
    }

    /**
     * Get whether subtitles are enabled on this system.
     *
     * @param token The token associated with this request.
     *
     * @return True if subtitles are enabled; or false otherwise.
     */
    @Override
    protected boolean Configuration_getSubtitlesEnabled(Token token) {
        return mTvBrowserCallback.getSubtitlesEnabled();
    }

    /**
     * Get whether audio description is enabled on this system.
     *
     * @param token The token associated with this request.
     *
     * @return True if audio description is enabled; or false otherwise.
     */
    @Override
    protected boolean Configuration_getAudioDescriptionEnabled(Token token) {
        return mTvBrowserCallback.getAudioDescriptionEnabled();
    }

    /**
     * Get the DVB network IDs of the channels in the broadcast channel list.
     *
     * @param token The token associated with this request.
     *
     * @return A list of DVB network IDs; or an empty list of not available.
     */
    @Override
    protected List<Integer> Configuration_getDttNetworkIds(Token token) {
        // TODO Make 1:1
        ArrayList<Integer> result = new ArrayList<>();
        for (int id : mTvBrowserCallback.getDttNetworkIds()) {
            result.add(id);
        }
        return result;
    }

    /**
     * Get a distinctive identifier for this terminal and calling origin; or a status code if not
     * available. To get access to a distinctive identifier, the calling origin must have previously
     * called Configuration_requestAccessToDistinctiveIdentifier.
     *
     * The calling origin is identified by the token associated with the request.
     *
     * Integrators should check HbbTV 12.1.5 for requirements about distinctive identifiers.
     *
     * @param token The token associated with this request.
     *
     * @return A distinctive identifier, uniquely generated for this terminal and calling origin; or
     *    a status code (DISTINCTIVE_IDENTIFIER_STATUS_ code).
     */
    @Override
    protected String Configuration_getDeviceId(Token token) {
        return mTvBrowserCallback.getDistinctiveIdentifier(token.getOrigin());
    }

    /**
     * Request distinctive identifier access for this calling origin.
     *
     * The calling origin is identified by the token associated with the request.
     *
     * The client application should display a dialog for the user to allow or deny this. When the
     * result is ready, it is dispatched to the bridge as a accesstodistinctiveidentifier event.
     *
     * @param token The token associated with this request.
     */
    @Override
    protected void Configuration_requestAccessToDistinctiveIdentifier(Token token) {
        // Check is not ephemeral UUID origin
        if (!token.getOrigin().startsWith("uuid-")) {
            mTvBrowserCallback.requestAccessToDistinctiveIdentifier(token.getOrigin());
        }
    }

    /**
     * @param token The token associated with this request.
     *
     * @return
     */
    @Override
    protected int MediaSynchroniser_instantiate(Token token) {
        int id = mMediaSyncManager.createMediaSynchroniser();
        Log.d(TAG, "Instantiated MediaSynchroniser with id " + id);
        return id;
    }

    /**
     * @param token The token associated with this request.
     * @param id
     * @param isMasterBroadcast
     *
     * @return
     */
    @Override
    protected boolean MediaSynchroniser_initialise(Token token, int id, boolean isMasterBroadcast) {
        return mMediaSyncManager.initialiseMediaSynchroniser(id, isMasterBroadcast);
    }

    /**
     * @param token The token associated with this request.
     * @param id
     */
    @Override
    protected void MediaSynchroniser_destroy(Token token, int id) {
        mMediaSyncManager.destroyMediaSynchroniser(id);
    }

    /**
     * @param token The token associated with this request.
     * @param id
     */
    @Override
    protected boolean MediaSynchroniser_enableInterDeviceSync(Token token, int id) {
        return mMediaSyncManager.enableInterDeviceSync(mTvBrowserCallback.getHostAddress());
    }

    /**
     * @param token The token associated with this request.
     * @param id
     */
    @Override
    protected void MediaSynchroniser_disableInterDeviceSync(Token token, int id) {
        mMediaSyncManager.disableInterDeviceSync();
    }

    /**
     * @param token The token associated with this request.
     * @param id
     *
     * @return
     */
    @Override
    protected int MediaSynchroniser_nrOfSlaves(Token token, int id) {
        return mMediaSyncManager.nrOfSlaves(id);
    }

    /**
     * @param token The token associated with this request.
     * @param id
     *
     * @return
     */
    @Override
    protected boolean MediaSynchroniser_interDeviceSyncEnabled(Token token, int id) {
        return mMediaSyncManager.interDeviceSyncEnabled(id);
    }

    /**
     * @param token The token associated with this request.
     * @param id
     *
     * @return
     */
    @Override
    protected String MediaSynchroniser_getContentIdOverride(Token token, int id) {
        return mMediaSyncManager.getContentIdOverride(id);
    }

    /**
     * @param token The token associated with this request.
     * @param id
     * @param timeline
     *
     * @return
     */
    @Override
    protected boolean MediaSynchroniser_startTimelineMonitoring(Token token, int id,
                                                                String timelineSelector, boolean isMaster) {
        return mMediaSyncManager.startTimelineMonitoring(timelineSelector, isMaster);
    }

    /**
     * @param token The token associated with this request.
     * @param id
     * @param timelineSelector
     */
    @Override
    protected void MediaSynchroniser_stopTimelineMonitoring(Token token, int id, String timelineSelector, boolean forceStop) {
        mMediaSyncManager.stopTimelineMonitoring(timelineSelector, forceStop);
    }

    /**
     * @param token The token associated with this request.
     * @param id
     * @param contentIdOverride
     */
    @Override
    protected void MediaSynchroniser_setContentIdOverride(Token token, int id,
                                                          String contentIdOverride) {
        mMediaSyncManager.setContentIdOverride(id, contentIdOverride);
    }

    /**
     * @param token The token associated with this request.
     * @param id
     * @param contentTime
     * @param speed
     */
    @Override
    protected void MediaSynchroniser_setContentTimeAndSpeed(Token token, int id, String timelineSelector, long contentTime,
                                                            double speed) {
        mMediaSyncManager.setContentTimeAndSpeed(timelineSelector, contentTime, speed);
    }

    /**
     * @param token The token associated with this request.
     * @param id
     * @param properties
     */
    @Override
    protected void MediaSynchroniser_updateCssCiiProperties(Token token, int id, String contentId, String presentationStatus, String contentIdStatus, String mrsUrl) {
        mMediaSyncManager.updateCssCiiProperties(contentId, presentationStatus, contentIdStatus, mrsUrl);
    }

    /**
     * @param token The token associated with this request.
     * @param id
     * @param properties
     */
    @Override
    protected long MediaSynchroniser_getBroadcastCurrentTime(Token token, String timelineSelector) {
        return mMediaSyncManager.getContentTime(timelineSelector);
    }

    /**
     * @param token The token associated with this request.
     * @param id
     * @param properties
     */
    @Override
    protected boolean MediaSynchroniser_setTimelineAvailability(Token token, int id, String timelineSelector, boolean isAvailable, long ticks, double speed) {
        return mMediaSyncManager.setTimelineAvailability(id, timelineSelector, isAvailable, ticks, speed);
    }

    /**
     * @param token The token associated with this request.
     *
     * @return
     */
    @Override
    protected String CSManager_getApp2AppLocalBaseURL(Token token) {
        return mTvBrowserSession.getApp2AppLocalBaseUrl();
    }

    /**
     * @param token The token associated with this request.
     *
     * @return
     */
    @Override
    protected String CSManager_getInterDevSyncURL(Token token) {
        return mTvBrowserSession.getInterDevSyncUrl();
    }

    /**
     * @param token The token associated with this request.
     *
     * @return
     */
    @Override
    protected String CSManager_getApp2AppRemoteBaseURL(Token token) {
        return mTvBrowserSession.getApp2AppRemoteBaseUrl();
    }

    /**
     * Get the list of supported DRM System IDs currently available. Once called,
     * the caller can track the availability changes by listening to
     * onDRMSystemStatusChange events. DRM System ID can enter the following states:
     *    - 0 READY, fully initialised and ready
     *    - 1 UNKNOWN, no longer available
     *    - 2 INITIALISING, initialising and not ready to communicate
     *    - 3 ERROR, in error state
     *
     * @return JSONObject {
     *    "drmSystemIDs": array // Array of supported DRM System IDs currently available
     *       [
     *          {
     *             "DRMSystemID": string,        // ID of the DRM System
     *             "status": int,                // status of the indicated DRM system
     *             "protectionGateways": string, // space separated list of zero or more
     *                // CSP Gateway types that are capable of supporting the DRM system
     *             "supportedFormats": string,   // space separated list of zero or more
     *                // supported file and/or container formats by the DRM system
     *          }, (...)
     *       ]
     * }
     * @throws JSONException
     */
    @Override
    protected List<TvBrowserTypes.DRMSystemStatus> Drm_getSupportedDRMSystemIDs(Token token) {
        return mTvBrowserCallback.getSupportedDRMSystemIDs();
    }

    /**
     * Send message to the DRM system.
     *
     * @param params JSONObject {
     *    "msgID": string,      // unique ID to identify the message, to be passed as
     *                          // the 'msgID' argument for onDRMMessageResult
     *    "msgType": string,    // message type as defined by the DRM system
     *    "msg": string,        // message to be provided to the underlying DRM system
     *    "DRMSystemID": string // ID of the DRM System
     * }
     * @return JSONObject {
     * }
     * @throws JSONException
     */
    @Override
    protected String Drm_sendDRMMessage(Token token, String msgID, String msgType, String msg,
                                        String DRMSystemID, boolean block) {
        return mTvBrowserCallback.sendDRMMessage(msgID, msgType, msg, DRMSystemID, block);
    }

    /**
     * Checks the availability of a valid license for playing a protected content item.
     *
     * @param params JSONObject {
     *    "DRMPrivateData": string, // DRM proprietary private data
     *    "DRMSystemID": string,    // ID of the DRM System
     * }
     * @return JSONObject {
     *    "canPlayContent": boolean, // true if the application can be created, false otherwise
     * }
     * @throws JSONException
     */
    @Override
    protected boolean Drm_canPlayContent(Token token, String DRMPrivateData, String DRMSystemID) {
        return mTvBrowserCallback.canPlayContent(DRMPrivateData, DRMSystemID);
    }

    /**
     * Checks the availability of a valid license for recording a protected content item.
     *
     * @param params JSONObject {
     *    "DRMPrivateData": string, // DRM proprietary private data
     *    "DRMSystemID": string,    // ID of the DRM System
     * }
     * @return JSONObject {
     *    "canRecordContent": boolean, // true if the application can be created, false otherwise
     * }
     * @throws JSONException
     */
    @Override
    protected boolean Drm_canRecordContent(Token token, String DRMPrivateData, String DRMSystemID) {
        return mTvBrowserCallback.canRecordContent(DRMPrivateData, DRMSystemID);
    }

    /**
     * Set the DRM system, that the terminal shall use for playing protected broadband content.
     *
     * @param params JSONObject {
     *    "DRMSystemID": string, // ID of the DRM System
     * }
     * @return JSONObject {
     *    "success": boolean, // true if the application can be created, false otherwise
     * }
     * @throws JSONException
     */
    @Override
    protected boolean Drm_setActiveDRM(Token token, String DRMSystemID) {
        return mTvBrowserCallback.setActiveDRM(DRMSystemID);
    }

    /**
     * Publish a test report (debug build only).
     *
     * @param token The token associated with this request.
     * @param testSuite A unique test suite name.
     * @param xml The XML test report.
     */
    @Override
    protected void OrbDebug_publishTestReport(Token token, String testSuite, String xml) {
        mTvBrowserCallback.publishTestReport(testSuite, xml);
    }
}
