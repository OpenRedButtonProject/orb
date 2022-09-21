/**
 * @fileOverview Android native
 * See: {@link https://web.archive.org/web/20200219165053/http://www.oipf.tv/web-spec/volume5.html#configuration-class}
 * @license ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

hbbtv.nativeManager.registerNative({
    name: 'android',
    __capabilities: {
        options: [], // TODO ["+PVR", "+DRM"]
        broadcastSystems: ['+DVB_T'],
        parentalSchemes: 'dvb-si',
        extraSDVideoDecodes: 0,
        extraHDVideoDecodes: 0,
        extraUHDVideoDecodes: 0,
        audioFormats: [{
            name: 'MPEG1_L3',
            type: 'audio/mpeg',
        }, {
            name: 'HEAAC',
            type: 'audio/mp4',
        }, {
            name: 'MP4_HEAAC',
            type: 'audio/mp4',
            transport: 'dash',
            sync_tl: 'dash_pr',
        }, ],
        videoFormats: [{
            name: 'MP4_AVC_SD_25_HEAAC',
            type: 'video/mp4',
            transport: 'dash',
            sync_tl: 'dash_pr',
        }, {
            name: 'MP4_AVC_HD_25_HEAAC',
            type: 'video/mp4',
            transport: 'dash',
            sync_tl: 'dash_pr',
        }, {
            name: 'MP4_AVC_SD_25_HEAAC_EBUTTD',
            type: 'video/mp4',
            transport: 'dash',
            sync_tl: 'dash_pr',
        }, {
            name: 'MP4_AVC_HD_25_HEAAC_EBUTTD',
            type: 'video/mp4',
            transport: 'dash',
            sync_tl: 'dash_pr',
        }, {
            name: 'TS_AVC_SD_25_HEAAC',
            type: 'video/mpeg',
            sync_tl: 'temi',
        }, {
            name: 'TS_AVC_HD_25_HEAAC',
            type: 'video/mpeg',
            sync_tl: 'temi',
        }, {
            name: 'MP4_AVC_SD_25_HEAAC',
            type: 'video/mp4',
        }, {
            name: 'MP4_AVC_HD_25_HEAAC',
            type: 'video/mp4',
        }, ],
    },
    isNative: function() {
        return navigator.userAgent.indexOf('Android') !== -1;
    },
    initialise: function() {
        this.token = Object.assign({}, document.token);
    },
    request: function(method, params) {
        const body = {
            token: this.token,
            method: method,
            params: params || {},
        };
        const responseText = androidBridge.request(JSON.stringify(body));
        if (typeof responseText !== 'string') {
            console.debug('Invalid response');
            return false;
        }
        const response = JSON.parse(responseText);
        if (response.error !== undefined) {
            if (response.error === 'SecurityError') {
                throw new DOMException('', 'SecurityError');
            }
            console.debug('Error response from ' + method + ': ' + response.error);
            return false;
        }
        //console.log("Response from " + method + ": " + JSON.stringify(response));
        return response;
    },
    setDispatchEventCallback: function(callback) {
        // TEMPORARY Dispatch an event and properties. TODO Replace me in events part 2!
        document.dispatchBridgeEvent = (type, properties) => {
            callback(type, properties);
        };
    },
    getCapabilities: function() {
        return Object.freeze(this.__capabilities);
    },
    isDebugBuild: function() {
        return true; // TODO Move
    },
    isFeatureEnabled(name) {
        if (name === 'dash-scheme') {
            return true;
        }
        return false;
    },
});