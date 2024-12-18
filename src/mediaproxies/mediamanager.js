/**
 * @fileOverview Monitors the document and certain JavaScript mechanisms for objects. Builds or
 * upgrades those objects if there is a registered handler.
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
hbbtv.mediaManager = (function() {
    let objectHandlers = {};
    let fallbackHandlers = undefined;
    let mediaType = undefined;

    const mediaProxy = hbbtv.objects.createIFrameObjectProxy();
    window.orbNetwork = {
        resolveHostAddress: (hostname) =>
            hbbtv.native.request('Network.resolveHostAddress', {
                hostname
            }).result,

        resolveNetworkError: (responseText) =>
            hbbtv.native.request('Network.resolveNetworkError', {
                responseText
            }).result,
    };
    window.orbParentalRating = {
        isRatingBlocked: (scheme, region, value) =>
            hbbtv.native.request('ParentalControl.isRatingBlocked', {
                "scheme": scheme,
                "region": region,
                "value": value
            }).result,
    };

    hbbtv.bridge = {
        configuration: {
            getPreferredAudioLanguage: () =>
                hbbtv.native.request('Configuration.getPreferredAudioLanguage').result,
            getCleanAudioEnabled: () =>
                hbbtv.native.request('Configuration.getCleanAudioEnabled').result,
            getSubtitlesEnabled: () =>
                hbbtv.native.request('Configuration.getSubtitlesEnabled').result
        },
    };

    function initialise() {
        addMutationIntercept();
        addSourceManipulationIntercept();

        const __play = HTMLMediaElement.prototype.play;
        const __load = HTMLMediaElement.prototype.load;
        HTMLMediaElement.prototype.load = function() {
            let tracks = this.textTracks;
            if (tracks) {
                for (const track of tracks) {
                    track.mode = 'hidden';
                }
            }

            __load.call(this);
        };

        // we override play() for the DASH playback as we end up receiving
        // Uncaught (in promise) DOMException: The play() request was interrupted by a new load request.
        // when calling play() immediately after setting the src attribute
        HTMLMediaElement.prototype.play = function() {
            const thiz = this;

            return new Promise((resolve, reject) => {
                if (thiz.readyState < 2) {
                    const playFcn = function() {
                        thiz.removeEventListener('loadeddata', playFcn, true);
                        thiz.removeEventListener('progress', playFcn, true);
                        __play.call(thiz).then(resolve, reject);
                    };
                    thiz.addEventListener('loadeddata', playFcn, true);
                    thiz.addEventListener('progress', playFcn, true);
                } else {
                    __play.call(thiz).then(resolve, reject);
                }
            });
        };
    }

    function registerObject(handlers) {
        objectHandlers[handlers.getName()] = handlers;
        if (handlers.getName() === 'native') {
            fallbackHandlers = handlers;
        }
    }

    function getHandlerByContentType(type) {
        for (const key in objectHandlers) {
            const supportedTypes = objectHandlers[key].getSupportedContentTypes();
            for (const t of supportedTypes) {
                if (type.includes(t)) {
                    return objectHandlers[key];
                }
            }
        }
        return undefined;
    }

    function setObjectHandler(nextHandler, src) {
        this.__objectType = nextHandler.getName();
        nextHandler.initialise(this, src);
    }

    function upgradeObject(src) {
        const object = this;
        let objType = object.__objectType;
        if (!src) {
            return Promise.reject('Playback source is not defined.');
        } else if (
            (object.getAttribute('src') === src && objType) ||
            src.toLowerCase().startsWith('blob:')
        ) {
            return Promise.resolve();
        }

        if (mediaType) {
            const nextHandler = getHandlerByContentType(mediaType);
            mediaType = undefined;
            if (nextHandler) {
                setObjectHandler.call(object, nextHandler, src);
                return Promise.resolve();
            }
        }

        // consider playback anchors with url
        let ext = src.split('#')[0];
        ext = ext.split('?')[0];
        // first check each objectHandler supported extensions
        if (ext) {
            ext = ext.split('.').pop().toLowerCase();
            console.log('MediaManager: Checking extension support for .' + ext + '...');
            for (const key in objectHandlers) {
                if (objectHandlers[key].getSupportedExtensions().indexOf(ext) >= 0) {
                    setObjectHandler.call(object, objectHandlers[key], src);
                    return Promise.resolve();
                }
            }
        }

        // if no supported extension is found, request the content-type of the source
        return new Promise((resolve, reject) => {
            const request = new XMLHttpRequest();
            request.onabort = request.onerror = () => {
                reject('An error occurred while requesting the content type.');
            };
            let timeoutId = -1;
            request.onload = () => {
                clearTimeout(timeoutId);
                try {
                    const contentType = request
                        .getAllResponseHeaders()
                        .split('\n')
                        .find((header) => header.toLowerCase().startsWith('content-type'))
                        .split(':')[1]
                        .trim();

                    console.log('MediaManager: Requested content of type ' + contentType);
                    const nextHandler = getHandlerByContentType(contentType);
                    if (nextHandler) {
                        setObjectHandler.call(object, nextHandler, src);
                        resolve();
                    } else {
                        reject(
                            'Failed to find a registered playback proxy for the content type ' +
                            contentType
                        );
                    }
                } catch (e) {
                    reject(e);
                }
            };
            timeoutId = setTimeout(request.onload, 5000);
            request.open('HEAD', src);
            request.send();
        });
    }

    function upgradeToFallback(node, src, err) {
        console.warn(
            'MediaManager: Failed to upgrade object. Fallback to native proxy. Error: ' + err
        );
        if (fallbackHandlers) {
            node.__objectType = fallbackHandlers.getName();
            fallbackHandlers.initialise(node, src);
        }
    }

    function addSourceManipulationIntercept() {
        const ownProperty = Object.getOwnPropertyDescriptor(HTMLMediaElement.prototype, 'src');
        Object.defineProperty(HTMLMediaElement.prototype, 'src', {
            set(val) {
                this.setAttribute('src', val);
            },
            get() {
                return ownProperty.get.call(this);
            },
        });

        HTMLMediaElement.prototype.setAttribute = function(name, value) {
            if (name === 'src' && !this.__objectType) {
                const thiz = this;
                console.log('MediaManager: intercepted src manipulation. new src: ' + value);
                if (value && !value.startsWith('http')) {
                    value =
                        document.baseURI.substring(
                            document.baseURI.indexOf('base=') + 5,
                            document.baseURI.lastIndexOf('/') + 1
                        ) + value;
                }
                upgradeObject.call(this, value).catch((e) => upgradeToFallback(thiz, value, e));
                return;
            }
            Element.prototype.setAttribute.call(this, name, value);
        };
    }

    function registerObservers(media) {
        const MEDIA_PROXY_ID = 'HTMLMediaElement';
        const MEDIA_KEYS_ID = 'MediaKeys';
        const MEDIA_KEY_SESSION_ID = 'MediaKeySession';
        mediaProxy.registerObserver(MEDIA_PROXY_ID, media);

        const audioTracks = hbbtv.objects.createAudioTrackList(mediaProxy);
        const videoTracks = hbbtv.objects.createVideoTrackList(mediaProxy);
        const textTracks = hbbtv.objects.createTextTrackList(media, mediaProxy);

        media.setMediaKeys = function(mediaKeys) {
            if (mediaKeys && mediaKeys.mediaKeySystemAccess) {
                let _mediaKeys;
                return navigator
                    .requestMediaKeySystemAccess(
                        mediaKeys.mediaKeySystemAccess.keySystem,
                        mediaKeys.mediaKeySystemAccess.configuration
                    )
                    .then((mediaKeySystemAccess) => mediaKeySystemAccess.createMediaKeys())
                    .then((keys) => {
                        _mediaKeys = keys;
                        _mediaKeys.createSession = function(sessionType) {
                            const session = MediaKeys.prototype.createSession.call(
                                this,
                                sessionType || undefined
                            );
                            mediaProxy.registerObserver(MEDIA_KEY_SESSION_ID, session);
                            session.generateRequest = function(initDataType, initData) {
                                return MediaKeySession.prototype.generateRequest.call(
                                    this,
                                    initDataType,
                                    new Uint8Array(initData).buffer
                                );
                            };
                            session.update = function(licence) {
                                return MediaKeySession.prototype.update.call(
                                    this,
                                    new Uint8Array(licence)
                                );
                            };
                            session.close = function() {
                                return MediaKeySession.prototype.close.call(this);
                            };
                            session.load = function(sessionId) {
                                return MediaKeySession.prototype.load.call(this, sessionId);
                            };
                            session.remove = function() {
                                return MediaKeySession.prototype.remove.call(this);
                            };
                            session.onmessage = (e) => {
                                mediaProxy.dispatchEvent(MEDIA_KEY_SESSION_ID, e);
                            };
                            session.onkeystatuseschange = (e) => {
                                mediaProxy.dispatchEvent(MEDIA_KEY_SESSION_ID, e);
                            };
                            session.orb_closed = function() {
                                return this.closed;
                            };
                            return session;
                        };
                        _mediaKeys.setServerCertificate = function(cert) {
                            return MediaKeys.prototype.setServerCertificate.call(
                                this,
                                new Uint8Array(cert).buffer
                            );
                        };
                        return HTMLMediaElement.prototype.setMediaKeys.call(media, keys);
                    })
                    .then(() => mediaProxy.registerObserver(MEDIA_KEYS_ID, _mediaKeys));
            }
            return HTMLMediaElement.prototype.setMediaKeys.call(media, mediaKeys);
        };

        Object.defineProperty(media, 'audioTracks', {
            value: audioTracks,
            writable: false,
        });
        Object.defineProperty(media, 'videoTracks', {
            value: videoTracks,
            writable: false,
        });
        Object.defineProperty(media, 'textTracks', {
            value: textTracks,
            writable: false,
        });
        const genericEvents = [
            'loadstart',
            'suspend',
            'abort',
            'emptied',
            'stalled',
            'canplay',
            'canplaythrough',
            'playing',
            'seeking',
            'seeked',
            '__orb_onerror__',
            '__orb_onperiodchanged__',
            '__orb_onstreamupdated__',
        ];
        const genericHandler = (e) => {
            mediaProxy.dispatchEvent(MEDIA_PROXY_ID, e);
        };
        const propsUpdateCallback = function(e) {
            const props = {};
            const keys = [
                'currentTime',
                'playbackRate',
                'volume',
                'muted',
                'loop',
                'defaultMuted',
                'defaultPlaybackRate',
                'disableRemotePlayback',
                'preservesPitch',
                'paused',
                'ended',
                'currentSrc',
                'duration',
                'networkState',
                'readyState',
            ];
            for (const key of keys) {
                props[key] = media[key];
            }
            mediaProxy.updateObserverProperties(MEDIA_PROXY_ID, props);
            mediaProxy.dispatchEvent(MEDIA_PROXY_ID, e);
            console.log('iframe: update properties', e.type, props);
        };

        const updateBuffered = function(e) {
            const ranges = [];
            for (let i = 0; i < media.buffered.length; ++i) {
                ranges.push({
                    start: media.buffered.start(i),
                    end: media.buffered.end(i),
                });
            }
            mediaProxy.callObserverMethod(MEDIA_PROXY_ID, 'setBuffered', [ranges]);
            mediaProxy.dispatchEvent(MEDIA_PROXY_ID, e);
        };
        const makeCallback = function(property) {
            return function(e) {
                mediaProxy.updateObserverProperties(MEDIA_PROXY_ID, {
                    [property]: media[property],
                });
                mediaProxy.dispatchEvent(MEDIA_PROXY_ID, e);
            };
        };
        for (const evt of genericEvents) {
            media.addEventListener(evt, genericHandler);
        }
        media.addEventListener('encrypted', (e) => {
            const evt = new Event(e.type, e);
            Object.assign(evt, {
                initDataType: e.initDataType,
                initData: [...new Uint8Array(e.initData)],
            });
            mediaProxy.dispatchEvent(MEDIA_PROXY_ID, evt);
        });

        const propsUpdateEvents = ['loadeddata', 'waiting', 'loadedmetadata', 'ended', 'playing'];
        for (const evt of propsUpdateEvents) {
            media.addEventListener(evt, propsUpdateCallback);
        }

        media.addEventListener('resize', (e) => {
            const widthProperty = Object.getOwnPropertyDescriptor(
                HTMLVideoElement.prototype,
                'videoWidth'
            );
            const heightProperty = Object.getOwnPropertyDescriptor(
                HTMLVideoElement.prototype,
                'videoHeight'
            );
            mediaProxy.updateObserverProperties(MEDIA_PROXY_ID, {
                videoWidth: widthProperty.get.call(media),
                videoHeight: heightProperty.get.call(media),
            });
            mediaProxy.dispatchEvent(MEDIA_PROXY_ID, e);
        });
        media.addEventListener('__orb_startDateUpdated__', (e) =>
            mediaProxy.dispatchEvent(MEDIA_PROXY_ID, e)
        );

        media.addEventListener('play', (e) => {
            // some tests ask for the seekable property just after the playing event
            hbbtv.native.updateSeekable(e);
            let evt = new Event('__orb_onplayspeedchanged__');
            Object.assign(evt, {
                speed: media.playbackRate
            });
            mediaProxy.dispatchEvent(MEDIA_PROXY_ID, evt);
            propsUpdateCallback(e);
        });
        media.addEventListener('pause', (e) => {
            let evt = new Event('__orb_onplayspeedchanged__');
            Object.assign(evt, {
                speed: 0
            });
            hbbtv.native.setPausedDelta?.(true);
            mediaProxy.dispatchEvent(MEDIA_PROXY_ID, evt);
            propsUpdateCallback(e);
        });
        media.addEventListener('durationchange', makeCallback('duration'));
        media.addEventListener('ratechange', makeCallback('playbackRate'));
        media.addEventListener('__orb_onplayspeedchanged__', makeCallback('playbackRate'));
        media.addEventListener('volumechange', makeCallback('volume'));
        videoTracks.addEventListener('change', () => {
            for (const track of videoTracks) {
                if (track.selected) {
                    media.style.display = 'block';
                    return;
                }
            }
            media.style.display = 'none';
        });
        media.addEventListener('error', (e) => {
            mediaProxy.updateObserverProperties(MEDIA_PROXY_ID, {
                error: {
                    code: media.error ? media.error.code : e.error.code,
                    message: media.error ? media.error.message : e.error.message,
                },
            });
            mediaProxy.dispatchEvent(MEDIA_PROXY_ID, e);
        });
        hbbtv.native.setMedia(media);
        hbbtv.native.setMediaProxy(mediaProxy);
        // add extra event handlers that native requires
        hbbtv.native.addMediaNativeListeners?.();
        media.addEventListener(hbbtv.native.getSeekablePollingEvent(), (e) => hbbtv.native.updateSeekable(e));
        media.addEventListener(hbbtv.native.getBufferedPollingEvent(), updateBuffered);

        media.addEventListener('timeupdate', makeCallback('currentTime'));
        media.addTextTrack = function() {
            return textTracks.orb_addTextTrack.apply(textTracks, arguments);
        };

        media.addTrackEvent = function() {
            return textTracks.orb_addTrackEvent.apply(textTracks);
        };
        media.removeTrackEvent = function() {
            return textTracks.orb_removeTrackEvent.apply(textTracks);
        };

    }

    // Mutation observer
    function addMutationIntercept() {
        const observer = new MutationObserver(function(mutationsList) {
            for (const mutation of mutationsList) {
                for (const node of mutation.addedNodes) {
                    if (node.nodeName.toLowerCase() === 'source' && mediaType === undefined) {
                        mediaType = node.type;
                    } else if (
                        (node.nodeName && node.nodeName.toLowerCase() === 'video') ||
                        node.nodeName.toLowerCase() === 'audio'
                    ) {
                        registerObservers(node);
                    }
                }
            }
        });

        observer.observe(document.documentElement || document.body, {
            childList: true,
            subtree: true,
        });
    }

    return {
        initialise: initialise,
        registerObject: registerObject,
    };
})();