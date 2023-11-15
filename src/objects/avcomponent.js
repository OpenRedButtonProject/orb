/**
 * @fileOverview OIPF AVComponent class
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

hbbtv.objects.AVComponent = (function() {
    const prototype = {};
    const privates = new WeakMap();
    const publicProperties = ['componentTag', 'pid', 'type', 'encoding', 'encrypted'];

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
    });

    // Initialise an instance of prototype
    function initialise(avComponentData, vb) {
        console.log('AVComponent initialise');
        privates.set(this, {});
        const p = privates.get(this);
        p.avComponentData = avComponentData; // Hold reference to caller's object
        p.vb = vb && hbbtv.utils.HAS_WEAKREF_SUPPORT ? new WeakRef(vb) : vb;
    }

    // Private method to get a copy of the AVComponent data
    /*function cloneAVComponentData() {
      return Object.assign({}, privates.get(this).avComponentData);
   }*/

    prototype.toString = function() {
        return JSON.stringify(privates.get(this).avComponentData, publicProperties);
    };

    return {
        prototype: prototype,
        initialise: initialise,
        //cloneAVComponentData: cloneAVComponentData
    };
})();

hbbtv.objects.createAVComponent = function(avComponentData) {
    // Create new instance of hbbtv.objects.Channel.prototype
    const avComponent = Object.create(hbbtv.objects.AVComponent.prototype);
    hbbtv.objects.AVComponent.initialise.call(avComponent, avComponentData);
    return avComponent;
};