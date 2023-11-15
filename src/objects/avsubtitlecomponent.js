/**
 * @fileOverview OIPF AVSubtitleComponent class
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