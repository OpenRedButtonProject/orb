/**
 * @fileOverview Collection class
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

hbbtv.objects.Collection = (function() {
    const prototype = {};
    const privates = new WeakMap();

    Object.defineProperty(prototype, 'length', {
        get() {
            return privates.get(this).collection.length;
        },
    });

    prototype.item = function(index) {
        return privates.get(this).collection[index];
    };

    function initialise(items) {
        privates.set(this, {});
        const p = privates.get(this);
        p.collection = Array.isArray(items) ? items : [];
        p.collection.forEach((e) => Object.freeze(e));
    }

    return {
        prototype: prototype,
        initialise: initialise,
    };
})();

hbbtv.objects.CollectionProxy = {
    get: function(target, name) {
        if (name in target) {
            if (typeof target[name] === 'function') {
                return target[name].bind(target);
            }
            return target[name];
        }
        if (typeof name === 'string' || name instanceof String) {
            const index = parseInt(name);
            if (!isNaN(index)) {
                return target.item(index);
            }
        }
    },
};

hbbtv.objects.createCollection = function(items) {
    const collection = Object.create(hbbtv.objects.Collection.prototype);
    hbbtv.objects.Collection.initialise.call(collection, items);
    return new Proxy(collection, hbbtv.objects.CollectionProxy);
};