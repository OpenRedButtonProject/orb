/**
 * @fileOverview AdioTrackList class
 * @license ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */
hbbtv.objects.AudioTrackList = (function() {
    const AUDIO_TRACK_KEY_PREFIX = 'AudioTrack_';
    const listProto = {};
    const trackProto = {};
    const privates = new WeakMap();
    const events = ['change', 'addtrack', 'removetrack'];
    const evtTargetMethods = ['addEventListener', 'removeEventListener', 'dispatchEvent'];
    const trackProps = [
        'index',
        'id',
        'kind',
        'label',
        'language',
        'encoding',
        'encrypted',
        'numChannels',
    ];

    Object.defineProperty(listProto, 'length', {
        get() {
            return privates.get(this).length;
        },
    });

    for (const key of events) {
        Object.defineProperty(listProto, 'on' + key, {
            set(callback) {
                const p = privates.get(this);
                if (p['on' + key]) {
                    p.eventTarget.removeEventListener(key, p['on' + key]);
                }

                if (callback instanceof Object) {
                    p['on' + key] = callback;
                    if (callback) {
                        p.eventTarget.addEventListener(key, callback);
                    }
                } else {
                    p['on' + key] = null;
                }
            },
            get() {
                return privates.get(this)['on' + key];
            },
        });
    }

    function makeEventTargetMethod(name) {
        return function() {
            EventTarget.prototype[name].apply(privates.get(this).eventTarget, arguments);
        };
    }

    for (const func of evtTargetMethods) {
        listProto[func] = makeEventTargetMethod(func);
    }

    for (const key of trackProps) {
        Object.defineProperty(trackProto, key, {
            get() {
                return privates.get(this).properties[key];
            },
        });
    }

    Object.defineProperty(trackProto, 'enabled', {
        get() {
            return privates.get(this).properties.enabled;
        },
        set(value) {
            const p = privates.get(this);
            if (value !== p.properties.enabled) {
                if (value) {
                    for (let track of p.trackList) {
                        if (track.enabled && track !== this) {
                            track.enabled = false;
                            break;
                        }
                    }
                }
                p.properties.enabled = !!value;
                p.trackList.dispatchEvent(new Event('change'));
            }
        },
    });

    listProto.getTrackById = function(id) {
        for (const track of this) {
            if (track.id === id.toString()) {
                return track;
            }
        }
    };

    listProto[Symbol.iterator] = function*() {
        for (let i = 0; i < this.length; ++i) {
            yield this[i];
        }
    };

    listProto.orb_setTrackList = function(trackList) {
        let p = privates.get(this);
        let preferredAudioLanguageTrack = null;
        let cleanAudioTrack = null;
        for (let i = trackList.length; i < this.length; ++i) {
            p.proxy.unregisterObserver(AUDIO_TRACK_KEY_PREFIX + i);
            delete this[i];
        }

        for (let i = 0; i < trackList.length; ++i) {
            this[i] = makeAudioTrack(this, i, trackList[i]);
            if (p.defaultAudioLanguage) {
                if (this[i].language === p.defaultAudioLanguage ||
                    hbbtv.languageCodes.ISO639_2_to_ISO639_1[this[i].language] === p.defaultAudioLanguage ||
                    hbbtv.languageCodes.ISO639_2_to_ISO639_1[p.defaultAudioLanguage] === this[i].language) {
                        preferredAudioLanguageTrack = this[i];
                }
            }
            if (this[i].kind === "alternative" && trackList[i].accessibility === 2) {
                cleanAudioTrack = this[i];
            }
        }

        privates.get(this).length = trackList.length;

        if (preferredAudioLanguageTrack) {
            preferredAudioLanguageTrack.enabled = true;
            p.defaultAudioLanguage = null; // the property is valid only during first playback
        }

        if (p.cleanAudioEnabled && cleanAudioTrack) {
            cleanAudioTrack.enabled = true;
        } else if (!p.cleanAudioEnabled && cleanAudioTrack !== null) {
            cleanAudioTrack.enabled = false;
        }
    };

    listProto.orb_appendTrack = function(track) {
        const p = privates.get(this);
        const t = makeAudioTrack(this, p.length, track);
        this[p.length++] = t;
        p.eventTarget.dispatchEvent(new TrackEvent('addtrack'));
    };

    listProto.orb_removeTrackAt = function(index) {
        const p = privates.get(this);
        if (index >= 0 && index < p.length) {
            // TODO: update all tracks indexes and keys with the iframe proxy
            for (let i = index + 1; i < p.length; i++) {
                this[i - 1] = this[i];
            }
            delete this[--p.length];
            p.eventTarget.dispatchEvent(new TrackEvent('removetrack'));
        }
    };

    function makeAudioTrack(trackList, index, properties) {
        const track = Object.create(trackProto);
        const proxy = privates.get(trackList).proxy;
        proxy.registerObserver(AUDIO_TRACK_KEY_PREFIX + index, track);
        privates.set(track, {
            trackList,
            properties,
        });

        // We create a new Proxy object which we return in order to avoid ping-pong calls
        // between the iframe and the main window when the user requests a property update
        // or a function call.
        const trackProxy = new Proxy(track, {
            get: (target, property) => {
                return target[property];
            },
            set: (target, property, value) => {
                if (property === 'enabled') {
                    proxy.updateObserverProperties(AUDIO_TRACK_KEY_PREFIX + index, {
                        [property]: value,
                    });
                }
                target[property] = value;
                return true;
            },
        });
        return trackProxy;
    }

    function initialise(proxy) {
        const AUDIO_TRACK_LIST_KEY = 'AudioTrackList';
        privates.set(this, {
            length: 0,
            eventTarget: document.createDocumentFragment(),
            defaultAudioLanguage: hbbtv.bridge.configuration.getPreferredAudioLanguage(),
            cleanAudioEnabled: hbbtv.bridge.configuration.getCleanAudioEnabled(),
            proxy,
        });
        proxy.registerObserver(AUDIO_TRACK_LIST_KEY, this);

        // We create a new Proxy object which we return in order to avoid ping-pong calls
        // between the iframe and the main window when the user requests a property update
        // or a function call.
        const tracksProxy = new Proxy(this, {
            get: (target, property) => {
                if (typeof target[property] === 'function') {
                    if (!evtTargetMethods.includes(property)) {
                        return function() {
                            proxy.callObserverMethod(
                                AUDIO_TRACK_LIST_KEY,
                                property,
                                Array.from(arguments)
                            );
                            return target[property].apply(target, arguments);
                        };
                    }
                    return target[property].bind(target);
                }
                return target[property];
            },
            set: (target, property, value) => {
                if (typeof target[property] !== 'function') {
                    proxy.updateObserverProperties(AUDIO_TRACK_LIST_KEY, {
                        [property]: value,
                    });
                }
                target[property] = value;
                return true;
            },
        });
        return tracksProxy;
    }

    return {
        prototype: listProto,
        initialise: initialise,
    };
})();

hbbtv.objects.createAudioTrackList = function(proxy) {
    const trackList = Object.create(hbbtv.objects.AudioTrackList.prototype);
    return hbbtv.objects.AudioTrackList.initialise.call(trackList, proxy);
};