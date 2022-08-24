/**
 * @fileOverview OIPF ChannelConfig class
 * See: {@link https://web.archive.org/web/20200219165053/http://www.oipf.tv/web-spec/volume5.html#channelconfig-class}
 * @license ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
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