/**
 * @fileOverview application/hbbtvMediaSwitcher object
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

hbbtv.objects.MediaSwitcher = (function() {
    const prototype = Object.create(HTMLObjectElement.prototype);
    const privates = new WeakMap();
    let mediaSwitchers = {};

    window.addEventListener('beforeunload', () => {
        for (let id in mediaSwitchers) {
            hbbtv.bridge.mediaSwitcher.destroy(id);
        }
    });

    /**
     * Check if an element is a video/broadcast object.
     */
    function isVideoBroadcastObject(element) {
        return element.getAttribute && element.getAttribute('__mimeType') === 'video/broadcast';
    }

    /**
     * Check if an element is an HTML5 video element.
     */
    function isHTML5VideoElement(element) {
        return element instanceof HTMLVideoElement;
    }

    /**
     * Get the play state of a video/broadcast object.
     */
    function getPlayState(videoBroadcastObject) {
        if (!isVideoBroadcastObject(videoBroadcastObject)) {
            return null;
        }
        // Assuming the playState property exists on video/broadcast objects
        return videoBroadcastObject.playState;
    }

    /**
     * Check if two elements have the same parent.
     */
    function haveSameParent(element1, element2) {
        return element1.parentElement === element2.parentElement;
    }

    /**
     * Check if element1 is immediately in front of element2 on the CSS z-axis.
     */
    function isInFrontOnZAxis(element1, element2) {
        // This is a simplified check - actual z-axis ordering would require
        // checking computed styles and DOM order
        const parent = element1.parentElement;
        if (!parent || parent !== element2.parentElement) {
            return false;
        }
        const children = Array.from(parent.children);
        const index1 = children.indexOf(element1);
        const index2 = children.indexOf(element2);
        return index1 === index2 - 1;
    }

    /**
     * Check if element1 is immediately behind element2 on the CSS z-axis.
     */
    function isBehindOnZAxis(element1, element2) {
        return isInFrontOnZAxis(element2, element1);
    }

    /**
     * Get computed CSS visibility.
     */
    function getComputedVisibility(element) {
        if (!element) return 'visible';
        const style = window.getComputedStyle(element);
        return style.visibility || 'visible';
    }

    /**
     * Validate preconditions for switchMediaPresentation.
     */
    function validatePreconditions(originalMediaObject, timelineSelector, timelineSource, switchTime, newMediaObject, minimumSwitchPerformanceRequired) {
        // Check originalMediaObject type
        if (!isVideoBroadcastObject(originalMediaObject) && !isHTML5VideoElement(originalMediaObject)) {
            return { valid: false, error: new DOMException('originalMediaObject must be a video/broadcast object or HTML5 video element', 'NotSupportedError') };
        }

        // Check originalMediaObject conditions
        if (isVideoBroadcastObject(originalMediaObject)) {
            const playState = getPlayState(originalMediaObject);
            const PLAY_STATE_PRESENTING = hbbtv.objects.BroadcastObserver.prototype.PLAY_STATE_PRESENTING;
            if (playState !== PLAY_STATE_PRESENTING) {
                return { valid: false, error: new DOMException('originalMediaObject must be in presenting state', 'NotSupportedError') };
            }
            if (!isHTML5VideoElement(newMediaObject)) {
                return { valid: false, error: new DOMException('newMediaObject must be an HTML5 video element when originalMediaObject is video/broadcast', 'NotSupportedError') };
            }
        } else if (isHTML5VideoElement(originalMediaObject)) {
            if (!isHTML5VideoElement(newMediaObject) && !isVideoBroadcastObject(newMediaObject)) {
                return { valid: false, error: new DOMException('newMediaObject must be an HTML5 video element or video/broadcast object', 'NotSupportedError') };
            }
            if (originalMediaObject.readyState < HTMLMediaElement.HAVE_FUTURE_DATA) {
                return { valid: false, error: new DOMException('originalMediaObject readyState must be HAVE_FUTURE_DATA or HAVE_ENOUGH_DATA', 'NotSupportedError') };
            }
        }

        // Check newMediaObject conditions
        if (isVideoBroadcastObject(newMediaObject)) {
            const visibility = getComputedVisibility(newMediaObject);
            if (visibility !== 'hidden') {
                return { valid: false, error: new DOMException('newMediaObject CSS visibility must be hidden', 'NotSupportedError') };
            }
            const playState = getPlayState(newMediaObject);
            const PLAY_STATE_PRESENTING = hbbtv.objects.BroadcastObserver.prototype.PLAY_STATE_PRESENTING;
            const PLAY_STATE_STOPPED = hbbtv.objects.BroadcastObserver.prototype.PLAY_STATE_STOPPED;
            if (playState !== PLAY_STATE_PRESENTING && playState !== PLAY_STATE_STOPPED) {
                return { valid: false, error: new DOMException('newMediaObject must be in presenting or stopped state', 'NotSupportedError') };
            }
        } else if (isHTML5VideoElement(newMediaObject)) {
            if (newMediaObject.readyState < HTMLMediaElement.HAVE_ENOUGH_DATA) {
                return { valid: false, error: new DOMException('newMediaObject readyState must be HAVE_ENOUGH_DATA', 'NotSupportedError') };
            }
            if (newMediaObject.seeking) {
                return { valid: false, error: new DOMException('newMediaObject must not be seeking', 'NotSupportedError') };
            }
        }

        // Check timelineSelector
        if (timelineSelector === null && !isHTML5VideoElement(originalMediaObject)) {
            return { valid: false, error: new DOMException('timelineSelector can only be null when originalMediaObject is an HTML5 video element', 'NotSupportedError') };
        }

        // Check parent relationship
        if (!haveSameParent(originalMediaObject, newMediaObject)) {
            return { valid: false, error: new DOMException('originalMediaObject and newMediaObject must have the same parent element', 'NotSupportedError') };
        }

        // Check z-axis ordering
        const visibility = getComputedVisibility(newMediaObject);
        if (!isInFrontOnZAxis(originalMediaObject, newMediaObject) &&
            !(isBehindOnZAxis(originalMediaObject, newMediaObject) && visibility === 'hidden')) {
            return { valid: false, error: new DOMException('Invalid z-axis ordering between originalMediaObject and newMediaObject', 'NotSupportedError') };
        }

        return { valid: true };
    }

    prototype.switchMediaPresentation = function(
        originalMediaObject,
        timelineSelector,
        timelineSource,
        switchTime,
        newMediaObject,
        minimumSwitchPerformanceRequired
    ) {
        const p = privates.get(this);
        
        // Step 1: Create a new promise
        let promiseResolve, promiseReject;
        const promise = new Promise((resolve, reject) => {
            promiseResolve = resolve;
            promiseReject = reject;
        });

        // Step 2: Validate preconditions
        const validation = validatePreconditions(
            originalMediaObject,
            timelineSelector,
            timelineSource,
            switchTime,
            newMediaObject,
            minimumSwitchPerformanceRequired
        );
        if (!validation.valid) {
            promiseReject(validation.error);
            return promise;
        }

        // Step 3: Check if a previous call is in progress
        if (p.switchInProgress) {
            promiseResolve('CallInProgress');
            return promise;
        }

        // Step 4: If timelineSelector is null, return promise (switch at end of media)
        if (timelineSelector === null) {
            p.switchInProgress = true;
            p.currentPromise = { resolve: promiseResolve, reject: promiseReject };
            // The actual switch will be handled when media ends
            return promise;
        }

        // Step 5: Start monitoring timeline if not already monitoring
        if (!p.monitoredTimelines || !p.monitoredTimelines.has(timelineSelector)) {
            const timelineObject = timelineSource ? originalMediaObject : newMediaObject;
            if (!hbbtv.bridge.mediaSwitcher.startTimelineMonitoring(p.id, timelineSelector, timelineSource)) {
                promiseReject(new DOMException('Failed to start timeline monitoring', 'NotSupportedError'));
                return promise;
            }
            if (!p.monitoredTimelines) {
                p.monitoredTimelines = new Set();
            }
            p.monitoredTimelines.add(timelineSelector);
        }

        // Step 6: Check if switchTime is in the past or more than 10 minutes in the future
        const currentTime = hbbtv.bridge.mediaSwitcher.getTimelineCurrentTime(timelineSelector);
        if (!isNaN(currentTime)) {
            if (switchTime < currentTime) {
                promiseResolve('InThePast');
                return promise;
            }
            const tenMinutes = 600; // 10 minutes in seconds
            if (switchTime > currentTime + tenMinutes) {
                promiseResolve('InThePast');
                return promise;
            }
        }

        // Step 7: Check switch preparation deadline (2 seconds advance notice required)
        const switchPreparationDeadline = switchTime - 2;
        if (!isNaN(currentTime) && currentTime > switchPreparationDeadline) {
            promiseResolve('SwitchPreparationDeadlinePassed');
            return promise;
        }

        // Step 8: Asynchronously invoke decoder allocation algorithm
        // This will be handled by the native implementation

        // Store switch parameters
        p.switchInProgress = true;
        p.currentSwitch = {
            originalMediaObject: originalMediaObject,
            timelineSelector: timelineSelector,
            timelineSource: timelineSource,
            switchTime: switchTime,
            newMediaObject: newMediaObject,
            minimumSwitchPerformanceRequired: minimumSwitchPerformanceRequired,
            promise: { resolve: promiseResolve, reject: promiseReject }
        };

        // Request the switch from native
        // The native side will fire events to notify us of the switch result
        // For now, we'll set up event listeners and make the request
        const switchParams = {
            originalMediaObject: originalMediaObject,
            timelineSelector: timelineSelector,
            timelineSource: timelineSource,
            switchTime: switchTime,
            newMediaObject: newMediaObject,
            minimumSwitchPerformanceRequired: minimumSwitchPerformanceRequired || ''
        };

        // Set up event handlers for switch completion
        const onSwitchComplete = (e) => {
            if (e.id === p.id) {
                hbbtv.bridge.removeWeakEventListener('MediaSwitchComplete', onSwitchComplete);
                hbbtv.bridge.removeWeakEventListener('MediaSwitchError', onSwitchError);
                p.switchInProgress = false;
                if (e.result === undefined || e.result === null) {
                    promiseResolve(undefined);
                } else {
                    promiseResolve(e.result);
                }
            }
        };

        const onSwitchError = (e) => {
            if (e.id === p.id) {
                hbbtv.bridge.removeWeakEventListener('MediaSwitchComplete', onSwitchComplete);
                hbbtv.bridge.removeWeakEventListener('MediaSwitchError', onSwitchError);
                p.switchInProgress = false;
                promiseReject(new DOMException(e.message || 'Media switch failed', e.name || 'NotSupportedError'));
            }
        };

        hbbtv.bridge.addWeakEventListener('MediaSwitchComplete', onSwitchComplete);
        hbbtv.bridge.addWeakEventListener('MediaSwitchError', onSwitchError);

        // Make the native request
        const accepted = hbbtv.bridge.mediaSwitcher.switchMediaPresentation(p.id, switchParams);
        if (!accepted) {
            hbbtv.bridge.removeWeakEventListener('MediaSwitchComplete', onSwitchComplete);
            hbbtv.bridge.removeWeakEventListener('MediaSwitchError', onSwitchError);
            p.switchInProgress = false;
            promiseReject(new DOMException('Switch request was rejected', 'NotSupportedError'));
        }

        // Step 9: Return promise
        return promise;
    };

    function initialise() {
        privates.set(this, {
            id: hbbtv.bridge.mediaSwitcher.instantiate(),
            switchInProgress: false,
            currentSwitch: null,
            monitoredTimelines: null,
        });
        mediaSwitchers[privates.get(this).id] = this;
    }

    return {
        prototype: prototype,
        initialise: initialise,
    };
})();

hbbtv.objects.upgradeToMediaSwitcher = function(object) {
    Object.setPrototypeOf(object, hbbtv.objects.MediaSwitcher.prototype);
    hbbtv.objects.MediaSwitcher.initialise.call(object);
};

hbbtv.objectManager.registerObject({
    name: 'application/hbbtvMediaSwitcher',
    mimeTypes: ['application/hbbtvmediaswitcher'],
    oipfObjectFactoryMethodName: 'createMediaSwitcher',
    upgradeObject: function(object) {
        hbbtv.objects.upgradeToMediaSwitcher(object);
    },
});
