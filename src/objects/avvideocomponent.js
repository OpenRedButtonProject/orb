/**
 * @fileOverview OIPF AVVideoComponent class
 * @license ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

hbbtv.objects.AVVideoComponent = (function() {
    const prototype = {};
    const privates = new WeakMap();
    const publicProperties = [
        'componentTag',
        'pid',
        'type',
        'encoding',
        'encrypted',
        'aspectRatio',
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
        aspectRatio() {
            const p = privates.get(this);
            return p.avComponentData.aspectRatio;
        },
    });

    // Initialise an instance of prototype
    function initialise(avVideoComponentData, vb) {
        privates.set(this, {});
        const p = privates.get(this);
        p.avComponentData = avVideoComponentData; // Hold reference to caller's object
    }

    // Private method to get a copy of the AVVideoComponent data
    /*function cloneAVVideoComponentData() {
      return Object.assign({}, privates.get(this).avComponentData);
   }*/

    prototype.toString = function() {
        return JSON.stringify(privates.get(this).avComponentData, publicProperties);
    };

    return {
        prototype: prototype,
        initialise: initialise,
        //cloneAVVideoComponentData: cloneAVVideoComponentData
    };
})();

hbbtv.objects.createAVVideoComponent = function(avVideoComponentData) {
    // Create new instance of hbbtv.objects.AVVideoComponent.prototype
    const avVideoComponent = Object.create(hbbtv.objects.AVVideoComponent.prototype);
    hbbtv.objects.AVVideoComponent.initialise.call(avVideoComponent, avVideoComponentData);
    return avVideoComponent;
};