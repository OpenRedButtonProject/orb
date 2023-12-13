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

import org.orbtv.orbpolyfill.BridgeTypes;

import java.util.List;

public interface IOrbSessionCallback {
    enum ApplicationStatus {
        /* The application has not been started yet. */
        NOT_STARTED,

        /* The application is running and visible to the user. */
        VISIBLE,

        /* The application is running but not currently visible to the user. */
        INVISIBLE,
    }
    
    /**
     * The level of support a TV OS has for a particular Accessibility Feature.
     *
     * TODO Change names to THIS_STYLE.
     */
    enum SupportType {
        notSupported,
        tvosSettingOnly,
        tvosOnly,
        tvosAndHbbTV,
        supportedNoSetting
    }

    /**
     * The result of an Application to request that a TV OS suppresses its support for an
     * Accessibility Feature.
     *
     * TODO Change names to THIS_STYLE.
     */
    enum SuppressType {
        none,
        suppressing,
        notSuppressing,
        featureNotSupported
    }

    /**
     * This method is called once the session is ready to be called by the client and present HbbTV
     * applications.
     *
     * @param session The session (for convenience)
     */
    void onSessionReady(IOrbSession session);

    /**
     * Get the current capabilities of the terminal.
     *
     * @return A Capabilities object.
     */
    BridgeTypes.Capabilities getCapabilities();

    /**
     * Get a list of audio profiles supported by the terminal, as defined by HBBTV 10.2.4.7 for
     * the audio_profile element.
     *
     * @return A list of audio profiles supported by the terminal.
     */
    List<BridgeTypes.AudioProfile> getAudioProfiles();

    /**
     * Get a list of video profiles supported by the terminal, as defined by HBBTV 10.2.4.7 for
     * the video_profile element.
     *
     * @return A list of video profiles supported by the terminal.
     */
    List<BridgeTypes.VideoProfile> getVideoProfiles();

    /**
     * If the terminal supports UHD, get a list that describes the highest quality video format the
     * terminal supports, as defined by HBBTV 10.2.4.7 for the video_display_format element;
     * otherwise get an empty list.
     * <p>
     * Note: If the terminal changes its display format based on the content being played, multiple
     * elements may be included in the list when multiple frame rate families are usable or the
     * highest resolution does not support each highest quality parameter.
     *
     * @return A list that describes the highest quality video format.
     */
    List<BridgeTypes.VideoDisplayFormat> getVideoDisplayFormats();

    /**
     * Get the current number of additional media streams containing SD video accompanied by audio
     * that can be decoded and presented by an A/V control object or HTML5 media element.
     *
     * @return The current number of additional media streams. If the value is non-zero, then a call
     * to play an A/V control object, HTML5 media element or video/broadcast object shall not fail
     * due to lack of resources for SD media.
     */
    int getExtraSDVideoDecodes();

    /**
     * Get the current number of additional media streams containing HD video accompanied by audio
     * that can be decoded and presented by an A/V control object or HTML5 media element.
     *
     * @return The current number of additional media streams. If the value is non-zero, then a call
     * to play an A/V control object, HTML5 media element or video/broadcast object shall not fail
     * due to lack of resources for HD media.
     */
    int getExtraHDVideoDecodes();

    /**
     * Get the current number of additional media streams containing UHD video accompanied by audio
     * that can be decoded and presented by an A/V control object or HTML5 media element.
     *
     * @return The current number of additional media streams. If the value is non-zero, then a call
     * to play an A/V control object, HTML5 media element or video/broadcast object shall not fail
     * due to lack of resources for UHD media.
     */
    int getExtraUHDVideoDecodes();

    /**
     * Get immutable system information.
     *
     * @return Valid SystemInformation on success, otherwise invalid SystemInformation
     */
    BridgeTypes.SystemInformation getSystemInformation();

    /**
     * Gets a string containing languages to be used for audio playback, in order of preference.
     *
     * @return Comma separated string of languages (ISO 639-2 codes)
     */
    String getPreferredAudioLanguage();

    /**
     * Gets a string containing languages to be used for subtitles, in order of preference.
     *
     * @return Comma separated string of languages (ISO 639-2 codes)
     */
    String getPreferredSubtitleLanguage();

    /**
     * Gets a string containing languages to be used for the UI, in order of preference.
     *
     * @return Comma separated string of languages (ISO 639-2 codes)
     */
    String getPreferredUILanguage();

    /**
     * Gets a string containing the three character country code identifying the country in which the
     * receiver is deployed.
     *
     * @return Country code (ISO 3166 alpha-3) string
     */
    String getCountryId();

    /**
     * Gets whether subtitles are enabled in the TV context. So HbbTV knows to start subtitle
     * components on channel change, for example.
     *
     * @return true if enabled, false otherwise
     */
    boolean getSubtitlesEnabled();

    /**
     * Gets whether audio description is enabled in the TV context.
     *
     * @return true if enabled, false otherwise
     */
    boolean getAudioDescriptionEnabled();

    /**
     * Get DTT network IDs.
     *
     * @return Array of DTT network IDs.
     */
    int[] getDttNetworkIds();

    /**
     * Retrieves an array containing the supported Broadcast Delivery Systems (DVB_S, DVB_C, DVB_T,
     * DVB_C2, DVB_T2 or DVB_S2) as defined in Section 9.2, Table 15, under "UI Profile Name
     * Fragment".
     *
     * @return Array of delivery systems
     */
    String[] getSupportedDeliverySystems();

    /**
     * Set whether presentation of any broadcast components must be suspended.
     *
     * @param presentationSuspended true if must be suspended, false otherwise
     */
    void setPresentationSuspended(boolean presentationSuspended);

    /**
     * Override the default component selection of the terminal for the specified type.
     * <p>
     * If id is empty, no component shall be selected for presentation (presentation is explicitly
     * disabled). Otherwise, the specified component shall be selected for presentation.
     * <p>
     * If playback has already started, the presented component shall be updated.
     * <p>
     * Default component selection shall be restored (revert back to the control of the terminal)
     * when: (1) the application terminates, (2) the channel is changed, (3) presentation has not
     * been explicitly disabled and the user selects another track in the terminal UI, or (4) the
     * restoreComponentSelection method is called.
     *
     * @param type Type of component selection to override (COMPONENT_TYPE_* code).
     * @param id   A platform-defined component id or an empty string to disable presentation.
     */
    void overrideComponentSelection(int type, String id);

    /**
     * Restore the default component selection of the terminal for the specified type.
     * <p>
     * If playback has already started, the presented component shall be updated.
     *
     * @param type Type of component selection override to clear (COMPONENT_TYPE_* code).
     */
    void restoreComponentSelection(int type);

    /**
     * Sets the presentation window of the DVB video. Values are in HbbTV 1280x720 coordinates.
     *
     * @param x      Rectangle definition
     * @param y      Rectangle definition
     * @param width  Rectangle definition
     * @param height Rectangle definition
     */
    void setDvbVideoRectangle(final int x, final int y, final int width, final int height);

    /**
     * Get the list of channels available.
     *
     * @return List of channels available
     */
    List<BridgeTypes.Channel> getChannelList();

    /**
     * Returns the CCID of the current channel
     *
     * @return A CCID on success, an empty string otherwise
     */
    String getCurrentCcid();

    /**
     * Find the channel with the given LCN and return its CCID.
     *
     * @param lcn LCN to find
     * @return A CCID on success, an empty string otherwise
     */
    //String findCcidWithLcn(String lcn);

    /**
     * Get the channel with the given CCID.
     *
     * @param ccid CCID for the required channel
     * @return Channel on success
     */
    BridgeTypes.Channel getChannel(String ccid);

    /**
     * Tune to specified channel. The implementation relies on the 'idType' parameter to
     * determine the valid fields that describe the channel. Possible idTypes are:
     * ID_IPTV_SDS/ID_IPTV_URI - where 'ipBroadcastID' and 'sourceId' fields are valid
     * other ID_.. values - where 'onid', 'tsid' and 'sid' fields are valid
     * ID_DVB_SI_DIRECT - is supposed to be handled by setChannelByDsd()
     *
     * @param idType                     The type of channel
     * @param onid                       The original network ID for the required channel.
     * @param tsid                       The transport stream ID for the required channel.
     * @param sid                        The service ID for the required channel.
     * @param sourceID                   The ATSC source_ID of the channel.
     * @param ipBroadcastID              The DVB textual service identifier of the IP broadcast service.
     * @param trickplay                  Ignore unless PVR functionality is supported (does not affect return)
     * @param contentAccessDescriptorURL May be required by DRM-protected IPTV broadcasts
     * @param quiet                      Channel change operation
     *                                   0 - normal tune
     *                                   1 - normal tune and no UI displayed
     *                                   2 - quiet tune (user does not know)
     * @return negative value (e.g. BridgeTypes.CHANNEL_STATUS_CONNECTING) on success, or
     * zero/positive value (see BridgeTypes.CHANNEL_STATUS_.. error values) on failure
     */
    int setChannelByTriplet(int idType, int onid, int tsid, int sid, int sourceID,
                            String ipBroadcastID, boolean trickplay,
                            String contentAccessDescriptorURL, int quiet);

    /**
     * Tune to specified channel using DSD.
     *
     * @param dsd                        DSD for the required channel.
     * @param sid                        SID for the required channel.
     * @param trickplay                  Ignore unless PVR functionality is supported (does not affect return)
     * @param contentAccessDescriptorURL May be required by DRM-protected IPTV broadcasts
     * @param quiet                      Channel change operation
     *                                   0 - normal tune
     *                                   1 - normal tune and no UI displayed
     *                                   2 - quiet tune (user does not know)
     * @return negative value (e.g. BridgeTypes.CHANNEL_STATUS_CONNECTING) on success, or
     * zero/positive value (see BridgeTypes.CHANNEL_STATUS_.. error values) on failure
     */
    int setChannelByDsd(String dsd, int sid, boolean trickplay,
                        String contentAccessDescriptorURL, int quiet);

    /**
     * @param ccid The CCID of the channel to set.
     * @param ccid The CCID of the channel to set.
     * @param trickplay True if the application has optionally hinted trickplay resources are
     *    required; or false otherwise. Does not affect the success of this operation.
     * @param contentAccessDescriptorURL Optionally, additional information for DRM-protected IPTV
     *    broadcasts; or an empty string otherwise.
     * @param quiet Type of channel change: 0 for normal; 1 for normal, no UI; 2 for quiet (HbbTV
     *    A.2.4.3.2).
     * @return A CHANNEL_STATUS_* code (on success, the code has a value less than 0).
     */
    int setChannelToCcid(String ccid, boolean trickplay,
        String contentAccessDescriptorURL, int quiet);

    /**
     * Get the list of programmes available for a channel.
     *
     * @param ccid CCID for the required channel
     * @return List of programmes available for the channel
     */
    List<BridgeTypes.Programme> getProgrammeList(String ccid);

    /**
     * Get information about the present and following programmes on a channel.
     *
     * @param ccid CCID for the required channel
     * @return List of containing the present and following programmes, in that order
     */
    List<BridgeTypes.Programme> getPresentFollowingProgrammes(String ccid);

    /**
     * Get the list of components available for a channel.
     *
     * @param ccid     CCID for the required channel
     * @param typeCode Required component type as defined by BridgeTypes.COMPONENT_TYPE_*
     * @return List of components available for the channel
     */
    List<BridgeTypes.Component> getComponentList(String ccid, int typeCode);

    /**
     * Get a private audio component in the selected channel.
     *
     * @param componentTag The component_tag of the component.
     * @return The private component with the specified component_tag in the PMT of the currently
     * selected broadcast channel; or null if unavailable or the component is not private (i.e.
     * the stream type is audio, video or subtitle).
     * <p>
     * Mandatory properties: id, pid and encrypted. The id property shall be usable with the
     * overrideComponentSelection method to select the component as an audio track. Other
     * Component properties are not required.
     */
    BridgeTypes.Component getPrivateAudioComponent(String componentTag);

    /**
     * Get a private video component in the selected channel.
     *
     * @param componentTag The component_tag of the component.
     * @return The private component with the specified component_tag in the PMT of the currently
     * selected broadcast channel; or null if unavailable or the component is not private (i.e.
     * the stream type is audio, video or subtitle).
     * <p>
     * Mandatory properties: id, pid and encrypted. The id property shall be usable with the
     * overrideComponentSelection method to select the component as an video track. Other
     * Component properties are not required.
     */
    BridgeTypes.Component getPrivateVideoComponent(String componentTag);

    /**
     * Experimental: Do we actually need this data (as in 1.5) or can we use a different interface?
     * <p>
     * Retrieves raw SI descriptor data with the defined descriptor tag id, and optionally the
     * extended descriptor tag id, for an event on a service.
     *
     * @param ccid                 CCID for the required channel
     * @param eventId              Event ID for the required programme
     * @param tagId                Descriptor tag ID of data to be returned
     * @param extendedTagId        Optional extended descriptor tag ID of data to be returned, or -1
     * @param privateDataSpecifier Optional private data specifier of data to be returned, or 0
     * @return The buffer containing the data. If there are multiple descriptors with the same
     * tag id then they will all be returned.
     */
    List<String> getSiDescriptorData(String ccid, String eventId, int tagId, int extendedTagId,
                                     int privateDataSpecifier);

    /**
     * Retrieves the locked status of the specified channel. The correct implementation of this
     * function is not mandatory for HbbTV 1.2.1. It is used to implement the channel's locked
     * property as defined in OIPF vol. 5, section 7.13.11.2.
     *
     * @param ccid CCID of the required channel
     * @return true if parental control prevents the channel being viewed, e.g. when a PIN needs to
     * be entered by the user, false otherwise
     */
    boolean getChannelLocked(String ccid);

    /**
     * Returns the current age set for parental control. 0 will be returned if parental control is
     * disabled or no age is set.
     *
     * @return age in the range 4-18, or 0
     */
    int getParentalControlAge();

    /**
     * Returns the region set for parental control.
     *
     * @return country using the 3-character code as specified in ISO 3166
     */
    String getParentalControlRegion();

    /**
     * Called when the application at origin requests access to the distinctive identifier. The
     * client application should display a dialog for the user to allow or deny this and:
     * <p>
     * 1. TvBrowser.onAccessToDistinctiveIdentifierDecided should be called with the user choice.
     * 2. TvBrowserSessionCallback.getDistinctiveIdentifier should honour the user choice.
     * <p>
     * The client application should allow the user to reset access from a settings menu. This shall
     * result in a new distinctive identifier being generated for an origin next time access is
     * allowed.
     * <p>
     * The helper method TvBrowser.generateDistinctiveIdentifier may be used.
     * <p>
     * Integrators should check 12.1.5 for requirements about distinctive identifiers.
     *
     * @param origin The origin of the application
     * @return true if access already granted, false otherwise
     */
    boolean requestAccessToDistinctiveIdentifier(final String origin);

    /**
     * The distinctive identifier for origin or a distinctive identifier status string (for statuses
     * see BridgeTypes.DISTINCTIVE_IDENTIFIER_STATUS_*).
     * <p>
     * Integrators should check 12.1.5 for requirements about distinctive identifiers.
     *
     * @param origin The origin of the requesting application
     * @return The distinctive identifier for origin or a distinctive identifier status string.
     */
    String getDistinctiveIdentifier(String origin);

    /**
     * Enables the application developer to query information about the current memory available
     * to the application. This is used to help during application development to find application
     * memory leaks and possibly allow an application to make decisions related to its caching
     * strategy (e.g. for images).
     *
     * @return The available memory to the application (in MBs) or -1 if the information is not available.
     */
    long getFreeMemory();

    /**
     * TODO(library)
     *
     * @param q
     * @param offset
     * @param count
     * @param channelConstraints
     * @return
     */
    boolean startSearch(final BridgeTypes.Query q, int offset, int count, List<String> channelConstraints);

    /**
     * TODO(library)
     *
     * @param queryId
     * @return
     */
    boolean abortSearch(int queryId);

    /**
     * Convert the Android key code to a TV Browser (BridgeTypes.VK_*) key code.
     *
     * @param androidKeyCode The Android key code (KeyEvent.KEYCODE_*)
     * @return The TV Browser (BridgeTypes.VK_*) key code.
     */
    int getTvBrowserKeyCode(int androidKeyCode);

    /**
     * Called when the active key set and optional other keys are changed.
     *
     * @param keySet Key set (a bitwise mask of constants, as defined by HbbTV).
     * @param otherKeys Optional other keys.
     */
    void onKeySetChanged(int keySet, int[] otherKeys);

    /**
     * Notify that the application status is changed.
     *
     * @param status The application status.
     */
    void onApplicationStatusChanged(ApplicationStatus status);

    /**
     * Start TEMI timeline monitoring.
     *
     * @param componentTag The component tag of the temi timeline to monitor.
     * @param timelineId   The timeline id of the temi timeline to monitor.
     * @return The associated filter id upon success, -1 otherwise
     */
    int startTEMITimelineMonitoring(int componentTag, int timelineId);

    /**
     * Stop TEMI timeline monitoring.
     *
     * @param filterId The filter id of the temi timeline to stop monitoring.
     * @return true on success, false otherwise
     */
    boolean stopTEMITimelineMonitoring(int filterId);

    /**
     * Finalises TEMI timeline monitoring
     *
     * @return true on success, false otherwise
     */
    boolean finaliseTEMITimelineMonitoring();

    /**
     * Get current TEMI time.
     *
     * @return current TEMI time, -1 if not available
     */
    long getCurrentTemiTime(int filterId);

    /**
     * Get current PTS time.
     *
     * @return current PTS time, -1 if not available
     */
    long getCurrentPtsTime();

    /**
     * Get the IP address that should be used for network services.
     *
     * @return An IP address.
     */
    String getHostAddress();

    /**
     * Get the list of supported DRM System IDs currently available. Once called,
     * the caller can track the availability changes by listening to
     * onDRMSystemStatusChange events. DRM System ID can enter the following states:
     * - 0 READY, fully initialised and ready
     * - 1 UNKNOWN, no longer available
     * - 2 INITIALISING, initialising and not ready to communicate
     * - 3 ERROR, in error state
     *
     * @return List of supported DRM System IDs currently available.
     */
    List<BridgeTypes.DRMSystemStatus> getSupportedDRMSystemIDs();

    /**
     * Checks the availability of a valid license for playing a protected content item.
     *
     * @param DRMPrivateData DRM proprietary private data
     * @param DRMSystemID    ID of the DRM System
     * @return true if there is a valid license available that may allow playing the
     * content
     */
    boolean canPlayContent(String DRMPrivateData, String DRMSystemID);

    /**
     * Checks the availability of a valid license for recording a protected content item.
     *
     * @param DRMPrivateData DRM proprietary private data
     * @param DRMSystemID    ID of the DRM System
     * @return true if there is a valid license available locally that may allow recording
     * the content
     */
    boolean canRecordContent(String DRMPrivateData, String DRMSystemID);

    /**
     * Send message to the DRM system.
     *
     * @param msgId       unique ID to identify the message, to be passed as the 'msgID'
     *                    argument for onDRMMessageResult
     * @param msgType     message type as defined by the DRM system
     * @param msg         message to be provided to the underlying DRM system
     * @param drmSystemID ID of the DRM System
     * @param block       Whether the function needs to block until the reply is received
     */
    String sendDRMMessage(String msgId, String msgType, String msg, String drmSystemID, boolean block);

    /**
     * Set the DRM system, that the terminal shall use for playing protected broadband content.
     *
     * @param DRMSystemID ID of the DRM System
     * @return false if the terminal is unable to set the specified DRM system as requested,
     * true otherwise
     */
    boolean setActiveDRM(String DRMSystemID);

    /**
     * Request file from DSM-CC
     *
     * @param url       DVB Url of requested file
     * @param requestId ID of request (returned to DsmccClient.receiveContent)
     */
    boolean requestDsmccDvbContent(String url, int requestId);

    /**
     * Release resources for DSM-CC file request
     *
     * @param requestId ID of request
     */
    void closeDsmccDvbContent(int requestId);

    /**
     * Subscribe to DSM-CC Stream Event with URL and event name
     *
     * @param url      DVB Url of event object
     * @param name     Name of stream event
     * @param listenId ID of subscriber
     */
    boolean subscribeDsmccStreamEventName(String url, String name, int listenId);

    /**
     * Subscribe to DSM-CC Stream Event with component tag and event ID
     *
     * @param name         Name of stream event
     * @param componentTag Component tag for stream event
     * @param eventId      Event Id of stream event
     * @param listenId     ID of subscriber
     */
    boolean subscribeDsmccStreamEventId(String name, int componentTag, int eventId, int listenId);

    /**
     * Subscribe to DSM-CC Stream Event with component tag and event ID
     *
     * @param listenId ID of subscriber
     */
    void unsubscribeDsmccStreamEvent(int listenId);

    /**
     * Publish a test report (debug build only).
     *
     * @param testSuite A unique test suite name.
     * @param xml       The XML test report.
     */
    void publishTestReport(String testSuite, String xml);

    /**
     * @since 204
     *
     * Request a negotiation for methods
     */
    default void onRequestNegotiateMethods() {
        throw new UnsupportedOperationException("Unsupported 204 API.");
    }

    /**
     * @since 204
     *
     * Request to subscribe/unsubscribe some particular accessibility features
     *
     * @param isSubscribe          The request to subscribe/unsubscribe
     *                             - true: subscribe
     *                             - false: unsubscribe
     *                             User preference change of 8 accessibility features:
     * @param subtitles            Subtitles
     * @param dialogueEnhancement  Dialogue enhancement
     * @param uiMagnifier          Magnification UI
     * @param highContrastUI       High contrast UI
     * @param screenReader         Screen reader
     * @param responseToUserAction Response to user action
     * @param audioDescription     Audio description
     * @param inVisionSigning      In-vision signing
     */
    default void onRequestSubscribe(boolean isSubscribe,
                            boolean subtitles, boolean dialogueEnhancement,
                            boolean uiMagnifier, boolean highContrastUI,
                            boolean screenReader, boolean responseToUserAction,
                            boolean audioDescription, boolean inVisionSigning) {
        throw new UnsupportedOperationException("Unsupported 204 API.");
    }

    /**
     * @since 204
     *
     * Request for a overriding dialogue enhancement
     *
     * @param connection              The request and response should have the same value
     * @param id                      The request and response should have the same value
     * @param dialogueEnhancementGain The requested gain value in dB of the dialogue enhancement
     */
    default void onRequestDialogueEnhancementOverride(int connection, String id,
                                              int dialogueEnhancementGain) {
        throw new UnsupportedOperationException("Unsupported 204 API.");
    }

    /**
     * @since 204
     *
     * Request for a trigger response to user action
     *
     * @param connection The request and response should have the same value
     * @param id         The request and response should have the same value
     * @param magnitude  The magnitude of response to user action
     */
    default void onRequestTriggerResponseToUserAction(int connection, String id, String magnitude) {
        throw new UnsupportedOperationException("Unsupported 204 API.");
    }

    /**
     * @since 204
     *
     * Request for the support information of a feature
     *
     * @param connection The request and response should have the same value
     * @param id         The request and response should have the same value
     * @param featureId  The index of a particular accessibility feature
     */
    default void onRequestFeatureSupportInfo(int connection, String id, int featureId) {
        throw new UnsupportedOperationException("Unsupported 204 API.");
    }

    /**
     * @since 204
     *
     * Request to query settings of a particular feature
     *
     * @param connection The request and response should have the same value
     * @param id         The request and response should have the same value
     * @param featureId  The index of a particular accessibility feature
     */
    default void onRequestFeatureSettingsQuery(int connection, String id, int featureId) {
        throw new UnsupportedOperationException("Unsupported 204 API.");
    }

    /**
     * @since 204
     *
     * Request for suppressing the support of a feature
     *
     * @param connection The request and response should have the same value
     * @param id         The request and response should have the same value
     * @param featureId  The index of a particular accessibility feature
     */
    default void onRequestFeatureSuppress(int connection, String id, int featureId) {
        throw new UnsupportedOperationException("Unsupported 204 API.");
    }

    /**
     * @since 204
     *
     * Receive a response for a request that expresses user intent.
     *
     * @param connection The request and response should have the same value
     * @param id         The request and response should have the same value
     * @param method     The string value that has the same value as the method of the original request
     */
    default void onReceiveIntentConfirm(int connection, String id, String method) {
        throw new UnsupportedOperationException("Unsupported 204 API.");
    }

    /**
     * @since 204
     *
     * Receive a notification of voice-readiness
     *
     * @param isReady The boolean value of the status of voice-readiness
     *                - true: the application is voice-ready
     *                - false: not voice-ready
     */
    default void onNotifyVoiceReady(boolean isReady) {
        throw new UnsupportedOperationException("Unsupported 204 API.");
    }

    /**
     * @since 204
     *
     * Receive a notification that describes the state of media presentation by the application at the time
     *
     * @param state The description of state with respect to media playback
     */
    default void onNotifyStateMedia(String state) {
        throw new UnsupportedOperationException("Unsupported 204 API.");
    }

    /**
     * @since 204
     *
     * Called to send a response message
     *
     * @param info The content of the message
     */
    default void onRespondMessage(String info) {
        throw new UnsupportedOperationException("Unsupported 204 API.");
    }

    /**
     * @since 204
     *
     * Called to send an error message
     *
     * @param code    The error code
     * @param message The error message
     */
    default void onReceiveError(int code, String message) {
        throw new UnsupportedOperationException("Unsupported 204 API.");
    }

    /**
     * @since 204
     *
     * Called to send an error message with some data
     *
     * @param code    The error code
     * @param message The error message
     * @param method  The method same as in the original request
     * @param data    The error data
     */
    default void onReceiveError(int code, String message,
                        String method, String data) {
        throw new UnsupportedOperationException("Unsupported 204 API.");
    }
}
