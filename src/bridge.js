/**
 * @fileOverview Native-agnostic interface to outside the browser context (e.g. to the broadcast stack).
 * @license ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */
hbbtv.bridge = (function() {
    const exported = {};

    const gEventDispatcher = new hbbtv.utils.EventDispatcher();

    /**
     * Initialise the bridge.
     */
    exported.initialise = function() {
        hbbtv.native.setDispatchEventCallback((type, properties) => {
            gEventDispatcher.dispatchEvent(Object.assign(new Event(type), properties));
        });
    };

    /**
     * The `listenerCallback` is called when an event is received.
     *
     * @callback listenerCallback
     * @param {Event} event An Event object that contains the received properties.
     */

    /**
     * Add a "strong" event listener to the bridge that will be called when an event of the given
     * type is received.
     *
     * This method creates a strong reference from the bridge to the listener, which will prevent
     * the listener and its references from being collected.
     *
     * @param {string} type The event type to receive.
     * @param {listenerCallback} callback The callback that is called when the event is received.
     */
    exported.addStrongEventListener = function(type, callback) {
        gEventDispatcher.addEventListener(type, callback);
    };

    /**
     * Remove a previously added "strong" event listener.
     *
     * @param {string} type The previously added event type.
     * @param {listenerCallback} callbackThe The previously added callback.
     */
    exported.removeStrongEventListener = function(type, callback) {
        gEventDispatcher.removeEventListener(type, callback);
    };

    /**
     * Add a "weak" event listener to the bridge that will be called when an event of the given type
     * is received, until the event listener is collected.
     *
     * This method creates a weak reference from the bridge to the listener, so that the listener
     * and its references can be collected when they become otherwise unreachable.
     *
     * Note: Passing in an anonymous function (for example) will probably result in listener being
     * collected immediately! A reference to listener needs to be held by the caller, for example
     * in an object with an associated lifetime.
     *
     * @param {string} type The event type to subcribe to.
     * @param {listenerCallback} callback The callback that is called when the event is received.
     */
    exported.addWeakEventListener = function(type, callback) {
        gEventDispatcher.addWeakEventListener(type, callback);
    };

    /**
     * Remove a previously added "weak" event listener.
     *
     * @param {string} type The previously added event type.
     * @param {listenerCallback} callbackThe The previously added callback.
     */
    exported.removeWeakEventListener = function(type, callback) {
        gEventDispatcher.removeWeakEventListener(type, callback);
    };

    return exported;
})();

hbbtv.bridge.broadcast = (function() {
    const exported = {};

    /**
     * Set the size and position of the broadcast video.
     *
     * Values in pixels are relative to a 1280x720 logical coordinate system and should be scaled to
     * the actual width and height of the browser.
     *
     * @param {number} x The X position in pixels.
     * @param {number} y The Y position in pixels.
     * @param {number} width The width in pixels.
     * @param {number} height The height in pixels.
     * @param {boolean} fullScreen True if the broadcast video should be full screen, layered above
     *    the browser; or false if the broadcast video should be the size and position of the
     *    rectangle, layered below the browser.
     */
    exported.setVideoRectangle = function(x, y, width, height, fullScreen) {
        hbbtv.native.request('Broadcast.setVideoRectangle', {
            x: x,
            y: y,
            width: width,
            height: height,
            fullScreen: fullScreen,
        });
    };

    /**
     * Get the current broadcast channel.
     *
     * Security: FOR_BROADCAST_APP_ONLY.
     *
     * @returns {Channel} A Channel object representing the current broadcast channel; or null if
     *    not available.
     */
    exported.getCurrentChannel = function() {
        return hbbtv.native.request('Broadcast.getCurrentChannel').result;
    };

    /**
     * Get the current broadcast channel.
     *
     * This method can be called while transitioning to broadcast-related, allowing certain events to
     * be implemented.
     *
     * Security: FOR_BROADCAST_OR_TRANSITIONING_APP_ONLY.
     *
     * @returns {Channel} A Channel object representing the current broadcast channel; or null if not
     *    available.
     */
    exported.getCurrentChannelForEvent = function() {
        return hbbtv.native.request('Broadcast.getCurrentChannelForEvent').result;
    };

    /**
     * Get the broadcast channel list.
     *
     * Security: FOR_BROADCAST_APP_ONLY.
     *
     * @returns {Array.<Channel>} A list of Channel objects representing the broadcast channel list;
     *    or an empty list if not available.
     */
    exported.getChannelList = function() {
        return hbbtv.native.request('Broadcast.getChannelList').result;
    };

    /**
     * Select a logically null broadcast channel (e.g. tune off).
     *
     * When a logically null broadcast channel is selected, the Application Manager must transition
     * the running application to broadcast-independent or kill it, depending on the signalling.
     *
     * Security: FOR_RUNNING_APP_ONLY.
     *
     *
     * @returns {number} A CHANNEL_STATUS_* code (on success, the code has a value less than 0).
     */
    exported.setChannelToNull = function(quiet) {
        return hbbtv.native.request('Broadcast.setChannelToNull').result;
    };

    /**
     * Select the broadcast channel (e.g. tune) with the given CCID.
     *
     * Security: FOR_RUNNING_APP_ONLY.
     *
     * @param {string} ccid The CCID of the channel to set.
     * @param {boolean} trickplay True if the application has optionally hinted trickplay resources
     *    are required; or false otherwise. Does not affect the success of this operation.
     * @param {string} contentAccessDescriptorURL Optionally, additional information for
     *    DRM-protected IPTV  broadcasts; or an empty string otherwise.
     * @param {number} quiet Type of channel change: 0 for normal; 1 for normal, no UI; 2 for quiet
     *    (HbbTV A.2.4.3.2).
     *
     * @returns {number} A CHANNEL_STATUS_* code (on success, the code has a value less than 0).
     */
    exported.setChannelToCcid = function(ccid, trickplay, contentAccessDescriptorURL, quiet) {
        return hbbtv.native.request('Broadcast.setChannelToCcid', {
            ccid: ccid,
            trickplay: trickplay,
            contentAccessDescriptorURL: contentAccessDescriptorURL,
            quiet: quiet,
        }).result;
    };

    /**
     * Select the given broadcast channel (e.g. tune) with the given triplet and information.
     *
     * Security: FOR_RUNNING_APP_ONLY.
     *
     * @param {number} idType The type of the channel to set (ID_* code).
     * @param {number} onid The original network ID of the channel to set.
     * @param {number} tsid The transport stream ID of the channel to set.
     * @param {number} sid The service ID of the channel to set.
     * @param {number} sourceID Optionally, the ATSC source_ID of the channel to set; or -1
     *    otherwise.
     * @param {string} ipBroadcastID Optionally, the DVB textual service ID of the (IP broadcast)
     *    channel to set; or an empty string otherwise.
     * @param {boolean} trickplay True if the application has optionally hinted trickplay resources
     *    are required; or false otherwise. Does not affect the success of this operation.
     * @param {string} contentAccessDescriptorURL Optionally, additional information for
     *    DRM-protected IPTV broadcasts; or an empty string otherwise.
     * @param {number} quiet Type of channel change: 0 for normal, 1 for normal no UI, 2 for quiet
     *    (HbbTV A.2.4.3.2).
     *
     * @returns {number} A CHANNEL_STATUS_* code (on success, the code has a value less than 0).
     */
    exported.setChannelToTriplet = function(
        idType,
        onid,
        tsid,
        sid,
        sourceID,
        ipBroadcastID,
        trickplay,
        contentAccessDescriptorURL,
        quiet
    ) {
        return hbbtv.native.request('Broadcast.setChannelToTriplet', {
            idType: idType,
            onid: onid,
            tsid: tsid,
            sid: sid,
            sourceID: sourceID,
            ipBroadcastID: ipBroadcastID,
            trickplay: trickplay,
            contentAccessDescriptorURL: contentAccessDescriptorURL,
            quiet: quiet,
        }).result;
    };

    /**
     * Select the broadcast channel with the given DSD. 8 Security: FOR_RUNNING_APP_ONLY.
     *
     * @param {string} dsd The DSD of the channel to set.
     * @param {number} sid The service ID of the channel to set.
     * @param {boolean} trickplay True if the application has optionally hinted trickplay resources
     *    are  required; or false otherwise. Does not affect the success of this operation.
     * @param {string} contentAccessDescriptorURL Optionally, additional information for
     *    DRM-protected IPTV broadcasts; or an empty string otherwise.
     * @param {number} quiet Type of channel change: 0 for normal, 1 for normal no UI, 2 for quiet
     *    (HbbTV A.2.4.3.2).
     *
     * @returns {number} A CHANNEL_STATUS_* code (on success, the code has a value less than 0).
     */
    exported.setChannelToDsd = function(dsd, sid, trickplay, contentAccessDescriptorURL, quiet) {
        return hbbtv.native.request('Broadcast.setChannelToDsd', {
            dsd: dsd,
            sid: sid,
            trickplay: trickplay,
            contentAccessDescriptorURL: contentAccessDescriptorURL,
            quiet: quiet,
        }).result;
    };

    /**
     * Get the programme list for the given broadcast channel.
     *
     * Security: FOR_BROADCAST_APP_ONLY.
     *
     * @param {string} ccid The CCID of the broadcast channel.
     *
     * @returns {Array.<Programme>} A list of Programme objects available for the broadcast channel;
     *    or an empty list if not available.
     */
    exported.getProgrammes = function(ccid) {
        return hbbtv.native.request('Broadcast.getProgrammes', {
            ccid: ccid,
        }).result;
    };

    /**
     * Get the component list for the given broadcast channel.
     *
     * Security: FOR_BROADCAST_APP_ONLY.
     *
     * @param {string} ccid The CCID of the broadcast channel.
     * @param {number} type Optionally the component type (COMPONENT_TYPE_* code); or -1 otherwise.
     *
     * @returns {Array.<Component>} A list of Component objects available for the broadcast channel;
     *    or an empty list if not available.
     */
    exported.getComponents = function(ccid, type) {
        return hbbtv.native.request('Broadcast.getComponents', {
            ccid: ccid,
            type: type,
        }).result;
    };

    /**
     * Select the broadcast component with the given type, PID and optionally language.
     *
     * Security: FOR_BROADCAST_APP_ONLY.
     *
     * @param {number} type The type of the component to select (COMPONENT_TYPE_* code).
     * @param {number} pid The PID of the component to select.
     * @param {string} language Optionally, the language of the component to select; or an empty
     *    string otherwise.
     */
    exported.selectComponent = function(type, pid, language) {
        hbbtv.native.request('Broadcast.selectComponent', {
            type: type,
            pid: pid,
            language: language,
        });
    };

    /**
     * Unselect the broadcast component with the given type and PID.
     *
     * Security: FOR_BROADCAST_APP_ONLY.
     *
     * @param {number} type The type of the component to unselect (COMPONENT_TYPE_* code).
     * @param {number} pid The PID of the component to unselect.
     */
    exported.unselectComponent = function(type, pid) {
        hbbtv.native.request('Broadcast.unselectComponent', {
            type: type,
            pid: pid,
        });
    };

    /**
     * Start a metadata search.
     *
     * When the result is ready, it is dispatched to the bridge as a MetadataSearch event.
     *
     * Security: FOR_BROADCAST_APP_ONLY.
     *
     * @param {string} query A string that represents the query the application wants to carry out.
     * @param {number} offset Offset into the total result set.
     * @param {number} count Maximum number of items to include in the result from the offset.
     * @param {Array.<string>} channelConstraintList Optionally, a list of strings describing
     *    constraints the application wants applied to the query; or an empty list otherwise.
     */
    exported.startSearch = function(query, offset, count, channelConstraints) {
        hbbtv.native.request('Broadcast.startSearch', {
            channelConstraints: channelConstraints,
            count: count,
            offset: offset,
            query: query,
        });
    };

    /**
     * Abort a started metadata search.
     *
     * Security: FOR_BROADCAST_APP_ONLY.
     *
     * @param {number} queryId The ID from the query string that started the search.
     */
    exported.abortSearch = function(queryId) {
        hbbtv.native.request('Broadcast.abortSearch', {
            queryId: queryId,
        });
    };

    /**
     * Add a stream event listener subscribed to a DSM-CC event.
     *
     * When an event is transmitted, it is dispatched to the bridge as a StreamEvent event.
     *
     * @param {string} targetURL Optionally, the DSM-CC StreamEvent object URL; or an empty string
     *    otherwise.
     * @param {string} eventName The event name to subscribe to.
     * @param {number} componentTag If no targetURL is provided, the component tag; or -1 otherwise.
     * @param {number} streamEventId If no targetURL is provided, the event ID; or -1 otherwise.
     *
     * @returns {number} The listener ID. Associated StreamEvent events shall include this ID.
     */
    exported.addStreamEventListener = function(targetURL, eventName, componentTag, streamEventId) {
        return hbbtv.native.request('Broadcast.addStreamEventListener', {
            targetURL: targetURL,
            eventName: eventName,
            componentTag: componentTag,
            streamEventId: streamEventId,
        }).result;
    };

    /**
     * Remove the given stream event listener.
     *
     * @param {number} id The listener ID to remove.
     */
    exported.removeStreamEventListener = function(id) {
        hbbtv.native.request('Broadcast.removeStreamEventListener', {
            id: id,
        });
    };

    /**
     * Set whether the broadcast presentation should be suspended.
     *
     * Presentation of broadcast components such as audio, video and text tracks should be suspended
     * or resumed, with the component selections preserved. The channel should remain logically
     * selected (e.g. tuned).
     *
     * Security: FOR_BROADCAST_APP_ONLY.
     *
     * @param {boolean} presentationSuspended True if the broadcast presentation should be suspended;
     *    false otherwise.
     */
    exported.setPresentationSuspended = function(presentationSuspended) {
        hbbtv.native.request('Broadcast.setPresentationSuspended', {
            presentationSuspended: presentationSuspended,
        });
    };

    return exported;
})();

hbbtv.bridge.programme = (function() {
    const exported = {};

    /**
     * Get the parental rating of the current broadcast programme.
     *
     * @return {ParentalRating} A ParentalRating object representing the parental rating; or null if
     *    not available.
     */
    exported.getParentalRating = function() {
        return hbbtv.native.request('Programme.getParentalRating').result;
    };

    /**
     * Get a list of the raw SI descriptor data for a programme.
     *
     * @param {string} ccid The CCID (channel ID) of the programme.
     * @param {string} programmeID The ID of the programme.
     * @param {number} descriptorTag The descriptor tag ID of the data to get.
     * @param {number} descriptorTagExtension Optionally, the extended descriptor tag ID of the data
     *    to get; -1 otherwise.
     * @param {number} privateDataSpecifier Optionally, the private data specifier of the data to be
     *    returned; 0 otherwise.
     *
     * @return {Array.<string>} A list of SI descriptor data. If there are multiple descriptors with
     *    the same tag ID, they will all be returned.
     */
    exported.getSIDescriptors = function(
        ccid,
        programmeID,
        descriptorTag,
        descriptorTagExtension,
        privateDataSpecifier
    ) {
        return hbbtv.native.request('Programme.getSIDescriptors', {
            ccid: ccid,
            programmeID: programmeID,
            descriptorTag: descriptorTag,
            descriptorTagExtension: descriptorTagExtension,
            privateDataSpecifier: privateDataSpecifier,
        }).result;
    };

    return exported;
})();

hbbtv.bridge.manager = (function() {
    const exported = {};

    /**
     * Create and run a new application.
     *
     * The calling application is identified by the token associated with the request, to check
     * whether it is allowed to create and run a new application.
     *
     * @param {string} url A HTTP/HTTPS or DVB URL.
     *
     *    A HTTP/HTTPS URL may refer to the entry page or XML AIT of a broadcast-independent app.
     *
     *    A DVB URL may refer to a broadcast-related app signalled in the current service AIT. This
     *    will result in the signalled URL being loaded, which may be HTTP/HTTPS for broadband or DVB
     *    for carousel.
     *
     * @return {boolean} True if the application can be created; false otherwise.
     */
    exported.createApplication = function(url) {
        return hbbtv.native.request('Manager.createApplication', {
            url: url,
        }).result;
    };

    /**
     * Destroy the calling application.
     *
     * The calling application is identified by the token associated with the request.
     */
    exported.destroyApplication = function() {
        hbbtv.native.request('Manager.destroyApplication');
    };

    /**
     * Show the calling application.
     *
     * The calling application is identified by the token associated with the request.
     */
    exported.showApplication = function() {
        hbbtv.native.request('Manager.showApplication');
    };

    /**
     * Hide the calling application.
     *
     * The calling application is identified by the token associated with the request.
     */
    exported.hideApplication = function() {
        hbbtv.native.request('Manager.hideApplication');
    };

    /**
     * Get the free memory available to the application.
     *
     * @returns {number} The free memory in bytes.
     */
    exported.getFreeMem = function() {
        return hbbtv.native.request('Manager.getFreeMem').result;
    };

    /**
     * Get the keyset for this application.
     *
     * The calling application is identified by the token associated with the request.
     *
     * @return {number} The keyset for this application.
     */
    exported.getKeyValues = function() {
        return hbbtv.native.request('Manager.getKeyValues').result;
    };

    /**
     * Get the maximum keyset available to applications.
     *
     * @return {number} }he maximum keyset available to applications.
     */
    exported.getKeyMaximumValue = function() {
        return hbbtv.native.request('Manager.getKeyMaximumValue').result;
    };

    /**
     * Set the keyset for this application.
     *
     * The calling application is identified by the token associated with the request.
     *
     * @param {number} value The keyset to set for this application.
     *
     * @return {number} The keyset for this application.
     */
    exported.setKeyValue = function(value, otherKeys) {
        return hbbtv.native.request('Manager.setKeyValue', {
            value: value,
            otherKeys: otherKeys,
        }).result;
    };

    /**
     * Get the URL of an icon that represents the given key.
     *
     * @param {number} code The code of the key to get an icon for (VK_ code).
     *
     * @return {string} A URL of an icon that represents the given key; or null if not available.
     */
    exported.getKeyIcon = function(code) {
        return hbbtv.native.request('Manager.getKeyIcon', {
            code: code,
        }).result;
    };

    return exported;
})();

hbbtv.bridge.parentalControl = (function() {
    const exported = {};

    /**
     * Get a list of rating schemes supported by this integration.
     *
     * @return {Array.<ParentalRatingScheme>} A list of ParentalRatingScheme objects representing the
     *    supported parental rating schemes; or an empty list of not available.
     */
    exported.getRatingSchemes = function() {
        return hbbtv.native.request('ParentalControl.getRatingSchemes').result;
    };

    /**
     * Get the parental rating threshold currently set on the system for the given scheme.
     *
     * @param {string} schemeName The name of the scheme to get the threshold of.
     *
     * @return {ParentalRating} A ParentalRating object representing the threshold.
     */
    exported.getThreshold = function(schemeName) {
        return hbbtv.native.request('ParentalControl.getThreshold', {
            scheme: schemeName,
        }).result;
    };

    /**
     * Test whether the rating is blocked for the given scheme, region and value.
     *
     * @param {string} scheme The name of the scheme to test.
     * @param {string} region The code of the region to test (ISO 3166-1 alpha-3 codes).
     * @param {number} value The value to test.
     *
     * @return {boolean} True if the rating is blocked; or false otherwise.
     */
    exported.isRatingBlocked = function(rating) {
        return hbbtv.native.request('ParentalControl.isRatingBlocked', rating).result;
    };

    return exported;
})();

hbbtv.bridge.configuration = (function() {
    const exported = {};

    /**
     * Get certain immutable information about the system.
     *
     * @return {SystemInformation} A SystemInformation object.
     */
    exported.getLocalSystem = function() {
        return hbbtv.native.request('Configuration.getLocalSystem').result;
    };

    /**
     * Get preferred languages to be used for audio playback on this system.
     *
     * @return {string} Comma separated string of languages (ISO 639-2 codes), in order of
     *    preference.
     */
    exported.getPreferredAudioLanguage = function() {
        return hbbtv.native.request('Configuration.getPreferredAudioLanguage').result;
    };

    /**
     * Get preferred languages to be used for the user-interface on this system.
     *
     * @return {string} Comma separated string of languages (ISO 639-2 codes), in order of
     *    preference.
     */
    exported.getPreferredSubtitleLanguage = function() {
        return hbbtv.native.request('Configuration.getPreferredSubtitleLanguage').result;
    };

    /**
     * Get preferred languages to be used for the user-interface on this system.
     *
     *
     * @return {string} Comma separated string of languages (ISO 639-2 codes), in order of
     *    preference.
     */
    exported.getPreferredUILanguage = function() {
        return hbbtv.native.request('Configuration.getPreferredUILanguage').result;
    };

    /**
     * Get a string containing the three character country code identifying the country this system
     * is deployed.
     *
     * @return {string} Country code the receiver is deployed (ISO 3166-1 alpha-3 code).
     */
    exported.getCountryId = function() {
        return hbbtv.native.request('Configuration.getCountryId').result;
    };

    /**
     * Get whether subtitles are enabled on this system.
     *
     * @return {boolean} True if subtitles are enabled; or false otherwise.
     */
    exported.getSubtitlesEnabled = function() {
        return hbbtv.native.request('Configuration.getSubtitlesEnabled').result;
    };

    /**
     * Get whether audio description is enabled on this system.
     *
     * @return {boolean} True if audio description is enabled; or false otherwise.
     */
    exported.getAudioDescriptionEnabled = function() {
        return hbbtv.native.request('Configuration.getAudioDescriptionEnabled').result;
    };

    /**
     * Get the DVB network IDs of the channels in the broadcast channel list.
     *
     * @return {Array.<String>} A list of DVB network IDs; or an empty list of not available.
     */
    exported.getDttNetworkIds = function() {
        return hbbtv.native.request('Configuration.getDttNetworkIds').result;
    };

    /**
     * Get a distinctive identifier for this terminal and calling origin; or a status code if not
     * available. To get access to a distinctive identifier, the calling origin must have previously
     * called Configuration_requestAccessToDistinctiveIdentifier.
     *
     * The calling origin is identified by the token associated with the request.
     *
     * Integrators should check HbbTV 12.1.5 for requirements about distinctive identifiers.
     *
     * @return {string} A distinctive identifier, uniquely generated for this terminal and calling
     *    origin; or a status code (DISTINCTIVE_IDENTIFIER_STATUS_ code).
     */
    exported.getDeviceId = function() {
        return hbbtv.native.request('Configuration.getDeviceId').result;
    };

    /**
     * Request distinctive identifier access for this calling origin.
     *
     * The calling origin is identified by the token associated with the request.
     *
     * The client application should display a dialog for the user to allow or deny this. When the
     * result is ready, it is dispatched to the bridge as a accesstodistinctiveidentifier event.
     */
    exported.requestAccessToDistinctiveIdentifier = function() {
        hbbtv.native.request('Configuration.requestAccessToDistinctiveIdentifier');
    };

    return exported;
})();

hbbtv.bridge.mediaSync = (function() {
    const exported = {};

    /**
     *
     * @return {number}
     */
    exported.instantiate = function() {
        return hbbtv.native.request('MediaSynchroniser.instantiate').result;
    };

    /**
     * @param {number} id
     * @param {boolean} isMasterBroadcast
     *
     * @return {boolean}
     */
    exported.initialise = function(id, isMasterBroadcast) {
        return hbbtv.native.request('MediaSynchroniser.initialise', {
            id: id,
            isMasterBroadcast: isMasterBroadcast,
        }).result;
    };

    /**
     * @param {number} id
     */
    exported.destroy = function(id) {
        hbbtv.native.request('MediaSynchroniser.destroy', {
            id: id,
        });
    };

    /**
     * @param {number} id
     */
    exported.enableInterDeviceSync = function(id) {
        hbbtv.native.request('MediaSynchroniser.enableInterDeviceSync', {
            id: id,
        });
    };

    /**
     * @param {number} id
     */
    exported.disableInterDeviceSync = function(id) {
        hbbtv.native.request('MediaSynchroniser.disableInterDeviceSync', {
            id: id,
        });
    };

    /**
     * @param {number} id
     *
     * @return {number}
     */
    exported.nrOfSlaves = function(id) {
        return hbbtv.native.request('MediaSynchroniser.nrOfSlaves', {
            id: id,
        }).result;
    };

    /**
     * @param {number} id
     *
     * @return {boolean}
     */
    exported.interDeviceSyncEnabled = function(id) {
        return hbbtv.native.request('MediaSynchroniser.interDeviceSyncEnabled', {
            id: id,
        }).result;
    };

    /**
     * @param {number} id
     *
     * @return {string}
     */
    exported.getContentIdOverride = function(id) {
        return hbbtv.native.request('MediaSynchroniser.getContentIdOverride', {
            id: id,
        }).result;
    };

    /**
     * @param {number} id
     * @param {Object} timeline
     *
     * @return {number}
     */
    exported.startTimelineMonitoring = function(id, timelineSelector, isMaster) {
        return hbbtv.native.request('MediaSynchroniser.startTimelineMonitoring', {
            id: id,
            timelineSelector: timelineSelector,
            isMaster: isMaster,
        }).result;
    };

    /**
     * @param {number} id
     * @param {number} timelineId
     */
    exported.stopTimelineMonitoring = function(id, timelineSelector, forceStop) {
        hbbtv.native.request('MediaSynchroniser.stopTimelineMonitoring', {
            id: id,
            timelineSelector: timelineSelector,
            forceStop: forceStop,
        });
    };

    /**
     * @param {number} timelineSelector
     */
    exported.getBroadcastCurrentTime = function(timelineSelector) {
        return hbbtv.native.request('MediaSynchroniser.getBroadcastCurrentTime', {
            timelineSelector: timelineSelector,
        }).result;
    };

    /**
     * @param {number} id
     * @param {string} contentIdOverride
     */
    exported.setContentIdOverride = function(id, contentIdOverride) {
        hbbtv.native.request('MediaSynchroniser.setContentIdOverride', {
            id: id,
            contentIdOverride: contentIdOverride,
        });
    };

    /**
     * @param {number} id
     * @param {number} contentTime
     * @param {number} speed
     */
    exported.setContentTimeAndSpeed = function(id, timelineSelector, contentTime, speed) {
        hbbtv.native.request('MediaSynchroniser.setContentTimeAndSpeed', {
            id: id,
            timelineSelector: timelineSelector,
            contentTime: contentTime,
            speed: speed,
        });
    };

    /**
     * @param {number} id
     * @param {Object} properties
     */
    exported.updateCssCiiProperties = function(
        id,
        contentId,
        presentationStatus,
        contentIdStatus,
        mrsUrl
    ) {
        hbbtv.native.request('MediaSynchroniser.updateCssCiiProperties', {
            id: id,
            contentId: contentId,
            presentationStatus: presentationStatus,
            contentIdStatus: contentIdStatus,
            mrsUrl: mrsUrl,
        });
    };

    /**
     * @param {number} id
     * @param {Object} properties
     */
    exported.setTimelineAvailability = function(id, timelineSelector, isAvailable, ticks, speed) {
        return hbbtv.native.request('MediaSynchroniser.setTimelineAvailability', {
            id: id,
            timelineSelector: timelineSelector,
            isAvailable: isAvailable,
            ticks: ticks || 0,
            speed: speed || 0,
        }).result;
    };

    return exported;
})();

hbbtv.bridge.csManager = (function() {
    const exported = {};

    /**
     *
     * @return {string}
     */
    exported.getApp2AppLocalBaseURL = function() {
        return hbbtv.native.request('CSManager.getApp2AppLocalBaseURL').result;
    };

    /**
     *
     * @return {string}
     */
    exported.getInterDevSyncURL = function() {
        return hbbtv.native.request('CSManager.getInterDevSyncURL').result;
    };

    /**
     *
     * @return {string}
     */
    exported.getApp2AppRemoteBaseURL = function() {
        return hbbtv.native.request('CSManager.getApp2AppRemoteBaseURL').result;
    };

    return exported;
})();

hbbtv.bridge.orbDebug = (function() {
    const exported = {};

    /**
     * Publish a test report (debug build only).
     *
     * @param {string} testSuite A unique test suite name.
     * @param {string} xml The XML test report.
     */
    exported.publishTestReport = function(testSuite, xml) {
        return hbbtv.native.request('OrbDebug.publishTestReport', {
            testSuite: testSuite,
            xml: xml,
        });
    };

    return exported;
})();