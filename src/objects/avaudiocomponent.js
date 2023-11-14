/**
 * @fileOverview OIPF AVAudioComponent class
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

hbbtv.objects.createAVAudioComponent = function(avAudioComponentData) {
    // Create new instance of hbbtv.objects.AVAudioComponent.prototype
    const avAudioComponent = Object.create(hbbtv.objects.AVAudioComponent.prototype);
    hbbtv.objects.AVAudioComponent.initialise.call(avAudioComponent, avAudioComponentData);
    return avAudioComponent;
};