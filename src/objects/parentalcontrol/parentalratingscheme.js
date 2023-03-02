/**
 * @fileOverview ParentalRatingScheme class
 * See: {@link https://web.archive.org/web/20200219165053/http://www.oipf.tv/web-spec/volume5.html#parentalratingscheme-class} 
 * @license ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

hbbtv.objects.ParentalRatingScheme = (function() {
   const prototype = {};
   const privates = new WeakMap();

   /**
    * Specifications:
    *
    * <p>
    *
    * @returns {Length}
    *
    * @name length
    * @memberof ParentalRatingScheme#
    */
   Object.defineProperty(prototype, "length", {
      get: function() {
         // in case the rating scheme is dvb-si, the returned
         // length should be always 0
         if (this.name.toLowerCase() === "dvb-si") {
            return 0;
         }
         return privates.get(this).ratings.length;
      }
   });

   /**
    * Specifications:
    *
    * <p>
    *
    * @returns {Name}
    *
    * @name name
    * @memberof ParentalRatingScheme#
    */
   Object.defineProperty(prototype, "name", {
      get: function() {
         return privates.get(this).name;
      }
   });

   /**
    * Specifications:
    *
    * <p>
    *
    * @returns {Threshold}
    *
    * @name threshold
    * @memberof ParentalRatingScheme#
    */
   Object.defineProperty(prototype, "threshold", {
      get: function() {
         return hbbtv.objects.createParentalRating(
            hbbtv.bridge.parentalControl.getThreshold(this.name)
         );
      }
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
      initialise: initialise
   };
})();

hbbtv.objects.createParentalRatingScheme = function(schemeName, ratings) {
   const parentalRatingScheme = Object.create(hbbtv.objects.ParentalRatingScheme.prototype);
   hbbtv.objects.ParentalRatingScheme.initialise.call(parentalRatingScheme, schemeName, ratings);
   return parentalRatingScheme;
}