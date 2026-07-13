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
        
        // Debug: Log what we're searching for and what's available
        console.log('[ChannelList.getChannel] Searching for channelID:', channelID);
        const channelInfo = [];
        for (let i = 0; i < p.channelDataList.length; i++) {
            const ch = p.channelDataList[i];
            channelInfo.push('Channel ' + i + ': ccid="' + (ch.ccid || '') + '", ipBroadcastID="' + (ch.ipBroadcastID || '') + '", name="' + (ch.name || '') + '"');
        }
        console.log('[ChannelList.getChannel] Available channels (' + p.channelDataList.length + '):', channelInfo.join('; '));
        
        const channelData = p.channelDataList
            .filter((channel) => {
                return (channel.ccid && channel.ccid === channelID) ||
                    (channel.ipBroadcastID && channel.ipBroadcastID === channelID);
            })
            .pop();
        if (channelData) {
            console.log('[ChannelList.getChannel] Found channel:', {
                ccid: channelData.ccid,
                ipBroadcastID: channelData.ipBroadcastID,
                name: channelData.name
            });
            return hbbtv.objects.createChannel(channelData);
        }
        console.warn('[ChannelList.getChannel] No channel found for channelID:', channelID);
        return null;
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
            function isMatch(channel) {
                for (let key in props) {
                    if (props[key] != undefined && props[key] !== channel[key]) {
                        return false;
                    }
                }
                return true;
            }
            const p = privates.get(this);
            // HbbTV TS 102 796 clause O.5.2: getChannelByTriplet shall prefer DVB-I
            // Channel objects over classic RF when both are present in the ChannelList:
            //  1) DVB-I service (IdentifierTriplet) -> return that service
            //  2) DVB-I service instance (RF triplet) -> return that instance
            //  3) If both 1 and 2 apply -> return the service
            // Classic RF / other Channel objects are used only when neither applies.
            let serviceMatch = undefined;
            let instanceMatch = undefined;
            let otherMatch = undefined;
            for (let channel of p.channelDataList) {
                if (channel.serviceInstances) {
                    if (!serviceMatch && isMatch(channel)) {
                        serviceMatch = channel;
                    }
                    if (!instanceMatch) {
                        for (let instance of channel.serviceInstances) {
                            if (isMatch(instance)) {
                                instance.parentService = channel;
                                instanceMatch = instance;
                                break;
                            }
                        }
                    }
                    if (serviceMatch && instanceMatch) {
                        break;
                    }
                } else if (!otherMatch && isMatch(channel)) {
                    otherMatch = channel;
                }
            }
            const channelData = serviceMatch || instanceMatch || otherMatch;
            if (channelData) {
                return hbbtv.objects.createChannel(channelData);
            }
            return null;
        },
    });

    function initialise(data) {
        privates.set(this, {});
        const p = privates.get(this);
        if (!data) {
            // Keeps reference to channel data
            getChannelList(p, false);
        } else {
            p.channelDataList = data;
        }
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

hbbtv.objects.createChannelList = function(data) {
    const channelList = Object.create(hbbtv.objects.ChannelList.prototype);
    hbbtv.objects.ChannelList.initialise.call(channelList, data);
    return new Proxy(channelList, hbbtv.objects.ChannelListProxy);
};