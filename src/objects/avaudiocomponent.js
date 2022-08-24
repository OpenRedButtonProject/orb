/**
 * @fileOverview OIPF AVAudioComponent class
 * @license ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

hbbtv.objects.AVAudioComponent = (function() {
    const prototype = {};
    const privates = new WeakMap();
    const publicProperties = [
        'componentTag',
        'pid',
        'type',
        'encoding',
        'encrypted',
        'language',
        'audioDescription',
        'audioChannels',
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
        audioDescription() {
            const p = privates.get(this);
            return p.avComponentData.audioDescription;
        },
        audioChannels() {
            const p = privates.get(this);
            return p.avComponentData.audioChannels;
        },
    });

    // Initialise an instance of prototype
    function initialise(avAudioComponentData) {
        privates.set(this, {});

        const p = privates.get(this);
        p.avComponentData = avAudioComponentData; // Hold reference to caller's object
    }

    // Private method to get a copy of the AVAudioComponent data
    /*function cloneAVAudioComponentData() {
      return Object.assign({}, privates.get(this).avComponentData);
   }*/

    prototype.toString = function() {
        return JSON.stringify(privates.get(this).avComponentData, publicProperties);
    };

    return {
        prototype: prototype,
        initialise: initialise,
        //cloneAVAudioComponentData: cloneAVAudioComponentData
    };
})();

hbbtv.objects.createAVAudioComponent = function(avAudioComponentData) {
    // Create new instance of hbbtv.objects.Channel.prototype
    const avAudioComponent = Object.create(hbbtv.objects.AVAudioComponent.prototype);
    hbbtv.objects.AVAudioComponent.initialise.call(avAudioComponent, avAudioComponentData);
    return avAudioComponent;
};