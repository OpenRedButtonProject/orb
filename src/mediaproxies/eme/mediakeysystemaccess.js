/**
 * @fileOverview MediaKeySystemAccess class
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
hbbtv.objects.MediaKeySystemAccess = (function() {
    const privates = new WeakMap();
    const prototype = {};

    hbbtv.utils.defineGetterProperties(prototype, {
        keySystem() {
            return privates.get(this).keySystem;
        },
    });

    prototype.getConfiguration = function() {
        return privates.get(this).configuration;
    };

    prototype.toJSON = function() {
        const p = privates.get(this);
        return {
            keySystem: p.keySystem,
            configuration: p.configuration
        };
    };

    prototype.createMediaKeys = function() {
        return Promise.resolve(hbbtv.objects.createMediaKeys(this));
    };

    function instantiate(keySystem, configuration) {
        const obj = Object.create(prototype);
        Object.freeze(configuration);
        privates.set(obj, {
            keySystem,
            configuration
        });
        return obj;
    }

    return {
        instantiate,
    };
})();

hbbtv.objects.createMediaKeySystemAccess = function(keySystem, configuration) {
    return hbbtv.objects.MediaKeySystemAccess.instantiate(keySystem, configuration);
};