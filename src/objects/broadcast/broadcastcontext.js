/**
 * @fileOverview BroadcastContext interface for BroadcastHelper
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
 * BroadcastContext defines the data interface that BroadcastHelper
 * requires from its host object.
 * 
 * This is a pure interface definition - the actual context object
 * should be created and populated by the host objects (VideoBroadcast,
 * BroadcastSupervisor) directly.
 */
hbbtv.objects.BroadcastContext = (function() {

    /**
     * Creates an empty BroadcastContext with the required interface
     * @returns {Object} - A context object with the required properties
     */
    function instantiate() {
        return {
            // State properties
            playState: 0,
            isBroadcastRelated: false,
            isTransitioningToBroadcastRelated: false,
            quiet: 0,
            waitingPlayStateConnectingConfirm: false,
            
            // VideoBroadcast-specific properties
            fullScreen: false,
            x: 0,
            y: 0,
            width: 1280,
            height: 720,
            display_none: false,
            
            // BroadcastSupervisor-specific properties
            playStateError: null,
            playbackOffset: 0,
            maxOffset: 0,
            recordingState: 0,
            playPosition: 0,
            playSpeed: 1,
            playSpeeds: [1],
            timeShiftMode: 0,
            currentTimeShiftMode: 0,
            
            // Channel properties
            channelConfig: null,
            currentChannel: null,
            currentChannelData: null,
            currentInstanceIndex: null,
            currentNonQuietChannelData: null,
            
            // Event dispatching
            eventDispatcher: null,
            
            // Component properties
            components: null,
            currentActiveComponents: null,
            currentChannelComponents: null,
            
            // Program properties
            programmes: null,
            currentChannelProgrammes: null,
            
            // Method references (to be set by host objects)
            acquireActiveState: null,
            releaseActiveState: null,
            addBridgeEventListeners: null,
            removeBridgeEventListeners: null,
            release: null,
            unregisterAllStreamEventListeners: null
        };
    }

    return {
        instantiate: instantiate
    };
})(); 