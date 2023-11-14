/**
 * @fileOverview ParentalRatingSchemeCollection class
 * See: {@link https://web.archive.org/web/20200219165053/http://www.oipf.tv/web-spec/volume5.html#parentalratingschemecollection-class}
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

hbbtv.objects.ParentalRatingSchemeCollection = (function() {
    const prototype = {};
    const privates = new WeakMap();

    Object.defineProperty(prototype, 'length', {
        get() {
            return privates.get(this).parentalRatingSchemes.length;
        },
    });

    prototype.item = function(index) {
        return privates.get(this).parentalRatingSchemes[index];
    };

    prototype.getParentalRatingScheme = function(name) {
        return privates.get(this).parentalRatingSchemes.find((scheme) => scheme.name === name);
    };

    function initialise() {
        privates.set(this, {});
        const p = privates.get(this);
        p.parentalRatingSchemes = [];
        const schemes = hbbtv.bridge.parentalControl.getRatingSchemes();
        for (const scheme of schemes) {
            p.parentalRatingSchemes.push(
                hbbtv.objects.createParentalRatingScheme(scheme.name, scheme.ratings)
            );
        }
    }

    return {
        prototype: prototype,
        initialise: initialise,
    };
})();

hbbtv.objects.createParentalRatingSchemeCollection = function() {
    const parentalRatingSchemeCollection = Object.create(
        hbbtv.objects.ParentalRatingSchemeCollection.prototype
    );
    hbbtv.objects.ParentalRatingSchemeCollection.initialise.call(parentalRatingSchemeCollection);
    return parentalRatingSchemeCollection;
};