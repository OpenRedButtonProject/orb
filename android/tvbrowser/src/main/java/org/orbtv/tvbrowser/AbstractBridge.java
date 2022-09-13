/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 * <p>
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

package org.orbtv.tvbrowser;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.util.ArrayList;
import java.util.List;

public abstract class AbstractBridge {
    public interface SessionCallback {
        int FOR_RUNNING_APP_ONLY = 0;
        int FOR_BROADCAST_APP_ONLY = 1;
        int FOR_BROADCAST_OR_TRANSITIONING_APP_ONLY = 2;
        int FOR_TRUSTED_APP_ONLY = 3;

        boolean isRequestAllowed(int callingAppId, String callingPageUrl, int methodRequirement);

        void dispatchEvent(String type, JSONObject properties);
    }

    private SessionCallback mSessionCallback;

    /**
     * Set the session callback.
     *
     * @param sessionCallback An implementation of AbstractBridge.SessionCallback.
     */
    public void setSessionCallback(SessionCallback sessionCallback) {
        mSessionCallback = sessionCallback;
    }

    /**
     * Called when the service list changes.
     */
    public void dispatchServiceListChangedEvent() {
        // TODO Is it correct this does nothing? Can we remove it?
    }

    /**
     * Called when the parental rating of the broadcast content changes.
     *
     * @param blocked True if the broadcast content is blocked; or false otherwise.
     */
    public void dispatchParentalRatingChangeEvent(boolean blocked) {
        JSONObject properties = new JSONObject();
        try {
            properties.put("blocked", blocked);
        } catch (JSONException ignored) {
        }
        mSessionCallback.dispatchEvent("ParentalRatingChange", properties);
    }

    /**
     * Called when there is a parental rating error.
     */
    public void dispatchParentalRatingErrorEvent() {
        // TODO Check whether duplicate emit of ParentalRatingChange event is correct
        JSONObject properties = new JSONObject();
        mSessionCallback.dispatchEvent("ParentalRatingChange", properties);
    }

    /**
     * Called when the set of components presented on the selected broadcast channel changes.
     *
     * @param componentType The component type (COMPONENT_TYPE_* code).
     */
    public void dispatchSelectedComponentChangedEvent(int componentType) {
        JSONObject properties = new JSONObject();
        // TODO Can this check be moved to the caller?
        if ((componentType >= 0) && (componentType <= 2)) {
            try {
                properties.put("componentType", componentType);
            } catch (JSONException ignored) {
            }
        }
        mSessionCallback.dispatchEvent("SelectedComponentChanged", properties);
    }

    /**
     * Called when the set of available components on the selected broadcast channel changes.
     *
     * @param componentType The component type (COMPONENT_TYPE_* code).
     */
    public void dispatchComponentChangedEvent(int componentType) {
        JSONObject properties = new JSONObject();
        // TODO Can this check be moved to the caller?
        if ((componentType >= 0) && (componentType <= 2)) {
            try {
                properties.put("componentType", componentType);
            } catch (JSONException ignored) {
            }
        }
        mSessionCallback.dispatchEvent("ComponentChanged", properties);
    }

    /**
     * Called when the playing status of the selected broadcast channel changes.
     *
     * @param onetId Original network ID of the channel.
     * @param transId Transport stream ID of the channel.
     * @param servId Service ID of the channel.
     * @param statusCode The new play status (CHANNEL_STATUS_ code).
     * @param permanentError True if the error is permanent; or false if the error is transient
     *    (as described in OIPF Table 8).
     */
    public void dispatchChannelStatusChangedEvent(int onetId, int transId, int servId, int statusCode,
                                                  boolean permanentError) {
        JSONObject properties = new JSONObject();
        boolean permanent = false;
        try {
            properties.put("onetId", onetId);
            properties.put("transId", transId);
            properties.put("servId", servId);
            properties.put("statusCode", statusCode);
            // TODO Can this logic be moved to the caller
            if (statusCode >= TvBrowserTypes.CHANNEL_STATUS_NOT_SUPPORTED) {
                properties.put("permanentError", permanentError);
                permanent = permanentError;
            }
        } catch (JSONException ignored) {
        }

        mSessionCallback.dispatchEvent("ChannelStatusChanged", properties);
    }

    /**
     * Called when the present/following programme changes on the selected broadcast channel.
     */
    public void dispatchProgrammesChangedEvent() {
        JSONObject properties = new JSONObject();
        mSessionCallback.dispatchEvent("ProgrammesChanged", properties);
    }

    /**
     * Called when the video aspect for the selected channel changes.
     *
     * @param aspectRatioCode Code as defined by TvBrowserTypes.ASPECT_RATIO_*
     */
    public void dispatchVideoAspectRatioChangedEvent(int aspectRatioCode) {
        // TODO
    }

    /**
     * Called when an application fails to load.
     */
    public void dispatchApplicationLoadErrorEvent() {
        JSONObject properties = new JSONObject();
        mSessionCallback.dispatchEvent("ApplicationLoadError", properties);
    }

    /**
     * Called when the running application transitions to broadcast-related.
     */
    public void dispatchTransitionedToBroadcastRelatedEvent() {
        JSONObject properties = new JSONObject();
        mSessionCallback.dispatchEvent("TransitionedToBroadcastRelated", properties);
    }

    /**
     * Called when the system is running low on available memory for the application.
     */
    public void dispatchLowMemoryEventEvent() {
        JSONObject properties = new JSONObject();
        mSessionCallback.dispatchEvent("LowMemory", properties);
    }

    /**
     * TODO Comment for media synchroniser
     */
    public void dispatchTimelineAvailableEvent(TvBrowserTypes.Timeline timeline) {
        JSONObject properties = new JSONObject();
        try {
            properties.put("timeline", timeline.toJSONObject());
        } catch (JSONException ignored) {
        }
        mSessionCallback.dispatchEvent("TimelineAvailable", properties);
    }

    /**
     * TODO Comment for media synchroniser
     */
    public void dispatchTimelineUnavailableEvent(String timelineSelector) {
        JSONObject properties = new JSONObject();
        try {
            properties.put("timelineSelector", timelineSelector);
        } catch (JSONException ignored) {
        }
        mSessionCallback.dispatchEvent("TimelineUnavailable", properties);
    }

    /**
     * TODO Comment for media synchroniser
     */
    public void dispatchInterDeviceSyncEnabled(int mediaSyncId) {
        JSONObject properties = new JSONObject();
        try {
            properties.put("id", mediaSyncId);
        } catch (JSONException ignored) {
        }
        mSessionCallback.dispatchEvent("InterDeviceSyncEnabled", properties);
    }

    /**
     * TODO Comment for media synchroniser
     */
    public void dispatchInterDeviceSyncDisabled(int mediaSyncId) {
        JSONObject properties = new JSONObject();
        try {
            properties.put("id", mediaSyncId);
        } catch (JSONException ignored) {
        }
        mSessionCallback.dispatchEvent("InterDeviceSyncDisabled", properties);
    }

    /**
     * TODO Comment for media synchroniser
     *
     * @param discoveredTerminals
     */
    public void dispatchHbbTVTerminalDiscoveryEvent(JSONArray discoveredTerminals) {
        JSONObject properties = new JSONObject();
        try {
            properties.put("discoveredTerminals", discoveredTerminals);
        } catch (JSONException ignored) {
        }
        mSessionCallback.dispatchEvent("HbbTVTerminalDiscovery", properties);
    }

    /**
     * Called when the user has decided whether the application at the given origin should be allowed
     * access to a distinctive identifier or not.
     *
     * @param origin The origin of the requesting application
     * @param accessAllowed True if access allowed; or false otherwise.
     */
    public void dispatchAccessToDistinctiveIdentifierEvent(String origin, boolean accessAllowed) {
        JSONObject properties = new JSONObject();
        try {
            properties.put("allowAccess", accessAllowed);
        } catch (JSONException ignored) {
        }
        mSessionCallback.dispatchEvent("accesstodistinctiveidentifier", properties);
    }

    /**
     * Called when the status of a metadata search changes.
     *
     * @param search The ID from the query string that started the search.
     * @param status The search status (SEARCH_STATUS_ code).
     * @param programmes The result, a list of Programmes, possibly a subset of the total result
     *    set.
     * @param offset Offset into the total result set.
     * @param totalSize Size of the total result set.
     */
    public void dispatchMetadataSearchCompleted(int search, int status,
                                                List<TvBrowserTypes.Programme> programmes, int offset, int totalSize) {
        JSONObject properties = new JSONObject();
        try {
            properties.put("search", search);
            properties.put("status", status);
            properties.put("programmeList", TvBrowserTypes.toJSONArray(programmes));
            properties.put("offset", offset);
            properties.put("totalSize", totalSize);
        } catch (JSONException ignored) {
        }
        mSessionCallback.dispatchEvent("MetadataSearch", properties);
    }

   /*
   public void onUrlLoadingError(String url, String errorMsg) {
      JSONObject properties = new JSONObject();
      try {
         properties.put("url", url);
      } catch (JSONException ignored) {
      }
      dispatchEvent("ApplicationLoadError", properties;
      mApplicationManager.destroyApplication();
   }
   */

    /**
     * @param id The ID of the added stream event listener.
     * @param name The name of the event.
     * @param data The data associated with the stream event.
     * @param text The data associated with the stream event, converted to a UTF-8 string.
     * @param status The status of the stream event.
     */
    public void dispatchStreamEvent(int id, String name, String data, String text, String status) {
        JSONObject properties = new JSONObject();
        try {
            properties.put("id", id);
            properties.put("name", name);
            properties.put("data", data);
            properties.put("text", text);
            properties.put("status", status);
        } catch (JSONException ignored) {
        }
        mSessionCallback.dispatchEvent("StreamEvent", properties);
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
     * @param fullScreen True if the broadcast video should be full screen, layered above the
     *    browser; or false if the broadcast video should be the size and position of the rectangle,
     *    layered below the browser.
     */
    protected abstract void Broadcast_setVideoRectangle(Token token, int x, int y, int width,
                                                        int height, boolean fullScreen);

    /**
     * Get the current broadcast channel.
     *
     * Security: FOR_BROADCAST_APP_ONLY.
     *
     * @param token The token associated with this request.
     *
     * @return A Channel object representing the current broadcast channel; or null if not available.
     */
    protected abstract TvBrowserTypes.Channel Broadcast_getCurrentChannel(Token token);

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
    protected abstract TvBrowserTypes.Channel Broadcast_getCurrentChannelForEvent(Token token);

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
    protected abstract List<TvBrowserTypes.Channel> Broadcast_getChannelList(Token token);

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
    protected abstract int Broadcast_setChannelToNull(Token token);

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
    protected abstract int Broadcast_setChannelToCcid(Token token, String ccid, boolean trickplay,
                                                      String contentAccessDescriptorURL, int quiet);

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
    protected abstract int Broadcast_setChannelToTriplet(Token token, int idType, int onid, int tsid,
                                                         int sid, int sourceID, String ipBroadcastID, boolean trickplay,
                                                         String contentAccessDescriptorURL, int quiet);

    /**
     * Select the broadcast channel with the given DSD. 8 Security: FOR_RUNNING_APP_ONLY.
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
    protected abstract int Broadcast_setChannelToDsd(Token token, String dsd, int sid,
                                                     boolean trickplay, String contentAccessDescriptorURL, int quiet);

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
    protected abstract List<TvBrowserTypes.Programme> Broadcast_getProgrammes(Token token,
                                                                              String ccid);

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
    protected abstract List<TvBrowserTypes.Component> Broadcast_getComponents(Token token,
                                                                              String ccid, int type);

    /**
     * Select the broadcast component with the given type, PID and optionally language.
     *
     * Security: FOR_BROADCAST_APP_ONLY.
     *
     * @param token The token associated with this request.
     * @param type The type of the component to select (COMPONENT_TYPE_* code).
     * @param pid The PID of the component to select.
     * @param language Optionally, the language of the component to select; or an empty string
     *    otherwise.
     */
    protected abstract void Broadcast_selectComponent(Token token, int type, int pid,
                                                      String language);

    /**
     * Unselect the broadcast component with the given type and PID.
     *
     * Security: FOR_BROADCAST_APP_ONLY.
     *
     * @param token The token associated with this request.
     * @param type The type of the component to unselect (COMPONENT_TYPE_* code).
     * @param pid The PID of the component to unselect.
     */
    protected abstract void Broadcast_unselectComponent(Token token, int type, int pid);

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
    protected abstract void Broadcast_startSearch(Token token, String query, int offset, int count,
                                                  List<String> channelConstraintList);

    /**
     * Abort a started metadata search.
     *
     * Security: FOR_BROADCAST_APP_ONLY.
     *
     * @param token The token associated with this request.
     * @param queryId The ID from the query string that started the search.
     */
    protected abstract void Broadcast_abortSearch(Token token, int queryId);

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
    protected abstract int Broadcast_addStreamEventListener(Token token, String targetURL,
                                                            String eventName, int componentTag, int streamEventId);

    /**
     * Remove the given stream event listener.
     *
     * @param token The token associated with this request.
     * @param id The listener ID to remove.
     */
    protected abstract void Broadcast_removeStreamEventListener(Token token, int id);

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
    protected abstract void Broadcast_setPresentationSuspended(Token token,
                                                               boolean presentationSuspended);

    /**
     * Get the parental rating of the current broadcast programme.
     *
     * @param token The token associated with this request.
     *
     * @return A ParentalRating object representing the parental rating; or null if not available.
     */
    protected abstract TvBrowserTypes.ParentalRating Programme_getParentalRating(Token token);

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
    protected abstract List<String> Programme_getSIDescriptors(String ccid, String programmeID,
                                                               int descriptorTag, int descriptorTagExtension, int privateDataSpecifier);

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
    protected abstract boolean Manager_createApplication(Token token, String url);

    /**
     * Destroy the calling application.
     *
     * The calling application is identified by the token associated with the request.
     *
     * @param token The token associated with this request.
     */
    protected abstract void Manager_destroyApplication(Token token);

    /**
     * Show the calling application.
     *
     * The calling application is identified by the token associated with the request.
     *
     * @param token The token associated with this request.
     */
    protected abstract void Manager_showApplication(Token token);

    /**
     * Hide the calling application.
     *
     * The calling application is identified by the token associated with the request.
     *
     * @param token The token associated with this request.
     */
    protected abstract void Manager_hideApplication(Token token);

    /**
     * Get the free memory available to the application.
     *
     * @param token The token associated with this request.
     *
     * @return The free memory in bytes.
     */
    protected abstract long Manager_getFreeMem(Token token);

    /**
     * Get the keyset for this application.
     *
     * The calling application is identified by the token associated with the request.
     *
     * @param token The token associated with this request.
     *
     * @return The keyset for this application.
     */
    protected abstract int Manager_getKeyValues(Token token);

    /**
     * Get the maximum keyset available to applications.
     *
     * @param token The token associated with this request.
     *
     * @return The maximum keyset available to applications.
     */
    protected abstract int Manager_getKeyMaximumValue(Token token);

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
    protected abstract int Manager_setKeyValue(Token token, int value);

    /**
     * Get the URL of an icon that represents the given key.
     *
     * @param token The token associated with this request.
     * @param code The code of the key to get an icon for (VK_ code).
     *
     * @return A URL of an icon that represents the given key; or null if not available.
     */
    protected abstract String Manager_getKeyIcon(Token token, int code);

    /**
     * Get a list of rating schemes supported by this integration.
     *
     * @param token The token associated with this request.
     *
     * @return A list of ParentalRatingScheme objects representing the supported parental rating
     *    schemes; or an empty list of not available.
     */
    protected abstract List<TvBrowserTypes.ParentalRatingScheme> ParentalControl_getRatingSchemes(Token token);

    /**
     * Get the parental rating threshold currently set on the system for the given scheme.
     *
     * @param token The token associated with this request.
     * @param schemeName The name of the scheme to get the threshold of.
     *
     * @return A ParentalRating object representing the threshold.
     */
    protected abstract TvBrowserTypes.ParentalRating ParentalControl_getThreshold(Token token, String schemeName);

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
    protected abstract boolean ParentalControl_isRatingBlocked(Token token, String scheme, String region, int value);

    /**
     * Get certain immutable information about the system.
     *
     * @param token The token associated with this request.
     *
     * @return A SystemInformation object.
     */
    protected abstract TvBrowserTypes.SystemInformation Configuration_getLocalSystem(Token token);

    /**
     * Get preferred languages to be used for audio playback on this system.
     *
     * @param token The token associated with this request.
     *
     * @return Comma separated string of languages (ISO 639-2 codes), in order of preference.
     */
    protected abstract String Configuration_getPreferredAudioLanguage(Token token);

    /**
     * Get preferred languages to be used for subtitles on this system.
     *
     * @param token The token associated with this request.
     *
     * @return Comma separated string of languages (ISO 639-2 codes), in order of preference.
     */
    protected abstract String Configuration_getPreferredSubtitleLanguage(Token token);

    /**
     * Get preferred languages to be used for the user-interface on this system.
     *
     * @param token The token associated with this request.
     *
     * @return Comma separated string of languages (ISO 639-2 codes), in order of preference.
     */
    protected abstract String Configuration_getPreferredUILanguage(Token token);

    /**
     * Get a string containing the three character country code identifying the country this system
     * is deployed.
     *
     * @param token The token associated with this request.
     *
     * @return Country code the receiver is deployed (ISO 3166-1 alpha-3 code).
     */
    protected abstract String Configuration_getCountryId(Token token);

    /**
     * Get whether subtitles are enabled on this system.
     *
     * @param token The token associated with this request.
     *
     * @return True if subtitles are enabled; or false otherwise.
     */
    protected abstract boolean Configuration_getSubtitlesEnabled(Token token);

    /**
     * Get whether audio description is enabled on this system.
     *
     * @param token The token associated with this request.
     *
     * @return True if audio description is enabled; or false otherwise.
     */
    protected abstract boolean Configuration_getAudioDescriptionEnabled(Token token);

    /**
     * Get the DVB network IDs of the channels in the broadcast channel list.
     *
     * @param token The token associated with this request.
     *
     * @return A list of DVB network IDs; or an empty list of not available.
     */
    protected abstract List<Integer> Configuration_getDttNetworkIds(Token token);

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
    protected abstract String Configuration_getDeviceId(Token token);

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
    protected abstract void Configuration_requestAccessToDistinctiveIdentifier(Token token);

    /**
     * @param token The token associated with this request.
     *
     * @return
     */
    protected abstract int MediaSynchroniser_instantiate(Token token);

    /**
     * @param token The token associated with this request.
     * @param id
     * @param isMasterBroadcast
     *
     * @return
     */
    protected abstract boolean MediaSynchroniser_initialise(Token token, int id,
                                                            boolean isMasterBroadcast);

    /**
     * @param token The token associated with this request.
     * @param id
     */
    protected abstract void MediaSynchroniser_destroy(Token token, int id);

    /**
     * @param token The token associated with this request.
     * @param id
     */
    protected abstract boolean MediaSynchroniser_enableInterDeviceSync(Token token, int id);

    /**
     * @param token The token associated with this request.
     * @param id
     */
    protected abstract void MediaSynchroniser_disableInterDeviceSync(Token token, int id);

    /**
     * @param token The token associated with this request.
     * @param id
     *
     * @return
     */
    protected abstract int MediaSynchroniser_nrOfSlaves(Token token, int id);

    /**
     * @param token The token associated with this request.
     * @param id
     *
     * @return
     */
    protected abstract boolean MediaSynchroniser_interDeviceSyncEnabled(Token token, int id);

    /**
     * @param token The token associated with this request.
     * @param id
     *
     * @return
     */
    protected abstract String MediaSynchroniser_getContentIdOverride(Token token, int id);

    /**
     * @param token The token associated with this request.
     * @param id
     * @param timelineSelector
     *
     * @return
     */
    protected abstract long MediaSynchroniser_getBroadcastCurrentTime(Token token, String timelineSelector);

    /**
     * @param token The token associated with this request.
     * @param id
     * @param timelineSelector
     * @param isMaster
     *
     * @return
     */
    protected abstract boolean MediaSynchroniser_startTimelineMonitoring(Token token, int id,
                                                                         String timelineSelector, boolean isMaster);

    /**
     * @param token The token associated with this request.
     * @param id
     * @param timelineSelector
     * @param forceStop
     */
    protected abstract void MediaSynchroniser_stopTimelineMonitoring(Token token, int id,
                                                                     String timelineSelector, boolean forceStop);

    /**
     * @param token The token associated with this request.
     * @param id
     * @param contentIdOverride
     */
    protected abstract void MediaSynchroniser_setContentIdOverride(Token token, int id,
                                                                   String contentIdOverride);

    /**
     * @param token The token associated with this request.
     * @param id
     * @param timelineSelector
     * @param contentTime
     * @param speed
     */
    protected abstract void MediaSynchroniser_setContentTimeAndSpeed(Token token, int id,
                                                                     String timelineSelector, long contentTime, double speed);

    /**
     * @param token The token associated with this request.
     * @param id
     * @param properties
     */
    protected abstract void MediaSynchroniser_updateCssCiiProperties(Token token, int id,
                                                                     String contentId, String presentationStatus, String contentIdStatus, String mrsUrl);

    /**
     * @param token The token associated with this request.
     * @param id
     * @param timelineSelector
     * @param isAvailable
     * @param ticks
     * @param speed
     */
    protected abstract boolean MediaSynchroniser_setTimelineAvailability(Token token, int id,
                                                                         String timelineSelector, boolean isAvailable, long ticks, double speed);


    /**
     * @param token The token associated with this request.
     *
     * @return
     */
    protected abstract String CSManager_getApp2AppLocalBaseURL(Token token);

    /**
     * @param token The token associated with this request.
     *
     * @return
     */
    protected abstract String CSManager_getInterDevSyncURL(Token token);

    /**
     * @param token The token associated with this request.
     *
     * @return
     */
    protected abstract String CSManager_getApp2AppRemoteBaseURL(Token token);

    /**
     * Publish a test report (debug build only).
     *
     * @param token The token associated with this request.
     * @param testSuite A unique test suite name.
     * @param xml The XML test report.
     */
    protected abstract void OrbDebug_publishTestReport(Token token, String testSuite, String xml);

    public JSONObject request(String method, Token token, JSONObject params) throws JSONException {
        JSONObject response = new JSONObject();

        switch (method) {
            case "Broadcast.setVideoRectangle": {
                Broadcast_setVideoRectangle(
                        token,
                        params.getInt("x"),
                        params.getInt("y"),
                        params.getInt("width"),
                        params.getInt("height"),
                        params.getBoolean("fullScreen")
                );
                break;
            }

            case "Broadcast.getCurrentChannel": {
                if (!isRequestAllowed(token, SessionCallback.FOR_BROADCAST_APP_ONLY)) {
                    response.put("error", "SecurityError");
                    break;
                }
                TvBrowserTypes.Channel result = Broadcast_getCurrentChannel(
                        token
                );
                response.put("result", TvBrowserTypes.toJSONObject(result));
                break;
            }

            case "Broadcast.getCurrentChannelForEvent": {
                if (!isRequestAllowed(token, SessionCallback.FOR_BROADCAST_OR_TRANSITIONING_APP_ONLY)) {
                    response.put("error", "SecurityError");
                    break;
                }
                TvBrowserTypes.Channel result = Broadcast_getCurrentChannelForEvent(
                        token
                );
                response.put("result", TvBrowserTypes.toJSONObject(result));
                break;
            }

            case "Broadcast.getChannelList": {
                if (!isRequestAllowed(token, SessionCallback.FOR_BROADCAST_APP_ONLY)) {
                    response.put("error", "SecurityError");
                    break;
                }
                List<TvBrowserTypes.Channel> result = Broadcast_getChannelList(
                        token
                );
                response.put("result", TvBrowserTypes.toJSONArray(result));
                break;
            }

            case "Broadcast.setChannelToCcid": {
                if (!isRequestAllowed(token, SessionCallback.FOR_RUNNING_APP_ONLY)) {
                    response.put("error", "SecurityError");
                    break;
                }
                int result = Broadcast_setChannelToCcid(
                        token,
                        params.getString("ccid"),
                        params.getBoolean("trickplay"),
                        params.getString("contentAccessDescriptorURL"),
                        params.getInt("quiet")
                );
                response.put("result", result);
                break;
            }

            case "Broadcast.setChannelToNull": {
                if (!isRequestAllowed(token, SessionCallback.FOR_RUNNING_APP_ONLY)) {
                    response.put("error", "SecurityError");
                    break;
                }
                int result = Broadcast_setChannelToNull(
                        token
                );
                response.put("result", result);
                break;
            }

            case "Broadcast.setChannelToTriplet": {
                if (!isRequestAllowed(token, SessionCallback.FOR_RUNNING_APP_ONLY)) {
                    response.put("error", "SecurityError");
                    break;
                }
                int result = Broadcast_setChannelToTriplet(
                        token,
                        params.getInt("idType"),
                        params.getInt("onid"),
                        params.getInt("tsid"),
                        params.getInt("sid"),
                        params.getInt("sourceID"),
                        params.getString("ipBroadcastID"),
                        params.getBoolean("trickplay"),
                        params.getString("contentAccessDescriptorURL"),
                        params.getInt("quiet")
                );
                response.put("result", result);
                break;
            }

            case "Broadcast.setChannelToDsd": {
                if (!isRequestAllowed(token, SessionCallback.FOR_RUNNING_APP_ONLY)) {
                    response.put("error", "SecurityError");
                    break;
                }
                int result = Broadcast_setChannelToDsd(
                        token,
                        params.getString("dsd"),
                        params.getInt("sid"),
                        params.getBoolean("trickplay"),
                        params.getString("contentAccessDescriptorURL"),
                        params.getInt("quiet")
                );
                response.put("result", result);
                break;
            }

            case "Broadcast.getProgrammes": {
                if (!isRequestAllowed(token, SessionCallback.FOR_BROADCAST_APP_ONLY)) {
                    response.put("error", "SecurityError");
                    break;
                }
                List<TvBrowserTypes.Programme> result = Broadcast_getProgrammes(
                        token,
                        params.getString("ccid")
                );
                response.put("result", TvBrowserTypes.toJSONArray(result));
                break;
            }

            case "Broadcast.getComponents": {
                if (!isRequestAllowed(token, SessionCallback.FOR_BROADCAST_APP_ONLY)) {
                    response.put("error", "SecurityError");
                    break;
                }
                List<TvBrowserTypes.Component> result = Broadcast_getComponents(
                        token,
                        params.getString("ccid"),
                        params.getInt("type")
                );
                response.put("result", TvBrowserTypes.toJSONArray(result));
                break;
            }

            case "Broadcast.selectComponent": {
                if (!isRequestAllowed(token, SessionCallback.FOR_BROADCAST_APP_ONLY)) {
                    response.put("error", "SecurityError");
                    break;
                }
                Broadcast_selectComponent(
                        token,
                        params.getInt("type"),
                        params.getInt("pid"),
                        params.getString("language")
                );
                break;
            }
            case "Broadcast.unselectComponent": {
                if (!isRequestAllowed(token, SessionCallback.FOR_BROADCAST_APP_ONLY)) {
                    response.put("error", "SecurityError");
                    break;
                }
                Broadcast_unselectComponent(
                        token,
                        params.getInt("type"),
                        params.getInt("pid")
                );
                break;
            }

            case "Broadcast.startSearch": {
                if (!isRequestAllowed(token, SessionCallback.FOR_BROADCAST_APP_ONLY)) {
                    response.put("error", "SecurityError");
                    break;
                }
                // TODO
                List<String> channelConstraintList = new ArrayList<>();
                JSONArray channelConstraints = params.getJSONArray("channelConstraints");
                for (int i = 0; i < channelConstraints.length(); i++) {
                    channelConstraintList.add(channelConstraints.getString(i));
                }

                Broadcast_startSearch(
                        token,
                        params.getString("query"),
                        params.getInt("offset"),
                        params.getInt("count"),
                        channelConstraintList
                );
                break;
            }

            case "Broadcast.abortSearch": {
                if (!isRequestAllowed(token, SessionCallback.FOR_BROADCAST_APP_ONLY)) {
                    response.put("error", "SecurityError");
                    break;
                }
                Broadcast_abortSearch(
                        token,
                        params.getInt("queryId")
                );
                break;
            }

            case "Broadcast.addStreamEventListener": {
                int result = Broadcast_addStreamEventListener(
                        token,
                        params.getString("targetURL"),
                        params.getString("eventName"),
                        params.getInt("componentTag"),
                        params.getInt("streamEventId")
                );
                response.put("result", result);
                break;
            }

            case "Broadcast.removeStreamEventListener": {
                Broadcast_removeStreamEventListener(
                        token,
                        params.getInt("id")
                );
                break;
            }

            case "Broadcast.setPresentationSuspended": {
                if (!isRequestAllowed(token, SessionCallback.FOR_BROADCAST_APP_ONLY)) {
                    response.put("error", "SecurityError");
                    break;
                }
                Broadcast_setPresentationSuspended(
                        token,
                        params.getBoolean("presentationSuspended")
                );
                break;
            }

            case "Programme.getParentalRating": {
                TvBrowserTypes.ParentalRating result = Programme_getParentalRating(
                        token
                );
                response.put("result", TvBrowserTypes.toJSONObject(result));
                break;
            }

            case "Programme.getSIDescriptors": {
                List<String> result = Programme_getSIDescriptors(
                        params.getString("ccid"),
                        params.getString("programmeID"),
                        params.getInt("descriptorTag"),
                        params.getInt("descriptorTagExtension"),
                        params.getInt("privateDataSpecifier")
                );
                response.put("result", new JSONArray(result));
                break;
            }

            case "Manager.createApplication": {
                boolean result = Manager_createApplication(
                        token,
                        params.getString("url")
                );
                response.put("result", result);
                break;
            }

            case "Manager.destroyApplication": {
                Manager_destroyApplication(
                        token
                );
                break;
            }

            case "Manager.showApplication": {
                Manager_showApplication(
                        token
                );
                break;
            }

            case "Manager.hideApplication": {
                Manager_hideApplication(
                        token
                );
                break;
            }

            case "Manager.getFreeMem": {
                long result = Manager_getFreeMem(
                        token
                );
                response.put("result", result);
                break;
            }

            case "Manager.getKeyValues": {
                int result = Manager_getKeyValues(
                        token
                );
                response.put("result", result);
                break;
            }

            case "Manager.getKeyMaximumValue": {
                int result = Manager_getKeyMaximumValue(
                        token
                );
                response.put("result", result);
                break;
            }

            case "Manager.setKeyValue": {
                int result = Manager_setKeyValue(
                        token,
                        params.getInt("value")
                );
                response.put("result", result);
                break;
            }

            case "Manager.getKeyIcon": {
                String result = Manager_getKeyIcon(
                        token,
                        params.getInt("code")
                );
                response.put("result", result);
                break;
            }

            case "ParentalControl.getRatingSchemes": {
                List<TvBrowserTypes.ParentalRatingScheme> result = ParentalControl_getRatingSchemes(
                        token
                );
                response.put("result", TvBrowserTypes.toJSONArray(result));
                break;
            }

            case "ParentalControl.getThreshold": {
                TvBrowserTypes.ParentalRating result = ParentalControl_getThreshold(
                        token,
                        params.getString("schemeName")
                );
                response.put("result", TvBrowserTypes.toJSONObject(result));
                break;
            }

            case "ParentalControl.isRatingBlocked": {
                boolean result = ParentalControl_isRatingBlocked(
                        token,
                        params.getString("scheme"),
                        params.getString("region"),
                        params.getInt("value")
                );
                response.put("result", result);
                break;
            }

            case "Configuration.getLocalSystem": {
                TvBrowserTypes.SystemInformation result = Configuration_getLocalSystem(
                        token
                );
                response.put("result", TvBrowserTypes.toJSONObject(result));
                break;
            }

            case "Configuration.getPreferredAudioLanguage": {
                String result = Configuration_getPreferredAudioLanguage(
                        token
                );
                response.put("result", result);
                break;
            }

            case "Configuration.getPreferredSubtitleLanguage": {
                String result = Configuration_getPreferredSubtitleLanguage(
                        token
                );
                response.put("result", result);
                break;
            }

            case "Configuration.getPreferredUILanguage": {
                String result = Configuration_getPreferredUILanguage(
                        token
                );
                response.put("result", result);
                break;
            }

            case "Configuration.getCountryId": {
                String result = Configuration_getCountryId(
                        token
                );
                response.put("result", result);
                break;
            }

            case "Configuration.getSubtitlesEnabled": {
                boolean result = Configuration_getSubtitlesEnabled(
                        token
                );
                response.put("result", result);
                break;
            }

            case "Configuration.getAudioDescriptionEnabled": {
                boolean result = Configuration_getAudioDescriptionEnabled(
                        token
                );
                response.put("result", result);
                break;
            }

            case "Configuration.getDttNetworkIds": {
                List<Integer> result = Configuration_getDttNetworkIds(
                        token
                );
                if (result == null) {
                    response.put("result", new JSONArray());
                } else {
                    response.put("result", new JSONArray(result));
                }
                break;
            }

            case "Configuration.getDeviceId": {
                String result = Configuration_getDeviceId(
                        token
                );
                response.put("result", result);
                break;
            }

            case "Configuration.requestAccessToDistinctiveIdentifier": {
                Configuration_requestAccessToDistinctiveIdentifier(
                        token
                );
                break;
            }

            case "MediaSynchroniser.instantiate": {
                int result = MediaSynchroniser_instantiate(
                        token
                );
                response.put("result", result);
                break;
            }

            case "MediaSynchroniser.initialise": {
                boolean result = MediaSynchroniser_initialise(
                        token,
                        params.getInt("id"),
                        params.getBoolean("isMasterBroadcast")
                );
                response.put("result", result);
                break;
            }

            case "MediaSynchroniser.destroy": {
                MediaSynchroniser_destroy(
                        token,
                        params.getInt("id")
                );
                break;
            }

            case "MediaSynchroniser.enableInterDeviceSync": {
                boolean result = MediaSynchroniser_enableInterDeviceSync(
                        token,
                        params.getInt("id")
                );
                response.put("result", result);
                break;
            }

            case "MediaSynchroniser.disableInterDeviceSync": {
                MediaSynchroniser_disableInterDeviceSync(
                        token,
                        params.getInt("id")
                );
                break;
            }

            case "MediaSynchroniser.nrOfSlaves": {
                int result = MediaSynchroniser_nrOfSlaves(
                        token,
                        params.getInt("id")
                );
                response.put("result", result);
                break;
            }

            case "MediaSynchroniser.interDeviceSyncEnabled": {
                boolean result = MediaSynchroniser_interDeviceSyncEnabled(
                        token,
                        params.getInt("id")
                );
                response.put("result", result);
                break;
            }

            case "MediaSynchroniser.getContentIdOverride": {
                String result = MediaSynchroniser_getContentIdOverride(
                        token,
                        params.getInt("id")
                );
                response.put("result", result);
                break;
            }

            case "MediaSynchroniser.getBroadcastCurrentTime": {
                long result = MediaSynchroniser_getBroadcastCurrentTime(
                        token,
                        params.getString("timelineSelector")
                );
                response.put("result", result);
                break;
            }

            case "MediaSynchroniser.startTimelineMonitoring": {
                boolean result = MediaSynchroniser_startTimelineMonitoring(
                        token,
                        params.getInt("id"),
                        params.getString("timelineSelector"),
                        params.getBoolean("isMaster")
                );
                response.put("result", result);
                break;
            }

            case "MediaSynchroniser.stopTimelineMonitoring": {
                MediaSynchroniser_stopTimelineMonitoring(
                        token,
                        params.getInt("id"),
                        params.getString("timelineSelector"),
                        params.getBoolean("forceStop")
                );
                break;
            }

            case "MediaSynchroniser.setContentIdOverride": {
                MediaSynchroniser_setContentIdOverride(
                        token,
                        params.getInt("id"),
                        params.getString("contentIdOverride")
                );
                break;
            }

            case "MediaSynchroniser.setContentTimeAndSpeed": {
                MediaSynchroniser_setContentTimeAndSpeed(
                        token,
                        params.getInt("id"),
                        params.getString("timelineSelector"),
                        params.getLong("contentTime"),
                        params.getDouble("speed")
                );
                break;
            }

            case "MediaSynchroniser.updateCssCiiProperties": {
                MediaSynchroniser_updateCssCiiProperties(
                        token,
                        params.getInt("id"),
                        params.getString("contentId"),
                        params.getString("presentationStatus"),
                        params.getString("contentIdStatus"),
                        params.getString("mrsUrl")
                );
                break;
            }

            case "MediaSynchroniser.setTimelineAvailability": {
                boolean result = MediaSynchroniser_setTimelineAvailability(
                        token,
                        params.getInt("id"),
                        params.getString("timelineSelector"),
                        params.getBoolean("isAvailable"),
                        params.getLong("ticks"),
                        params.getDouble("speed")
                );
                response.put("result", result);
                break;
            }

            case "CSManager.getApp2AppLocalBaseURL": {
                String result = CSManager_getApp2AppLocalBaseURL(
                        token
                );
                response.put("result", result);
                break;
            }

            case "CSManager.getInterDevSyncURL": {
                String result = CSManager_getInterDevSyncURL(
                        token
                );
                response.put("result", result);
                break;
            }

            case "CSManager.getApp2AppRemoteBaseURL": {
                String result = CSManager_getApp2AppRemoteBaseURL(
                        token
                );
                response.put("result", result);
                break;
            }

            case "OrbDebug.publishTestReport": {
                if (!BuildConfig.DEBUG) {
                    break;
                }
                OrbDebug_publishTestReport(
                        token,
                        params.getString("testSuite"),
                        params.getString("xml")
                );
                break;
            }

            default: {
                response.put("error", "Unknown method");
                break;
            }
        }

        return response;
    }

    private boolean isRequestAllowed(Token token, int methodType) {
        return mSessionCallback.isRequestAllowed(token.getAppId(), token.getUri(), methodType);
    }
}
