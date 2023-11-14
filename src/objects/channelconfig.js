/**
 * @fileOverview OIPF ChannelConfig class
 * See: {@link https://web.archive.org/web/20200219165053/http://www.oipf.tv/web-spec/volume5.html#channelconfig-class}
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

hbbtv.objects.ChannelConfig = (function() {
    const prototype = {};
    const privates = new WeakMap();

    Object.defineProperty(prototype, 'channelList', {
        get() {
            const p = privates.get(this);
            if (!p.isBroadcastRelated) {
                throw new DOMException('', 'SecurityError');
            }
            return p.channelList;
        },
    });

    function initialise() {
        privates.set(this, {});
        const p = privates.get(this);
        p.isBroadcastRelated = true;
        p.channelList = hbbtv.objects.createChannelList();
    }

    function setIsBroadcastRelated(value) {
        const p = privates.get(this);
        p.isBroadcastRelated = value;
    }

    return {
        prototype: prototype,
        initialise: initialise,
        setIsBroadcastRelated: setIsBroadcastRelated,
    };
})();

hbbtv.objects.createChannelConfig = function() {
    const channelConfig = Object.create(hbbtv.objects.ChannelConfig.prototype);
    hbbtv.objects.ChannelConfig.initialise.call(channelConfig);
    return channelConfig;
};