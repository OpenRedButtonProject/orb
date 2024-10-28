/**
 * @fileOverview keyset class
 * See: {@link https://web.archive.org/web/20200219165053/http://www.oipf.tv/web-spec/volume5.html#keyset-class}
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

hbbtv.objects.KeySet = (function() {
    const prototype = {};
    const privates = new WeakMap();

    hbbtv.utils.defineConstantProperties(prototype, {
        RED: 0x1,
        GREEN: 0x2,
        YELLOW: 0x4,
        BLUE: 0x8,
        NAVIGATION: 0x10,
        VCR: 0x20,
        SCROLL: 0x40,
        INFO: 0x80,
        NUMERIC: 0x100,
        ALPHA: 0x200,
        OTHER: 0x400,
    });

    Object.defineProperty(prototype, 'value', {
        get() {
            if (privates.get(this).disabled) {
                return 0;
            }
            return hbbtv.bridge.manager.getKeyValues();
        },
    });

    Object.defineProperty(prototype, 'otherKeys', {
        get() {
            if (privates.get(this).disabled) {
                return [];
            }
            return hbbtv.bridge.manager.getOtherKeyValues();
        },
    });

    Object.defineProperty(prototype, 'maximumValue', {
        get() {
            return hbbtv.bridge.manager.getKeyMaximumValue();
        },
    });

    Object.defineProperty(prototype, 'maximumOtherKeys', {
        get() {
            return hbbtv.bridge.manager.getKeyMaximumOtherKeys();
        },
    });

    Object.defineProperty(prototype, 'supportsPointer', {
        get() {
            return false; // Not currently supported.
        },
        set(ignored) {
        }
    });

    prototype.setValue = function(value, otherKeys) {
        if (privates.get(this).disabled) {
            return 0;
        }
        return hbbtv.bridge.manager.setKeyValue(value, otherKeys);
    };

    prototype.getKeyIcon = function(code) {
        if (privates.get(this).disabled) {
            return '';
        }
        return hbbtv.bridge.manager.getKeyIcon(code);
    };

    function initialise(data) {
        privates.set(this, {});
        const p = privates.get(this);
        p.disabled = data.disabled;
    }

    return {
        prototype: prototype,
        initialise: initialise,
    };
})();

hbbtv.objects.createKeySet = function(data) {
    const keyset = Object.create(hbbtv.objects.KeySet.prototype);
    hbbtv.objects.KeySet.initialise.call(keyset, data);
    return keyset;
};