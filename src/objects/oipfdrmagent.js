/**
 * @fileOverview OIPF DRM Agent object.
 * @license ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

hbbtv.objects.OipfDrmAgent = (function() {
    const prototype = Object.create(HTMLObjectElement.prototype);
    const privates = new WeakMap();
    const gGarbageCollectionBlocked = new Set();

    prototype.sendDRMMessage = function(msgType, msg, DRMSystemID) {
        return hbbtv.drmManager.sendDRMMessage(msgType, msg, DRMSystemID);
    };

    prototype.DRMSystemStatus = function(DRMSystemID) {
        return hbbtv.drmManager.getDRMSystemStatus(DRMSystemID);
    };

    prototype.canPlayContent = function(DRMPrivateData, DRMSystemID) {
        return hbbtv.drmManager.canPlayContent(DRMPrivateData, DRMSystemID);
    };

    prototype.canRecordContent = function(DRMPrivateData, DRMSystemID) {
        return hbbtv.drmManager.canRecordContent(DRMPrivateData, DRMSystemID);
    };

    prototype.setActiveDRM = function(DRMSystemID) {
        return hbbtv.drmManager.setActiveDRM(DRMSystemID);
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
    Object.defineProperty(prototype, 'onDRMSystemStatusChange', {
        get() {
            return privates.get(this).onDRMSystemStatusChangeDomLevel0;
        },
        set(listener) {
            const p = privates.get(this);
            if (p.onDRMSystemStatusChangeDomLevel0) {
                this.removeEventListener('DRMSystemStatusChange', p.onDRMSystemStatusChangeWrapper);
                p.onDRMSystemStatusChangeWrapper = null;
            }
            p.onDRMSystemStatusChangeDomLevel0 = listener;
            if (listener) {
                p.onDRMSystemStatusChangeWrapper = (ev) => {
                    listener(ev.DRMSystemID);
                };
                this.addEventListener('DRMSystemStatusChange', p.onDRMSystemStatusChangeWrapper);
            }
        },
    });

    Object.defineProperty(prototype, 'onDRMMessageResult', {
        get() {
            return privates.get(this).onDRMMessageResultDomLevel0;
        },
        set(listener) {
            const p = privates.get(this);
            if (p.onDRMMessageResultDomLevel0) {
                this.removeEventListener('DRMMessageResult', p.onDRMMessageResultWrapper);
                p.onDRMMessageResultWrapper = null;
            }
            p.onDRMMessageResultDomLevel0 = listener;
            if (listener) {
                p.onDRMMessageResultWrapper = (ev) => {
                    listener(ev.msgID, ev.resultMsg, ev.resultCode);
                };
                this.addEventListener('DRMMessageResult', p.onDRMMessageResultWrapper);
            }
        },
    });

    Object.defineProperty(prototype, 'onDRMSystemMessage', {
        get() {
            return privates.get(this).onDRMSystemMessageDomLevel0;
        },
        set(listener) {
            const p = privates.get(this);
            if (p.onDRMSystemMessageDomLevel0) {
                this.removeEventListener('DRMSystemMessage', p.onDRMSystemMessageWrapper);
                p.onDRMSystemMessageWrapper = null;
            }
            p.onDRMSystemMessageDomLevel0 = listener;
            if (listener) {
                p.onDRMSystemMessageWrapper = (ev) => {
                    listener(ev.msg, ev.DRMSystemID);
                };
                this.addEventListener('DRMSystemMessage', p.onDRMSystemMessageWrapper);
            }
        },
    });

    function dispatchDRMSystemStatusChange(DRMSystemID) {
        const event = new Event('DRMSystemStatusChange');
        Object.assign(event, {
            DRMSystemID: DRMSystemID,
        });
        privates.get(this).eventDispatcher.dispatchEvent(event);
    }

    function dispatchDRMMessageResult(msgID, resultMsg, resultCode) {
        const ev = new Event('DRMMessageResult');
        Object.assign(ev, {
            msgID: msgID,
            resultMsg: resultMsg,
            resultCode: resultCode,
        });
        privates.get(this).eventDispatcher.dispatchEvent(ev);
    }

    function dispatchDRMSystemMessage(msg, DRMSystemID) {
        const ev = new Event('DRMSystemMessage');
        Object.assign(ev, {
            msg: msg,
            DRMSystemID: DRMSystemID,
        });
        privates.get(this).eventDispatcher.dispatchEvent(ev);
    }

    function initialise() {
        privates.set(this, {});
        const p = privates.get(this);
        p.eventDispatcher = new hbbtv.utils.EventDispatcher(this);

        hbbtv.drmManager.registerOipfDrmAgent(
            this,
            dispatchDRMSystemStatusChange,
            dispatchDRMMessageResult,
            dispatchDRMSystemMessage
        );
    }

    return {
        prototype: prototype,
        initialise: initialise,
    };
})();

hbbtv.objects.upgradeToOipfDrmAgent = function(object) {
    Object.setPrototypeOf(object, hbbtv.objects.OipfDrmAgent.prototype);
    hbbtv.objects.OipfDrmAgent.initialise.call(object);
};

hbbtv.objectManager.registerObject({
    name: 'application/oipfdrmagent',
    mimeTypes: ['application/oipfdrmagent'],
    oipfObjectFactoryMethodName: 'createDrmAgentObject',
    upgradeObject: hbbtv.objects.upgradeToOipfDrmAgent,
});