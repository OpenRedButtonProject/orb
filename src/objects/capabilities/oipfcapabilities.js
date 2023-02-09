/**
 * @fileOverview application/oipfcapabilities embedded object.
 * @license ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
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
    const privates = new WeakMap();
    let gSharedXmlCapabilities = null;

    const Result = Object.freeze({
        STATUS_READY: 0,
        STATUS_UNKNOWN: 1,
        STATUS_INITIALISING: 2,
        STATUS_ERROR: 3,
        STATUS_DECODING: 4,
    });

    hbbtv.utils.defineGetterProperties(prototype, {
        xmlCapabilities() {
            return getXmlCapabilities(privates.get(this));
        },
        extraSDVideoDecodes() {
            return hbbtv.native.getCapabilities().extraSDVideoDecodes;
        },
        extraHDVideoDecodes() {
            return hbbtv.native.getCapabilities().extraHDVideoDecodes;
        },
        extraUHDVideoDecodes() {
            return hbbtv.native.getCapabilities().extraUHDVideoDecodes;
        },
    });

    function getXmlCapabilities(p) {
        function createXmlCapabilities(capabilities) {
            function appendMediaProfile(type, profile, activeCIPlusDescrambling) {
                const media = doc.createElementNS(ns, type + '_profile');
                for (const attribute in profile) {
                    media.setAttribute(attribute, profile[attribute]);
                }
                for (let sys of activeCIPlusDescrambling) {
                    if (sys.descramblingFormats.includes(profile.name)) {
                        media.setAttribute('DRMSystemID', sys.DRMSystemID);
                    }
                }
                doc.documentElement.appendChild(media);
            }

            function appendDrm(value, key) {
                if (value.status == Result.STATUS_READY || value.status == Result.STATUS_DECODING) {
                    const drm = doc.createElementNS(ns, 'drm');
                    drm.setAttribute('DRMSystemID', key);
                    if (value.protectionGateways) {
                        drm.setAttribute('protectionGateways', value.protectionGateways);
                    }
                    if (value.supportedFormats) {
                        drm.append(value.supportedFormats);
                    }
                    ext.appendChild(drm);
                }
            }
            const ns = 'urn:hbbtv:config:oitf:oitfCapabilities:2017-1';
            const doc = document.implementation.createDocument(ns, 'profilelist', null);
            doc.documentElement.setAttribute(
                'xmlns:xsi',
                'http://www.w3.org/2001/XMLSchema-instance'
            );
            doc.documentElement.setAttribute(
                'xsi:schemaLocation',
                'urn:hbbtv:config:oitf:oitfCapabilities:2017-1 ' +
                'config-hbbtv-oitfCapabilities.xsd'
            );
            const ui = doc.createElementNS(ns, 'ui_profile');
            const broadcastSystems = capabilities.broadcastSystems.join();
            const options = capabilities.options.join();
            ui.setAttribute('name', 'OITF_HD_UIPROF' + broadcastSystems + options + ',+TRICKMODE');
            doc.documentElement.appendChild(ui);
            const ext = doc.createElementNS(ns, 'ext');
            ui.appendChild(ext);
            const parental = doc.createElementNS(ns, 'parentalcontrol');
            parental.setAttribute('schemes', capabilities.parentalSchemes);
            parental.append('true');
            ext.appendChild(parental);
            const client = doc.createElementNS(ns, 'clientMetadata');
            client.setAttribute('type', 'dvb-si');
            client.append('true');
            ext.appendChild(client);
            const temporal = doc.createElementNS(ns, 'temporalClipping');
            ext.appendChild(temporal);
            const activeCIPlusDescrambling = [];
            p.drmSystemIdStatusMap.forEach(function(value, key, map) {
                if (value.status === Result.STATUS_DECODING) {
                    activeCIPlusDescrambling.push({
                        DRMSystemID: key,
                        descramblingFormats: value.descramblingFormats,
                    });
                }
            });
            capabilities.audioFormats.forEach(function(format) {
                appendMediaProfile('audio', format, activeCIPlusDescrambling);
            });
            capabilities.videoFormats.forEach(function(format) {
                appendMediaProfile('video', format, activeCIPlusDescrambling);
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

        return doc;
    }

    // Internal listeners
    function addBridgeEventListeners() {
        const p = privates.get(this);
        p.onDRMSystemStatusChange = (event) => {
            const p = privates.get(this);
            if (p.drmSystemIdStatusMap.has(event.DRMSystemID)) {
                if (event.status === Result.UNKNOWN) {
                    p.drmSystemIdStatusMap.delete(event.DRMSystemID);
                } else {
                    p.drmSystemIdStatusMap.set(event.DRMSystemID, keepRelevantProps(event));
                }
            } else if (event.status !== Result.UNKNOWN) {
                p.drmSystemIdStatusMap.set(event.DRMSystemID, keepRelevantProps(event));
            }
            gSharedXmlCapabilities = null;
        };
        hbbtv.bridge.addWeakEventListener('DRMSystemStatusChange', p.onDRMSystemStatusChange);
    }

    function initialise() {
        // This class is atypical in that all its data is globally shared
        privates.set(this, {});
        const p = privates.get(this);
        addBridgeEventListeners.call(this);
        /* Associates DRMSystemID with status */
        p.drmSystemIdStatusMap = new Map();
        let sysIds = hbbtv.bridge.drm.getSupportedDRMSystemIDs();
        sysIds.forEach((element) =>
            p.drmSystemIdStatusMap.set(element.DRMSystemID, keepRelevantProps(element))
        );
    }

    function keepRelevantProps(element) {
        return {
            status: element.status,
            protectionGateways: element.protectionGateways,
            supportedFormats: element.supportedFormats,
            descramblingFormats: element.descramblingFormats,
        };
    }

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