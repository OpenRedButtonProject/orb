/**
 * @fileOverview OIPF ChannelList class
 * See: {@link https://web.archive.org/web/20200219165053/http://www.oipf.tv/web-spec/volume5.html#channellist-class}
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

hbbtv.objects.ChannelList = (function() {
    const prototype = {};
    const privates = new WeakMap();

    Object.defineProperty(prototype, 'length', {
        get() {
            const p = privates.get(this);
            return p.channelDataList.length;
        },
    });

    // We lazily create Channel instances when they are accessed. As Channel does not modify the
    // data (or allow its consumers to) we pass a reference to the object rather a clone

    prototype.item = function(index) {
        const p = privates.get(this);
        if (index < 0 || p.channelDataList.length <= index) {
            return undefined;
        }
        return hbbtv.objects.createChannel(p.channelDataList[index]);
    };

    prototype.getChannel = function(channelID) {
        const p = privates.get(this);
        const channelData = p.channelDataList
            .filter((channel) => {
                return channel.ccid === channelID;
            })
            .pop();
        return hbbtv.objects.createChannel(channelData);
    };

    prototype.getChannelByTriplet = function(onid, tsid, sid, nid) {
        return this.findChannel({
            onid: onid,
            tsid: tsid,
            sid: sid,
            nid: nid,
        });
    };

    // Note: non enumerable function for internal use
    Object.defineProperty(prototype, 'findChannel', {
        writable: false,
        value: function(props) {
            const p = privates.get(this);
            const channelData = p.channelDataList.find((channel) => {
                let match = true;
                for (let key in props) {
                    if (props[key] != undefined && props[key] !== channel[key]) {
                        return false;
                    }
                }
                return true;
            });
            if (channelData) {
                return hbbtv.objects.createChannel(channelData);
            }
            return null;
        },
    });

    function initialise() {
        privates.set(this, {});
        const p = privates.get(this);
        // Keeps reference to channel data
        getChannelList(p, false);
    }

    function getChannelList(p, thrw) {
        try {
            p.channelDataList = hbbtv.bridge.broadcast.getChannelList();
        } catch (e) {
            if (e.name === 'SecurityError') {
                /* App is broadcast independent, SecurityError will be thrown later */
                p.channelDataList = null;
            }
            if (thrw) {
                throw e;
            }
        }
    }

    return {
        prototype: prototype,
        initialise: initialise,
    };
})();

hbbtv.objects.ChannelListProxy = {
    get: function(target, name, receiver) {
        if (name in target) {
            const origMethod = target[name];
            if (origMethod instanceof Function) {
                return function(...args) {
                    return origMethod.apply(target, args);
                };
            }
            return target[name];
        }
        if (typeof name === 'string' || name instanceof String) {
            const index = parseInt(name);
            if (!isNaN(index)) {
                return target.item(index);
            }
        }
    },
};

hbbtv.objects.createChannelList = function() {
    const channelList = Object.create(hbbtv.objects.ChannelList.prototype);
    hbbtv.objects.ChannelList.initialise.call(channelList);
    return new Proxy(channelList, hbbtv.objects.ChannelListProxy);
};