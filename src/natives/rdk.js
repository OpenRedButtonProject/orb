// RDK/Linux native
hbbtv.native = {
    name: 'rdk',
    media: undefined,
    mediaProxy: undefined,
    dashProxy: undefined,
    proprietary: false,
    orb_timeShiftBufferDepthReceived: undefined,
    currentPeriod: undefined,
    currentRepresentation: undefined,
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
    setProprietary: function(proprietary) {
        console.log('[RDK-Native::setProprietary]');
        console.log(proprietary);
        this.proprietary = proprietary;
    },
    getProprietary: function() {
        console.log('[RDK-Native::getProprietary]');
        console.log(this.proprietary);
        return this.proprietary;
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
    // private methods used for internal logic
    /**
     * This method will return:
     *     -1     if the key argument was removed in the latest mpd period/representation received
     *      0     if the key argument remains
     *      1     if the key argument was added in the latest mpd period/representation received 
     */
    metadataDelta: function({periodData, representationData} = {}, key) {
        console.log('[RDK-Native::metadataDelta]');
        let result = 0;

        // latest mpd period or representation
        let currentObj;
        if (periodData !== undefined) {
            currentObj = this.currentPeriod;
        }
        else {
            currentObj = this.currentRepresentation;
        }

        // new mpd period or representation
        let searchObj = 
            periodData !== undefined ? periodData : representationData;

        // check for delta
        if (currentObj !== undefined 
            && currentObj.hasOwnProperty(key) 
            && !searchObj.hasOwnProperty(key))
        {
            console.log(`RDK-Native::metadataDelta] ${key} Removed`);
            result = -1;
        }
        else if (currentObj !== undefined 
            && !currentObj.hasOwnProperty(key) 
            && searchObj.hasOwnProperty(key))
        {
            console.log(`RDK-Native::metadataDelta] ${key} Removed`);
            result = 1;
        }
        
        // finally, store the period or representation as the latest
        if (periodData !== undefined) {
            this.currentPeriod = periodData;
        } if (representationData !== undefined) {
            this.currentRepresentation = representationData;
        }
     
        return result;
    },
    // optional method to add native specific event listeners on mediamanager
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

        this.media.addEventListener('__orb_eventStreamReceived__', (e) => {
            console.log('[RDK-Native::__orb_eventStreamReceived__]');
            this.media.removeTrackEvent();
        });
        console.log('[RDK-Native::addMediaNativeListeners] Added __orb_eventStreamReceived__ listener');

        this.media.addEventListener('__orb_inbandEventStreamReceived__', (e) => {
            console.log('[RDK-Native::__orb_inbandEventStreamReceived__]');
            console.log(e);
            if (e.delta === -1) {
                console.log('[RDK-Native::addMediaNativeListeners] Dispatching RemoveTrack for Representation');
                this.media.removeTrackEvent();
            } else if (e.delta === 1) {
                console.log('[RDK-Native::addMediaNativeListeners] Dispatching AddTrack for Representation');
                this.media.addTrackEvent();
            }
        });
        console.log('[RDK-Native::addMediaNativeListeners] Added __orb_inbandEventStreamReceived__ listener');
    },

    // optional methods to dispatch native specific events
    /**
     * This method is called each time the manifest is received
     */
    dispatchManifestNativeEvents: function(e) {
        // dispatch __orb_timeShiftBufferDepthReceived__ event for seekable property in case of rdk native
  
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
            console.log('[RDK-Native] Dipsatching __orb_timeShiftBufferDepthReceived__');
            this.dashProxy.dispatchEvent(timeShiftEvt);
        }

        // check for event stream state
        if (this.metadataDelta({periodData: e.data.Period_asArray[0]}, 'EventStream')  === -1) {
            const eventStreamReceivedEvt = new Event('__orb_eventStreamReceived__');
            console.log('[RDK-Native] Dipsatching __orb_eventStreamReceived__');
            this.dashProxy.dispatchEvent(eventStreamReceivedEvt);
        }
    },

    /**
     * This method is called each time the representation is changed
     */
    dispatchRepresentationNativeEvents: function(representation) {
        const inbandEventStreamEvt = new Event('__orb_inbandEventStreamReceived__');
        Object.assign(inbandEventStreamEvt, {
            delta: this.metadataDelta({representationData: representation}, 'InbandEventStream') 
        }); 
        console.log('[RDK-Native] Dipsatching __orb_inbandEventStreamReceived__');
        this.dashProxy.dispatchEvent(inbandEventStreamEvt);
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
        if (this.media.currentTime < ranges[0].start) {
            const e = new Event('error');
            e.error = {}; 
            e.error.code = 2;
            e.error.message = '';
            this.dashProxy.dispatchEvent(e);
        }
        // console.log(`[RDK-NATIVE::updateSeekable] ${ranges[0].start} ${ranges[0].end}`);
        mediaProxy.callObserverMethod(MEDIA_PROXY_ID, 'setSeekable', [ranges]);
        mediaProxy.dispatchEvent(MEDIA_PROXY_ID, data);
    }
};