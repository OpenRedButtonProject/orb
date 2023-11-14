/**
 * @fileOverview MediaError class
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

hbbtv.objects.MediaError = (function() {
    const prototype = {};
    const privates = new WeakMap();
    const constants = [
        'MEDIA_ERR_ABORTED',
        'MEDIA_ERR_NETWORK',
        'MEDIA_ERR_DECODE',
        'MEDIA_ERR_SRC_NOT_SUPPORTED',
    ];
    const roProps = ['code', 'message'];

    for (const key of constants) {
        Object.defineProperty(prototype, key, {
            value: MediaError[key],
            writable: false,
        });
    }

    for (const key of roProps) {
        Object.defineProperty(prototype, key, {
            get() {
                return privates.get(this)[key];
            },
        });
    }

    function initialise(code, message) {
        privates.set(this, {
            code,
            message,
        });
    }

    return {
        prototype: prototype,
        initialise: initialise,
    };
})();

hbbtv.objects.createMediaError = function(code, message) {
    const mediaError = Object.create(hbbtv.objects.MediaError.prototype);
    hbbtv.objects.MediaError.initialise.call(mediaError, code, message);
    return mediaError;
};