/**
 * @fileOverview Native-agnostic interface to outside the browser context (e.g. to the broadcast stack).
 * @license ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
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

/**
 * ORB internal interface: system-agnostic bridge to native.
 *
 * @name bridge
 * @class
 * @constructor
 */
hbbtv.bridge = (function() {
    const exported = {};

    const gEventDispatcher = new hbbtv.utils.EventDispatcher();

    /**
     * Initialise the bridge.
     *
     * @method
     * @memberof bridge#
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
     *
     * @method
     * @memberof bridge#
     */
    exported.addStrongEventListener = function(type, callback) {
        gEventDispatcher.addEventListener(type, callback);
    };

    /**
     * Remove a previously added "strong" event listener.
     *
     * @param {string} type The previously added event type.
     * @param {listenerCallback} callbackThe The previously added callback.
     *
     * @method
     * @memberof bridge#
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
     *
     * @method
     * @memberof bridge#
     */
    exported.addWeakEventListener = function(type, callback) {
        gEventDispatcher.addWeakEventListener(type, callback);
    };

    /**
     * Remove a previously added "weak" event listener.
     *
     * @param {string} type The previously added event type.
     * @param {listenerCallback} callbackThe The previously added callback.
     *
     * @method
     * @memberof bridge#
     */
    exported.removeWeakEventListener = function(type, callback) {
        gEventDispatcher.removeWeakEventListener(type, callback);
    };

    return exported;
})();

/**
 * ORB internal interface: system-agnostic bridge to native broadcast.
 *
 * @name bridge.broadcast
 * @class
 * @constructor
 */
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
     *
     * @method
     * @memberof bridge.broadcast#
     */
    exported.setVideoRectangle = function(x, y, width, height, fullScreen) {
        hbbtv.native.request('Broadcast.setVideoRectangle', {
            x: x,
            y: y,
            width: width,
            height: height,
        });
    };

    /**
     * Get the current broadcast channel.
     *
     * Security: FOR_BROADCAST_APP_ONLY.
     *
     * @returns {Channel} A Channel object representing the current broadcast channel; or null if
     *    not available.
     *
     * @method
     * @memberof bridge.broadcast#
     */
    exported.getCurrentChannel = function() {
        let currentChannel = hbbtv.native.request('Broadcast.getCurrentChannel').result;
        if (currentChannel.idType === hbbtv.objects.Channel.prototype.ID_DVB_SI_DIRECT) {
            currentChannel.dsd = hbbtv.utils.base64Decode(currentChannel.dsd);
        }
        return currentChannel;
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
     *
     * @method
     * @memberof bridge.broadcast#
     */
    exported.getCurrentChannelForEvent = function() {
        let getCurrentChannelForEvent = hbbtv.native.request('Broadcast.getCurrentChannelForEvent').result;
        if (getCurrentChannelForEvent.idType === hbbtv.objects.Channel.prototype.ID_DVB_SI_DIRECT) {
            getCurrentChannelForEvent.dsd = hbbtv.utils.base64Decode(getCurrentChannelForEvent.dsd);
        }
        return getCurrentChannelForEvent;
    };

    /**
     * Get the broadcast channel list.
     *
     * Security: FOR_BROADCAST_APP_ONLY.
     *
     * @returns {Array.<Channel>} A list of Channel objects representing the broadcast channel list;
     *    or an empty list if not available.
     *
     * @method
     * @memberof bridge.broadcast#
     */
    exported.getChannelList = function() {
        let channelList = hbbtv.native.request('Broadcast.getChannelList').result;
        channelList.forEach(channel => {
            if (channel.idType === hbbtv.objects.Channel.prototype.ID_DVB_SI_DIRECT) {
                channel.dsd = hbbtv.utils.base64Decode(channel.dsd);
            }
        });
        return channelList;
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
     *
     * @method
     * @memberof bridge.broadcast#
     */
    exported.setChannelToNull = function(quiet) {
        return hbbtv.native.request('Broadcast.setChannelToNull').result;
    };

    /**
     * Returns the actual volume level set.
     *
     * @returns {number} Integer value between 0 up to and including 100 to indicate volume level.
     *
     * @method
     * @memberof bridge.broadcast#
     */
    exported.getVolume = function() {
        return hbbtv.native.request('Broadcast.getVolume').result;
    };

    /**
     * Adjusts the volume of the currently playing media to the volume as indicated by volume.
     *
     * @param {number} volume Integer value between 0 up to and including 100 to indicate volume level.
     * @returns {Boolean} true if the volume has changed. false if the volume has not changed.
     *
     * @method
     * @memberof bridge.broadcast#
     */
    exported.setVolume = function(volume) {
        return hbbtv.native.request('Broadcast.setVolume', {volume: volume}).result;
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
     *
     * @method
     * @memberof bridge.broadcast#
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
     *
     * @method
     * @memberof bridge.broadcast#
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
     *
     * @method
     * @memberof bridge.broadcast#
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
     *
     * @method
     * @memberof bridge.broadcast#
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
     *
     * @method
     * @memberof bridge.broadcast#
     */
    exported.getComponents = function(ccid, type) {
        return hbbtv.native.request('Broadcast.getComponents', {
            ccid: ccid,
            type: type,
        }).result;
    };

    /**
     * Get a private audio component in the selected channel.
     *
     * Security: FOR_BROADCAST_APP_ONLY.
     *
     * @param {string} componentTag The component_tag of the component.
     *
     * @returns {Component} The private component with the specified component_tag in the PMT of the
     * currently selected broadcast channel; or null if unavailable or the component is not
     * private (i.e. the stream type is audio, video or subtitle).
     *
     * Mandatory properties: id, pid and encrypted. The id property shall be usable with the
     * overrideComponentSelection method to select the component as an audio track. Other Component
     * properties are not required.
     *
     * @method
     * @memberof bridge.broadcast#
     */
    exported.getPrivateAudioComponent = function(componentTag) {
        return hbbtv.native.request('Broadcast.getPrivateAudioComponent', {
            componentTag: componentTag,
        }).result;
    };

    /**
     * Get a private video component in the selected channel.
     *
     * Security: FOR_BROADCAST_APP_ONLY.
     *
     * @param {string} componentTag The component_tag of the stream.
     *
     * @returns {Component} The private component with the specified component_tag in the PMT of the
     * currently selected broadcast channel; or null if unavailable or the component is not
     * private (i.e. the stream type is audio, video or subtitle).
     *
     * Mandatory properties: id, pid and encrypted. The id property shall be usable with the
     * overrideComponentSelection method to select the component as a video track. Other Component
     * properties are not required.
     *
     * @method
     * @memberof bridge.broadcast#
     */
    exported.getPrivateVideoComponent = function(componentTag) {
        return hbbtv.native.request('Broadcast.getPrivateVideoComponent', {
            componentTag: componentTag,
        }).result;
    };

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
     * @param {number} type Type of component selection to override (COMPONENT_TYPE_* code).
     * @param {string} id A platform-defined component id or an empty string to disable presentation.
     *
     * @method
     * @memberof bridge.broadcast#
     */
    exported.overrideComponentSelection = function(type, id) {
        hbbtv.native.request('Broadcast.overrideComponentSelection', {
            type: type,
            id: id,
        });
    };

    /**
     * Restore the default component selection of the terminal for the specified type.
     *
     * If playback has already started, the presented component shall be updated.
     *
     * Security: FOR_BROADCAST_APP_ONLY.
     *
     * @param {number} type Type of component selection to restore (COMPONENT_TYPE_* code).
     *
     * @method
     * @memberof bridge.broadcast#
     */
    exported.restoreComponentSelection = function(type) {
        hbbtv.native.request('Broadcast.restoreComponentSelection', {
            type: type,
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
     *
     * @method
     * @memberof bridge.broadcast#
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
     *
     * @method
     * @memberof bridge.broadcast#
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
     *
     * @method
     * @memberof bridge.broadcast#
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
     *
     * @method
     * @memberof bridge.broadcast#
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
     *
     * @method
     * @memberof bridge.broadcast#
     */
    exported.setPresentationSuspended = function(presentationSuspended) {
        hbbtv.native.request('Broadcast.setPresentationSuspended', {
            presentationSuspended: presentationSuspended,
        });
    };

    return exported;
})();

/**
 * ORB internal interface: system-agnostic bridge to native programme information.
 *
 * TODO Move to bridge.broadcast
 *
 * @name bridge.programme
 * @class
 * @constructor
 */
hbbtv.bridge.programme = (function() {
    const exported = {};

    /**
     * Get the parental rating of the current broadcast programme.
     *
     * @return {ParentalRating} A ParentalRating object representing the parental rating; or null if
     *    not available.
     *
     * @method
     * @memberof bridge.programme#
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
     *
     * @method
     * @memberof bridge.programme#
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

/**
 * ORB internal interface: system-agnostic bridge to native application manager.
 *
 * @name bridge.manager
 * @class
 * @constructor
 */
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
     *
     * @method
     * @memberof bridge.manager#
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
     *
     * @method
     * @memberof bridge.manager#
     */
    exported.destroyApplication = function() {
        hbbtv.native.request('Manager.destroyApplication');
    };

    /**
     * Show the calling application.
     *
     * The calling application is identified by the token associated with the request.
     *
     * @method
     * @memberof bridge.manager#
     */
    exported.showApplication = function() {
        hbbtv.native.request('Manager.showApplication');
    };

    /**
     * Hide the calling application.
     *
     * The calling application is identified by the token associated with the request.
     *
     * @method
     * @memberof bridge.manager#
     */
    exported.hideApplication = function() {
        hbbtv.native.request('Manager.hideApplication');
    };

    /**
     * Get the free memory available to the application.
     *
     * @returns {number} The free memory in bytes.
     *
     * @method
     * @memberof bridge.manager#
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
     *
     * @method
     * @memberof bridge.manager#
     */
    exported.getKeyValues = function() {
        return hbbtv.native.request('Manager.getKeyValues').result;
    };

    /**
     * Get the other for this application.
     *
     * The calling application is identified by the token associated with the request.
     *
     * @return {number} The other keys for this application.
     *
     * @method
     * @memberof bridge.manager#
     */
    exported.getOtherKeyValues = function() {
        return hbbtv.native.request('Manager.getOtherKeyValues').result;
    };

    /**
     * Get the maximum keyset available to applications.
     *
     * @return {number} }he maximum keyset available to applications.
     *
     * @method
     * @memberof bridge.manager#
     */
    exported.getKeyMaximumValue = function() {
        return hbbtv.native.request('Manager.getKeyMaximumValue').result;
    };

    /**
     * Get the maximum other keys available to applications.
     *
     * @return {number} The maximum other keys available to applications.
     *
     * @method
     * @memberof bridge.manager#
     */
    exported.getKeyMaximumValue = function() {
        return hbbtv.native.request('Manager.getKeyMaximumOtherKeys').result;
    };

    /**
     * Set the keyset for this application.
     *
     * The calling application is identified by the token associated with the request.
     *
     * @param {number} value The keyset to set for this application.
     *
     * @return {number} The keyset for this application.
     *
     * @method
     * @memberof bridge.manager#
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
     *
     * @method
     * @memberof bridge.manager#
     */
    exported.getKeyIcon = function(code) {
        return hbbtv.native.request('Manager.getKeyIcon', {
            code: code,
        }).result;
    };

    exported.getApplicationScheme = function() {
        return hbbtv.native.request('Manager.getApplicationScheme').result;
    }

    return exported;
})();

/**
 * ORB internal interface: system-agnostic bridge to native parental control.
 *
 * TODO Move to bridge.configuration.
 *
 * @name bridge.parentalControl
 * @class
 * @constructor
 */
hbbtv.bridge.parentalControl = (function() {
    const exported = {};

    /**
     * Get a list of rating schemes supported by this integration.
     *
     * @return {Array.<ParentalRatingScheme>} A list of ParentalRatingScheme objects representing the
     *    supported parental rating schemes; or an empty list of not available.
     *
     * @method
     * @memberof bridge.parentalControl#
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
     *
     * @method
     * @memberof bridge.parentalControl#
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
     *
     * @method
     * @memberof bridge.parentalControl#
     */
    exported.isRatingBlocked = function(rating) {
        return hbbtv.native.request('ParentalControl.isRatingBlocked', rating).result;
    };

    return exported;
})();

/**
 * ORB internal interface: system-agnostic bridge to native configuration.
 *
 * @name bridge.configuration
 * @class
 * @constructor
 */
hbbtv.bridge.configuration = (function() {
    const exported = {};

    /**
     * @typedef {Object} Capabilities Create a capabilities type that describes the current
     * capabilities of the terminal.
     *
     * @property {Array.<string>} optionStrings A list of HbbTV option strings supported by the
     * terminal.
     * Valid values as defined by HBBTV 10.2.4.8 table 13.
     * @property {Array.<string>} profileNameFragments A list of OIPF UI Profile Name Fragments
     * supported by the terminal; this shall always include trick mode ("+TRICKMODE"), any supported
     * broadcast delivery systems (e.g. "+DVB_S") and no other values.
     * Valid values as defined by OIPF DAE table 16.
     * @property {Array.<string>} parentalSchemes A list of parental scheme names registered with
     * the platform.
     * Valid values are usable as scheme names with other parental APIs.
     * @property {Array.<string>|null} graphicsLevels A list of graphics performance levels supported
     * by the terminal
     * (required if any of the graphics levels are supported, null to omit).
     * Valid values as defined by HBBTV 10.2.4.7 table 12c.
     * @property {Array.<string>|null} broadcastUrns A list of URNs for each supported broadcast
     * technology (required if any broadcast delivery is supported, null to omit).
     * Valid values as defined in HBBTV 10.2.4.7 for broadcast element value.
     * @property {string} displaySizeWidth The current width of the primary display in centimetres.
     * Valid values as defined by HBBTV 10.2.4.7 for display_size width.
     * @property {string} displaySizeHeight The current height of the primary display in centimetres.
     * Valid values as defined by HBBTV 10.2.4.7 for display_size height.
     * @property {string} displaySizeMeasurementType The measurement type.
     * Valid values as defined by HBBTV 10.2.4.7 for display_size measurement_type.
     * @property {string|null} audioOutputFormat The current multi-channel audio capabilities
     * (required where terminals support multi-channel audio, null to omit).
     * Valid values as defined by HBBTV 10.2.4.7 for audio_system audio_output_format.
     * @property {boolean|null} passThroughStatus True when the terminal's audio outputs are
     * operating in a pass-through mode in which broadcast or broadband audio bitstreams are output
     * directly by the terminal without modification. False otherwise.
     * @property {string|null} html5MediaVariableRateMin Minimum supported forward playback rate
     * (required where terminals support a playbackRate with a MediaSource object other than "1.0",
     * null to omit).
     * Valid values as defined by HBBTV 10.2.4.7 for html5_media_variable_rate min.
     * @property {string|null} html5MediaVariableRateMax Maximum supported forward playback rate
     * (required where terminals support a playbackRate with a MediaSource object other than "1.0",
     * null to omit).
     * Valid values as defined by HBBTV 10.2.4.7 for html5_media_variable_rate max.
     * @property {string|null} jsonRpcServerUrl The URL of the JSON RPC WebSocket server
     * (required where terminals support HbbTV 204, null to omit).
     * @property {string|null} jsonRpcServerVersion The version of the JSON RPC WebSocket server
     * (required where terminals support HbbTV 204, null to omit).
     */

    /**
     * @typedef {Object} AudioProfile Create an AudioProfile type that describes an audio profile,
     * valid combinations are as defined by HBBTV 10.2.4.7 for the audio_profile element.
     *
     * @property {string} name Name of profile (required).
     * Valid values as defined by OIPF DAE 9.3.11 for audio_profile name.
     * @property {string} type MIME type of profile (required).
     * Valid values as defined by OIPF DAE 9.3.11 for audio_profile type.
     * @property {string|null} transport Space separated list of supported protocol names (optional,
     * null to omit).
     * Valid values as defined by OIPF DAE 9.3.11 for audio_profile transport and HBBTV 10.2.4.7.
     * @property {string|null} syncTl Space separated list of timeline types (optional, null to
     * omit).
     * Valid values as defined by HBBTV 10.2.4.7 table 12a.
     * @property {string|null} drmSystemId Space separated list of DRM system IDs (optional, null to
     * omit).
     * Valid values as defined by OIPF DAE 9.3.11 for audio_profile DRMSystemID.
     */

    /**
     * @typedef {Object} VideoProfile Create a VideoProfile type that describes a video profile,
     * valid combinations are as defined by HBBTV 10.2.4.7 for the video_profile element.
     *
     * @property {string} name Name of profile (required).
     * Valid values as defined by OIPF DAE 9.3.11 for video_profile name.
     * @property {string} type MIME type of profile (required).
     * Valid values as defined by OIPF DAE 9.3.11 for video_profile type.
     * @property {string} transport Space separated list of supported protocol names (optional, null
     * to omit).
     * Valid values as defined by OIPF DAE 9.3.11 for video_profile transport and HBBTV 10.2.4.7.
     * @property {string|null} syncTl Space separated list of timeline types (optional, null to
     * omit).
     * Valid values as defined by HBBTV 10.2.4.7 table 12a.
     * @property {string|null} drmSystemId Space separated list of DRM system IDs (optional, null to
     * omit).
     * Valid values as defined by OIPF DAE 9.3.11 for video_profile DRMSystemID.
     * @property {string|null} hdr URI of HDR technology (optional, null to omit).
     * Valid values as defined by HBBTV 10.2.4.7 table 12b.
     */

    /**
     * @typedef {Object} VideoDisplayFormat Create a VideoDisplayFormat type that describes a video
     * display format, valid combinations are as defined by HBBTV 10.2.4.7 for the
     * video_display_format element.
     *
     * @property {number} width Width of the video content (required).
     * Valid values as defined by HBBTV 10.2.4.7 for video_display_format name.
     * @property {number} height Height of the video content (required).
     * Valid values as defined by HBBTV 10.2.4.7 for video_display_format height.
     * @property {number} frameRate Frame rate of the video content (required).
     * Valid values as defined by HBBTV 10.2.4.7 for video_display_format frame_rate.
     * @property {number} bitDepth Bit depth of the video content (required).
     * Valid values as defined by HBBTV 10.2.4.7 for video_display_format bit_depth.
     * @property {string} colorimetry A space separated list of colorimetry strings (required).
     * Valid values as defined by HBBTV 10.2.4.7 for video_display_format colorimetry.
     */

    /**
     * Get the current capabilities of the terminal.
     *
     * @return {Capabilities} A Capabilities object.
     *
     * @method
     * @memberof bridge.configuration#
     */
    exported.getCapabilities = function() {
        return hbbtv.native.request('Configuration.getCapabilities', {}).result;
    };

    /**
     * Get a list of audio profiles supported by the terminal, as defined by HBBTV 10.2.4.7 for
     * the audio_profile element.
     *
     * @return {Array.<AudioProfile>} A list of audio profiles supported by the terminal.
     *
     * @method
     * @memberof bridge.configuration#
     */
    exported.getAudioProfiles = function() {
        return hbbtv.native.request('Configuration.getAudioProfiles', {}).result;
    };

    /**
     * Get a list of video profiles supported by the terminal, as defined by HBBTV 10.2.4.7 for
     * the video_profile element.
     *
     * @return {Array.<VideoProfile>} A list of video profiles supported by the terminal.
     *
     * @method
     * @memberof bridge.configuration#
     */
    exported.getVideoProfiles = function() {
        return hbbtv.native.request('Configuration.getVideoProfiles', {}).result;
    };

    /**
     * If the terminal supports UHD, get a list that describes the highest quality video format the
     * terminal supports, as defined by HBBTV 10.2.4.7 for the video_display_format element;
     * otherwise get an empty list.
     *
     * Note: If the terminal changes its display format based on the content being played, multiple
     * elements may be included in the list when multiple frame rate families are usable or the
     * highest resolution does not support each highest quality parameter.
     *
     * @return {Array.<VideoDisplayFormat>} A list that describes the highest quality video format.
     *
     * @method
     * @memberof bridge.configuration#
     */
    exported.getVideoDisplayFormats = function() {
        return hbbtv.native.request('Configuration.getVideoDisplayFormats', {}).result;
    };

    /**
     * Get the current number of additional media streams containing SD video accompanied by audio
     * that can be decoded and presented by an A/V control object or HTML5 media element.
     *
     * @return {number} The current number of additional media streams. If the value is non-zero,
     * then a call to play an A/V control object, HTML5 media element or video/broadcast object
     * shall not fail due to lack of resources for SD media.
     *
     * @method
     * @memberof bridge.configuration#
     */
    exported.getExtraSDVideoDecodes = function() {
        return hbbtv.native.request('Configuration.getExtraSDVideoDecodes', {}).result;
    };

    /**
     * Get the current number of additional media streams containing HD video accompanied by audio
     * that can be decoded and presented by an A/V control object or HTML5 media element.
     *
     * @return {number} The current number of additional media streams. If the value is non-zero,
     * then a call to play an A/V control object, HTML5 media element or video/broadcast object
     * shall not fail due to lack of resources for HD media.
     *
     * @method
     * @memberof bridge.configuration#
     */
    exported.getExtraHDVideoDecodes = function() {
        return hbbtv.native.request('Configuration.getExtraHDVideoDecodes', {}).result;
    };

    /**
     * Get the current number of additional media streams containing UHD video accompanied by audio
     * that can be decoded and presented by an A/V control object or HTML5 media element.
     *
     * @return {number} The current number of additional media streams. If the value is non-zero,
     * then a call to play an A/V control object, HTML5 media element or video/broadcast object
     * shall not fail due to lack of resources for UHD media.
     *
     * @method
     * @memberof bridge.configuration#
     */
    exported.getExtraUHDVideoDecodes = function() {
        return hbbtv.native.request('Configuration.getExtraUHDVideoDecodes', {}).result;
    };

    /**
     * Get the maximum (static) broadband media decoding capabilities for the indicated decoder.
     * This method is defined in HBBTV-TA-1 v1.1.1 A.2.2.
     *
     * Media decoders are numbered from 1 increasing in steps of 1 up to the total number of
     * media decoders in the terminal. Decoders are ordered in decreasing capabilities (UHD before
     * HD, HD before SD). If decoderIndex is greater than the number of media decoders then null
     * shall be returned.
     *
     * @param {number} decoderIndex - The decoder index (1-based, starting from 1)
     * @return {Array.<string>|null} An array of video profile names (strings) for the decoder,
     *                               or null if decoderIndex is greater than the number of decoders.
     *                               Each string shall be one of the video_profile elements returned
     *                               by xmlCapabilities.
     *
     * @method
     * @memberof bridge.configuration#
     */
    exported.getBroadbandCapabilities = function(decoderIndex) {
        return hbbtv.native.request('Configuration.getBroadbandCapabilities', {
            decoderIndex: decoderIndex,
        }).result;
    };

    /**
     * Get certain immutable information about the system.
     *
     * @return {SystemInformation} A SystemInformation object.
     *
     * @method
     * @memberof bridge.configuration#
     */
    exported.getLocalSystem = function() {
        return hbbtv.native.request('Configuration.getLocalSystem').result;
    };

    /**
     * Get preferred languages to be used for audio playback on this system.
     *
     * @return {string} Comma separated string of languages (ISO 639-2 codes), in order of
     *    preference.
     *
     * @method
     * @memberof bridge.configuration#
     */
    exported.getPreferredAudioLanguage = function() {
        return hbbtv.native.request('Configuration.getPreferredAudioLanguage').result;
    };

    /**
     * Get preferred languages to be used for audio playback on this system.
     *
     * @return {string} Comma separated string of languages (IETF BCP47 codes), in order of
     *    preference.
     *
     * @method
     * @memberof bridge.configuration#
     */
    exported.getPreferredAudioLanguage47 = function() {
        return hbbtv.native.request('Configuration.getPreferredAudioLanguage47').result;
    };

    /**
     * Get preferred languages to be used for the user-interface on this system.
     *
     * @return {string} Comma separated string of languages (ISO 639-2 codes), in order of
     *    preference.
     *
     * @method
     * @memberof bridge.configuration#
     */
    exported.getPreferredSubtitleLanguage = function() {
        return hbbtv.native.request('Configuration.getPreferredSubtitleLanguage').result;
    };

    /**
     * Get preferred languages to be used for the user-interface on this system.
     *
     * @return {string} Comma separated string of languages (IETF BCP47 codes), in order of
     *    preference.
     *
     * @method
     * @memberof bridge.configuration#
     */
    exported.getPreferredSubtitleLanguage47 = function() {
        return hbbtv.native.request('Configuration.getPreferredSubtitleLanguage47').result;
    };

    /**
     * Get preferred languages to be used for the user-interface on this system.
     *
     *
     * @return {string} Comma separated string of languages (ISO 639-2 codes), in order of
     *    preference.
     *
     * @method
     * @memberof bridge.configuration#
     */
    exported.getPreferredUILanguage = function() {
        return hbbtv.native.request('Configuration.getPreferredUILanguage').result;
    };

    /**
     * Get a string containing the three character country code identifying the country this system
     * is deployed.
     *
     * @return {string} Country code the receiver is deployed (ISO 3166-1 alpha-3 code).
     *
     * @method
     * @memberof bridge.configuration#
     */
    exported.getCountryId = function() {
        return hbbtv.native.request('Configuration.getCountryId').result;
    };

    /**
     * Get whether subtitles are enabled on this system.
     *
     * @return {boolean} True if subtitles are enabled; or false otherwise.
     *
     * @method
     * @memberof bridge.configuration#
     */
    exported.getSubtitlesEnabled = function() {
        return hbbtv.native.request('Configuration.getSubtitlesEnabled').result;
    };

    /**
     * Get whether audio description is enabled on this system.
     *
     * @return {boolean} True if audio description is enabled; or false otherwise.
     *
     * @method
     * @memberof bridge.configuration#
     */
    exported.getAudioDescriptionEnabled = function() {
        return hbbtv.native.request('Configuration.getAudioDescriptionEnabled').result;
    };

    /**
     * Get whether clean audio is enabled on this system.
     *
     * @return {boolean} True if clean audio is enabled; or false otherwise.
     *
     * @method
     * @memberof bridge.configuration#
     */
    exported.getCleanAudioEnabled = function() {
        return hbbtv.native.request('Configuration.getCleanAudioEnabled').result;
    };

    /**
     * Get the DVB network IDs of the channels in the broadcast channel list.
     *
     * @return {Array.<String>} A list of DVB network IDs; or an empty list of not available.
     *
     * @method
     * @memberof bridge.configuration#
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
     *
     * @method
     * @memberof bridge.configuration#
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
     *
     * @method
     * @memberof bridge.configuration#
     */
    exported.requestAccessToDistinctiveIdentifier = function() {
        hbbtv.native.request('Configuration.requestAccessToDistinctiveIdentifier');
    };

    /**
     * Get the primary display info from the system.
     *
     * @return {DisplayInfo} A DisplayInfo object.
     *
     * @method
     * @memberof bridge.configuration#
     */
    exported.getPrimaryDisplay = function() {
        return hbbtv.native.request('Configuration.getPrimaryDisplay').result;
    };

    return exported;
})();

/**
 * ORB internal interface: system-agnostic bridge to native media sync.
 *
 * TODO All these methods need to be commented.
 *
 * @name bridge.mediaSync
 * @class
 * @constructor
 */
hbbtv.bridge.mediaSync = (function() {
    const exported = {};

    /**
     *
     * @return {number}
     *
     * @method
     * @memberof bridge.mediaSync#
     */
    exported.instantiate = function() {
        return hbbtv.native.request('MediaSynchroniser.instantiate').result;
    };

    /**
     * @param {number} id
     * @param {boolean} isMasterBroadcast
     *
     * @return {boolean}
     *
     * @method
     * @memberof bridge.mediaSync#
     */
    exported.initialise = function(id, isMasterBroadcast) {
        return hbbtv.native.request('MediaSynchroniser.initialise', {
            id: id,
            isMasterBroadcast: isMasterBroadcast,
        }).result;
    };

    /**
     * @param {number} id
     *
     * @method
     * @memberof bridge.mediaSync#
     */
    exported.destroy = function(id) {
        hbbtv.native.request('MediaSynchroniser.destroy', {
            id: id,
        });
    };

    /**
     * @param {number} id
     *
     * @method
     * @memberof bridge.mediaSync#
     */
    exported.enableInterDeviceSync = function(id) {
        hbbtv.native.request('MediaSynchroniser.enableInterDeviceSync', {
            id: id,
        });
    };

    /**
     * @param {number} id
     *
     * @method
     * @memberof bridge.mediaSync#
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
     *
     * @method
     * @memberof bridge.mediaSync#
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
     *
     * @method
     * @memberof bridge.mediaSync#
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
     *
     * @method
     * @memberof bridge.mediaSync#
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
     *
     * @method
     * @memberof bridge.mediaSync#
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
     *
     * @method
     * @memberof bridge.mediaSync#
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
     *
     * @method
     * @memberof bridge.mediaSync#
     */
    exported.getBroadcastCurrentTime = function(timelineSelector) {
        return hbbtv.native.request('MediaSynchroniser.getBroadcastCurrentTime', {
            timelineSelector: timelineSelector,
        }).result;
    };

    /**
     * @param {number} id
     * @param {string} contentIdOverride
     *
     * @method
     * @memberof bridge.mediaSync#
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
     *
     * @method
     * @memberof bridge.mediaSync#
     */
    exported.setContentTimeAndSpeed = function(id, timelineSelector, contentTime, speed) {
        const re = new RegExp(/:temi:|:pts$/);
        if (!re.test(timelineSelector)) {
            hbbtv.native.request('MediaSynchroniser.setContentTimeAndSpeed', {
                id: id,
                timelineSelector: timelineSelector,
                contentTime: contentTime,
                speed: speed,
            });
        }
    };

    /**
     * @param {number} id
     * @param {Object} properties
     *
     * @method
     * @memberof bridge.mediaSync#
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
     *
     * @method
     * @memberof bridge.mediaSync#
     */
    exported.setTimelineAvailability = function(id, timelineSelector, isAvailable, ticks, speed) {
        const re = new RegExp(/:temi:|:pts$/);
        if (!re.test(timelineSelector)) {
            return hbbtv.native.request('MediaSynchroniser.setTimelineAvailability', {
                id: id,
                timelineSelector: timelineSelector,
                isAvailable: isAvailable,
                ticks: ticks || 0,
                speed: speed || 0,
            }).result;
        }
        return false;
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

hbbtv.bridge.drm = (function() {
    const exported = {};

    exported.getSupportedDRMSystemIDs = function() {
        return hbbtv.native.request('Drm.getSupportedDRMSystemIDs').result;
    };

    exported.sendDRMMessage = function(msgID, msgType, msg, DRMSystemID, block) {
        return hbbtv.native.request('Drm.sendDRMMessage', {
            msgID: msgID,
            msgType: msgType,
            msg: msg,
            DRMSystemID: DRMSystemID,
            block: block ? true : false,
        });
    };

    exported.canPlayContent = function(DRMPrivateData, DRMSystemID) {
        return hbbtv.native.request('Drm.canPlayContent', {
            DRMPrivateData: DRMPrivateData,
            DRMSystemID: DRMSystemID,
        }).result;
    };

    exported.canRecordContent = function(DRMPrivateData, DRMSystemID) {
        return hbbtv.native.request('Drm.canRecordContent', {
            DRMPrivateData: DRMPrivateData,
            DRMSystemID: DRMSystemID,
        }).result;
    };

    exported.setActiveDRM = function(DRMSystemID) {
        return hbbtv.native.request('Drm.setActiveDRM', {
            DRMSystemID: DRMSystemID,
        }).result;
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
