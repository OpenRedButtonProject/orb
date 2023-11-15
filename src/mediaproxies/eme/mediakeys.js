/**
 * @fileOverview MediaKeys class
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
hbbtv.objects.MediaKeys = (function() {
    const MEDIA_KEYS_ID = 'MediaKeys';
    const privates = new WeakMap();
    const prototype = {};

    hbbtv.utils.defineGetterProperties(prototype, {
        orb_polyfilled() {
            return true;
        },
    });

    prototype.createSession = function(sessionType) {
        const proxy = privates.get(this).proxy;
        proxy.callObserverMethod(MEDIA_KEYS_ID, 'createSession', [sessionType]);
        return hbbtv.objects.createMediaKeySession(proxy);
    };

    prototype.setServerCertificate = function(cert) {
        return privates
            .get(this)
            .proxy.callAsyncObserverMethod(MEDIA_KEYS_ID, 'setServerCertificate', [
                [...new Uint8Array(cert)],
            ]);
    };

    prototype.orb_setIFrameProxy = function(proxy) {
        privates.get(this).proxy = proxy;
        proxy.registerObserver(MEDIA_KEYS_ID, this);
    };

    prototype.toJSON = function() {
        const mediaKeySystemAccess = privates.get(this).mediaKeySystemAccess;
        return {
            mediaKeySystemAccess: mediaKeySystemAccess.toJSON()
        };
    };

    function instantiate(mediaKeySystemAccess) {
        const obj = Object.create(prototype);
        privates.set(obj, {
            mediaKeySystemAccess
        });
        return obj;
    }

    return {
        instantiate,
    };
})();

hbbtv.objects.createMediaKeys = function(mediaKeySystemAccess) {
    return hbbtv.objects.MediaKeys.instantiate(mediaKeySystemAccess);
};