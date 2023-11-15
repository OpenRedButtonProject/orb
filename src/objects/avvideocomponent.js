/**
 * @fileOverview OIPF AVVideoComponent class
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
            let aspectRatio = p.avComponentData.aspectRatio;
            if (aspectRatio !== undefined) {
                if (aspectRatio === 0) {
                    aspectRatio = 1.33; // 4:3
                } else {
                    aspectRatio = 1.78; // 16:9
                }
            }
            return aspectRatio;
        },
    });

    // Initialise an instance of prototype
    function initialise(avVideoComponentData, vb) {
        privates.set(this, {});
        const p = privates.get(this);
        p.avComponentData = avVideoComponentData; // Hold reference to caller's object
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

hbbtv.objects.createAVVideoComponent = function(avVideoComponentData) {
    // Create new instance of hbbtv.objects.AVVideoComponent.prototype
    const avVideoComponent = Object.create(hbbtv.objects.AVVideoComponent.prototype);
    hbbtv.objects.AVVideoComponent.initialise.call(avVideoComponent, avVideoComponentData);
    return avVideoComponent;
};