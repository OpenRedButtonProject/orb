/**
 * @fileOverview application/oipfcapabilities embedded object.
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

/**
 * HbbTV programming interface: application/oipfcapabilities embedded object.
 *
 * Specifiations:
 * HBBTV.
 * <p>
 * Important sections, in brief:
 * HBBTV A.1 (Detailed section-by-section definition for volume 5);
 * OIPF DAE 7.15.3 (The application/oipfCapabilities embedded object);
 * HBBTV 10.2.4 (HbbTV reported capabilities and option strings);
 * HBBTV A.2.15 (Extensions to the OIPF-defined capability negotiation mechanism);
 * OIPF DAE (9.3 Client capability description);
 * HBBTV A.2.30 (Extensions and clarifications to the application/oipfCapabilities embedded object);
 * HBBTV A.2.1 (Resource management).
 *
 * @name OipfCapabilities
 * @class
 * @constructor
 */
hbbtv.objects.OipfCapabilities = (function() {
    const prototype = Object.create(HTMLObjectElement.prototype);

    let gSharedProfileNames = null;

    /**
     * Specifications:
     * HBBTV A.1/OIPF DAE 7.15.3 (The application/oipfCapabilities embedded object);
     * HBBTV 10.2.4 (HbbTV reported capabilities and option strings);
     * HBBTV A.2.15 (Extensions to the OIPF-defined capability negotiation mechanism);
     * OIPF DAE (9.3 Client capability description).
     * <p>
     * Security: none.
     *
     * @returns {Document}
     *
     * @name xmlCapabilities
     * @readonly
     * @memberof OipfCapabilities#
     */
    Object.defineProperty(prototype, 'xmlCapabilities', {
        get: function() {
            return createXmlCapabilities();
        },
    });

    /**
     * Specifications:
     * HBBTV A.1/OIPF DAE 7.15.3 (The application/oipfCapabilities embedded object);
     * HBBTV A.2.30 (Extensions and clarifications to the application/oipfCapabilities embedded
     *    object);
     * HBBTV A.2.1 (Resource management).
     * <p>
     * Security: none.
     *
     * @returns {number}
     *
     * @name extraSDVideoDecodes
     * @readonly
     * @memberof OipfCapabilities#
     */
    Object.defineProperty(prototype, 'extraSDVideoDecodes', {
        get: function() {
            return hbbtv.bridge.configuration.getExtraSDVideoDecodes();
        },
    });

    /**
     * Specifications:
     * HBBTV A.1/OIPF DAE 7.15.3 (The application/oipfCapabilities embedded object);
     * HBBTV A.2.30 (Extensions and clarifications to the application/oipfCapabilities embedded
     *    object);
     * HBBTV A.2.1 (Resource management).
     * <p>
     * Security: none.
     *
     * @returns {number}
     *
     * @name extraHDVideoDecodes
     * @readonly
     * @memberof OipfCapabilities#
     */
    Object.defineProperty(prototype, 'extraHDVideoDecodes', {
        get: function() {
            return hbbtv.bridge.configuration.getExtraHDVideoDecodes();
        },
    });

    /**
     * Specifications:
     * HBBTV A.1/OIPF DAE 7.15.3 (The application/oipfCapabilities embedded object);
     * HBBTV A.2.30 (Extensions and clarifications to the application/oipfCapabilities embedded
     *    object).
     * <p>
     * Security: none.
     *
     * @returns {number}
     *
     * @name extraUHDVideoDecodes
     * @readonly
     * @memberof OipfCapabilities#
     */
    Object.defineProperty(prototype, 'extraUHDVideoDecodes', {
        get: function() {
            return hbbtv.bridge.configuration.getExtraUHDVideoDecodes();
        },
    });

    /**
     * Specifications:
     * HBBTV A.1/OIPF DAE 7.13.1 (The video/broadcast embedded object);
     * HBBTV A.2.4 (Extensions to the video/broadcast object);
     * OIPF DAE 9.3 (Client capability description).
     * <p>
     * Security: none.
     *
     * @returns {boolean}
     *
     * @method
     * @memberof OipfCapabilities#
     */
    prototype.hasCapability = function(profileName) {
        if (gSharedProfileNames === null) {
            const capabilities = hbbtv.bridge.configuration.getCapabilities();
            gSharedProfileNames = ['OITF_HD_UIPROF'];
            gSharedProfileNames = gSharedProfileNames.concat(capabilities.optionStrings);
            gSharedProfileNames = gSharedProfileNames.concat(capabilities.profileNameFragments);
            Object.freeze(gSharedProfileNames);
        }
        return gSharedProfileNames.includes(profileName);
    };

    function createXmlCapabilities() {
        const capabilities = hbbtv.bridge.configuration.getCapabilities();
        const audioProfiles = hbbtv.bridge.configuration.getAudioProfiles();
        const videoProfiles = hbbtv.bridge.configuration.getVideoProfiles();
        console.log('createXmlCapabilities videoProfiles');
        console.log(videoProfiles);
        console.log(JSON.stringify(videoProfiles));

        const videoDisplayFormats = hbbtv.bridge.configuration.getVideoDisplayFormats();

        const ns = 'urn:hbbtv:config:oitf:oitfCapabilities:2017-1';
        const doc = document.implementation.createDocument(ns, 'profilelist', null);
        doc.documentElement.setAttribute('xmlns:xsi', 'http://www.w3.org/2001/XMLSchema-instance');
        doc.documentElement.setAttribute(
            'xsi:schemaLocation',
            'urn:hbbtv:config:oitf:oitfCapabilities:2017-1 ' + 'config-hbbtv-oitfCapabilities.xsd'
        );

        // ui_profile
        const uiProfileElement = doc.createElementNS(ns, 'ui_profile');

        let uiProfileNames = ['OITF_HD_UIPROF'];
        uiProfileNames.push.apply(uiProfileNames, capabilities.optionStrings);
        uiProfileNames.push.apply(uiProfileNames, capabilities.profileNameFragments);
        uiProfileElement.setAttribute('name', uiProfileNames.join(''));
        doc.documentElement.appendChild(uiProfileElement);

        // ext
        const extElement = doc.createElementNS(ns, 'ext');
        uiProfileElement.appendChild(extElement);

        // parentalcontrol
        const parentalcontrolElement = doc.createElementNS(ns, 'parentalcontrol');
        parentalcontrolElement.setAttribute('schemes', capabilities.parentalSchemes.join(' '));
        parentalcontrolElement.append('true');
        extElement.appendChild(parentalcontrolElement);

        // clientMetadata
        const clientMetadataElement = doc.createElementNS(ns, 'clientMetadata');
        clientMetadataElement.setAttribute('type', 'dvb-si');
        clientMetadataElement.append('true');
        extElement.appendChild(clientMetadataElement);

        // temporalClipping
        const temporalClippingElement = doc.createElementNS(ns, 'temporalClipping');
        extElement.appendChild(temporalClippingElement);

        // graphicsPerformance
        if (typeof capabilities.graphicsLevels === 'object') {
            const graphicsPerformanceElement = doc.createElementNS(ns, 'graphicsPerformance');
            graphicsPerformanceElement.setAttribute('level', capabilities.graphicsLevels.join(' '));
            extElement.appendChild(graphicsPerformanceElement);
        }

        // audio_profile
        audioProfiles.forEach(function(profile) {
            const element = doc.createElementNS(ns, 'audio_profile');
            element.setAttribute('name', profile.name);
            element.setAttribute('type', profile.type);
            if (typeof profile.transport === 'string') {
                element.setAttribute('transport', profile.transport);
            }
            if (typeof profile.syncTl === 'string') {
                element.setAttribute('sync_tl', profile.syncTl);
            }
            if (typeof profile.drmSystemId === 'string') {
                element.setAttribute('DRMSystemID', profile.drmSystemId);
            }
            doc.documentElement.appendChild(element);
        });

        // video_profile
        videoProfiles.forEach(function(profile) {
            const element = doc.createElementNS(ns, 'video_profile');
            element.setAttribute('name', profile.name);
            element.setAttribute('type', profile.type);
            if (typeof profile.transport === 'string') {
                element.setAttribute('transport', profile.transport);
            }
            if (typeof profile.syncTl === 'string') {
                element.setAttribute('sync_tl', profile.syncTl);
            }
            if (typeof profile.drmSystemId === 'string') {
                element.setAttribute('DRMSystemID', profile.drmSystemId);
            }
            if (typeof profile.hdr === 'string') {
                element.setAttribute('hdr', profile.hdr);
            }
            doc.documentElement.appendChild(element);
        });

        //drm
        const status = hbbtv.drmManager.getCSPGCIPlusStatus();
        if (status) {
            status.DRMSystemIDs.forEach((value) => {
                const drm = doc.createElementNS(ns, 'drm');
                drm.setAttribute('DRMSystemID', value);
                if (status.protectionGateways) {
                    drm.setAttribute('protectionGateways', status.protectionGateways);
                }
                if (status.supportedFormats) {
                    drm.append(status.supportedFormats);
                }
                extElement.appendChild(drm);
            });
        }

        // html5_media
        const html5MediaElement = doc.createElementNS(ns, 'html5_media');
        html5MediaElement.append('true');
        doc.documentElement.appendChild(html5MediaElement);

        // broadcast
        if (typeof capabilities.broadcastUrns === 'object') {
            capabilities.broadcastUrns.forEach(function(urn) {
                const element = doc.createElementNS(ns, 'broadcast');
                element.append(urn);
                extElement.appendChild(element);
            });
        }

        // video_display_format
        if (typeof videoDisplayFormats === 'object') {
            videoDisplayFormats.forEach(function(format) {
                const element = doc.createElementNS(ns, 'video_display_format');
                element.setAttribute('width', format.width);
                element.setAttribute('height', format.height);
                element.setAttribute('frame_rate', format.frameRate);
                element.setAttribute('bit_depth', format.bitDepth);
                element.setAttribute('colorimetry', format.colorimetry);
                doc.documentElement.appendChild(element);
            });
        }

        // display_size
        const displaySizeElement = doc.createElementNS(ns, 'display_size');
        displaySizeElement.setAttribute('width', capabilities.displaySizeWidth);
        displaySizeElement.setAttribute('height', capabilities.displaySizeHeight);
        displaySizeElement.setAttribute(
            'measurement_type',
            capabilities.displaySizeMeasurementType
        );
        doc.documentElement.appendChild(displaySizeElement);

        // audio_system
        if (typeof capabilities.audioOutputFormat === 'string') {
            const element = doc.createElementNS(ns, 'audio_system');
            element.setAttribute('audio_output_format', capabilities.audioOutputFormat);
            if (typeof capabilities.passThroughStatus === 'boolean') {
                element.setAttribute('pass_through', capabilities.passThroughStatus);
            }
            doc.documentElement.appendChild(element);
        }

        // html5_media_variable_rate
        if (
            typeof capabilities.html5MediaVariableRateMin === 'string' &&
            typeof capabilities.html5MediaVariableRateMax === 'string'
        ) {
            const element = doc.createElementNS(ns, 'html5_media_variable_rate');
            element.setAttribute('min', capabilities.html5MediaVariableRateMin);
            element.setAttribute('max', capabilities.html5MediaVariableRateMax);
            doc.documentElement.appendChild(element);
        }

        // json_rpc_server
        if (
            typeof capabilities.jsonRpcServerUrl === 'string' &&
            typeof capabilities.jsonRpcServerVersion === 'string'
        ) {
            const element = doc.createElementNS(ns, 'json_rpc_server');
            element.setAttribute('url', capabilities.jsonRpcServerUrl);
            element.setAttribute('version', capabilities.jsonRpcServerVersion);
            doc.documentElement.appendChild(element);
        }

        return doc;
    }

    function initialise() {}

    return {
        prototype: prototype,
        initialise: initialise,
    };
})();

hbbtv.objects.upgradeToOipfCapabilities = function(object) {
    Object.setPrototypeOf(object, hbbtv.objects.OipfCapabilities.prototype);
    hbbtv.objects.OipfCapabilities.initialise.call(object);
};

hbbtv.objectManager.registerObject({
    name: 'application/oipfcapabilities',
    mimeTypes: ['application/oipfcapabilities'],
    oipfObjectFactoryMethodName: 'createCapabilitiesObject',
    upgradeObject: hbbtv.objects.upgradeToOipfCapabilities,
});