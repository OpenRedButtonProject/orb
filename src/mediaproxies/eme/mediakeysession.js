/**
 * @fileOverview MediaKeySession class
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
hbbtv.objects.MediaKeySession = (function() {
    const MEDIA_KEY_SESSION_ID = 'MediaKeySession';
    const privates = new WeakMap();
    const prototype = {};
    const events = ['keystatuseschange', 'message'];
    const evtTargetMethods = ['addEventListener', 'removeEventListener', 'dispatchEvent'];

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

    hbbtv.utils.defineGetterProperties(prototype, {
        closed() {
            return privates
                .get(this)
                .proxy.callAsyncObserverMethod(MEDIA_KEY_SESSION_ID, 'orb_closed');
        },
        expiration() {
            return NaN;
        }, // TODO
        keyStatuses() {
            return {};
        }, // TODO
        sessionId() {
            return '';
        }, // TODO
    });

    prototype.generateRequest = function(initDataType, initData) {
        return privates
            .get(this)
            .proxy.callAsyncObserverMethod(MEDIA_KEY_SESSION_ID, 'generateRequest', [
                initDataType,
                [...new Uint8Array(initData)],
            ]);
    };

    prototype.update = function(licence) {
        return privates
            .get(this)
            .proxy.callAsyncObserverMethod(MEDIA_KEY_SESSION_ID, 'update', [
                [...new Uint8Array(licence)],
            ]);
    };

    prototype.close = function() {
        return privates.get(this).proxy.callAsyncObserverMethod(MEDIA_KEY_SESSION_ID, 'close');
    };

    prototype.load = function(sessionId) {
        return privates
            .get(this)
            .proxy.callAsyncObserverMethod(MEDIA_KEY_SESSION_ID, 'load', [sessionId]);
    };

    prototype.remove = function() {
        return privates.get(this).proxy.callAsyncObserverMethod(MEDIA_KEY_SESSION_ID, 'remove');
    };

    function instantiate(proxy) {
        const obj = Object.create(prototype);
        privates.set(obj, {
            proxy,
            eventTarget: document.createDocumentFragment(),
        });
        proxy.registerObserver(MEDIA_KEY_SESSION_ID, obj);
        return obj;
    }

    return {
        instantiate,
    };
})();

hbbtv.objects.createMediaKeySession = function(iframeProxy) {
    return hbbtv.objects.MediaKeySession.instantiate(iframeProxy);
};