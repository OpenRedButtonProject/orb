/**
 * @fileOverview AdioTrackList class
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
hbbtv.objects.AudioTrackList = (function() {
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
                    for (let track of this) {
                        if (track.enabled && track !== this) {
                            track.enabled = false;
                            break;
                        }
                    }
                }
                p.properties.enabled = !!value;
                this.dispatchEvent(new Event('change'));
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
            delete this[i];
        }

        for (let i = 0; i < trackList.length; ++i) {
            this[i] = makeAudioTrack(this, i, trackList[i]);
            if (p.defaultAudioLanguage) {
                let trackLanguage = this[i].language.includes('-') ? this[i].language.split('-')[0] : this[i].language;
                if (trackLanguage === p.defaultAudioLanguage ||
                    hbbtv.languageCodes.ISO639_2_to_ISO639_1[trackLanguage] === p.defaultAudioLanguage ||
                    hbbtv.languageCodes.ISO639_2_to_ISO639_1[p.defaultAudioLanguage] === trackLanguage) {
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
            for (let i = index + 1; i < p.length; i++) {
                this[i - 1] = this[i];
            }
            delete this[--p.length];
            p.eventTarget.dispatchEvent(new TrackEvent('removetrack'));
        }
    };

    function makeAudioTrack(trackList, index, properties) {
        const track = Object.create(trackProto);
        privates.set(track, {
            trackList,
            properties,
        });
        return track;
    }

    function initialise(proxy) {
        privates.set(this, {
            length: 0,
            eventTarget: document.createDocumentFragment()
        });
        return this;
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