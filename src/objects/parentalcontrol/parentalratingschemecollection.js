/**
 * @fileOverview ParentalRatingSchemeCollection class
 * See: {@link https://web.archive.org/web/20200219165053/http://www.oipf.tv/web-spec/volume5.html#parentalratingschemecollection-class} 
 * @license ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

hbbtv.objects.ParentalRatingSchemeCollection = (function() {
   const prototype = {};
   const privates = new WeakMap();

   Object.defineProperty(prototype, "length", {
      get() {
         return privates.get(this).parentalRatingSchemes.length;
      }
   });

   prototype.item = function(index) {
      return privates.get(this).parentalRatingSchemes[index];
   };

   prototype.getParentalRatingScheme = function(name) {
      return privates.get(this).parentalRatingSchemes.find(scheme => scheme.name === name);
   }

   function initialise() {
      privates.set(this, {});
      const p = privates.get(this);
      p.parentalRatingSchemes = [];
      const schemes = hbbtv.bridge.parentalControl.getRatingSchemes();
      for (const scheme of schemes) {
         p.parentalRatingSchemes.push(hbbtv.objects.createParentalRatingScheme(scheme.name, scheme.ratings));
      }
   }

   return {
      prototype: prototype,
      initialise: initialise
   };
})();

hbbtv.objects.createParentalRatingSchemeCollection = function() {
   const parentalRatingSchemeCollection = Object.create(hbbtv.objects.ParentalRatingSchemeCollection.prototype);
   hbbtv.objects.ParentalRatingSchemeCollection.initialise.call(parentalRatingSchemeCollection);
   return parentalRatingSchemeCollection;
}