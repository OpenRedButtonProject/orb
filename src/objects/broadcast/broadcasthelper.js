/**
 * @fileOverview BroadcastHelper utility class
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
hbbtv.objects.BroadcastHelper = (function() {

    const prototype = Object.create({});
    const privates = new WeakMap();

        // Shared constants - organized in a Constants object
    const Constants = Object.freeze({
        // Play states
        PLAY_STATE_UNREALIZED: 0,
        PLAY_STATE_CONNECTING: 1,
        PLAY_STATE_PRESENTING: 2,
        PLAY_STATE_STOPPED: 3,

        // Channel status
        CHANNEL_STATUS_UNREALIZED: -4,
        CHANNEL_STATUS_PRESENTING: -3,
        CHANNEL_STATUS_CONNECTING: -2,
        CHANNEL_STATUS_CONNECTING_RECOVERY: -1,
        CHANNEL_STATUS_WRONG_TUNER: 0,
        CHANNEL_STATUS_NO_SIGNAL: 1,
        CHANNEL_STATUS_TUNER_IN_USE: 2,
        CHANNEL_STATUS_PARENTAL_LOCKED: 3,
        CHANNEL_STATUS_ENCRYPTED: 4,
        CHANNEL_STATUS_UNKNOWN_CHANNEL: 5,
        CHANNEL_STATUS_INTERRUPTED: 6,
        CHANNEL_STATUS_RECORDING_IN_PROGRESS: 7,
        CHANNEL_STATUS_CANNOT_RESOLVE_URI: 8,
        CHANNEL_STATUS_INSUFFICIENT_BANDWIDTH: 9,
        CHANNEL_STATUS_CANNOT_BE_CHANGED: 10,
        CHANNEL_STATUS_INSUFFICIENT_RESOURCES: 11,
        CHANNEL_STATUS_CHANNEL_NOT_IN_TS: 12,
        CHANNEL_STATUS_UNKNOWN_ERROR: 100,

        // Component types
        COMPONENT_TYPE_ANY: -1,
        COMPONENT_TYPE_VIDEO: 0,
        COMPONENT_TYPE_AUDIO: 1,
        COMPONENT_TYPE_SUBTITLE: 2,

        // Error codes
        ERROR_TUNER_UNAVAILABLE: 2,
        ERROR_UNKNOWN_CHANNEL: 5,

        LINKED_APP_SCHEME_1_1: "urn:dvb:metadata:cs:LinkedApplicationCS:2019:1.1"
    });

    // Methods

    /**
     * Get channel configuration
     * @returns {ChannelConfig}
     */
    prototype.getChannelConfig = function() {
        const context = privates.get(this).context;
        mandatoryBroadcastRelatedSecurityCheck(context);
        return context.channelConfig;
    };

    /**
     * Create channel object
     * @param {number} idType
     * @param {string|number} dsdOrOnid
     * @param {number} sidOrTsid
     * @param {number} sid
     * @param {number} sourceID
     * @param {string} ipBroadcastID
     * @returns {Channel}
     */
    prototype.createChannelObject = function(idType, dsdOrOnid, sidOrTsid, sid, sourceID, ipBroadcastID) {
        if (idType == 13) {
            // createChannelObject(idType, dsd, tsid)
            return hbbtv.objects.createChannel({
                idType: idType,
                dsd: dsdOrOnid,
                sid: sidOrTsid,
            });
        } else {
            // createChannelObject(idType, onid, tsid, sid, sourceID, ipBroadcastID)
            let channel = {
                idType: idType,
                onid: dsdOrOnid,
                tsid: sidOrTsid,
                sid: sid,
                sourceID: sourceID,
                ipBroadcastID: ipBroadcastID,
            };
            const context = privates.get(this).context;
            if (context.isBroadcastRelated) {
                let foundChannel = context.channelConfig ?
                    context.channelConfig.channelList.findChannel(channel) :
                    null;
                if (foundChannel !== null) {
                    return foundChannel;
                }
            }
            return hbbtv.objects.createChannel(channel);
        }
    };

    /**
     * Set channel
     * @param {Channel} channel
     * @param {boolean} trickplay
     * @param {string} contentAccessDescriptorURL
     * @param {number} quiet
     */
    prototype.setChannel = function(channel, trickplay = false, contentAccessDescriptorURL = '', quiet = 0) {
        const context = privates.get(this).context;
        const appScheme = hbbtv.bridge.manager.getApplicationScheme();
        let releaseOnError = false;
        
        if (channel === null) {
            // Clean up context when setting channel to null
            context.currentChannelData = null;
            context.currentInstanceIndex = null;
            context.currentNonQuietChannelData = null;
            context.currentChannelComponents = null;
            context.playState = Constants.PLAY_STATE_UNREALIZED;
            this.setIsBroadcastRelated(false);
            const errorState = hbbtv.bridge.broadcast.setChannelToNull(
                trickplay,
                contentAccessDescriptorURL,
                quiet
            );
            if (errorState < 0) {
                dispatchChannelChangeSucceeded(context, null);
            }
            return;
        }

        // Acquire active state if required
        if (context.playState === Constants.PLAY_STATE_UNREALIZED || context.playState === Constants.PLAY_STATE_STOPPED) {
            if (!context.acquireActiveState()) {
                hbbtv.bridge.broadcast.setPresentationSuspended(false);
                dispatchChannelChangeError(context, channel, Constants.ERROR_TUNER_UNAVAILABLE);
                return;
            }
            context.addBridgeEventListeners();
            releaseOnError = true;
        }
        
        // Change channel
        context.isTransitioningToBroadcastRelated = true;
        context.quiet = quiet;
        let errorState = 0;
        
        if (channel.idType == 13) {
            // ID_DVB_SI_DIRECT
            if (channel.dsd !== undefined && channel.sid !== undefined) {
                errorState = hbbtv.bridge.broadcast.setChannelToDsd(
                    hbbtv.utils.base64Encode(channel.dsd),
                    channel.sid,
                    trickplay,
                    contentAccessDescriptorURL,
                    quiet
                );
            }
        } else {
            if (channel.ccid !== undefined) {
                errorState = hbbtv.bridge.broadcast.setChannelToCcid(
                    channel.ccid,
                    trickplay,
                    contentAccessDescriptorURL,
                    quiet
                );
            } else {
                if (
                    channel.onid !== undefined &&
                    channel.tsid !== undefined &&
                    channel.sid !== undefined
                ) {
                    errorState = hbbtv.bridge.broadcast.setChannelToTriplet(
                        channel.idType,
                        channel.onid,
                        channel.tsid,
                        channel.sid,
                        typeof channel.sourceID !== 'undefined' ? channel.sourceID : -1,
                        typeof channel.ipBroadcastID !== 'undefined' ? channel.ipBroadcastID : '',
                        trickplay,
                        contentAccessDescriptorURL,
                        quiet
                    );
                }
            }
        }

        if (errorState >= 0) {
            context.isTransitioningToBroadcastRelated = false;
            if (releaseOnError) {
                context.removeBridgeEventListeners();
                hbbtv.holePuncher.setBroadcastVideoObject(null);
                context.releaseActiveState();
            }
            if (errorState === Constants.CHANNEL_STATUS_CHANNEL_NOT_IN_TS) {
                context.playState = Constants.PLAY_STATE_UNREALIZED;
                dispatchPlayStateChange(context, context.playState, Constants.CHANNEL_STATUS_CHANNEL_NOT_IN_TS);
            }
            dispatchChannelChangeError(context, channel, errorState);
            return;
        }

        if (context.isBroadcastRelated && quiet !== 2) {
            try {
                const channelData = hbbtv.bridge.broadcast.getCurrentChannel();
                context.currentChannelData = hbbtv.objects.createChannel(channelData);
                context.currentInstanceIndex = channelData.currentInstanceIndex;
                if (context.channelConfig === null) {
                    context.channelConfig = hbbtv.objects.createChannelConfig();
                }
            } catch (e) {
                if (e.name === 'SecurityError') {
                    console.log(
                        'setChannel, unexpected condition: app appears broadcast-independent.'
                    );
                }
                throw e;
            }
        }

        context.unregisterAllStreamEventListeners();
        context.playState = Constants.PLAY_STATE_CONNECTING;
        context.waitingPlayStateConnectingConfirm = appScheme === Constants.LINKED_APP_SCHEME_1_1;
        dispatchPlayStateChange(context, context.playState, Constants.PLAY_STATE_CONNECTING);
    };

    /**
     * Previous channel
     */
    prototype.prevChannel = function() {
        const context = privates.get(this).context;
        mandatoryBroadcastRelatedSecurityCheck(context);
        cycleChannel(context, -1).call(this);
    };

    /**
     * Next channel
     */
    prototype.nextChannel = function() {
        const context = privates.get(this).context;
        mandatoryBroadcastRelatedSecurityCheck(context);
        cycleChannel(context, 1).call(this);
    };

    /**
     * Get components
     * @param {number} componentType
     * @returns {AVComponentCollection}
     */
    prototype.getComponents = function(componentType) {
        const context = privates.get(this).context;
        mandatoryBroadcastRelatedSecurityCheck(context);
        if (!context.currentChannelComponents) {
            context.currentChannelComponents = hbbtv.bridge.broadcast.getComponents(
                context.currentChannelData.ccid,
                -1
            );
        }
        let result;
        if (componentType === null || componentType === undefined) {
            result = context.currentChannelComponents.filter((component) => {
                return !component.hidden;
            });
        } else {
            result = context.currentChannelComponents.filter((component) => {
                return component.type === componentType && !component.hidden;
            });
        }
        return avComponentArrayToCollection(result);
    };

    /**
     * Get current active components
     * @param {number} componentType
     * @returns {AVComponentCollection}
     */
    prototype.getCurrentActiveComponents = function(componentType) {
        const context = privates.get(this).context;
        mandatoryBroadcastRelatedSecurityCheck(context);
        if (!context.currentChannelComponents) {
            context.currentChannelComponents = hbbtv.bridge.broadcast.getComponents(
                context.currentChannelData.ccid,
                -1
            );
        }
        if (componentType === null || componentType === undefined) {
            let result = context.currentChannelComponents.filter((component) => {
                return component.active;
            });
            return avComponentArrayToCollection(result);
        } else {
            let result = context.currentChannelComponents.filter((component) => {
                return component.type === componentType && component.active;
            });
            return avComponentArrayToCollection(result);
        }
    };

    /**
     * Select component
     * @param {AVComponent|number} component
     */
    prototype.selectComponent = function(component) {
        const context = privates.get(this).context;
        mandatoryBroadcastRelatedSecurityCheck(context);
        if (!context.currentChannelComponents) {
            context.currentChannelComponents = hbbtv.bridge.broadcast.getComponents(
                context.currentChannelData.ccid,
                -1
            );
        }
        if (!context.currentChannelComponents || context.currentChannelComponents.length === 0) {
            throw new DOMException('', 'InvalidStateError');
        }
        if (isNaN(component)) {
            const componentId = getComponentId(component);
            if (componentId !== null) {
                hbbtv.bridge.broadcast.overrideComponentSelection(component.type, componentId);
            }
        } else {
            // Select the default component
            hbbtv.bridge.broadcast.restoreComponentSelection(component);
        }
    };

    /**
     * Unselect component
     * @param {AVComponent|number} component
     */
    prototype.unselectComponent = function(component) {
        const context = privates.get(this).context;
        mandatoryBroadcastRelatedSecurityCheck(context);
        if (isNaN(component)) {
            context.currentChannelComponents = hbbtv.bridge.broadcast.getComponents(
                context.currentChannelData.ccid,
                -1
            );
            const componentId = getComponentId(component);
            const found = context.currentChannelComponents.find((item) => {
                return componentId !== null && componentId === item.id;
            });
            if (found.active) {
                if (
                    component.type == Constants.COMPONENT_TYPE_AUDIO ||
                    component.type == Constants.COMPONENT_TYPE_SUBTITLE
                ) {
                    // Select the default component
                    hbbtv.bridge.broadcast.restoreComponentSelection(component.type);
                } else {
                    // Suspend video
                    hbbtv.bridge.broadcast.overrideComponentSelection(component.type, '');
                }
            }
        } else {
            // Suspend this component type
            hbbtv.bridge.broadcast.overrideComponentSelection(component, '');
        }
    };

    /**
     * Set broadcast related state
     * @param {boolean} value - Whether the object is broadcast-related
     */
    prototype.setIsBroadcastRelated = function(value) {
        const context = privates.get(this).context;
        context.isBroadcastRelated = value;
        if (context.channelConfig !== null) {
            hbbtv.objects.ChannelConfig.setIsBroadcastRelated.call(context.channelConfig, value);
        }
    };

    prototype.setIntrinsicCallback = function(eventName, callback, parameterOrder = []) {
        const p = privates.get(this);
        const callbacks = p.intrinsicCallbacks;
        const wrappers = p.intrinsicWrappers;

        console.log(`Setting intrinsic callback for event '${eventName}' with parameter order ${parameterOrder}.`);

        if (callbacks[eventName]) {
            p.context.eventDispatcher.removeEventListener(eventName, wrappers[eventName]);
            wrappers[eventName] = null;
        }
        callbacks[eventName] = callback;
        if (callback) {
            wrappers[eventName] = (ev) => {
                const args = parameterOrder.map(prop => ev[prop]);
                callback(...args);
            };
            p.context.eventDispatcher.addEventListener(eventName, wrappers[eventName]);
        }
    };

    prototype.getIntrinsicCallback = function(eventName) {
        return privates.get(this).intrinsicCallbacks[eventName];
    }

    // Internal utility functions

    function mandatoryBroadcastRelatedSecurityCheck(context) {
        if (!context.isBroadcastRelated) {
            throw new DOMException('', 'SecurityError');
        }
    }

    function cycleChannel(context, delta) {
        if (!context.isBroadcastRelated) {
            throw new DOMException('', 'SecurityError');
        }
        if (context.playState === Constants.PLAY_STATE_UNREALIZED || context.channelConfig.channelList.length < 2) {
            dispatchChannelChangeError(context, context.currentNonQuietChannelData, Constants.CHANNEL_STATUS_CANNOT_BE_CHANGED);
            return;
        }
        let i;
        for (i = 0; i < context.channelConfig.channelList.length; i++) {
            if (context.channelConfig.channelList.item(i).ccid === context.currentNonQuietChannelData.ccid) {
                let n = context.channelConfig.channelList.length;
                this.setChannel(context.channelConfig.channelList.item((i + delta + n) % n));
                return;
            }
        }
        if (context.playState === Constants.PLAY_STATE_CONNECTING) {
            context.unregisterAllStreamEventListeners();
            context.playState = Constants.PLAY_STATE_UNREALIZED;
            dispatchChannelChangeError(context, context.currentNonQuietChannelData, Constants.CHANNEL_STATUS_CANNOT_BE_CHANGED);
            dispatchPlayStateChange(context, context.playState, Constants.CHANNEL_STATUS_CANNOT_BE_CHANGED);
        } else {
            dispatchChannelChangeError(context, context.currentNonQuietChannelData, Constants.CHANNEL_STATUS_CANNOT_BE_CHANGED);
        }
    }

    function avComponentArrayToCollection(avArray) {
        let result = [];
        avArray.forEach(function(item, index) {
            switch (item.type) {
                case Constants.COMPONENT_TYPE_VIDEO:
                    result[index] = hbbtv.objects.createAVVideoComponent(item);
                    break;
                case Constants.COMPONENT_TYPE_AUDIO:
                    result[index] = hbbtv.objects.createAVAudioComponent(item);
                    break;
                case Constants.COMPONENT_TYPE_SUBTITLE:
                    result[index] = hbbtv.objects.createAVSubtitleComponent(item);
                    break;
                default:
                    result[index] = hbbtv.objects.createAVComponent(item);
            }
        });
        return hbbtv.objects.createCollection(result);
    }

    function getComponentId(component) {
        let id = null;
        if (
            component.type == Constants.COMPONENT_TYPE_VIDEO &&
            hbbtv.objects.AVVideoComponent.prototype.isPrototypeOf(component)
        ) {
            id = hbbtv.objects.AVVideoComponent.getId.call(component);
        } else if (
            component.type == Constants.COMPONENT_TYPE_AUDIO &&
            hbbtv.objects.AVAudioComponent.prototype.isPrototypeOf(component)
        ) {
            id = hbbtv.objects.AVAudioComponent.getId.call(component);
        } else if (
            component.type == Constants.COMPONENT_TYPE_SUBTITLE &&
            hbbtv.objects.AVSubtitleComponent.prototype.isPrototypeOf(component)
        ) {
            id = hbbtv.objects.AVSubtitleComponent.getId.call(component);
        }
        if (id === undefined) {
            id = null;
        }
        return id;
    }

    // helper function for dispatching events
    function dispatchEvent(context, eventName, contextInfo) {
        console.log("Dispatched '" + eventName + "' event.");
        const event = new Event(eventName);
        if (contextInfo) {
            Object.assign(event, contextInfo);
        }
        context.eventDispatcher.dispatchEvent(event);
    }

    // Specific event dispatching methods to prevent typos
    function dispatchChannelChangeSucceeded(context, channel) {
        dispatchEvent(context, 'ChannelChangeSucceeded', { channel: channel });
    }

    function dispatchChannelChangeError(context, channel, errorState) {
        dispatchEvent(context, 'ChannelChangeError', { 
            channel: channel, 
            errorState: errorState 
        });
    }

    function dispatchPlayStateChange(context, playState, error) {
        dispatchEvent(context, 'PlayStateChange', { 
            playState: playState, 
            error: error 
        });
    }

    function instantiate(context) {
        const obj = Object.create(prototype);
        privates.set(obj, {
            context: context,
            intrinsicCallbacks: { },
            intrinsicWrappers: { }
        });
        return obj;
    }

    return {
        instantiate: instantiate,
        Constants: Constants,
    };
})();