/**
 * @fileOverview Android native
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

hbbtv.native = {
    name: 'android',
    media: undefined,
    mediaProxy: undefined,
    dashProxy: undefined,
    orbModule: undefined,
    pausedDelta: false,

    initialise: function() {
        this.token = Object.assign({}, document.token);
    },
    // setters
    setDispatchEventCallback: function(callback) {
		this.orbModule = new OrbModule(callback);
    },
    setMedia: function(media) {
        this.media = media;
    },
    setMediaProxy: function(mediaProxy) {
        this.mediaProxy = mediaProxy;
    },
    setDashProxy: function(dashProxy) {
        this.dashProxy = dashProxy;
    },
    setPausedDelta: function(pausedDelta) {
        console.log(`[Android-Native::getProprietary] PausedDelta = ${this.pausedDelta}`);
        this.pausedDelta = pausedDelta;
    },
    request: function(method, params) {
        const body = {
            token: this.token,
            method: method,
            params: params || {},
        };
        console.debug('SendingRequest: ' + method);
        const responseText = this.orbModule.request(JSON.stringify(body));
        if (typeof responseText !== 'string') {
            console.debug('Invalid response');
            return false;
        }
        const response = JSON.parse(responseText);
        if (response.error !== undefined) {
            if (response.error === 'SecurityError') {
                throw new DOMException('', 'SecurityError');
            }
            console.debug('Error response from ' + method + ': ' + response.error);
            return false;
        }
        //console.log("Response from " + method + ": " + JSON.stringify(response));
        return response;
    },
    isDebugBuild: function() {
        return true; // TODO Move
    },
    isFeatureEnabled(name) {
        if (name === 'dash-scheme') {
            return true;
        }
        return false;
    },

    // optional method to add native specific event listeners on mediamanager
    addMediaNativeListeners: function() {
        console.log('[Android-Native::addMediaNativeListeners] Adding media native listeners');
        // in case of dynamic mpd, update the seekable property based on timeupdate
        // dashjs will update the seekable property based on the dvr property continuously.
        // we will need to filter the diff and see if it exceeds MPD@timeShiftBufferDepth
        this.media.addEventListener('__orb_timeShiftBufferDepthReceived__', (e) => {
            console.log('[Android-Native::__orb_timeShiftBufferDepthReceived__]');
            this.orb_timeShiftBufferDepthReceived = e;
        });
        console.log('[Android-Native::addMediaNativeListeners] Added __orb_timeShiftBufferDepthReceived__ listener');
    },

    // optional methods to dispatch native specific events
    /**
     * This method is called each time the manifest is received
     */
    dispatchManifestNativeEvents: function(e) {
        // dispatch __orb_timeShiftBufferDepthReceived__ event for seekable property in case of android native

        if (e.data.type === 'dynamic') {
            const timeShiftEvt = new Event('__orb_timeShiftBufferDepthReceived__');
            if (e.data.hasOwnProperty('timeShiftBufferDepth')) {
                Object.assign(timeShiftEvt, {
                    timeShiftBufferDepth: e.data.timeShiftBufferDepth,
                });
            } else {
                Object.assign(timeShiftEvt, {
                    firstPeriodStart: e.data.Period_asArray[0].start,
                });
            }
            console.log('[Android-Native] Dipsatching __orb_timeShiftBufferDepthReceived__');
            this.dashProxy.dispatchEvent(timeShiftEvt);
        }
    },

    // Polling events
    getSeekablePollingEvent: function() {
        return 'timeupdate';
    },
    getBufferedPollingEvent: function() {
        return 'timeupdate';
    },

    // Event handlers
    updateSeekable: function(e) {
        const ranges = [];
        const media = this.media;
        const mediaProxy = this.mediaProxy;
        const MEDIA_PROXY_ID = 'HTMLMediaElement';
        const data = this.orb_timeShiftBufferDepthReceived;

        for (let i = 0; i < media.seekable.length; ++i) {
            const seekableEnd = media.seekable.end(i);

            /**
             * call with MPD@timeShiftBufferDepth set. Will need to adjust
             * the seekable ranges so that the diff is not exceeding timeShiftBufferDepth.
             * Progress seekableStart accordingly
             */
            if (data !== undefined && data.hasOwnProperty('timeShiftBufferDepth')) {
                let seekableStart = media.seekable.start(i);
                if (seekableEnd - media.seekable.start(i) > data.timeShiftBufferDepth) {
                    seekableStart = seekableEnd - data.timeShiftBufferDepth;
                }
                ranges.push({
                    start: seekableStart,
                    end: seekableEnd,
                });
            }
            /**
             * The seekable parameter reflected the removal of a period when MPD@timeShiftBufferDepth is not present
             * MPD@timeShiftBufferDepth was not present and MPD@type was dynamic.
             * The seekable start should be equal to the removed period duration.
             */
            else if (data !== undefined && data.hasOwnProperty('firstPeriodStart')){
                ranges.push({
                    start: data.firstPeriodStart,
                    end: media.currentTime,
                });
            }
            else {
                ranges.push({
                    start: media.seekable.start(i),
                    end: media.seekable.end(i),
                });
            }
        }

        /**
         * Cover cases where the MPD is dynamic and the media element is paused until the start is greater than currentTime.
         * In this case, an error event should be generated of type MEDIA_ERR_NETWORK (error.code = 2). DashProxy will dispatch
         * this error event.
         */
        if (this.pausedDelta && this.media.currentTime < ranges[0].start) {
            const e = new Event('error');
            e.error = {};
            e.error.code = 2;
            e.error.message = '';
            this.dashProxy.dispatchEvent(e);
            this.pausedDelta = false;
        }

        // make sure to dispatch the event with defined data
        mediaProxy.callObserverMethod(MEDIA_PROXY_ID, 'setSeekable', [ranges]);
        mediaProxy.dispatchEvent(MEDIA_PROXY_ID, data !== undefined ? data : e);
    }
};
