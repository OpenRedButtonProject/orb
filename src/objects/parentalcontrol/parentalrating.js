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

   /**
    * Specifications:
    *
    * <p>
    * Security: none.
    *
    * @returns {Name}
    *
    * @name name
    * @memberof ParentalRating#
    */
   Object.defineProperty(prototype, "name", {
      get: function() {
         return privates.get(this).parentalRatingData.name;
      }
   });

   /**
    * Specifications:
    *
    * <p>
    * Security: none.
    *
    * @returns {Scheme}
    *
    * @name scheme
    * @memberof ParentalRating#
    */
   Object.defineProperty(prototype, "scheme", {
      get: function() {
         return privates.get(this).parentalRatingData.scheme;
      }
   });

   /**
    * Specifications:
    *
    * <p>
    * Security: none.
    *
    * @returns {Value}
    *
    * @name value
    * @memberof ParentalRating#
    */
   Object.defineProperty(prototype, "value", {
      get: function() {
         return privates.get(this).parentalRatingData.value;
      }
   });

   /**
    * Specifications:
    *
    * <p>
    * Security: none.
    *
    * @returns {Labels}
    *
    * @name labels
    * @memberof ParentalRating#
    */
   Object.defineProperty(prototype, "labels", {
      get: function() {
         return privates.get(this).parentalRatingData.labels;
      }
   });

   /**
    * Specifications:
    *
    * <p>
    * Security: none.
    *
    * @returns {Region}
    *
    * @name region
    * @memberof ParentalRating#
    */
   Object.defineProperty(prototype, "region", {
      get: function() {
         return privates.get(this).parentalRatingData.region;
      }
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
   }
})();

hbbtv.objects.createParentalRating = function(parentalRatingData) {
   // Create new instance of hbbtv.objects.ParentalRating.prototype
   const pr = Object.create(hbbtv.objects.ParentalRating.prototype);
   hbbtv.objects.ParentalRating.initialise.call(pr, parentalRatingData);
   return pr;
};