/**
 * @fileOverview The proxy for DASH playback.
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

hbbtv.objects.DashProxy = (function() {
    const prototype = Object.create(HTMLMediaElement.prototype);
    const privates = new WeakMap();
    const PARENTAL_CONTROL_EVENT_SCHEMES = ['urn:dvb:iptv:cpm:2014'];
    const PARENTAL_CONTROL_EVENT_SCHEME_LOOKUP = {
        'urn:dvb:iptv:cpm:2014': 'dvb-si',
    };

    /**
     * Error code returned when a manifest parsing error occurs
     */
    const MANIFEST_LOADER_PARSING_FAILURE_ERROR_CODE = 10;

    /**
     * Error code returned when a manifest loading error occurs
     */
    const MANIFEST_LOADER_LOADING_FAILURE_ERROR_CODE = 11;

    /**
     * Error code returned when a xlink loading error occurs
     */
    const XLINK_LOADER_LOADING_FAILURE_ERROR_CODE = 12;

    /**
     * Error code returned when no segment ranges could be determined from the sidx box
     */
    const SEGMENT_BASE_LOADER_ERROR_CODE = 15;

    /**
     * Error code returned when the time synchronization failed
     */
    const TIME_SYNC_FAILED_ERROR_CODE = 16;

    /**
     * Error code returned when loading a fragment failed
     */
    const FRAGMENT_LOADER_LOADING_FAILURE_ERROR_CODE = 17;

    /**
     * Error code returned when the FragmentLoader did not receive a request object
     */
    const FRAGMENT_LOADER_NULL_REQUEST_ERROR_CODE = 18;

    /**
     * Error code returned when the BaseUrl resolution failed
     */
    const URL_RESOLUTION_FAILED_GENERIC_ERROR_CODE = 19;

    /**
     * Error code returned when the append operation in the SourceBuffer failed
     */
    const APPEND_ERROR_CODE = 20;

    /**
     * Error code returned when the remove operation in the SourceBuffer failed
     */
    const REMOVE_ERROR_CODE = 21;

    /**
     * Error code returned when updating the internal objects after loading an MPD failed
     */
    const DATA_UPDATE_FAILED_ERROR_CODE = 22;

    /**
     * Error code returned when MediaSource is not supported by the browser
     */
    const CAPABILITY_MEDIASOURCE_ERROR_CODE = 23;

    /**
     * Error code returned when Protected contents are not supported
     */
    const CAPABILITY_MEDIAKEYS_ERROR_CODE = 24;

    /**
     * Error code returned when loading the manifest failed
     */
    const DOWNLOAD_ERROR_ID_MANIFEST_CODE = 25;

    /**
     * Error code returned when loading the sidx failed
     */
    const DOWNLOAD_ERROR_ID_SIDX_CODE = 26;

    /**
     * Error code returned when loading the media content failed
     */
    const DOWNLOAD_ERROR_ID_CONTENT_CODE = 27;

    /**
     * Error code returned when loading the init segment failed
     */
    const DOWNLOAD_ERROR_ID_INITIALIZATION_CODE = 28;

    /**
     * Error code returned when loading the XLink content failed
     */
    const DOWNLOAD_ERROR_ID_XLINK_CODE = 29;

    /**
     * Error code returned when parsing the MPD resulted in a logical error
     */
    const MANIFEST_ERROR_ID_PARSE_CODE = 31;

    /**
     * Error code returned when no stream (period) has been detected in the manifest
     */
    const MANIFEST_ERROR_ID_NOSTREAMS_CODE = 32;

    /**
     * Error code returned when something wrong has happened during parsing and appending subtitles (TTML or VTT)
     */
    const TIMED_TEXT_ERROR_ID_PARSE_CODE = 33;

    /**
     * Error code returned when a 'muxed' media type has been detected in the manifest. This type is not supported
     */

    const MANIFEST_ERROR_ID_MULTIPLEXED_CODE = 34;

    /**
     * Error code returned when a media source type is not supported
     */
    const MEDIASOURCE_TYPE_UNSUPPORTED_CODE = 35;

    Object.defineProperty(prototype, 'playbackRate', {
        get() {
            let ownProperty = Object.getOwnPropertyDescriptor(
                HTMLMediaElement.prototype,
                'playbackRate'
            );
            return ownProperty ? ownProperty.get.call(this) : undefined;
        },
        set(value) {
            const player = privates.get(this).player;
            try {
                var streamInfo = player.getActiveStream().getStreamInfo();
                var dashMetrics = player.getDashMetrics();
                var dashAdapter = player.getDashAdapter();

                if (streamInfo) {
                    const periodIdx = streamInfo.index;

                    let streamType = 'video';

                    var repSwitch = dashMetrics.getCurrentRepresentationSwitch(streamType, true);
                    if (!repSwitch) {
                        streamType = 'audio';
                        repSwitch = dashMetrics.getCurrentRepresentationSwitch(streamType, true);
                    }

                    var adaptation = dashAdapter.getAdaptationForType(
                        periodIdx,
                        streamType,
                        streamInfo
                    );
                    var currentRep = adaptation.Representation_asArray.find(function(rep) {
                        return rep.id === repSwitch.to;
                    });
                    if (currentRep) {
                        if (!currentRep.maxPlayoutRate) {
                            currentRep.maxPlayoutRate = 1;
                        }
                        if (value > currentRep.maxPlayoutRate) {
                            value = currentRep.maxPlayoutRate;
                        }
                    }
                }
            } catch (e) {
                //value = 1;
                console.warn('DashProxy:', e);
            }
            let ownProperty = Object.getOwnPropertyDescriptor(
                HTMLMediaElement.prototype,
                'playbackRate'
            );
            if (ownProperty) {
                ownProperty.set.call(this, value);
            }
            if (!this.paused) {
                let evt = new Event('__orb_onplayspeedchanged__');
                Object.assign(evt, {
                    speed: this.playbackRate
                });
                this.dispatchEvent(evt);
            }
        },
    });

    Object.defineProperty(prototype, 'error', {
        get() {
            let ownProperty = Object.getOwnPropertyDescriptor(HTMLMediaElement.prototype, 'error');
            return ownProperty.get.call(this) || privates.get(this).error;
        },
    });

    Object.defineProperty(prototype, 'audioTracks', {
        get() {
            return privates.get(this).audioTracks;
        },
    });

    Object.defineProperty(prototype, 'videoTracks', {
        get() {
            return privates.get(this).videoTracks;
        },
    });

    // we override load() for the DASH playback as we receive an internal
    // error when playing and calling the original load
    prototype.load = function() {
        const p = privates.get(this);
        if (p) {
            delete p.startDate;
            if (!this.src) {
                // in this case, the source url is removed
                // so we call the original load() in order to
                // release all resources
                HTMLMediaElement.prototype.load.call(this);
            } else {
                try {
                    let src = p.player.getSource();
                    p.player.attachSource(src);
                    p.error = null;
                } catch (e) {
                    console.warn('DashProxy:', e);
                }
            }
        } else {
            HTMLMediaElement.prototype.load.call(this);
            console.warn('DashProxy: Not initialised.');
        }
    };

    prototype.orb_unload = function() {
        privates.get(this).player.reset();
        this.load();
        return Promise.resolve();
    };

    prototype.orb_getMrsUrl = function() {
        const mrsUrl = privates.get(this).mrsUrl;
        return Promise.resolve(mrsUrl ? mrsUrl.toString() : undefined);
    };

    prototype.orb_getCurrentPeriod = function() {
        let period;
        let p = privates.get(this);
        try {
            period = p.periods[p.player.getActiveStream().getStreamInfo().index];
        } catch (e) {
            console.warn('DashProxy: ' + e);
        }
        return Promise.resolve(period);
    };

    prototype.orb_getPeriods = function() {
        return Promise.resolve(privates.get(this).periods);
    };

    prototype.orb_getCiAncillaryData = function() {
        const ciAncData = privates.get(this).ciAncillaryData;
        return Promise.resolve(ciAncData ? ciAncData.toString() : undefined);
    };

    prototype.orb_invalidate = function() {
        const p = privates.get(this);
        if (p) {
            console.log('DashProxy: Cleaning up...');
            this.removeEventListener('loadedmetadata', p.onLoadedMetaData, true);
            this.textTracks.removeEventListener('change', p.onTextTrackChange);
            p.audioTracks.removeEventListener('change', p.onAudioTrackChange);
            p.videoTracks.removeEventListener('change', p.onVideoTrackChange);
            p.player.off('error', p.onError);
            p.player.off('manifestLoaded', p.onManifestLoaded);
            p.player.off('periodSwitchCompleted', p.onPeriodChanged);
            p.player.off('streamUpdated', p.onPeriodChanged);

            for (const scheme of PARENTAL_CONTROL_EVENT_SCHEMES) {
                p.player.off(scheme, p.onParentalRatingChange);
            }
            if (p.subs.parentNode) {
                p.subs.parentNode.removeChild(p.subs);
            }
            p.player.destroy();
            privates.delete(this);
        } else {
            console.log('DashProxy: Unable to invalidate. Skipping...');
        }
    };
    

    function onLoadedMetaData() {
        const p = privates.get(this);
        if (p) {
            console.log('DashProxy: Loaded data.');
            p.audioTracks.orb_setTrackList(getAudioTracks.call(this));
            p.videoTracks.orb_setTrackList(getVideoTracks.call(this));
            p.onTextTrackChange();
        } else {
            console.warn('DashProxy: Not initialised.');
        }
    }

    function onTextTrackChange() {
        const p = privates.get(this);
        if (p) {
            let index = -1;
            for (let i = 0; i < this.textTracks.length; ++i) {
                if (this.textTracks[i].mode === 'showing') {
                    index = i;
                    break;
                }
            }
            if (index !== -1) {
                if (this.parentNode && !p.subs.parentNode) {
                    if (this.nextSibling) {
                        this.parentNode.insertBefore(p.subs, this.nextSibling);
                    } else {
                        this.parentNode.appendChild(p.subs);
                    }
                }
            } else if (p.subs.parentNode) {
                p.subs.parentNode.removeChild(p.subs);
            }
            p.player.setTextTrack(index);
        }
    }

    function onError(e) {
        if (e.error.code < MANIFEST_LOADER_PARSING_FAILURE_ERROR_CODE) {
            // in case the error is dispatched by this handler, return
            return;
        }
        const p = privates.get(this);
        let nativeEvt = new Event('error');
        let evt = new Event('__orb_onerror__');
        let data = {
            code: 2,
            message: e.error.message,
        };
        console.warn('DashProxy: ' + e.error.message + ' Error code: ' + e.error.code);
        switch (e.error.code) {
            case MANIFEST_ERROR_ID_NOSTREAMS_CODE:
                p.error = hbbtv.objects.createMediaError(
                    MediaError.MEDIA_ERR_SRC_NOT_SUPPORTED,
                    e.error.message
                );
                this.dispatchEvent(nativeEvt);
            case MANIFEST_LOADER_PARSING_FAILURE_ERROR_CODE: // intentional fall-through
            case MANIFEST_ERROR_ID_PARSE_CODE:
            case MANIFEST_ERROR_ID_MULTIPLEXED_CODE:
            case MEDIASOURCE_TYPE_UNSUPPORTED_CODE:
                data.code = 4; // MEDIA_ERR_DECODE
                break;
            case DOWNLOAD_ERROR_ID_CONTENT_CODE:
            case DOWNLOAD_ERROR_ID_INITIALIZATION_CODE:
                p.error = hbbtv.objects.createMediaError(
                    MediaError.MEDIA_ERR_NETWORK,
                    e.error.message
                );
                this.dispatchEvent(nativeEvt);
                data.code = 5; // content not available
                break;
            case DOWNLOAD_ERROR_ID_XLINK_CODE:
                // xlink:href couldn't be fetched. Xlink attributes are simply removed by dash.js
                return;
            default:
                data.code = 2; // Unidentified error
                break;
        }
        Object.assign(evt, {
            error: data,
        });
        this.dispatchEvent(evt);
    }

    function getAudioTracks() {
        const p = privates.get(this);

        function extractKindFromTrack(track) {
            if (track.roles) {
                if (track.roles.length === 1) {
                    switch (track.roles[0]) {
                        case 'alternate':
                            return 'alternative';
                        case 'main':
                            return 'main';
                        case 'commentary':
                            return 'commentary';
                        default:
                            break;
                    }
                } else if (track.roles.length === 2) {
                    if (
                        track.roles.includes('description') &&
                        track.roles.includes('supplementary')
                    ) {
                        return 'descriptions';
                    }
                    if (track.roles.includes('description') && track.roles.includes('main')) {
                        return 'main-desc';
                    }
                    if (track.roles.includes('dub') && track.roles.includes('main')) {
                        return 'translation';
                    }
                }
            }
            return '';
        }

        if (p) {
            let tracks = [];
            const currentTrack = p.player.getCurrentTrackFor('audio');
            p.player.getTracksFor('audio').forEach((track) => {
                let info = {
                    enabled: currentTrack && currentTrack.index === track.index,
                    index: track.index,
                    id: track.id ? track.id.toString() : track.index.toString(),
                    kind: extractKindFromTrack(track),
                    label: track.labels[0],
                    language: track.lang,
                    numChannels: parseInt(track.audioChannelConfiguration[0]),
                    encoding: track.codec,
                    encrypted: track.contentProtection ? true : false,
                    accessibility: parseInt(track.accessibility[0]),
                };
                tracks.push(info);
            });
            return tracks;
        }
        return [];
    }

    function getVideoTracks() {
        const p = privates.get(this);

        function extractKindFromTrack(track) {
            if (track.roles) {
                if (track.roles.length === 1) {
                    switch (track.roles[0]) {
                        case 'alternate':
                            return 'alternative';
                        case 'main':
                            return 'main';
                        case 'commentary':
                            return 'commentary';
                        default:
                            break;
                    }
                } else if (track.roles.length === 2) {
                    if (track.roles.includes('subtitle') && track.roles.includes('main')) {
                        return 'subtitles';
                    }
                    if (track.roles.includes('caption') && track.roles.includes('main')) {
                        return 'captions';
                    }
                }
            }
            return '';
        }

        if (p) {
            let tracks = [];
            const currentTrack = p.player.getCurrentTrackFor('video');
            p.player.getTracksFor('video').forEach((track) => {
                let info = {
                    selected: currentTrack && currentTrack.index === track.index,
                    index: track.index,
                    id: track.id ? track.id.toString() : track.index.toString(),
                    kind: extractKindFromTrack(track),
                    label: track.labels[0],
                    language: track.lang,
                    encoding: track.codec,
                    encrypted: track.contentProtection ? true : false,
                };
                tracks.push(info);
            });
            console.log("video tracks:", tracks);
            return tracks;
        }
        return [];
    }

    function onParentalRatingChange(payload) {
        let evt = new Event('__orb_onparentalratingchange__');
        let nativeEvt = new Event('error');
        let data = {
            contentID: undefined,
            ratings: undefined,
            DRMSystemID: undefined,
            blocked: true,
        };
        let parser = new DOMParser();
        try {
            let xmlDoc = parser.parseFromString(payload.event.messageData, 'text/xml');
            xmlDoc = xmlDoc.getElementsByTagName('ParentalGuidance');
            for (const i of xmlDoc) {
                let rating = {
                    scheme: PARENTAL_CONTROL_EVENT_SCHEME_LOOKUP[payload.type],
                };
                for (const j of i.children) {
                    if (j.tagName.toLowerCase().includes('parentalrating')) {
                        let href = j.getAttribute('href');
                        if (href) {
                            rating.value = parseInt(href.split(':').pop());
                        }
                    } else if (j.tagName.toLowerCase().includes('region')) {
                        rating.region = j.innerHTML.toLowerCase();
                    }
                    if (
                        rating.region &&
                        rating.value &&
                        !window.orbParentalRating.isRatingBlocked(rating.scheme, rating.region, rating.value)
                    ) {
                        data.blocked = false;
                        break;
                    }
                }
                if (!data.blocked) {
                    break;
                }
            }
        } catch (e) {
            console.warn('DashProxy:', e);
        }
        if (data.blocked) {
            console.warn('DashProxy: Content was blocked by parental control.');
            nativeEvt.error = {
                code: 3,
                message: 'MEDIA_ERR_DECODE'
            }
            this.dispatchEvent(nativeEvt);
            privates.get(this).player.attachSource(null);
        }
        Object.assign(evt, data);
        this.dispatchEvent(evt);
    }

    function filterCapabilities(representation) {
        const SUPPORTED_PROFILES = [
            'urn:mpeg:dash:profile:full:2011',
            'urn:mpeg:dash:profile:isoff-on-demand:2011',
            'urn:mpeg:dash:profile:isoff-live:2011',
            'urn:mpeg:dash:profile:isoff-main:2011',
            'urn:mpeg:dash:profile:mp2t-main:2011',
            'urn:mpeg:dash:profile:mp2t-simple:2011',
            'urn:3GPP:PSS:profile:DASH10',
            'urn:dvb:dash:profile:dvb-dash:2014',
            'urn:hbbtv:dash:profile:isoff-live:2012',
            'urn:dvb:dash:profile:dvb-dash:2017',
        ];

        const SUPPORTED_ESSENTIAL_SCHEMES = [
            'urn:mpeg:mpegB:cicp:ColourPrimaries',
            'urn:mpeg:mpegB:cicp:MatrixCoefficients',
            'urn:mpeg:mpegB:cicp:TransferCharacteristics',
            'urn:mpeg:dash:preselection:2016',
            'urn:mpeg:mpegB:cicp:VideoFullRangeFlag',
            'urn:mpeg:mpegB:cicp:VideoFramePackingType',
            'urn:mpeg:mpegB:cicp:QuincunxSamplingFlag',
            'urn:mpeg:mpegB:cicp:PackedContentInterpretationType',
            'urn:mpeg:dash:14496:10:frame_packing_arrangement_type:2011',
            'urn:mpeg:dash:13818:1:stereo_video_format_type:2011',
        ];

        function hasValidProfile(profiles) {
            if (profiles) {
                let prfls = profiles.split(',');
                for (let profile of prfls) {
                    if (
                        SUPPORTED_PROFILES.includes(profile) ||
                        SUPPORTED_PROFILES.includes(profile.replace('isoff-live:', '')) ||
                        SUPPORTED_PROFILES.includes(profile.replace('isoff-ext-live:', ''))
                    ) {
                        return true;
                    }
                }
                return false;
            }
            return true;
        }

        function hasValidEssentialScheme() {
            if (representation.EssentialProperty) {
                const supportedEssentialProperties = representation.EssentialProperty.every(value => SUPPORTED_ESSENTIAL_SCHEMES.includes(
                    value.schemeIdUri
                ));
                return supportedEssentialProperties;
            }
            return true;
        }

        return (
            representation.bandwidth &&
            hasValidProfile(privates.get(this).profiles) &&
            hasValidProfile(representation.profiles) &&
            hasValidEssentialScheme()
        );
    }

    function onManifestLoaded(e) {
        const p = privates.get(this);
        p.profiles = e.data.profiles;
        p.periods = e.data.Period_asArray;
        p.mrsUrl = e.data.mrsUrl;
        p.ciAncillaryData = e.data.ciAncillaryData;
        if (!p.startDate) {
            p.startDate = Date.parse(e.data.availabilityStartTime);
            if ((p.periods.length > 0) && (p.periods[0].start)) {
                p.startDate += p.periods[0].start * 1000;
            }
        }

        const evt = new Event('__orb_startDateUpdated__');
        Object.assign(evt, {
            startDate: p.startDate
        });
        this.dispatchEvent(evt);

        hbbtv.native.dispatchManifestNativeEvents?.(e);

        console.log(
            'manifest availability start time:',
            e.data.availabilityStartTime,
            ' , current time:',
            new Date().toJSON()
        );
    }

    function makeStreamInfoCallback(context, eventName) {
        return function(e) {
            let evt = new Event(eventName);
            Object.assign(evt, {
                data: e.streamInfo ? e.streamInfo : e.toStreamInfo,
            });
            context.dispatchEvent(evt);
        };
    }

    function onAudioTrackChange() {
        const player = privates.get(this).player;
        let mute = true;
        for (let i = 0; i < this.audioTracks.length; ++i) {
            if (this.audioTracks[i].enabled) {
                mute = false;
                let nextTrack = player
                    .getTracksFor('audio')
                    .find((track) => track.index === this.audioTracks[i].index);
                if (player.getCurrentTrackFor('audio') !== nextTrack) {
                    player.setCurrentTrack(nextTrack);
                }
                break;
            }
        }
        this.muted = mute;
    }

    function onVideoTrackChange() {
        const player = privates.get(this).player;
        for (let i = 0; i < this.videoTracks.length; ++i) {
            if (this.videoTracks[i].selected) {
                let nextTrack = player
                    .getTracksFor('video')
                    .find((track) => track.index === this.videoTracks[i].index);
                if (player.getCurrentTrackFor('video') !== nextTrack) {
                    player.setCurrentTrack(nextTrack);
                }
                break;
            }
        }
    }

    function initialise(src) {
        if (!privates.get(this)) {
            this.orb_invalidate?.();
            Object.setPrototypeOf(this, prototype);
            if (src) {
                privates.set(this, {});
                const p = privates.get(this);
                p.error = null;
                p.audioTracks = hbbtv.objects.createAudioTrackList();
                p.videoTracks = hbbtv.objects.createVideoTrackList();
                p.onLoadedMetaData = onLoadedMetaData.bind(this);
                p.onManifestLoaded = onManifestLoaded.bind(this);
                p.onTextTrackChange = onTextTrackChange.bind(this);
                p.onAudioTrackChange = onAudioTrackChange.bind(this);
                p.onVideoTrackChange = onVideoTrackChange.bind(this);
                p.onPeriodChanged = makeStreamInfoCallback(this, '__orb_onperiodchanged__');
                p.onStreamUpdated = makeStreamInfoCallback(this, '__orb_onstreamupdated__');
                p.audioTracks.addEventListener('change', p.onAudioTrackChange);
                p.videoTracks.addEventListener('change', p.onVideoTrackChange);

                p.onError = onError.bind(this);
                p.onParentalRatingChange = onParentalRatingChange.bind(this);
                this.addEventListener('loadedmetadata', p.onLoadedMetaData, true);
                p.player = orb_dashjs.MediaPlayer().create();
                p.player.registerCustomCapabilitiesFilter(filterCapabilities.bind(this));
                p.player.updateSettings({
                    debug: {
                        //LOG_LEVEL_NONE     0
                        //LOG_LEVEL_FATAL    1
                        //LOG_LEVEL_ERROR    2
                        //LOG_LEVEL_WARNING  3
                        //LOG_LEVEL_INFO     4
                        //LOG_LEVEL_DEBUG    5
                        logLevel: 3,
                        dispatchEvent: false,
                    },
                    utcSynchronization: {
                        enableBackgroundSyncAfterSegmentDownloadError: true,
                        backgroundAttempts: 1,
                        timeBetweenSyncAttempts: 5,
                    },
                    streaming: {
                        fragmentRequestTimeout: 20000,
                        trackSwitchMode: {
                            audio: 'alwaysReplace',
                            video: 'neverReplace',
                        },
                        capabilities: {
                            filterUnsupportedEssentialProperties: true,
                        },
                        retryAttempts: {
                            MPD: 0,
                            MediaSegment: 0,
                            InitializationSegment: 0,
                            BitstreamSwitchingSegment: 0,
                            IndexSegment: 0,
                            FragmentInfoSegment: 0,
                        },
                        buffer: {
                            enableSeekDecorrelationFix: true,
                            fastSwitchEnabled: true,
                            flushBufferAtTrackSwitch: true,
                        },
                        utcSynchronization: {
                            enableBackgroundSyncAfterSegmentDownloadError: true,
                            backgroundAttempts: 1,
                            timeBetweenSyncAttempts: 5,
                        },
                    },
                    errors: {
                        recoverAttempts: {
                            mediaErrorDecode: 1,
                        },
                    },
                });
                hbbtv.native.setDashProxy(this);
                p.player.initialize(this, src, false);
                p.player.on('error', p.onError);
                p.player.on('manifestLoaded', p.onManifestLoaded);
                p.player.on('periodSwitchCompleted', p.onPeriodChanged);
                p.player.on('streamUpdated', p.onStreamUpdated);
                p.player.on('representationSwitch', (e) => {
                    hbbtv.native.dispatchRepresentationNativeEvents?.(e.currentRepresentation);
                });

                for (const scheme of PARENTAL_CONTROL_EVENT_SCHEMES) {
                    p.player.on(scheme, p.onParentalRatingChange);
                }
                const subtitlesRenderDiv = document.createElement('div');
                p.subs = subtitlesRenderDiv;
                subtitlesRenderDiv.id = 'subtitles-render-div';
                p.player.attachTTMLRenderingDiv(subtitlesRenderDiv);
                
                this.textTracks.addEventListener('change', p.onTextTrackChange);

                console.log('DashProxy: Initialised DashProxy.');
            } else {
                // if the src attribute is unset, fallback to the native proxy.
                // that way we let the browser handle any <source> children
                this.__objectType = 'native';
                hbbtv.objects.NativeProxy.initialise.call(this, src);
            }
        } else {
            console.log('DashProxy: Already initialised. Skipping...');
        }
    }

    return {
        initialise: initialise,
    };
})();

hbbtv.mediaManager.registerObject({
    initialise: function(object, src) {
        hbbtv.objects.DashProxy.initialise.call(object, src);
    },
    getName: function() {
        return 'application/dash+xml';
    },
    getSupportedExtensions: function() {
        return ['mpd'];
    },
    getSupportedContentTypes: function() {
        return ['application/dash+xml'];
    },
});