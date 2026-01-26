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

package org.orbtv.orbpolyfill;

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
     * @param contentID content ID to which the parental rating error applies
     * @param ratings the parental rating value of the currently playing content
     * @param DRMSystemID DRM System ID of the DRM system that generated the event
     */
    public void dispatchParentalRatingErrorEvent(String contentID,
                                                 List<BridgeTypes.ParentalRating> ratings,
                                                 String DRMSystemID) {
        JSONObject properties = new JSONObject();
        try {
            properties.put("contentID", contentID);
            properties.put("ratings", BridgeTypes.toJSONArray(ratings));
            properties.put("DRMSystemID", DRMSystemID);
        } catch (JSONException ignored) {
        }
        mSessionCallback.dispatchEvent("ParentalRatingError", properties);
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
     * @param componentType If the presentation has changed for only one component type, this value
     * should be set to BridgeTypes.COMPONENT_TYPE_* for that specific type. If the presentation has
     * changed for more than one component type, this value should be set to
     * BridgeTypes.COMPONENT_TYPE_ANY.
     */
    public void dispatchComponentChangedEvent(int componentType) {
        JSONObject properties = new JSONObject();
        // TODO This check should be unnecessary
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
            if (statusCode >= BridgeTypes.CHANNEL_STATUS_NOT_SUPPORTED) {
                properties.put("permanentError", permanentError);
                permanent = permanentError;
            }
        } catch (JSONException ignored) {
        }

        mSessionCallback.dispatchEvent("ChannelStatusChanged", properties);
    }

    public void dispatchServiceInstanceChangedEvent(int index) {
        JSONObject properties = new JSONObject();
        try {
            properties.put("serviceInstanceIndex", index);
        } catch (JSONException ignored) {
        }

        mSessionCallback.dispatchEvent("ServiceInstanceChanged", properties);
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
    public void dispatchTimelineAvailableEvent(BridgeTypes.Timeline timeline) {
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
     * Called when the user changes the audio language
     *
     * @param language      The new preferred audio language
     */
    public void dispatchPreferredAudioLanguageChanged(String language) {
        JSONObject properties = new JSONObject();
        try {
            properties.put("language", language);
        } catch (JSONException ignored) {
        }
        mSessionCallback.dispatchEvent("PreferredAudioLanguageChanged", properties);
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
                                                List<BridgeTypes.Programme> programmes, int offset, int totalSize) {
        JSONObject properties = new JSONObject();
        try {
            properties.put("search", search);
            properties.put("status", status);
            properties.put("programmeList", BridgeTypes.toJSONArray(programmes));
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
    public void dispatchStreamEvent(int id, String name, String data, String text, String status, BridgeTypes.DASHEvent dashEvent) {
        JSONObject properties = new JSONObject();
        try {
            properties.put("id", id);
            properties.put("name", name);
            properties.put("data", data);
            properties.put("text", text);
            properties.put("status", status);
            if (dashEvent != null) {
                properties.put("DASHEvent", dashEvent.toJSONObject());
            }
        } catch (JSONException ignored) {
        }
        mSessionCallback.dispatchEvent("StreamEvent", properties);
    }

    /**
     * Notify about DRM licensing errors during playback of DRM protected A/V content.
     *
     * @param errorState details the type of error:
     *           - 0: no license, consumption of the content is blocked.
     *           - 1: invalid license, consumption of the content is blocked.
     *           - 2: valid license, consumption of the content is unblocked.
     * @param contentID unique identifier of the protected content
     * @param DRMSystemID ID of the DRM System
     * @param rightsIssuerURL indicates the value of the rightsIssuerURL that can
     *        be used to non-silently obtain the rights for the content item
     */
    public void dispatchDRMRightsError(int errorState, String contentID, String DRMSystemID,
                                       String rightsIssuerURL) {
        JSONObject properties = new JSONObject();
        try {
            properties.put("errorState", errorState);
            properties.put("contentID", contentID);
            properties.put("DRMSystemID", DRMSystemID);
            properties.put("rightsIssuerURL", rightsIssuerURL);
        } catch (JSONException ignored) {
        }
        mSessionCallback.dispatchEvent("DRMRightsError", properties);
    }

    /**
     * Called when the status of a DRM system changes.
     *
     * @param drmSystem ID of the DRM System
     * @param drmSystemIds List of the DRM System IDs handled by the DRM System
     * @param status status of the indicated DRM system. Possible states:
     *    - 0 READY, fully initialised and ready
     *    - 1 UNKNOWN, no longer available
     *    - 2 INITIALISING, initialising and not ready to communicate
     *    - 3 ERROR, in error state
     * @param protectionGateways space separated list of zero or more CSP Gateway
     *        types that are capable of supporting the DRM system
     * @param supportedFormats space separated list of zero or more supported
     *        file and/or container formats by the DRM system
     */
    public void dispatchDRMSystemStatusChange(String drmSystem, List<String> drmSystemIds, int status,
                                              String protectionGateways, String supportedFormats) {
        JSONObject properties = new JSONObject();
        try {
            properties.put("DRMSystem", drmSystem);
            properties.put("DRMSystemIDs", new JSONArray(drmSystemIds));
            properties.put("status", status);
            properties.put("protectionGateways", protectionGateways);
            properties.put("supportedFormats", supportedFormats);
        } catch (JSONException ignored) {
        }
        mSessionCallback.dispatchEvent("DRMSystemStatusChange", properties);
    }

    /**
     * Called when the underlying DRM system has a result message as a consequence
     * of a call to sendDRMMessage.
     *
     * @param msgID identifies the original message which has led to this resulting message
     * @param resultMsg DRM system specific result message
     * @param resultCode result code. Valid values include:
     *    - 0 Successful
     *    - 1 Unknown error
     *    - 2 Cannot process request
     *    - 3 Unknown MIME type
     *    - 4 User consent needed
     *    - 5 Unknown DRM system
     *    - 6 Wrong format
     */
    public void dispatchDRMMessageResult(String msgID, String resultMsg, int resultCode) {
        JSONObject properties = new JSONObject();
        try {
            properties.put("msgID", msgID);
            properties.put("resultMsg", resultMsg);
            properties.put("resultCode", resultCode);
        } catch (JSONException ignored) {
        }
        mSessionCallback.dispatchEvent("DRMMessageResult", properties);
    }

    /**
     * Called when the underlying DRM system has a message to report.
     *
     * @param msg DRM system specific message
     * @param DRMSystemID ID of the DRM System
     */
    public void dispatchDRMSystemMessage(String msg, String DRMSystemID) {
        JSONObject properties = new JSONObject();
        try {
            properties.put("msg", msg);
            properties.put("DRMSystemID", DRMSystemID);
        } catch (JSONException ignored) {
        }
        mSessionCallback.dispatchEvent("DRMSystemMessage", properties);
    }

    public void dispatchApplicationSchemeUpdatedEvent(String scheme) {
        JSONObject properties = new JSONObject();
        try {
            properties.put("scheme", scheme);
        } catch (JSONException ignored) {
        }
        mSessionCallback.dispatchEvent("ApplicationSchemeUpdated", properties);
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
    protected abstract void Broadcast_setVideoRectangle(BridgeToken token, int x, int y, int width,
                                                        int height);

    /**
     * Get the current broadcast channel.
     *
     * Security: FOR_BROADCAST_APP_ONLY.
     *
     * @param token The token associated with this request.
     *
     * @return A Channel object representing the current broadcast channel; or null if not available.
     */
    protected abstract BridgeTypes.Channel Broadcast_getCurrentChannel(BridgeToken token);

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
    protected abstract BridgeTypes.Channel Broadcast_getCurrentChannelForEvent(BridgeToken token);

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
    protected abstract List<BridgeTypes.Channel> Broadcast_getChannelList(BridgeToken token);

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
    protected abstract int Broadcast_setChannelToNull(BridgeToken token);

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
    protected abstract int Broadcast_setChannelToCcid(BridgeToken token, String ccid, boolean trickplay,
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
    protected abstract int Broadcast_setChannelToTriplet(BridgeToken token, int idType, int onid, int tsid,
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
    protected abstract int Broadcast_setChannelToDsd(BridgeToken token, String dsd, int sid,
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
    protected abstract List<BridgeTypes.Programme> Broadcast_getProgrammes(BridgeToken token,
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
    protected abstract List<BridgeTypes.Component> Broadcast_getComponents(BridgeToken token,
                                                                           String ccid, int type);

    /**
     * Get a private audio component in the selected channel.
     *
     * Security: FOR_BROADCAST_APP_ONLY.
     *
     * @param token The token associated with this request.
     * @param componentTag The component_tag of the component.
     *
     * @return The private component with the specified component_tag in the PMT of the
     * currently selected broadcast channel; or null if unavailable or the component is not
     * private (i.e. the stream type is audio, video or subtitle).
     *
     * Mandatory properties: id, pid and encrypted. The id property shall be usable with the
     * overrideComponentSelection method to select the component as an audio track. Other Component
     * properties are not required.
     */
    protected abstract BridgeTypes.Component Broadcast_getPrivateAudioComponent(BridgeToken token,
                                                                                String componentTag);

    /**
     * Get a private video component in the selected channel.
     *
     * Security: FOR_BROADCAST_APP_ONLY.
     *
     * @param token The token associated with this request.
     * @param componentTag The component_tag of the component.
     *
     * @return The private component with the specified component_tag in the PMT of the
     * currently selected broadcast channel; or null if unavailable or the component is not
     * private (i.e. the stream type is audio, video or subtitle).
     *
     * Mandatory properties: id, pid and encrypted. The id property shall be usable with the
     * overrideComponentSelection method to select the component as an video track. Other Component
     * properties are not required.
     */
    protected abstract BridgeTypes.Component Broadcast_getPrivateVideoComponent(BridgeToken token,
                                                                                String componentTag);

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
    protected abstract void Broadcast_overrideComponentSelection(BridgeToken token, int type, String id);

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
    protected abstract void Broadcast_restoreComponentSelection(BridgeToken token, int type);

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
    protected abstract void Broadcast_startSearch(BridgeToken token, String query, int offset, int count,
                                                  List<String> channelConstraintList);

    /**
     * Abort a started metadata search.
     *
     * Security: FOR_BROADCAST_APP_ONLY.
     *
     * @param token The token associated with this request.
     * @param queryId The ID from the query string that started the search.
     */
    protected abstract void Broadcast_abortSearch(BridgeToken token, int queryId);

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
    protected abstract int Broadcast_addStreamEventListener(BridgeToken token, String targetURL,
                                                            String eventName, int componentTag, int streamEventId);

    /**
     * Remove the given stream event listener.
     *
     * @param token The token associated with this request.
     * @param id The listener ID to remove.
     */
    protected abstract void Broadcast_removeStreamEventListener(BridgeToken token, int id);

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
    protected abstract void Broadcast_setPresentationSuspended(BridgeToken token,
                                                               boolean presentationSuspended);

    /**
     * Returns the actual volume level set.
     *
     * @param token The token associated with this request.
     * @return Integer value between 0 up to and including 100 to indicate volume level.
     */
    protected abstract int Broadcast_getVolume(BridgeToken token);

    /**
     * Adjusts the volume of the currently playing media to the volume as indicated by volume.
     *
     * @param token The token associated with this request.
     * @param volume Integer value between 0 up to and including 100 to indicate volume level.
     * @return true if the volume has changed. false if the volume has not changed.
     */
    protected abstract boolean Broadcast_setVolume(BridgeToken token, int volume);

    /**
     * Get the parental rating of the current broadcast programme.
     *
     * @param token The token associated with this request.
     *
     * @return A ParentalRating object representing the parental rating; or null if not available.
     */
    protected abstract BridgeTypes.ParentalRating Programme_getParentalRating(BridgeToken token);

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
    protected abstract boolean Manager_createApplication(BridgeToken token, String url);

    /**
     * Destroy the calling application.
     *
     * The calling application is identified by the token associated with the request.
     *
     * @param token The token associated with this request.
     */
    protected abstract void Manager_destroyApplication(BridgeToken token);

    /**
     * Show the calling application.
     *
     * The calling application is identified by the token associated with the request.
     *
     * @param token The token associated with this request.
     */
    protected abstract void Manager_showApplication(BridgeToken token);

    /**
     * Hide the calling application.
     *
     * The calling application is identified by the token associated with the request.
     *
     * @param token The token associated with this request.
     */
    protected abstract void Manager_hideApplication(BridgeToken token);

    /**
     * Get the free memory available to the application.
     *
     * @param token The token associated with this request.
     *
     * @return The free memory in bytes.
     */
    protected abstract long Manager_getFreeMem(BridgeToken token);

    /**
     * Get the keyset for this application.
     *
     * The calling application is identified by the token associated with the request.
     *
     * @param token The token associated with this request.
     *
     * @return The keyset for this application.
     */
    protected abstract int Manager_getKeyValues(BridgeToken token);

    /**
     * Get the other keys for this application.
     *
     * The calling application is identified by the token associated with the request.
     *
     * @param token The token associated with this request.
     *
     * @return The other keys for this application.
     */
    protected abstract int[] Manager_getOtherKeyValues(BridgeToken token);

    /**
     * Get the maximum keyset available to applications.
     *
     * @param token The token associated with this request.
     *
     * @return The maximum keyset available to applications.
     */
    protected abstract int Manager_getKeyMaximumValue(BridgeToken token);

    /**
     * Get the maximum other keys available to applications.
     *
     * @param token The token associated with this request.
     *
     * @return The maximum other keys available to applications.
     */
    protected abstract int Manager_getKeyMaximumOtherKeys(BridgeToken token);

    /**
     * Set the keyset for this application.
     *
     * The calling application is identified by the token associated with the request.
     *
     * @param token The token associated with this request.
     * @param value The keyset to set for this application.
     * @param otherKeys The key events which are available to the browser & are not 
     *    included in one of the keySet constants 
     *
     * @return The keyset for this application.
     */
    protected abstract int Manager_setKeyValue(BridgeToken token, int value, List<String> otherKeys);

    /**
     * Get the URL of an icon that represents the given key.
     *
     * @param token The token associated with this request.
     * @param code The code of the key to get an icon for (VK_ code).
     *
     * @return A URL of an icon that represents the given key; or null if not available.
     */
    protected abstract String Manager_getKeyIcon(BridgeToken token, int code);

    /**
     * Get the currently running application scheme.
     *
     * @param token The token associated with this request.
     *
     * @return The currently running application scheme.
     */
    protected abstract String Manager_getApplicationScheme(BridgeToken token);

    /**
     * Get a list of rating schemes supported by this integration.
     *
     * @param token The token associated with this request.
     *
     * @return A list of ParentalRatingScheme objects representing the supported parental rating
     *    schemes; or an empty list of not available.
     */
    protected abstract List<BridgeTypes.ParentalRatingScheme> ParentalControl_getRatingSchemes(BridgeToken token);

    /**
     * Get the parental rating threshold currently set on the system for the given scheme.
     *
     * @param token The token associated with this request.
     * @param scheme The name of the scheme to get the threshold of.
     *
     * @return A ParentalRating object representing the threshold.
     */
    protected abstract BridgeTypes.ParentalRating ParentalControl_getThreshold(BridgeToken token, String scheme);

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
    protected abstract boolean ParentalControl_isRatingBlocked(BridgeToken token, String scheme, String region, int value);

    /**
     * Get the current capabilities of the terminal.
     *
     * @param token The token associated with this request.
     *
     * @return A Capabilities object.
     */
    protected abstract BridgeTypes.Capabilities Configuration_getCapabilities(BridgeToken token);

    /**
     * Get a list of audio profiles supported by the terminal, as defined by HBBTV 10.2.4.7 for
     * the audio_profile element.
     *
     * @param token The token associated with this request.
     *
     * @return A list of audio profiles supported by the terminal.
     */
    protected abstract List<BridgeTypes.AudioProfile> Configuration_getAudioProfiles(BridgeToken token);

    /**
     * Get a list of video profiles supported by the terminal, as defined by HBBTV 10.2.4.7 for
     * the video_profile element.
     *
     * @param token The token associated with this request.
     *
     * @return A list of video profiles supported by the terminal.
     */
    protected abstract List<BridgeTypes.VideoProfile> Configuration_getVideoProfiles(BridgeToken token);

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
    protected abstract List<BridgeTypes.VideoDisplayFormat> Configuration_getVideoDisplayFormats(BridgeToken token);

    /**
     * Get the current number of additional media streams containing SD video accompanied by audio
     * that can be decoded and presented by an A/V control object or HTML5 media element.
     *
     * @param token The token associated with this request.
     *
     * @return The current number of additional media streams. If the value is non-zero, then a call
     * to play an A/V control object, HTML5 media element or video/broadcast object shall not fail
     * due to lack of resources for SD media.
     */
    protected abstract int Configuration_getExtraSDVideoDecodes(BridgeToken token);

    /**
     * Get the current number of additional media streams containing HD video accompanied by audio
     * that can be decoded and presented by an A/V control object or HTML5 media element.
     *
     * @param token The token associated with this request.
     *
     * @return The current number of additional media streams. If the value is non-zero, then a call
     * to play an A/V control object, HTML5 media element or video/broadcast object shall not fail
     * due to lack of resources for HD media.
     */
    protected abstract int Configuration_getExtraHDVideoDecodes(BridgeToken token);

    /**
     * Get the current number of additional media streams containing UHD video accompanied by audio
     * that can be decoded and presented by an A/V control object or HTML5 media element.
     *
     * @param token The token associated with this request.
     *
     * @return The current number of additional media streams. If the value is non-zero, then a call
     * to play an A/V control object, HTML5 media element or video/broadcast object shall not fail
     * due to lack of resources for UHD media.
     */
    protected abstract int Configuration_getExtraUHDVideoDecodes(BridgeToken token);

    /**
     * Get the maximum (static) broadband media decoding capabilities for the indicated decoder.
     * This method is defined in HBBTV-TA-1 v1.1.1 A.2.2.
     * <p>
     * Media decoders are numbered from 1 increasing in steps of 1 up to the total number of
     * media decoders in the terminal. Decoders are ordered in decreasing capabilities (UHD before
     * HD, HD before SD). If decoderIndex is greater than the number of media decoders then null
     * shall be returned.
     * <p>
     * Each String returned shall be one of the video_profile elements returned by xmlCapabilities.
     *
     * @param token The token associated with this request.
     * @param decoderIndex The decoder index (1-based, starting from 1).
     *
     * @return A list of video profile names (strings) for the decoder, or null if decoderIndex
     *         is greater than the number of decoders. Each string shall be one of the video_profile
     *         elements returned by xmlCapabilities.
     */
    protected abstract List<String> Configuration_getBroadbandCapabilities(BridgeToken token, int decoderIndex);

    /**
     * Get certain immutable information about the system.
     *
     * @param token The token associated with this request.
     *
     * @return A SystemInformation object.
     */
    protected abstract BridgeTypes.SystemInformation Configuration_getLocalSystem(BridgeToken token);

    /**
     * Get preferred languages to be used for audio playback on this system.
     *
     * @param token The token associated with this request.
     *
     * @return Comma separated string of languages (ISO 639-2 codes), in order of preference.
     */
    protected abstract String Configuration_getPreferredAudioLanguage(BridgeToken token);

    /**
     * Get preferred languages to be used for audio playback on this system.
     *
     * @param token The token associated with this request.
     *
     * @return Comma separated string of languages (IETF BCP47 codes), in order of preference.
     */
    protected abstract String Configuration_getPreferredAudioLanguage47(BridgeToken token);

    /**
     * Get preferred languages to be used for subtitles on this system.
     *
     * @param token The token associated with this request.
     *
     * @return Comma separated string of languages (ISO 639-2 codes), in order of preference.
     */
    protected abstract String Configuration_getPreferredSubtitleLanguage(BridgeToken token);

    /**
     * Get preferred languages to be used for subtitles on this system.
     *
     * @param token The token associated with this request.
     *
     * @return Comma separated string of languages (IETF BCP47 codes), in order of preference.
     */
    protected abstract String Configuration_getPreferredSubtitleLanguage47(BridgeToken token);

    /**
     * Get preferred languages to be used for the user-interface on this system.
     *
     * @param token The token associated with this request.
     *
     * @return Comma separated string of languages (ISO 639-2 codes), in order of preference.
     */
    protected abstract String Configuration_getPreferredUILanguage(BridgeToken token);

    /**
     * Get a string containing the three character country code identifying the country this system
     * is deployed.
     *
     * @param token The token associated with this request.
     *
     * @return Country code the receiver is deployed (ISO 3166-1 alpha-3 code).
     */
    protected abstract String Configuration_getCountryId(BridgeToken token);

    /**
     * Get whether subtitles are enabled on this system.
     *
     * @param token The token associated with this request.
     *
     * @return True if subtitles are enabled; or false otherwise.
     */
    protected abstract boolean Configuration_getSubtitlesEnabled(BridgeToken token);

    /**
     * Get whether audio description is enabled on this system.
     *
     * @param token The token associated with this request.
     *
     * @return True if audio description is enabled; or false otherwise.
     */
    protected abstract boolean Configuration_getAudioDescriptionEnabled(BridgeToken token);

    /**
     * Get whether clean audio is enabled on this system.
     *
     * @param token The token associated with this request.
     *
     * @return True if audio description is enabled; or false otherwise.
     */
    protected abstract boolean Configuration_getCleanAudioEnabled(BridgeToken token);

    /**
     * Get the DVB network IDs of the channels in the broadcast channel list.
     *
     * @param token The token associated with this request.
     *
     * @return A list of DVB network IDs; or an empty list of not available.
     */
    protected abstract List<Integer> Configuration_getDttNetworkIds(BridgeToken token);

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
    protected abstract String Configuration_getDeviceId(BridgeToken token);

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
    protected abstract void Configuration_requestAccessToDistinctiveIdentifier(BridgeToken token);

    /**
     * @param token The token associated with this request.
     *
     * @return
     */
    protected abstract int MediaSynchroniser_instantiate(BridgeToken token);

    /**
     * @param token The token associated with this request.
     * @param id
     * @param isMasterBroadcast
     *
     * @return
     */
    protected abstract boolean MediaSynchroniser_initialise(BridgeToken token, int id,
                                                            boolean isMasterBroadcast);

    /**
     * @param token The token associated with this request.
     * @param id
     */
    protected abstract void MediaSynchroniser_destroy(BridgeToken token, int id);

    /**
     * @param token The token associated with this request.
     * @param id
     */
    protected abstract boolean MediaSynchroniser_enableInterDeviceSync(BridgeToken token, int id);

    /**
     * @param token The token associated with this request.
     * @param id
     */
    protected abstract void MediaSynchroniser_disableInterDeviceSync(BridgeToken token, int id);

    /**
     * @param token The token associated with this request.
     * @param id
     *
     * @return
     */
    protected abstract int MediaSynchroniser_nrOfSlaves(BridgeToken token, int id);

    /**
     * @param token The token associated with this request.
     * @param id
     *
     * @return
     */
    protected abstract boolean MediaSynchroniser_interDeviceSyncEnabled(BridgeToken token, int id);

    /**
     * @param token The token associated with this request.
     * @param id
     *
     * @return
     */
    protected abstract String MediaSynchroniser_getContentIdOverride(BridgeToken token, int id);

    /**
     * @param token The token associated with this request.
     * @param id
     * @param timelineSelector
     *
     * @return
     */
    protected abstract long MediaSynchroniser_getBroadcastCurrentTime(BridgeToken token,
                                                                      String timelineSelector);

    /**
     * @param token The token associated with this request.
     * @param id
     * @param timelineSelector
     * @param isMaster
     *
     * @return
     */
    protected abstract boolean MediaSynchroniser_startTimelineMonitoring(BridgeToken token, int id,
                                                                         String timelineSelector, boolean isMaster);

    /**
     * @param token The token associated with this request.
     * @param id
     * @param timelineSelector
     * @param forceStop
     */
    protected abstract void MediaSynchroniser_stopTimelineMonitoring(BridgeToken token, int id,
                                                                     String timelineSelector, boolean forceStop);

    /**
     * @param token The token associated with this request.
     * @param id
     * @param contentIdOverride
     */
    protected abstract void MediaSynchroniser_setContentIdOverride(BridgeToken token, int id,
                                                                   String contentIdOverride);

    /**
     * @param token The token associated with this request.
     * @param id
     * @param timelineSelector
     * @param contentTime
     * @param speed
     */
    protected abstract void MediaSynchroniser_setContentTimeAndSpeed(BridgeToken token, int id,
                                                                     String timelineSelector, long contentTime, double speed);

    /**
     * @param token The token associated with this request.
     * @param id
     * @param properties
     */
    protected abstract void MediaSynchroniser_updateCssCiiProperties(BridgeToken token, int id,
                                                                     String contentId, String presentationStatus, String contentIdStatus, String mrsUrl);

    /**
     * @param token The token associated with this request.
     * @param id
     * @param timelineSelector
     * @param isAvailable
     * @param ticks
     * @param speed
     */
    protected abstract boolean MediaSynchroniser_setTimelineAvailability(BridgeToken token, int id,
                                                                         String timelineSelector, boolean isAvailable, long ticks, double speed);


    /**
     * @param token The token associated with this request.
     *
     * @return
     */
    protected abstract String CSManager_getApp2AppLocalBaseURL(BridgeToken token);

    /**
     * @param token The token associated with this request.
     *
     * @return
     */
    protected abstract String CSManager_getInterDevSyncURL(BridgeToken token);

    /**
     * @param token The token associated with this request.
     *
     * @return
     */
    protected abstract String CSManager_getApp2AppRemoteBaseURL(BridgeToken token);

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
    protected abstract List<BridgeTypes.DRMSystemStatus> Drm_getSupportedDRMSystemIDs(
            BridgeToken token);

    /**
     * Send message to the DRM system.
     *
     * @param params JSONObject {
     *    "msgID": string,       // unique ID to identify the message, to be passed as
     *                           // the 'msgID' argument for onDRMMessageResult
     *    "msgType": string,     // message type as defined by the DRM system
     *    "msg": string,         // message to be provided to the underlying DRM system
     *    "DRMSystemID": string, // ID of the DRM System
     *    "block": boolean       // Whether the function needs to block until the reply is received
     * }
     * @return JSONObject {
     *    "result": string,      // Result message when block is true. Ignored otherwise.
     * }
     * @throws JSONException
     */
    protected abstract String Drm_sendDRMMessage(BridgeToken token, String msgID, String msgType,
                                                 String msg, String DRMSystemID, boolean block);

    /**
     * Checks the availability of a valid license for playing a protected content item.
     *
     * @param params JSONObject {
     *    "DRMPrivateData": string, // DRM proprietary private data
     *    "DRMSystemID": string,    // ID of the DRM System
     * }
     * @return JSONObject {
     *    "result": boolean,        // true if the content can be played, false otherwise
     * }
     * @throws JSONException
     */
    protected abstract boolean Drm_canPlayContent(BridgeToken token, String DRMPrivateData,
                                                  String DRMSystemID);

    /**
     * Checks the availability of a valid license for recording a protected content item.
     *
     * @param params JSONObject {
     *    "DRMPrivateData": string, // DRM proprietary private data
     *    "DRMSystemID": string,    // ID of the DRM System
     * }
     * @return JSONObject {
     *    "result": boolean,        // true if the content can be recorded, false otherwise
     * }
     * @throws JSONException
     */
    protected abstract boolean Drm_canRecordContent(BridgeToken token, String DRMPrivateData,
                                                    String DRMSystemID);

    /**
     * Set the DRM system, that the terminal shall use for playing protected broadband content.
     *
     * @param params JSONObject {
     *    "DRMSystemID": string, // ID of the DRM System
     * }
     * @return JSONObject {
     *    "result": boolean,     // true if the call was successful, false otherwise
     * }
     * @throws JSONException
     */
    protected abstract boolean Drm_setActiveDRM(BridgeToken token, String DRMSystemID);

    /**
     * Publish a test report (debug build only).
     *
     * @param token The token associated with this request.
     * @param testSuite A unique test suite name.
     * @param xml The XML test report.
     */
    protected abstract void OrbDebug_publishTestReport(BridgeToken token, String testSuite, String xml);

    /**
     * Retrieve the public ip address.
     *
     * @param token The token associated with this request.
     *
     * @return JSONObject {
     *    "result": String
     * }
     */
    protected abstract String Network_resolveHostAddress(BridgeToken token, String hostname);

    /**
     * Request the soft keyboard.
     *
     * @param token The token associated with this request.
     */
    protected abstract void SoftKeyboard_show(BridgeToken token);

    public JSONObject request(String method, BridgeToken token, JSONObject params) throws JSONException {
        JSONObject response = new JSONObject();

        switch (method) {
            case "Broadcast.setVideoRectangle": {
                Broadcast_setVideoRectangle(
                        token,
                        params.getInt("x"),
                        params.getInt("y"),
                        params.getInt("width"),
                        params.getInt("height")
                );
                break;
            }

            case "Broadcast.getCurrentChannel": {
                if (!isRequestAllowed(token, SessionCallback.FOR_BROADCAST_APP_ONLY)) {
                    response.put("error", "SecurityError");
                    break;
                }
                BridgeTypes.Channel result = Broadcast_getCurrentChannel(
                        token
                );
                response.put("result", BridgeTypes.toJSONObject(result));
                break;
            }

            case "Broadcast.getCurrentChannelForEvent": {
                if (!isRequestAllowed(token, SessionCallback.FOR_BROADCAST_OR_TRANSITIONING_APP_ONLY)) {
                    response.put("error", "SecurityError");
                    break;
                }
                BridgeTypes.Channel result = Broadcast_getCurrentChannelForEvent(
                        token
                );
                response.put("result", BridgeTypes.toJSONObject(result));
                break;
            }

            case "Broadcast.getChannelList": {
                if (!isRequestAllowed(token, SessionCallback.FOR_BROADCAST_APP_ONLY)) {
                    response.put("error", "SecurityError");
                    break;
                }
                List<BridgeTypes.Channel> result = Broadcast_getChannelList(
                        token
                );
                response.put("result", BridgeTypes.toJSONArray(result));
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
                List<BridgeTypes.Programme> result = Broadcast_getProgrammes(
                        token,
                        params.getString("ccid")
                );
                response.put("result", BridgeTypes.toJSONArray(result));
                break;
            }

            case "Broadcast.getComponents": {
                if (!isRequestAllowed(token, SessionCallback.FOR_BROADCAST_APP_ONLY)) {
                    response.put("error", "SecurityError");
                    break;
                }
                List<BridgeTypes.Component> result = Broadcast_getComponents(
                        token,
                        params.getString("ccid"),
                        params.getInt("type")
                );
                response.put("result", BridgeTypes.toJSONArray(result));
                break;
            }

            case "Broadcast.getPrivateAudioComponent": {
                if (!isRequestAllowed(token, SessionCallback.FOR_BROADCAST_APP_ONLY)) {
                    response.put("error", "SecurityError");
                    break;
                }
                BridgeTypes.Component result = Broadcast_getPrivateAudioComponent(
                        token,
                        params.getString("componentTag")
                );
                response.put("result", BridgeTypes.toJSONObject(result));
                break;
            }

            case "Broadcast.getPrivateVideoComponent": {
                if (!isRequestAllowed(token, SessionCallback.FOR_BROADCAST_APP_ONLY)) {
                    response.put("error", "SecurityError");
                    break;
                }
                BridgeTypes.Component result = Broadcast_getPrivateVideoComponent(
                        token,
                        params.getString("componentTag")
                );
                response.put("result", BridgeTypes.toJSONObject(result));
                break;
            }

            case "Broadcast.overrideComponentSelection": {
                if (!isRequestAllowed(token, SessionCallback.FOR_BROADCAST_APP_ONLY)) {
                    response.put("error", "SecurityError");
                    break;
                }
                Broadcast_overrideComponentSelection(
                        token,
                        params.getInt("type"),
                        params.getString("id")
                );
                break;
            }
            case "Broadcast.restoreComponentSelection": {
                if (!isRequestAllowed(token, SessionCallback.FOR_BROADCAST_APP_ONLY)) {
                    response.put("error", "SecurityError");
                    break;
                }
                Broadcast_restoreComponentSelection(
                        token,
                        params.getInt("type")
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

            case "Broadcast.getVolume": {
                int result = Broadcast_getVolume(token);
                response.put("result", result);
                break;
            }

            case "Broadcast.setVolume": {
                boolean result = Broadcast_setVolume(token, params.getInt("volume"));
                response.put("result", result);
                break;
            }

            case "Programme.getParentalRating": {
                BridgeTypes.ParentalRating result = Programme_getParentalRating(
                        token
                );
                response.put("result", BridgeTypes.toJSONObject(result));
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

            case "Manager.getOtherKeyValues": {
                int[] resultOtherKeys = Manager_getOtherKeyValues(
                        token
                );
                JSONArray result = new JSONArray();
                for (int i : resultOtherKeys) {
                    result.put(i);
                }
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

            case "Manager.getKeyMaximumOtherKeys": {
                int result = Manager_getKeyMaximumOtherKeys(
                        token
                );
                response.put("result", result);
                break;
            }

            case "Manager.setKeyValue": {
                List<String> otherKeysList = new ArrayList<>();
                if (!params.isNull("otherKeys")) {
                    JSONArray otherKeys = params.getJSONArray("otherKeys");
                    for (int i = 0; i < otherKeys.length(); i++) {
                        otherKeysList.add(otherKeys.getString(i));
                    }
                }

                int result = Manager_setKeyValue(
                        token,
                        params.getInt("value"),
                        otherKeysList
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

            case "Manager.getApplicationScheme": {
                String result = Manager_getApplicationScheme(token);
                response.put("result", result);
                break;
            }

            case "ParentalControl.getRatingSchemes": {
                List<BridgeTypes.ParentalRatingScheme> result = ParentalControl_getRatingSchemes(
                        token
                );
                response.put("result", BridgeTypes.toJSONArray(result));
                break;
            }

            case "ParentalControl.getThreshold": {
                BridgeTypes.ParentalRating result = ParentalControl_getThreshold(
                        token,
                        params.getString("scheme")
                );
                response.put("result", BridgeTypes.toJSONObject(result));
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

            case "Configuration.getCapabilities": {
                BridgeTypes.Capabilities result = Configuration_getCapabilities(
                        token
                );
                response.put("result", BridgeTypes.toJSONObject(result));
                break;
            }

            case "Configuration.getAudioProfiles": {
                List<BridgeTypes.AudioProfile> result = Configuration_getAudioProfiles(
                        token
                );
                response.put("result", BridgeTypes.toJSONArray(result));
                break;
            }

            case "Configuration.getVideoProfiles": {
                List<BridgeTypes.VideoProfile> result = Configuration_getVideoProfiles(
                        token
                );
                response.put("result", BridgeTypes.toJSONArray(result));
                break;
            }

            case "Configuration.getVideoDisplayFormats": {
                List<BridgeTypes.VideoDisplayFormat> result = Configuration_getVideoDisplayFormats(
                        token
                );
                response.put("result", BridgeTypes.toJSONArray(result));
                break;
            }

            case "Configuration.getExtraSDVideoDecodes": {
                int result = Configuration_getExtraSDVideoDecodes(
                        token
                );
                response.put("result", result);
                break;
            }

            case "Configuration.getExtraHDVideoDecodes": {
                int result = Configuration_getExtraHDVideoDecodes(
                        token
                );
                response.put("result", result);
                break;
            }

            case "Configuration.getExtraUHDVideoDecodes": {
                int result = Configuration_getExtraUHDVideoDecodes(
                        token
                );
                response.put("result", result);
                break;
            }

            case "Configuration.getBroadbandCapabilities": {
                List<String> result = Configuration_getBroadbandCapabilities(
                        token,
                        params.getInt("decoderIndex")
                );
                if (result == null) {
                    response.put("result", JSONObject.NULL);
                } else {
                    response.put("result", new JSONArray(result));
                }
                break;
            }

            case "Configuration.getLocalSystem": {
                BridgeTypes.SystemInformation result = Configuration_getLocalSystem(
                        token
                );
                response.put("result", BridgeTypes.toJSONObject(result));
                break;
            }

            case "Configuration.getPreferredAudioLanguage": {
                String result = Configuration_getPreferredAudioLanguage(
                        token
                );
                response.put("result", result);
                break;
            }

            case "Configuration.getPreferredAudioLanguage47": {
                String result = Configuration_getPreferredAudioLanguage47(
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

            case "Configuration.getPreferredSubtitleLanguage47": {
                String result = Configuration_getPreferredSubtitleLanguage47(
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

            case "Configuration.getCleanAudioEnabled": {
                boolean result = Configuration_getCleanAudioEnabled(
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

            case "Drm.getSupportedDRMSystemIDs": {
            /*if (!isRequestAllowed(token, TvBrowserTypes.SECURITY_LEVEL_NONE)) {
               return makeErrorResponse("SecurityError");
            }*/
                List<BridgeTypes.DRMSystemStatus> result = Drm_getSupportedDRMSystemIDs(token);
                response.put("result", BridgeTypes.toJSONArray(result));
                break;
            }

            case "Drm.sendDRMMessage": {
                // TODO Check security level (for this request and all other DRM requests)
            /*if (!isRequestAllowed(token, TvBrowserTypes.SECURITY_LEVEL_NONE)) {
               return makeErrorResponse("SecurityError");
            }*/
                String result = Drm_sendDRMMessage(
                        token,
                        params.getString("msgID"),
                        params.getString("msgType"),
                        params.getString("msg"),
                        params.getString("DRMSystemID"),
                        params.getBoolean("block")
                );
                response.put("result", result);
                break;
            }

            case "Drm.canPlayContent": {
            /*if (!isRequestAllowed(token, TvBrowserTypes.SECURITY_LEVEL_NONE)) {
               return makeErrorResponse("SecurityError");
            }*/
                boolean result = Drm_canPlayContent(
                        token,
                        params.getString("DRMPrivateData"),
                        params.getString("DRMSystemID")
                );
                response.put("result", result);
                break;
            }

            case "Drm.canRecordContent": {
            /*if (!isRequestAllowed(token, TvBrowserTypes.SECURITY_LEVEL_NONE)) {
               return makeErrorResponse("SecurityError");
            }*/
                boolean result = Drm_canRecordContent(
                        token,
                        params.getString("DRMPrivateData"),
                        params.getString("DRMSystemID")
                );
                response.put("result", result);
                break;
            }

            case "Drm.setActiveDRM": {
            /*if (!isRequestAllowed(token, TvBrowserTypes.SECURITY_LEVEL_NONE)) {
               return makeErrorResponse("SecurityError");
            }*/
                boolean result = Drm_setActiveDRM(
                        token,
                        params.getString("DRMSystemID")
                );
                response.put("result", result);
                break;
            }

            case "OrbDebug.publishTestReport": {
                OrbDebug_publishTestReport(
                        token,
                        params.getString("testSuite"),
                        params.getString("xml")
                );
                break;
            }

            case "Network.resolveHostAddress": {
                String result = Network_resolveHostAddress(
                        token,
                        params.getString("hostname")
                );
                response.put("result", result);
                break;
            }

            case "SoftKeyboard.show": {
                SoftKeyboard_show(token);
                break;
            }

            default: {
                response.put("error", "Unknown method");
                break;
            }
        }

        return response;
    }

    private boolean isRequestAllowed(BridgeToken token, int methodType) {
        return mSessionCallback.isRequestAllowed(token.getAppId(), token.getUri(), methodType);
    }
}
