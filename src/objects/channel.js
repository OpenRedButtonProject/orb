/**
 * @fileOverview OIPF Channel class
 * See: {@link https://web.archive.org/web/20200219165053/http://www.oipf.tv/web-spec/volume5.html#channel-class}
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

hbbtv.objects.Channel = (function() {
    const prototype = {};
    const privates = new WeakMap();

    hbbtv.utils.defineConstantProperties(prototype, {
        TYPE_TV: 0,
        TYPE_RADIO: 1,
        TYPE_OTHER: 2,
        TYPE_ALL: 128,
        TYPE_HBBTV_DATA: 256,
        ID_ANALOG: 0,
        ID_DVB_C: 10,
        ID_DVB_S: 11,
        ID_DVB_T: 12,
        ID_DVB_SI_DIRECT: 13,
        ID_DVB_C2: 14,
        ID_DVB_S2: 15,
        ID_DVB_T2: 16,
        ID_ISDB_C: 20,
        ID_ISDB_S: 21,
        ID_ISDB_T: 22,
        ID_ATSC_T: 30,
        ID_IPTV_SDS: 40,
        ID_IPTV_URI: 41,
        ID_DVB_I: 50,
        ID_DVB_DASH: 51,
    });

    hbbtv.utils.defineGetterProperties(prototype, {
        channelType() {
            const p = privates.get(this);
            return p.channelData.channelType;
        },
        ccid() {
            const p = privates.get(this);
            return p.channelData.ccid;
        },
        dsd() {
            const p = privates.get(this);
            return p.channelData.dsd;
        },
        idType() {
            const p = privates.get(this);
            return p.channelData.idType;
        },
        nid() {
            const p = privates.get(this);
            return p.channelData.nid;
        },
        onid() {
            const p = privates.get(this);
            return p.channelData.onid;
        },
        tsid() {
            const p = privates.get(this);
            return p.channelData.tsid;
        },
        sid() {
            const p = privates.get(this);
            return p.channelData.sid;
        },
        name() {
            const p = privates.get(this);
            return p.channelData.name;
        },
        majorChannel() {
            const p = privates.get(this);
            return p.channelData.majorChannel;
        },
        terminalChannel() {
            const p = privates.get(this);
            return p.channelData.terminalChannel;
        },
        sourceID() {
            const p = privates.get(this);
            return p.channelData.sourceID;
        },
        ipBroadcastID() {
            const p = privates.get(this);
            return p.channelData.ipBroadcastID;
        },
        serviceInstances() {
            const p = privates.get(this);
            return p.serviceInstances;
        },
        parentService() {
            const p = privates.get(this);
            return p.channelData.parentService;
        },
        tunerId() {
            const p = privates.get(this);
            return p.channelData.tunerId;
        },
    });

    // Initialise an instance of prototype
    function initialise(channelData) {
        privates.set(this, {});
        const p = privates.get(this);
        p.channelData = channelData; // Hold reference to caller's object
        if (channelData.serviceInstances) {
            for (const instance of channelData.serviceInstances) {
                instance.parentService = this;
            }
            p.serviceInstances = hbbtv.objects.createChannelList(channelData.serviceInstances);
        }
    }

    // Private method to get a copy of the channel data
    function cloneChannelData() {
        return Object.assign({}, privates.get(this).channelData);
    }

    prototype.toString = function() {
        return JSON.stringify(privates.get(this).channelData);
    };

    return {
        prototype: prototype,
        initialise: initialise,
        cloneChannelData: cloneChannelData,
    };
})();

hbbtv.objects.createChannel = function(channelData) {
    // Create new instance of hbbtv.objects.Channel.prototype
    const channel = Object.create(hbbtv.objects.Channel.prototype);
    hbbtv.objects.Channel.initialise.call(channel, channelData);
    return channel;
};