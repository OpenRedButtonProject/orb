/**
 * @fileOverview TextTrackCueList class
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
hbbtv.objects.TextTrackCueList = (function() {
    const prototype = Object.create(TextTrackCueList.prototype);
    const privates = new WeakMap();

    Object.defineProperty(prototype, 'length', {
        get() {
            return privates.get(this).length;
        },
    });

    prototype[Symbol.iterator] = function*() {
        for (let i = 0; i < this.length; ++i) {
            yield this[i];
        }
    };

    prototype.getCueById = function(id) {
        for (const cue of this) {
            if (cue.id === id.toString()) {
                return cue;
            }
        }
    };

    prototype.orb_addCue = function(cue) {
        for (const c of this) {
            if (cue === c) {
                return;
            }
        }
        const p = privates.get(this);
        
        // cue data needs to be an array of char codes
        if (Array.isArray(cue.data)) {
            cue.data = cue.data.map((e) => {
                if (typeof e === 'number') {
                    return e;
                }
                else {
                    return e.charCodeAt();
                }
            });
        }
        this[p.length] = cue;
        ++p.length;
    };

    prototype.orb_removeCue = function(cue) {
        const p = privates.get(this);
        let found = false;
        for (let i = 0; i < p.length; ++i) {
            if (found) {
                this[i - 1] = this[i];
            } else if (cue === this[i]) {
                found = true;
            }
        }
        if (found) {
            delete this[--p.length];
        }
    };

    prototype.orb_removeCueAt = function(index) {
        const p = privates.get(this);
        for (let i = index + 1; i < p.length; ++i) {
            this[i - 1] = this[i];
        }
        if (index >= 0 && index < p.length) {
            delete this[--p.length];
        }
    };

    prototype.orb_indexOf = function(cue) {
        for (let i = 0; i < this.length; ++i) {
            if (cue === this[i]) {
                return i;
            }
        }
        return -1;
    };

    prototype.orb_clear = function() {
        const p = privates.get(this);
        for (let i = 0; i < p.length; ++i) {
            delete this[i];
        }
        p.length = 0;
    };

    function initialise() {
        privates.set(this, {
            length: 0,
        });
        return this;
    }

    return {
        prototype: prototype,
        initialise: initialise,
    };
})();

hbbtv.objects.createTextTrackCueList = function() {
    const cueList = Object.create(hbbtv.objects.TextTrackCueList.prototype);
    return hbbtv.objects.TextTrackCueList.initialise.call(cueList);
};