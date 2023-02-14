/**
 * @fileOverview OIPF Gateway Info object
 * @license ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

hbbtv.objects.OipfGatewayInfo = (function() {
    const prototype = Object.create(HTMLObjectElement.prototype);
    const privates = new WeakMap();
    const gGarbageCollectionBlocked = new Set();

    /* readonly properties */
    hbbtv.utils.defineGetterProperties(prototype, {
        isIGDiscovered() {
            return false;
        },
        isAGDiscovered() {
            return false;
        },
        isCSPGCIPlusDiscovered() {
            return hbbtv.drmManager.isCSPGCIPlusDiscovered();
        },
        isCSPGDTCPDiscovered() {
            return false;
        },
        igURL() {
            return 'igURL';
        },
        agURL() {
            return 'agURL';
        },
        cspgDTCPURL() {
            return 'cspgDTCPURL';
        },
        interval() {
            return 30;
        },
        isIGSupported() {
            return false;
        },
        isAGSupported() {
            return false;
        },
        isCSPGCIPlusSupported() {
            return true;
        },
        isCSPGDTCPSupported() {
            return false;
        },
        CSPGCIPlusDRMType() {
            let result = [];
            const status = hbbtv.drmManager.getCSPGCIPlusStatus();
            if (status) {
                status.DRMSystemIDs.forEach((value) => {
                    result.push(value.DRMSystemID);
                });
            }
            return result;
        },
    });

    prototype.isIGSupportedMethod = function(methodName) {
        return false;
    };

    // DOM level 1 event methods
    prototype.addEventListener = function(type, listener) {
        if (privates.get(this).eventDispatcher.addCountedEventListener(type, listener) > 0) {
            gGarbageCollectionBlocked.add(this);
        }
    };

    prototype.removeEventListener = function(type, listener) {
        if (privates.get(this).eventDispatcher.removeCountedEventListener(type, listener) == 0) {
            gGarbageCollectionBlocked.delete(this);
        }
    };

    // DOM level 0 event properties
    Object.defineProperty(prototype, 'onDiscoverIG', {
        get() {
            return privates.get(this).onDiscoverIGDomLevel0;
        },
        set(listener) {
            const p = privates.get(this);
            if (p.onDiscoverIGDomLevel0) {
                this.removeEventListener('DiscoverIG', p.onDiscoverIGWrapper);
                p.onDiscoverIGWrapper = null;
            }
            p.onDiscoverIGDomLevel0 = listener;
            if (listener) {
                p.onDiscoverIGWrapper = (ev) => {
                    listener(ev.channel, ev.errorState);
                };
                this.addEventListener('DiscoverIG', p.onDiscoverIGWrapper);
            }
        },
    });

    Object.defineProperty(prototype, 'onDiscoverAG', {
        get() {
            return privates.get(this).onDiscoverAGDomLevel0;
        },
        set(listener) {
            const p = privates.get(this);
            if (p.onDiscoverAGDomLevel0) {
                this.removeEventListener('DiscoverAG', p.onDiscoverAGWrapper);
                p.onDiscoverAGWrapper = null;
            }
            p.onDiscoverAGDomLevel0 = listener;
            if (listener) {
                p.onDiscoverAGWrapper = (ev) => {
                    listener(ev.channel, ev.errorState);
                };
                this.addEventListener('DiscoverAG', p.onDiscoverAGWrapper);
            }
        },
    });

    Object.defineProperty(prototype, 'onDiscoverCSPGDTCP', {
        get() {
            return privates.get(this).onDiscoverCSPGDTCPDomLevel0;
        },
        set(listener) {
            const p = privates.get(this);
            if (p.onDiscoverCSPGDTCPDomLevel0) {
                this.removeEventListener('DiscoverCSPGDTCP', p.onDiscoverCSPGDTCPWrapper);
                p.onDiscoverCSPGDTCPWrapper = null;
            }
            p.onDiscoverCSPGDTCPDomLevel0 = listener;
            if (listener) {
                p.onDiscoverCSPGDTCPWrapper = (ev) => {
                    listener(ev.channel, ev.errorState);
                };
                this.addEventListener('DiscoverCSPGDTCP', p.onDiscoverCSPGDTCPWrapper);
            }
        },
    });

    Object.defineProperty(prototype, 'onDiscoverCSPGCIPlus', {
        get() {
            return privates.get(this).onDiscoverCSPGCIPlusDomLevel0;
        },
        set(listener) {
            const p = privates.get(this);
            if (p.onDiscoverCSPGCIPlusDomLevel0) {
                this.removeEventListener('DiscoverCSPGCIPlus', p.onDiscoverCSPGCIPlusWrapper);
                p.onDiscoverCSPGCIPlusWrapper = null;
            }
            p.onDiscoverCSPGCIPlusDomLevel0 = listener;
            if (listener) {
                p.onDiscoverCSPGCIPlusWrapper = (ev) => {
                    listener(ev.channel, ev.errorState);
                };
                this.addEventListener('DiscoverCSPGCIPlus', p.onDiscoverCSPGCIPlusWrapper);
            }
        },
    });

    function dispatchDiscoverIGEvent() {
        const event = new Event('DiscoverIG');
        privates.get(this).eventDispatcher.dispatchEvent(event);
    }

    function dispatchDiscoverAGEvent() {
        const event = new Event('DiscoverAG');
        privates.get(this).eventDispatcher.dispatchEvent(event);
    }

    function dispatchDiscoverCSPGDTCPEvent() {
        const event = new Event('DiscoverCSPGDTCP');
        privates.get(this).eventDispatcher.dispatchEvent(event);
    }

    function dispatchDiscoverCSPGCIPlusEvent() {
        const event = new Event('DiscoverCSPGCIPlus');
        privates.get(this).eventDispatcher.dispatchEvent(event);
    }

    function initialise() {
        privates.set(this, {});
        const p = privates.get(this);
        p.eventDispatcher = new hbbtv.utils.EventDispatcher(this);
        hbbtv.drmManager.registerOipfGatewayInfo(this, dispatchDiscoverCSPGCIPlusEvent);
    }

    return {
        prototype: prototype,
        initialise: initialise,
    };
})();

hbbtv.objects.upgradeToOipfGatewayInfo = function(object) {
    Object.setPrototypeOf(object, hbbtv.objects.OipfGatewayInfo.prototype);
    hbbtv.objects.OipfGatewayInfo.initialise.call(object);
};

hbbtv.objectManager.registerObject({
    name: 'application/oipfgatewayinfo',
    mimeTypes: ['application/oipfgatewayinfo'],
    oipfObjectFactoryMethodName: 'createGatewayInfoObject',
    upgradeObject: hbbtv.objects.upgradeToOipfGatewayInfo,
});