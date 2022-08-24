/**
 * @fileOverview The application/oipfSearchManager embedded object.
 * See: {@link https://web.archive.org/web/20200219165053/http://www.oipf.tv/web-spec/volume5.html#application-oipfsearchmanager}
 * @license ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

hbbtv.objects.OipfSearchManager = (function() {
    const prototype = Object.create(HTMLObjectElement.prototype);
    const privates = new WeakMap();
    const gGarbageCollectionBlocked = new Set();

    Object.defineProperty(prototype, 'broadcastIndependent', {
        enumerable: false,
        configurable: false,
        get() {
            return privates.get(this).broadcastIndependent;
        },
        set(broadcastIndependent) {
            privates.get(this).broadcastIndependent = broadcastIndependent;
        },
    });

    prototype.createSearch = function(searchTarget) {
        const p = privates.get(this);
        mandatoryBroadcastRelatedSecurityCheck(p);
        if (searchTarget !== 1) {
            return undefined;
        } else {
            return hbbtv.objects.createMetadataSearch(this, searchTarget);
        }
    };

    prototype.getChannelConfig = function() {
        const p = privates.get(this);
        mandatoryBroadcastRelatedSecurityCheck(p);
        if (p.channelConfig == null) {
            try {
                p.channelConfig = hbbtv.objects.createChannelConfig();
            } catch (e) {
                if (e.name === 'SecurityError') {
                    p.broadcastIndependent = true;
                }
                throw e;
            }
        }
        return p.channelConfig;
    };

    // DOM level 1 event methods
    prototype.addEventListener = function(type, listener) {
        noRestrictionSecurityCheck();
        if (privates.get(this).eventDispatcher.addCountedEventListener(type, listener) > 0) {
            gGarbageCollectionBlocked.add(this);
        }
    };

    prototype.removeEventListener = function(type, listener) {
        noRestrictionSecurityCheck();
        if (privates.get(this).eventDispatcher.removeCountedEventListener(type, listener) == 0) {
            gGarbageCollectionBlocked.delete(this);
        }
    };

    // DOM level 0 event properties
    Object.defineProperty(prototype, 'onMetadataSearch', {
        get() {
            return privates.get(this).onMetadataSearchDomLevel0;
        },
        set(listener) {
            const p = privates.get(this);
            if (p.onMetadataSearchDomLevel0) {
                this.removeEventListener('MetadataSearch', p.onMetadataSearchWrapper);
                p.onMetadataSearchWrapper = null;
            }
            p.onMetadataSearchDomLevel0 = listener;
            if (listener) {
                p.onMetadataSearchWrapper = (ev) => {
                    listener(ev.search, ev.state);
                };
                this.addEventListener('MetadataSearch', p.onMetadataSearchWrapper);
            }
        },
    });

    /** Broadcast-independent applications: shall throw a "Security Error" */
    function mandatoryBroadcastRelatedSecurityCheck(p) {
        if (p.broadcastIndependent) {
            throw new DOMException('', 'SecurityError');
        }
    }

    /** Broadcast-independent applications: shall have no restrictions */
    function noRestrictionSecurityCheck() {
        /* noop */
    }

    function dispatchMetadataSearch(search, state) {
        const p = privates.get(this);
        mandatoryBroadcastRelatedSecurityCheck(p);
        const event = new Event('MetadataSearch');
        Object.assign(event, {
            search: search,
            state: state,
        });
        p.eventDispatcher.dispatchEvent(event);
    }

    function initialise() {
        /* TODO: Follow security model of section 10 (in particular section 10.1.3.6) => TLS handshake
         * through a valid X.509v3 certificate */
        privates.set(this, {});
        const p = privates.get(this);
        /* Note: We asume App is broadcast related until proven wrong with a SecurityError */
        p.broadcastIndependent = false;
        p.eventDispatcher = new hbbtv.utils.EventDispatcher(this);
        p.channelConfig = this.getChannelConfig();
    }

    return {
        prototype: prototype,
        initialise: initialise,
        dispatchMetadataSearch: dispatchMetadataSearch,
    };
})();

hbbtv.objects.upgradeToOipfSearchManager = function(object) {
    Object.setPrototypeOf(object, hbbtv.objects.OipfSearchManager.prototype);
    hbbtv.objects.OipfSearchManager.initialise.call(object);
};

hbbtv.objectManager.registerObject({
    name: 'application/oipfsearchmanager',
    mimeTypes: ['application/oipfsearchmanager'],
    oipfObjectFactoryMethodName: 'createSearchManagerObject',
    upgradeObject: function(object) {
        hbbtv.objects.upgradeToOipfSearchManager(object);
    },
});