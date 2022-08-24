/**
 * @fileOverview ParentalRatingCollection class
 * @license ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

hbbtv.objects.ParentalRatingCollection = (function() {
   const prototype = {};
   const privates = new WeakMap();

   Object.defineProperty(prototype, "length", {
      get() {
         return privates.get(this).collection.length;
      }
   });

   prototype.item = function(index) {
      return privates.get(this).collection[index];
   };

   prototype.addParentalRating = function(scheme, name, value, labels, region) {
      privates.get(this).collection.push(hbbtv.objects.createParentalRating({
         scheme: scheme,
         name: name,
         value: value,
         labels: labels,
         region: region
      }));
   };

   function initialise(items) {
      privates.set(this, {});
      const p = privates.get(this);
      p.collection = Array.isArray(items) ? items : [];
      items.forEach(function(item, index) {
         p.collection[index] = hbbtv.objects.createParentalRating(item);
      });
      p.collection.forEach(e => Object.freeze(e));
   }

   return {
      prototype: prototype,
      initialise: initialise
   };
})();

hbbtv.objects.createParentalRatingCollection = function(items) {
   const collection = Object.create(hbbtv.objects.ParentalRatingCollection.prototype);
   hbbtv.objects.ParentalRatingCollection.initialise.call(collection, items);
   return collection;
}