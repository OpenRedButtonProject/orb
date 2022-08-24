/**
 * @fileOverview OIPF capabilities object.
 * See: {@link https://web.archive.org/web/20200219165053/http://www.oipf.tv/web-spec/volume5.html#application-oipfcapabilities}
 * @license ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

hbbtv.objects.OipfCapabilities = (function() {
    const prototype = Object.create(HTMLObjectElement.prototype);
    let gSharedXmlCapabilities = null;

    hbbtv.utils.defineGetterProperties(prototype, {
        xmlCapabilities() {
            return getXmlCapabilities();
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

    prototype.hasCapability = function(option) {
        return hbbtv.native.getCapabilities().options.includes(option);
    };

    function getXmlCapabilities() {
        function createXmlCapabilities(capabilities) {
            function appendMediaProfile(type, profile) {
                const media = doc.createElementNS(ns, type + '_profile');
                for (const attribute in profile) {
                    media.setAttribute(attribute, profile[attribute]);
                }
                doc.documentElement.appendChild(media);
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
            capabilities.audioFormats.forEach(function(format) {
                appendMediaProfile('audio', format);
            });
            capabilities.videoFormats.forEach(function(format) {
                appendMediaProfile('video', format);
            });
            const html5 = doc.createElementNS(ns, 'html5_media');
            html5.append('true');
            doc.documentElement.appendChild(html5);
            return doc;
        }
        if (gSharedXmlCapabilities === null) {
            gSharedXmlCapabilities = createXmlCapabilities(hbbtv.native.getCapabilities());
        }
        return gSharedXmlCapabilities;
    }

    function initialise() {
        // This class is atypical in that all its data is globally shared
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
    upgradeObject: function(object) {
        hbbtv.objects.upgradeToOipfCapabilities(object);
    },
});