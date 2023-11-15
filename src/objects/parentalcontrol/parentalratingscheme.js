/**
 * @fileOverview ParentalRatingScheme class
 * See: {@link https://web.archive.org/web/20200219165053/http://www.oipf.tv/web-spec/volume5.html#parentalratingscheme-class}
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

hbbtv.objects.ParentalRatingScheme = (function() {
    const prototype = {};
    const privates = new WeakMap();

    hbbtv.utils.defineGetterProperties(prototype, {
        length() {
            // in case the rating scheme is dvb-si, the returned
            // length should be always 0
            if (this.name.toLowerCase() === 'dvb-si') {
                return 0;
            }
            return privates.get(this).ratings.length;
        },
        name() {
            return privates.get(this).name;
        },
        threshold() {
            return hbbtv.objects.createParentalRating(
                hbbtv.bridge.parentalControl.getThreshold(this.name)
            );
        },
    });

    prototype.item = function(index) {
        return privates.get(this).ratings.item(index);
    };

    prototype.indexOf = function(ratingValue) {
        const p = privates.get(this);
        for (let i = 0; i < p.ratings.length; ++i) {
            if (p.ratings.item(i).name === ratingValue) {
                return i;
            }
        }
        return -1;
    };

    prototype.iconUri = function(index) {
        return null;
    };

    function initialise(schemeName, ratings) {
        privates.set(this, {});
        const p = privates.get(this);

        p.name = schemeName;
        p.ratings = hbbtv.objects.createCollection(ratings);
    }

    return {
        prototype: prototype,
        initialise: initialise,
    };
})();

hbbtv.objects.createParentalRatingScheme = function(schemeName, ratings) {
    const parentalRatingScheme = Object.create(hbbtv.objects.ParentalRatingScheme.prototype);
    hbbtv.objects.ParentalRatingScheme.initialise.call(parentalRatingScheme, schemeName, ratings);
    return parentalRatingScheme;
};