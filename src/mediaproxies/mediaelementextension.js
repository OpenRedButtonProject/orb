/**
 * @fileOverview MediaElementExtension class
 * @license ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

hbbtv.objects.MediaElementExtension = (function() {
    const privates = new WeakMap();
    const MEDIA_PROXY_ID = 'HTMLMediaElement';
    const props = [
        'autoplay',
        'controls',
        'currentTime',
        'playbackRate',
        'volume',
        'muted',
        'loop',
        'defaultMuted',
        'crossOrigin',
        'controlsList',
        'defaultPlaybackRate',
        'disableRemotePlayback',
        'preservesPitch',
        'srcObject',
    ];
    const mediaOwnProperties = Object.getOwnPropertyDescriptors(HTMLMediaElement.prototype);

    if (location.protocol !== 'http:' && location.hostname !== 'localhost') {
        const _requestMediaKeySystemAccess = Navigator.prototype.requestMediaKeySystemAccess;
        Navigator.prototype.requestMediaKeySystemAccess = function() {
            return Promise.resolve(
                hbbtv.objects.createMediaKeySystemAccess.apply(undefined, arguments)
            );
        };

        // consider custom EME playback
        const _setMediaKeys = HTMLMediaElement.prototype.setMediaKeys;
        HTMLMediaElement.prototype.setMediaKeys = function(mediaKeys) {
            const thiz = this;
            if (!mediaKeys || !mediaKeys.orb_polyfilled) {
                return _setMediaKeys.call(thiz, mediaKeys);
            }
            const conf = mediaKeys.toJSON().mediaKeySystemAccess;
            return _requestMediaKeySystemAccess
                .call(navigator, conf.keySystem, conf.configuration)
                .then((mksa) => mksa.createMediaKeys())
                .then((mkeys) => {
                    mediaKeys.createSession = function(sessionType) {
                        return mkeys.createSession(sessionType);
                    };
                    mediaKeys.setServerCertificate = function(cert) {
                        return mkeys.setServerCertificate(cert);
                    };
                    return _setMediaKeys.call(thiz, mkeys);
                });
        };
    }

    HTMLMediaElement.prototype.getStartDate = function() {
        return new Date(NaN);
    };
    HTMLMediaElement.prototype.orb_getPeriods =
        HTMLMediaElement.prototype.orb_getMrsUrl =
        HTMLMediaElement.prototype.orb_getCurrentPeriod =
        HTMLMediaElement.prototype.orb_getCiAncillaryData =
        HTMLMediaElement.prototype.orb_unload =
        function() {
            return Promise.resolve();
        };

    // Override default play() and load() for the case where it is being called
    // immediatelly after setting its source. Due to the fact
    // that the upgrade is asynchronous, the call is being skipped,
    // so we call it inside setTimeout()
    const __play = HTMLMediaElement.prototype.play;
    HTMLMediaElement.prototype.play = function() {
        const thiz = this;
        return new Promise((resolve, reject) => {
            setTimeout(() => {
                const playPromise = privates.get(thiz) ? thiz.play() : __play.call(thiz);
                playPromise.then(resolve).catch(reject);
            }, 0);
        });
    };

    const __load = HTMLMediaElement.prototype.load;
    HTMLMediaElement.prototype.load = function() {
        setTimeout(() => {
            privates.get(this) ? this.load() : __load.call(this);
        }, 0);
    };

    // Override default addEventListener and removeEventListener for HTMLMediaElement's
    // textTracks, so that callbacks are stored and will be registered once it is upgraded
    const __addTextTracksEventListener = TextTrackList.prototype.addEventListener;
    TextTrackList.prototype.addEventListener = function(type, callback) {
        let callbacks = privates.get(this);
        if (!callbacks) {
            callbacks = {};
            privates.set(this, callbacks);
        }
        if (!callbacks[type]) {
            callbacks[type] = new Set();
        }
        callbacks[type].add(callback);
        __addTextTracksEventListener.apply(this, arguments);
    }

    const __removeTextTracksEventListener = TextTrackList.prototype.removeEventListener;
    TextTrackList.prototype.removeEventListener = function(type, callback) {
        const callbacks = privates.get(this);
        if (callbacks && !callbacks[type]) {
            callbacks[type].delete(callback);
        }
        __removeTextTracksEventListener.apply(this, arguments);
    }


    function addSourceSetterIntercept() {
        const setAttribute = HTMLMediaElement.prototype.setAttribute;
        Object.defineProperty(HTMLMediaElement.prototype, 'src', {
            set(val) {
                this.setAttribute('src', val);
            },
            get() {
                return mediaOwnProperties.src.get.call(this);
            },
        });

        HTMLMediaElement.prototype.setAttribute = function(name, value) {
            if (name === 'src' && initialise.call(this, value)) {
                this.src = value;
                return;
            }
            return setAttribute.apply(this, arguments);
        };
    }

    function createPrototype() {
        const ORB_PLAYER_URL = 'orb://player';
        const prototype = Object.create(HTMLMediaElement.prototype);
        const methods = ['pause', 'load'];
        const asyncMethods = [
            'orb_getPeriods',
            'orb_getMrsUrl',
            'orb_getCurrentPeriod',
            'orb_getCiAncillaryData',
        ];
        const nodeMethods = [
            'appendChild',
            'insertBefore',
            'removeChild',
            'replaceChild',
            'hasChildNodes',
            'getElementsByTagName',
            'getElementById',
        ];
        const nodeProps = ['childNodes', 'firstChild', 'lastChild', 'children'];
        const roProps = [
            'textTracks',
            'audioTracks',
            'videoTracks',
            'paused',
            'ended',
            'currentSrc',
            'error',
            'networkState',
            'readyState',
            'seekable',
            'videoWidth',
            'videoHeight',
        ];
        let lastMediaElement = undefined;

        function makeNodeMethod(name) {
            return function() {
                const thiz = privates.get(this).divDummy;
                return thiz[name].apply(thiz, arguments);
            };
        }

        for (const name of nodeMethods) {
            prototype[name] = makeNodeMethod(name);
        }

        for (const key of nodeProps) {
            Object.defineProperty(prototype, key, {
                get() {
                    return privates.get(this).divDummy[key];
                },
            });
        }

        hbbtv.utils.defineGetterProperties(prototype, {
            nextSibling() {
                const nextSibling = Object.getOwnPropertyDescriptor(
                    Node.prototype,
                    'nextSibling'
                ).get.call(this);
                // skip the generated iframe
                if (nextSibling && nextSibling.orb_mediaElementExtension) {
                    return nextSibling.nextSibling;
                }
                return nextSibling;
            },
            previousSibling() {
                const previousSibling = Object.getOwnPropertyDescriptor(
                    Node.prototype,
                    'previousSibling'
                ).get.call(this);
                // check if there is a generated iframe from another video element
                if (previousSibling && previousSibling.orb_mediaElementExtension) {
                    return previousSibling.previousSibling;
                }
                return previousSibling;
            },
        });

        prototype.getStartDate = function() {
            return privates.get(this).videoDummy.startDate;
        };

        prototype.getAttribute = function(name) {
            if (props.includes(name)) {
                return this[name];
            }
            return HTMLMediaElement.prototype.getAttribute.apply(this, arguments);
        };

        prototype.setAttribute = function(name, value) {
            if (props.includes(name)) {
                this[name] = value;
            } else {
                HTMLMediaElement.prototype.setAttribute.call(this, name, value);
            }
        };

        prototype.removeAttribute = function(name) {
            const p = privates.get(this);
            if (props.includes(name)) {
                delete p.videoDummy[name];
                p.iframeProxy.callObserverMethod(MEDIA_PROXY_ID, 'removeAttribute', [name]);
            } else {
                HTMLMediaElement.prototype.removeAttribute.apply(this, arguments);
            }
        };

        prototype.orb_unload = function() {
            const p = privates.get(this);
            delete p.videoDummy.src;
            return p.iframeProxy.callAsyncObserverMethod(MEDIA_PROXY_ID, 'orb_unload');
        };

        prototype.play = function() {
            if (!this.__orb_addedToMediaSync__) {
                // check if the HTMLMediaElement is provided to MediaSynchroniser.addMediaObject() before we pause it
                if (lastMediaElement && lastMediaElement !== this && !lastMediaElement.paused) {
                    lastMediaElement.pause();
                }
                lastMediaElement = this;
            }
            return privates.get(this).iframeProxy.callAsyncObserverMethod(MEDIA_PROXY_ID, 'play');
        };

        prototype.setMediaKeys = function(mediaKeys) {
            mediaKeys.orb_setIFrameProxy(privates.get(this).iframeProxy);
            return privates
                .get(this)
                .iframeProxy.callAsyncObserverMethod(MEDIA_PROXY_ID, 'setMediaKeys', [mediaKeys]);
        };

        function makeMethod(name) {
            return function() {
                privates
                    .get(this)
                    .iframeProxy.callObserverMethod(MEDIA_PROXY_ID, name, Array.from(arguments));
            };
        }

        function makeAsyncMethod(name) {
            return function() {
                return privates
                    .get(this)
                    .iframeProxy.callAsyncObserverMethod(
                        MEDIA_PROXY_ID,
                        name,
                        Array.from(arguments)
                    );
            };
        }

        function resetProxySession() {
            const persistentProps = [
                'src',
                'autoplay',
                'controls',
                'playbackRate',
                'volume',
                'muted',
                'loop',
                'defaultMuted',
                'crossOrigin',
                'controlsList',
                'defaultPlaybackRate',
                'disableRemotePlayback',
                'preservesPitch',
            ];
            const p = privates.get(this);
            const properties = {};
            p.iframeProxy.invalidate();
            p.videoDummy.readyState = HTMLMediaElement.HAVE_NOTHING;
            p.videoDummy.error = null;
            p.videoDummy.startDate = new Date(NaN);
            for (const key in p.videoDummy) {
                if (persistentProps.includes(key)) {
                    properties[key] = p.videoDummy[key];
                }
            }
            p.iframeProxy.updateObserverProperties(MEDIA_PROXY_ID, properties);
        }

        // create the HTMLMediaElement's proxy methods
        for (const key of methods) {
            prototype[key] = makeMethod(key);
        }
        for (const key of asyncMethods) {
            prototype[key] = makeAsyncMethod(key);
        }

        // create the HTMLMediaElement's proxy properties
        for (const key of props) {
            Object.defineProperty(prototype, key, {
                set(value) {
                    const p = privates.get(this);
                    if (p.videoDummy[key] !== value) {
                        p.videoDummy[key] = value;
                        p.iframeProxy.updateObserverProperties(MEDIA_PROXY_ID, {
                            [key]: value,
                        });
                    }
                },
                get() {
                    return privates.get(this).videoDummy[key];
                },
            });
        }

        // mandatory step as setAttribute examines the props array
        // before setting a value to an attribute, and we need to be able
        // to change the src property that way as well
        props.push('src');
        Object.defineProperty(prototype, 'src', {
            set(value) {
                const p = privates.get(this);
                if (value !== p.videoDummy.src) {
                    p.videoDummy.src = value;
                    if (value) {
                        console.log(
                            "MediaElementExtension: Setting iframe src property to '" + value + "'."
                        );
                        resetProxySession.call(this);
                        p.iframe.src = ORB_PLAYER_URL + '?base=' + document.baseURI;
                    } else {
                        p.iframeProxy.updateObserverProperties(MEDIA_PROXY_ID, {
                            src: value,
                        });
                    }
                }
            },
            get() {
                return privates.get(this).videoDummy.src;
            },
        });

        props.push('width');
        Object.defineProperty(prototype, 'width', {
            set(value) {
                this.style.width = value + 'px';
            },
            get() {
                return this.getBoundingClientRect().width;
            },
        });

        props.push('height');
        Object.defineProperty(prototype, 'height', {
            set(value) {
                this.style.height = value + 'px';
            },
            get() {
                return this.getBoundingClientRect().height;
            },
        });

        const hiddenOwnProperty = Object.getOwnPropertyDescriptor(HTMLElement.prototype, 'hidden');
        Object.defineProperty(prototype, 'hidden', {
            set(value) {
                const iframe = privates.get(this).iframe;
                iframe.hidden = value;
                hiddenOwnProperty.set.call(this, value);
                hbbtv.utils.matchElementStyle(iframe, this);
            },
            get() {
                return hiddenOwnProperty.get.call(this);
            },
        });

        Object.defineProperty(prototype, 'innerHTML', {
            set(value) {
                privates.get(this).divDummy.innerHTML = value;
            },
            get() {
                return privates.get(this).divDummy.innerHTML;
            },
        });

        for (const key of roProps) {
            Object.defineProperty(prototype, key, {
                get() {
                    return privates.get(this).videoDummy[key];
                },
            });
        }

        Object.defineProperty(prototype, 'duration', {
            get() {
                const videoDummy = privates.get(this).videoDummy;
                if (videoDummy.duration === null) {
                    return Infinity;
                }
                return videoDummy.duration;
            },
        });

        return prototype;
    }

    // Mutation observer
    function addDocumentMutationIntercept() {
        const observer = new MutationObserver(function(mutationsList) {
            for (const mutation of mutationsList) {
                for (const node of mutation.removedNodes) {
                    if (
                        node.nodeName &&
                        (node.nodeName.toLowerCase() === 'video' ||
                            node.nodeName.toLowerCase() === 'audio')
                    ) {
                        const p = privates.get(node);
                        if (p && p.iframe.parentNode) {
                            p.iframe.parentNode.removeChild(p.iframe);
                        }
                    }
                }
                for (const node of mutation.addedNodes) {
                    if (node.nodeName) {
                        if (
                            node.nodeName.toLowerCase() === 'video' ||
                            node.nodeName.toLowerCase() === 'audio'
                        ) {
                            if (!initialise.call(node, node.src)) {
                                for (const child of node.children) {
                                    if (
                                        child.nodeName &&
                                        child.nodeName.toLowerCase() === 'source' &&
                                        initialise.call(node, child.src)
                                    ) {
                                        break;
                                    }
                                }
                            }
                        } else if (
                            node.nodeName.toLowerCase() === 'source' &&
                            node.parentNode &&
                            node.parentNode.nodeName &&
                            (node.parentNode.nodeName.toLowerCase() === 'video' ||
                                node.parentNode.nodeName.toLowerCase() === 'audio')
                        ) {
                            initialise.call(node.parentNode, node.src);
                        }
                    }
                }
            }
        });
        const config = {
            childList: true,
            subtree: true,
        };
        observer.observe(document.documentElement || document.body, config);
    }

    addSourceSetterIntercept();
    addDocumentMutationIntercept();
    const prototype = createPrototype();

    /**
     * Helper class to act as intermediate between MediaElementExtension and
     * IFrameMediaProxy, in order to prevent ping-pong effect when calling
     * a method or updating a property. In addition to that, updates
     * read-only properties of the media element from the other end, and when
     * requested with the MediaElementExtension, return those.
     */
    function VideoDummy(parent, iframeProxy) {
        let _error = null;
        let _timeUpdateTS = Date.now();
        let _currentTime = 0;
        const thiz = this;

        this.startDate = new Date(NaN);
        this.audioTracks = hbbtv.objects.createAudioTrackList(iframeProxy);
        this.videoTracks = hbbtv.objects.createVideoTrackList(iframeProxy);
        this.textTracks = hbbtv.objects.createTextTrackList(parent, iframeProxy);
        this.readyState = mediaOwnProperties.readyState.get.call(parent);
        this.paused = mediaOwnProperties.paused.get.call(parent);
        this.dispatchEvent = function(e) {
            if (e.type === '__orb_startDateUpdated__') {
                this.startDate = new Date(e.startDate);
            } else if (e.type === 'play') {
                _timeUpdateTS = Date.now();
            }
            parent.dispatchEvent(e);
        };
        this.addTextTrack = function() {
            this.textTracks.orb_addTextTrack.apply(this.textTracks, arguments);
        };
        this.setSeekable = function(ranges) {
            this.seekable = hbbtv.objects.createTimeRanges(ranges);
        };

        // Add the textTracks callbacks that were registered before the
        // upgrade of the video element to VideoDummy's textTracks
        const textCallbacks = privates.get(parent.textTracks);
        if (textCallbacks) {
            for (const type in textCallbacks) {
                textCallbacks[type].forEach((value1, value2, set) => {
                    this.textTracks.addEventListener(type, value2);
                    parent.textTracks.removeEventListener(type, value2);
                }, this);
            }
        }
        this.textTracks.onaddtrack = parent.textTracks.onaddtrack;
        hbbtv.utils.defineGetterSetterProperties(parent.textTracks, {
            onaddtrack: {
                set(val) {
                    thiz.textTracks.onaddtrack = val;
                },
                get() {
                    return thiz.textTracks.onaddtrack;
                },
            }
        });

        parent.textTracks.addEventListener = this.textTracks.addEventListener.bind(this.textTracks);
        parent.textTracks.removeEventListener = this.textTracks.removeEventListener.bind(this.textTracks);
        parent.textTracks.dispatchEvent = this.textTracks.dispatchEvent.bind(this.textTracks);

        Object.defineProperty(this, 'error', {
            set(value) {
                if (value) {
                    _error = hbbtv.objects.createMediaError(value.code, value.message);
                } else {
                    _error = value;
                }
            },
            get() {
                return _error;
            },
        });
        Object.defineProperty(this, 'currentTime', {
            set(value) {
                _currentTime = value;
                _timeUpdateTS = Date.now();
            },
            get() {
                return this.paused || this.ended ?
                    _currentTime :
                    _currentTime + (Date.now() - _timeUpdateTS) / 1000.0;
            },
        });
    }

    function initialise(src) {
        if (src && !src.startsWith('blob:') && !this._rdkHolepunch) {
            let p = privates.get(this);
            const thiz = this;
            if (!p) {
                const iframeProxy = hbbtv.objects.createIFrameObjectProxy();
                const videoDummy = new VideoDummy(this, iframeProxy);

                // will be used as a placeholder to store the attributes of
                // the original video element, and after the custom prototype
                // is set, update all the stored properties with the original
                // values
                const initialProps = {};

                // will be used to intercept child addition/removal of
                // the actual video element
                const divDummy = document.createElement('div');

                // extract attributes and reset them before setting the custom prototype
                for (const key of this.getAttributeNames()) {
                    if (props.includes(key)) {
                        initialProps[key] = this.getAttribute(key) || true;
                        this.removeAttribute(key);
                    }
                }
                initialProps.src = src;

                Object.setPrototypeOf(this, prototype);
                privates.set(this, {
                    videoDummy,
                    divDummy,
                    iframeProxy,
                    iframe: document.createElement('iframe'),
                });

                p = privates.get(this);
                iframeProxy.registerObserver(MEDIA_PROXY_ID, videoDummy);

                p.styleObservers = [];

                hbbtv.utils.defineGetterProperties(p.iframe, {
                    orb_mediaElementExtension() {
                        return true;
                    },
                });
                p.iframe.frameBorder = 0;
                p.iframe.allow = 'encrypted-media';
                p.iframe.addEventListener('load', () => {
                    if (thiz.src) {
                        console.log(
                            'MediaElementExtension: initialising iframe with src',
                            thiz.src + '...'
                        );

                        iframeProxy.initiateHandshake(p.iframe.contentWindow).then(() => {
                            console.log(
                                'MediaElementExtension: iframe proxy handshake completed successfully'
                            );
                        });
                    }
                });

                // whenever there is a change on the video element style,
                // update the iframe style as well
                const styleObserver = new MutationObserver(function() {
                    hbbtv.utils.matchElementStyle(p.iframe, thiz);
                });
                styleObserver.observe(this, {
                    attributes: true,
                    attributeFilter: ['style', 'class'],
                });

                // whenever there is a change in the childList of the divDummy,
                // check if the added/removed child is <source>, and if so
                // set/unset the iframe's src property
                const childListObserver = new MutationObserver(function(mutationsList) {
                    for (const mutation of mutationsList) {
                        for (const node of mutation.removedNodes) {
                            if (node.nodeName && node.nodeName.toLowerCase() === 'source') {
                                thiz.removeAttribute('src');
                            }
                        }
                        for (const node of mutation.addedNodes) {
                            if (
                                node.nodeName &&
                                node.nodeName.toLowerCase() === 'source' &&
                                !thiz.src
                            ) {
                                thiz.src = node.src;
                            }
                        }
                    }
                    iframeProxy.updateObserverProperties(MEDIA_PROXY_ID, {
                        innerHTML: divDummy.innerHTML,
                    });
                });
                childListObserver.observe(divDummy, {
                    childList: true,
                });

                // childNodes property is overrided at this point, so we need
                // to get the childNodes from the property descriptor and add
                // them dynamically to the divDummy
                const childNodes = Object.getOwnPropertyDescriptor(
                    Node.prototype,
                    'childNodes'
                ).get.call(this);
                while (childNodes.length) {
                    divDummy.appendChild(childNodes[0]);
                }

                // update the new prototype with the values stored in initialProps
                for (const prop in initialProps) {
                    this[prop] = initialProps[prop];
                }
                console.log('MediaElementExtension: initialised');
            }
            if (this.parentNode && p.iframe.parentNode !== this.parentNode) {
                hbbtv.utils.insertAfter(this.parentNode, p.iframe, this);
                hbbtv.utils.matchElementStyle(p.iframe, this);
            }

            // whenever there is a change on an ancestor style,
            // update the iframe style as well
            const ancestors = [];
            let parent = this.parentNode;
            while (parent) {
                ancestors.push(parent);
                parent = parent.parentNode;
            }

            for (const observer of p.styleObservers) {
                observer.disconnect();
            }
            p.styleObservers = [];

            const observerCallback = function() {
                hbbtv.utils.matchElementStyle(p.iframe, thiz);
            };
            for (const ancestor of ancestors) {
                const styleObserver = new MutationObserver(observerCallback);
                styleObserver.observe(ancestor, {
                    attributes: true,
                    attributeFilter: ['style', 'class'],
                });
                p.styleObservers.push(styleObserver);
            }
            return true;
        }
        console.log('MediaElementExtension: failed to initialise');
        return false;
    }
})();