/**
 * @fileOverview VideoTrackList class
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
hbbtv.objects.VideoTrackList = (function() {
    const listProto = {};
    const trackProto = {};
    const privates = new WeakMap();
    const events = ['change', 'addtrack', 'removetrack'];
    const evtTargetMethods = ['addEventListener', 'removeEventListener', 'dispatchEvent'];
    const trackProps = ['index', 'id', 'kind', 'label', 'language', 'encoding', 'encrypted'];

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

    Object.defineProperty(listProto, 'selectedIndex', {
        get() {
            for (const track of this) {
                if (track.selected) {
                    return track.index;
                }
            }
            return -1;
        },
    });

    Object.defineProperty(trackProto, 'selected', {
        get() {
            return privates.get(this).properties.selected;
        },
        set(value) {
            const p = privates.get(this);
            if (value !== p.properties.selected) {
                if (value) {
                    for (let track of this) {
                        if (track.selected && track !== this) {
                            track.selected = false;
                            break;
                        }
                    }
                }
                p.properties.selected = !!value;
                this.dispatchEvent(new Event('change'));
            }
        },
    });

    listProto[Symbol.iterator] = function*() {
        for (let i = 0; i < this.length; ++i) {
            yield this[i];
        }
    };

    listProto.getTrackById = function(id) {
        for (const track of this) {
            if (track.id === id.toString()) {
                return track;
            }
        }
    };

    listProto.orb_setTrackList = function(trackList) {
        const p = privates.get(this);
        for (let i = trackList.length; i < this.length; ++i) {
            delete this[i];
        }
        for (let i = 0; i < trackList.length; ++i) {
            this[i] = makeVideoTrack(this, i, trackList[i]);
        }
        privates.get(this).length = trackList.length;
    };

    listProto.orb_appendTrack = function(track) {
        const p = privates.get(this);
        const t = makeVideoTrack(this, p.length, track);
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

    function makeVideoTrack(trackList, index, properties) {
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

hbbtv.objects.createVideoTrackList = function(proxy) {
    const trackList = Object.create(hbbtv.objects.VideoTrackList.prototype);
    return hbbtv.objects.VideoTrackList.initialise.call(trackList, proxy);
};