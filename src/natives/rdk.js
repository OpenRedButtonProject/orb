// RDK/Linux native
hbbtv.native = {
    name: 'rdk',
    media: undefined,
    mediaProxy: undefined,
    dashProxy: undefined,
    orb_timeShiftBufferDepthReceived: undefined,
    initialise: function() {
        this.token = Object.assign({}, document.token);
    },
    // setters
    setMedia: function(media) {
        console.log('[RDK-Native::setMedia]');
        console.log(media);
        this.media = media;
    },
    setMediaProxy: function(mediaProxy) {
        console.log('[RDK-Native::setMediaProxy]');
        console.log(mediaProxy);
        this.mediaProxy = mediaProxy;
    },
    setDashProxy: function(dashProxy) {
        console.log('[RDK-Native::setDashProxy]');
        console.log(dashProxy);
        this.dashProxy = dashProxy;
    },
    request: function(method, params) {
        const body = {
            token: this.token,
            method: method,
            params: params || {},
        };
        const responseText = wpeBridge.request(JSON.stringify(body));
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
    setDispatchEventCallback: function(callback) {
        // TEMPORARY Dispatch an event and properties. TODO Replace me in events part 2!
        document.dispatchBridgeEvent = (type, properties) => {
            callback(type, properties);
        };
    },
    isDebugBuild: function() {
        return true; // TODO Move up. Note: This needs to be enabled to disable dash-scheme
    },
    isFeatureEnabled(name) {
        if (name === 'dash-scheme') {
            return false;
        }
        return false;
    },
    // optional methdod to add native specific event listeners on mediamanager
    addMediaNativeListeners: function() {
        console.log('[RDK-Native::addMediaNativeListeners] Adding media native listeners');
        // in case of dynamic mpd, update the seekable property based on timeupdate
        // dashjs will update the seekable property based on the dvr property continuously.
        // we will need to filter the diff and see if it exceeds MPD@timeShiftBufferDepth
        this.media.addEventListener('__orb_timeShiftBufferDepthReceived__', (e) => {
            console.log('[RDK-Native::__orb_timeShiftBufferDepthReceived__]');
            this.orb_timeShiftBufferDepthReceived = e;
        });
        console.log('[RDK-Native::addMediaNativeListeners] Added __orb_timeShiftBufferDepthReceived__ listener');
    },

    // optional method to dispatch native specific events from dashproxy
    dispatchManifestNativeEvents: function(e) {
        // dispatch __orb_timeShiftBufferDepthReceived__ event for seekable property in case of rdk native
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
        console.log('[RDK-Native] Dipsatching __orb_timeShiftBufferDepthReceived__');
        this.dashProxy.dispatchEvent(timeShiftEvt);
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

        mediaProxy.callObserverMethod(MEDIA_PROXY_ID, 'setSeekable', [ranges]);
        mediaProxy.dispatchEvent(MEDIA_PROXY_ID, data);
    }
};