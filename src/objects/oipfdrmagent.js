/**
 * @fileOverview OIPF DRM Agent object.
 * See: {@link https://web.archive.org/web/20200219165053/http://www.oipf.tv/web-spec/volume5.html#application-oipfdrmagent}
 * @preserve Copyright (c) Ocean Blue Software Ltd.
 * @license MIT (see LICENSE for full license)
 */
/*jshint esversion: 6 */

hbbtv.objects.OipfDrmAgent = (function() {
    const prototype = Object.create(HTMLObjectElement.prototype);
    const privates = new WeakMap();
    const gGarbageCollectionBlocked = new Set();
    let gMsgIdCounter = 0;

    const Result = Object.freeze({
        STATUS_READY: 0,
        STATUS_UNKNOWN: 1,
        STATUS_INITIALISING: 2,
        STATUS_ERROR: 3,

        MSG_SUCCESSFUL: 0,
        MSG_UNKNOWN_ERROR: 1,
        MSG_CANNOT_PROCESS_REQUEST: 2,
        MSG_UNKNOWN_MIME_TYPE: 3,
        MSG_USER_CONSENT_NEEDED: 4,
        MSG_UNKNOWN_DRM_SYSTEM: 5,
        MSG_WRONG_FORMAT: 6,
    });

    const DrmMimeTypes = Object.freeze([
        'application/vnd.marlin.drm.actiontoken+xml',
        'application/vnd.oipf.mippvcontrolmessage+xml',
        'application/vnd.oipf.cspg-hexbinary',
        'application/vnd.marlin.drm.actiontoken2+xml',
    ]);

    prototype.sendDRMMessage = function(msgType, msg, DRMSystemID) {
        const p = privates.get(this);
        gMsgIdCounter++;
        let msgId = gMsgIdCounter.toString();
        if (!DrmMimeTypes.includes(msgType)) {
            Promise.resolve().then(() =>
                dispatchDRMMessageResult.call(this, msgId, null, Result.MSG_UNKNOWN_MIME_TYPE)
            );
        } else if (p.drmSystemIdStatusMap.has(DRMSystemID)) {
            hbbtv.bridge.drm.sendDRMMessage(msgId, msgType, msg, DRMSystemID);
        } else {
            Promise.resolve().then(() =>
                dispatchDRMMessageResult.call(this, msgId, null, Result.MSG_UNKNOWN_DRM_SYSTEM)
            );
        }
        return msgId;
    };

    prototype.DRMSystemStatus = function(DRMSystemID) {
        const p = privates.get(this);
        if (p.drmSystemIdStatusMap.has(DRMSystemID)) {
            return p.drmSystemIdStatusMap.get(DRMSystemID);
        }
        return Result.STATUS_UNKNOWN;
    };

    prototype.canPlayContent = function(DRMPrivateData, DRMSystemID) {
        const p = privates.get(this);
        if (p.drmSystemIdStatusMap.has(DRMSystemID)) {
            return hbbtv.bridge.drm.canPlayContent(DRMPrivateData, DRMSystemID);
        }
        return false;
    };

    prototype.canRecordContent = function(DRMPrivateData, DRMSystemID) {
        const p = privates.get(this);
        if (p.drmSystemIdStatusMap.has(DRMSystemID)) {
            return hbbtv.bridge.drm.canRecordContent(DRMPrivateData, DRMSystemID);
        }
        return false;
    };

    prototype.setActiveDRM = function(DRMSystemID) {
        return hbbtv.bridge.drm.setActiveDRM(DRMSystemID);
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

    // Internal listeners
    function addBridgeEventListeners() {
        const p = privates.get(this);

        p.onDRMSystemStatusChange = (event) => {
            const p = privates.get(this);
            if (p.drmSystemIdStatusMap.has(event.DRMSystemID)) {
                if (event.status === Result.STATUS_UNKNOWN) {
                    p.drmSystemIdStatusMap.delete(event.DRMSystemID);
                } else {
                    p.drmSystemIdStatusMap.set(event.DRMSystemID, event.status);
                }
            } else if (event.status !== Result.UNKNOWN) {
                p.drmSystemIdStatusMap.set(event.DRMSystemID, event.status);
            }
            dispatchDRMSystemStatusChange.call(this, event.DRMSystemID);
        };
        hbbtv.bridge.addWeakEventListener('DRMSystemStatusChange', p.onDRMSystemStatusChange);

        p.onDRMMessageResult = (event) => {
            if (event.resultCode == 0x03) {
                event.resultCode = 0x06;
            }
            dispatchDRMMessageResult.call(this, event.msgID, event.resultMsg, event.resultCode);
        };
        hbbtv.bridge.addWeakEventListener('DRMMessageResult', p.onDRMMessageResult);

        p.onDRMSystemMessage = (event) => {
            const ev = new Event('DRMSystemMessage');
            Object.assign(ev, {
                msg: event.msg,
                DRMSystemID: event.DRMSystemID,
            });
            privates.get(this).eventDispatcher.dispatchEvent(ev);
        };
        hbbtv.bridge.addWeakEventListener('DRMSystemMessage', p.onDRMSystemMessage);
    }

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

    function initialise() {
        privates.set(this, {});
        const p = privates.get(this);
        p.eventDispatcher = new hbbtv.utils.EventDispatcher(this);

        /* Associates DRMSystemID with status */
        p.drmSystemIdStatusMap = new Map();
        let sysIds = hbbtv.bridge.drm.getSupportedDRMSystemIDs();
        sysIds.forEach((element) =>
            p.drmSystemIdStatusMap.set(element.DRMSystemID, element.status)
        );

        addBridgeEventListeners.call(this);
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