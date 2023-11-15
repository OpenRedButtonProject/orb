/**
 * @fileOverview TextTrack class
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
hbbtv.objects.TextTrack = (function() {
    const prototype = Object.create(TextTrack.prototype);
    const privates = new WeakMap();
    const events = ['cuechange'];
    const evtTargetMethods = ['addEventListener', 'removeEventListener', 'dispatchEvent'];
    const roProps = ['kind', 'label', 'language', 'activeCues', 'cues', 'id'];
    const props = ['isTTML', 'isEmbedded', 'inBandMetadataTrackDispatchType', 'renderingType'];
    const TRACK_MODE_SHOWING = 'showing';
    const TRACK_MODE_HIDDEN = 'hidden';
    const TRACK_MODE_DISABLED = 'disabled';

    for (const key of events) {
        Object.defineProperty(prototype, 'on' + key, {
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

    for (const key of roProps) {
        Object.defineProperty(prototype, key, {
            get() {
                return privates.get(this).properties[key];
            },
        });
    }

    for (const key of props) {
        Object.defineProperty(prototype, key, {
            set(value) {
                privates.get(this).properties[key] = value;
            },
            get() {
                return privates.get(this).properties[key];
            },
        });
    }

    Object.defineProperty(prototype, 'mode', {
        set(value) {
            const p = privates.get(this);
            if (
                value !== p.properties.enabled && [TRACK_MODE_DISABLED, TRACK_MODE_HIDDEN, TRACK_MODE_SHOWING].includes(value)
            ) {
                if (value !== TRACK_MODE_DISABLED) {
                    for (let track of p.mediaElement.textTracks) {
                        if (track.mode === TRACK_MODE_SHOWING && track !== p.trackProxy) {
                            track.mode = TRACK_MODE_HIDDEN;
                            break;
                        }
                    }
                    p.onTimeUpdate();
                    p.mediaElement.addEventListener('timeupdate', p.onTimeUpdate, true);
                } else {
                    p.properties.activeCues.orb_clear();
                    p.mediaElement.removeEventListener('timeupdate', p.onTimeUpdate, true);
                }
                p.properties.mode = value;
                p.mediaElement.textTracks.dispatchEvent(new Event('change'));
            }
        },
        get() {
            return privates.get(this).properties.mode;
        },
    });

    Object.defineProperty(prototype, 'default', {
        set(value) {
            privates.get(this).properties.default = !!value;
            if (value) {
                this.mode = TRACK_MODE_SHOWING;
            }
        },
        get() {
            return privates.get(this).properties.default;
        },
    });

    function makeEventTargetMethod(name) {
        return function() {
            EventTarget.prototype[name].apply(privates.get(this).eventTarget, arguments);
        };
    }

    for (const func of evtTargetMethods) {
        prototype[func] = makeEventTargetMethod(func);
    }

    prototype.addCue = function(cue) {
        const p = privates.get(this);
        p.properties.cues.orb_addCue(cue);
    };

    prototype.removeCue = function(cue) {
        const p = privates.get(this);
        p.properties.cues.orb_removeCue(cue);
        p.properties.activeCues.orb_removeCue(cue);
    };

    prototype.orb_finalize = function() {
        const p = privates.get(this);
        p.proxy.unregisterObserver(p.observerId);
        p.activeCues.orb_clear();
        p.cues.orb_clear();
        p.mediaElement.removeEventListener('timeupdate', p.onTimeUpdate, true);
        privates.delete(this);
    };

    function initialise(mediaElement, proxy, id, kind, label, language) {
        const thiz = this;
        const observerId = 'TextTrack_' + id;
        const properties = {
            id,
            kind,
            label,
            language,
            renderingType: 'html',
            cues: hbbtv.objects.createTextTrackCueList(),
            activeCues: hbbtv.objects.createTextTrackCueList(),
            mode: TRACK_MODE_DISABLED,
            default: false,
        };

        proxy.registerObserver(observerId, this);

        // We create a new Proxy object which we return in order to avoid ping-pong calls
        // between the iframe and the main window when the user requests a property update
        // or a function call.
        const trackProxy = new Proxy(this, {
            get: (target, property) => {
                if (typeof target[property] === 'function') {
                    if (!evtTargetMethods.includes(property)) {
                        return function() {
                            let args = [];
                            if (property !== 'addCue') {
                                args = Array.from(arguments);
                            } else {
                                const cueObj = {};
                                for (const key in arguments[0]) {
                                    let cueProp = arguments[0][key];
                                    if (typeof cueProp !== 'function') {
                                        if (key === 'data' && cueProp) {
                                            cueProp = [...Object.keys(cueProp).map(k => cueProp[k])];
                                        }
                                        cueObj[key] = cueProp;
                                    }
                                }
                                args.push(cueObj);
                            }
                            proxy.callObserverMethod(observerId, property, args);
                            return target[property].apply(target, arguments);
                        };
                    }
                    return target[property].bind(target);
                }
                return target[property];
            },
            set: (target, property, value) => {
                if (typeof target[property] !== 'function') {
                    proxy.updateObserverProperties(observerId, {
                        [property]: value,
                    });
                }
                target[property] = value;
                return true;
            },
        });

        privates.set(this, {
            length: 0,
            initialized: true,
            eventTarget: document.createDocumentFragment(),
            onTimeUpdate: (e) => {
                const time = mediaElement.currentTime;
                let changed = false;
                for (let i = properties.activeCues.length - 1; i >= 0; --i) {
                    const cue = properties.activeCues[i];
                    if (cue.endTime < time || cue.startTime > time) {
                        properties.activeCues.orb_removeCueAt(i);
                        if (typeof cue.onexit === 'function') {
                            cue.onexit();
                        }
                        changed = true;
                    }
                }
                for (const cue of properties.cues) {
                    if (
                        cue.endTime >= time &&
                        cue.startTime <= time &&
                        properties.activeCues.orb_indexOf(cue) < 0
                    ) {
                        properties.activeCues.orb_addCue(cue);
                        if (typeof cue.onenter === 'function') {
                            cue.onenter();
                        }
                        changed = true;
                    }
                }
                if (changed) {
                    thiz.dispatchEvent(new Event('cuechange'));
                }
            },
            properties,
            mediaElement,
            observerId,
            proxy,
            trackProxy,
        });

        this.mode = TRACK_MODE_HIDDEN;
        return trackProxy;
    }

    return {
        prototype,
        initialise,
    };
})();

hbbtv.objects.createTextTrack = function(mediaElement, proxy, index, kind, label, language) {
    const track = Object.create(hbbtv.objects.TextTrack.prototype);
    return hbbtv.objects.TextTrack.initialise.call(
        track,
        mediaElement,
        proxy,
        index,
        kind,
        label,
        language
    );
};