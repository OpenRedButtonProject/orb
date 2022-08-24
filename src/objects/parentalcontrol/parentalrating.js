/**
 * @fileOverview ParentalRating class
 * See: {@link https://web.archive.org/web/20200219165053/http://www.oipf.tv/web-spec/volume5.html#parentalrating-class}
 * @license ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
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