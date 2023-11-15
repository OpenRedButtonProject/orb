/**
 * @fileOverview ParentalRating class
 * See: {@link https://web.archive.org/web/20200219165053/http://www.oipf.tv/web-spec/volume5.html#parentalrating-class}
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

hbbtv.objects.ParentalRating = (function() {
    const prototype = {};
    const privates = new WeakMap();

    hbbtv.utils.defineGetterProperties(prototype, {
        name() {
            return privates.get(this).parentalRatingData.name;
        },
        scheme() {
            return privates.get(this).parentalRatingData.scheme;
        },
        value() {
            return privates.get(this).parentalRatingData.value;
        },
        labels() {
            return privates.get(this).parentalRatingData.labels;
        },
        region() {
            return privates.get(this).parentalRatingData.region;
        },
    });

    // Initialise an instance of prototype
    function initialise(parentalRatingData) {
        privates.set(this, {});
        const p = privates.get(this);
        p.parentalRatingData = parentalRatingData; // Hold reference to caller's object
    }

    // Private method to get a copy of the parental rating data
    /*function cloneParentalRatingData() {
      return Object.assign({}, privates.get(this).parentalRatingData);
   }*/

    /*prototype.toString = function() {
      return JSON.stringify(privates.get(this).parentalRatingData);
   }*/

    return {
        prototype: prototype,
        initialise: initialise,
        //cloneParentalRatingData: cloneParentalRatingData
    };
})();

hbbtv.objects.createParentalRating = function(parentalRatingData) {
    // Create new instance of hbbtv.objects.ParentalRating.prototype
    const pr = Object.create(hbbtv.objects.ParentalRating.prototype);
    hbbtv.objects.ParentalRating.initialise.call(pr, parentalRatingData);
    return pr;
};