/**
 * @fileOverview OIPF AVSubtitleComponent class
 * @license ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

hbbtv.objects.AVSubtitleComponent = (function() {
    const prototype = {};
    const privates = new WeakMap();
    const publicProperties = [
        'componentTag',
        'pid',
        'type',
        'encoding',
        'encrypted',
        'language',
        'hearingImpaired',
        'label',
    ];

    hbbtv.utils.defineGetterProperties(prototype, {
        componentTag() {
            const p = privates.get(this);
            return p.avComponentData.componentTag;
        },
        pid() {
            const p = privates.get(this);
            return p.avComponentData.pid;
        },
        type() {
            const p = privates.get(this);
            return p.avComponentData.type;
        },
        encoding() {
            const p = privates.get(this);
            return p.avComponentData.encoding;
        },
        encrypted() {
            const p = privates.get(this);
            return p.avComponentData.encrypted;
        },
        language() {
            const p = privates.get(this);
            return p.avComponentData.language;
        },
        hearingImpaired() {
            const p = privates.get(this);
            return p.avComponentData.hearingImpaired;
        },
        label() {
            const p = privates.get(this);
            return p.avComponentData.label;
        },
    });

    // Initialise an instance of prototype
    function initialise(avSubtitleComponentData) {
        privates.set(this, {});
        const p = privates.get(this);
        p.avComponentData = avSubtitleComponentData; // Hold reference to caller's object
    }

    function getId() {
        const p = privates.get(this);
        return p.avComponentData.id;
    }

    prototype.toString = function() {
        return JSON.stringify(privates.get(this).avComponentData, publicProperties);
    };

    return {
        prototype: prototype,
        initialise: initialise,
        getId: getId,
    };
})();

hbbtv.objects.createAVSubtitleComponent = function(avSubtitleComponentData) {
    // Create new instance of hbbtv.objects.AVSubtitleComponent.prototype
    const avSubtitleComponent = Object.create(hbbtv.objects.AVSubtitleComponent.prototype);
    hbbtv.objects.AVSubtitleComponent.initialise.call(avSubtitleComponent, avSubtitleComponentData);
    return avSubtitleComponent;
};