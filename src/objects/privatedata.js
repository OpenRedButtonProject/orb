/**
 * @fileOverview ApplicationPrivateData class
 * See: {@link https://web.archive.org/web/20200219165053/http://www.oipf.tv/web-spec/volume5.html#applicationprivatedata-class}
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

hbbtv.objects.PrivateData = (function() {
    const prototype = {};
    const privates = new WeakMap();

    Object.defineProperty(prototype, 'currentChannel', {
        get() {
            if (privates.get(this).disabled) {
                return null;
            }
            return hbbtv.objects.createChannel(hbbtv.bridge.broadcast.getCurrentChannel());
        },
    });

    Object.defineProperty(prototype, 'keyset', {
        get() {
            return privates.get(this).keySet;
        },
    });

    prototype.getFreeMem = function() {
        if (privates.get(this).disabled) {
            return 0;
        }
        return hbbtv.bridge.manager.getFreeMem();
    };

    function initialise(data) {
        privates.set(this, {});
        const p = privates.get(this);
        p.disabled = data.disabled;
        p.keySet = hbbtv.objects.createKeySet({
            disabled: p.disabled,
        });
    }

    return {
        prototype: prototype,
        initialise: initialise,
    };
})();

hbbtv.objects.createPrivateData = function(data) {
    const privateData = Object.create(hbbtv.objects.PrivateData.prototype);
    hbbtv.objects.PrivateData.initialise.call(privateData, data);
    return privateData;
};