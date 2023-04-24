/**
 * @fileOverview BroadcastObserver class
 * @license ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

hbbtv.objects.BroadcastObserver = (function() {
    const prototype = {};
    const privates = new WeakMap();

    hbbtv.utils.defineGetterProperties(prototype, {
        contentTime() {
            let ticks = this.contentTicks;
            if (!isNaN(ticks)) {
                return ticks / this.timeline.timelineProperties.unitsPerSecond;
            }
            return NaN;
        },
        timelineSpeedMultiplier() {
            return 1;
        },
        contentTicks() {
            if (this.timeline) {
                const ret = hbbtv.bridge.mediaSync.getBroadcastCurrentTime(
                    this.timeline.timelineSelector
                );
                if (ret >= 0) {
                    return ret;
                }
            }
            return NaN;
        },
        tickRate() {
            if (this.timeline) {
                return this.timeline.timelineProperties.unitsPerSecond;
            }
            return NaN;
        },
    });

    hbbtv.utils.defineGetterSetterProperties(prototype, {
        timeline: {
            get() {
                return privates.get(this).timeline;
            },
            set(val) {
                privates.get(this).timeline = val;
            },
        },
    });

    hbbtv.utils.defineConstantProperties(prototype, {
        PLAY_STATE_UNREALIZED: 0,
        PLAY_STATE_CONNECTING: 1,
        PLAY_STATE_PRESENTING: 2,
        PLAY_STATE_STOPPED: 3,
    });

    prototype.start = function() {
        const p = privates.get(this);
        let ret = true;
        if (!p.running) {
            p.running = true;
            p.mediaObject.addEventListener('PlayStateChange', p.onMediaObjectEvent);
        }
        return ret;
    };

    prototype.stop = function() {
        const p = privates.get(this);
        if (p.running) {
            p.running = false;
            p.mediaObject.removeEventListener('PlayStateChange', p.onMediaObjectEvent);
        }
    };

    prototype.addEventListener = function(type, listener) {
        privates.get(this).eventTarget.addEventListener(type, listener);
    };

    prototype.removeEventListener = function(type, listener) {
        privates.get(this).eventTarget.removeEventListener(type, listener);
    };

    function onMediaObjectEvent(e) {
        const p = privates.get(this);
        if (e.state === this.PLAY_STATE_STOPPED || e.state === this.PLAY_STATE_UNREALIZED) {
            p.eventTarget.dispatchEvent(new Event('Error'));
        } else {
            const event = new Event('MediaUpdated');
            event.data = {
                contentTime: this.contentTime,
                timelineSpeedMultiplier: this.timelineSpeedMultiplier,
            };
            p.eventTarget.dispatchEvent(event);
        }
    }

    function initialise(mediaObject) {
        privates.set(this, {
            mediaObject: mediaObject,
            eventTarget: document.createDocumentFragment(),
            onMediaObjectEvent: onMediaObjectEvent.bind(this),
            running: false,
        });
    }

    return {
        prototype: prototype,
        initialise: initialise,
    };
})();

hbbtv.objects.createBroadcastObserver = function(mediaObject) {
    const observer = Object.create(hbbtv.objects.BroadcastObserver.prototype);
    hbbtv.objects.BroadcastObserver.initialise.call(observer, mediaObject);
    return observer;
};