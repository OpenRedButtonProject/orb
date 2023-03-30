/**
 * @fileOverview TextTrackList class
 * @license ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */
hbbtv.objects.TextTrackList = (function() {
    const prototype = Object.create(TextTrackList.prototype);
    const privates = new WeakMap();
    const events = ['change', 'addtrack', 'removetrack'];
    const evtTargetMethods = ['addEventListener', 'removeEventListener', 'dispatchEvent'];

    Object.defineProperty(prototype, 'length', {
        get() {
            return privates.get(this).length;
        },
    });

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

    function makeEventTargetMethod(name) {
        return function() {
            EventTarget.prototype[name].apply(privates.get(this).eventTarget, arguments);
        };
    }

    for (const func of evtTargetMethods) {
        prototype[func] = makeEventTargetMethod(func);
    }

    prototype[Symbol.iterator] = function*() {
        for (let i = 0; i < this.length; ++i) {
            yield this[i];
        }
    };

    prototype.getTrackById = function(id) {
        for (const track of this) {
            if (track.id === id.toString()) {
                return track;
            }
        }
    };

    prototype.orb_addTextTrack = function(kind, label, language) {
        const p = privates.get(this);
        const track = hbbtv.objects.createTextTrack(
            p.mediaElement,
            p.proxy,
            label !== undefined ? label.toString() : '',
            kind,
            label,
            language
        );
        this[p.length++] = track;
        const evt = new TrackEvent('addtrack');
        Object.defineProperty(evt, 'track', {
            value: track,
            writable: false,
        });
        p.eventTarget.dispatchEvent(evt);
        return track;
    };

    prototype.orb_removeTrackAt = function(index) {
        const p = privates.get(this);
        if (index >= 0 && index < p.length) {
            this[i].orb_finalize();
            for (let i = index + 1; i < p.length; i++) {
                // TODO: update track indexes and registration with the proxy
                this[i - 1] = this[i];
            }
            delete this[--p.length];
            p.eventTarget.dispatchEvent(new TrackEvent('removetrack'));
        }
    };

    function initialise(mediaElement, proxy) {
        const TEXT_TRACK_LIST_KEY = 'TextTrackList';
        privates.set(this, {
            length: 0,
            eventTarget: document.createDocumentFragment(),
            proxy,
            mediaElement,
        });
        proxy.registerObserver(TEXT_TRACK_LIST_KEY, this);

        // We create a new Proxy object which we return in order to avoid ping-pong calls
        // between the iframe and the main window when the user requests a property update
        // or a function call.
        const tracksProxy = new Proxy(this, {
            get: (target, property) => {
                if (typeof target[property] === 'function') {
                    if (!evtTargetMethods.includes(property)) {
                        return function() {
                            proxy.callObserverMethod(
                                TEXT_TRACK_LIST_KEY,
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
                    proxy.updateObserverProperties(TEXT_TRACK_LIST_KEY, {
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
        prototype: prototype,
        initialise: initialise,
    };
})();

hbbtv.objects.createTextTrackList = function(mediaElement, proxy) {
    const trackList = Object.create(hbbtv.objects.TextTrackList.prototype);
    return hbbtv.objects.TextTrackList.initialise.call(trackList, mediaElement, proxy);
};