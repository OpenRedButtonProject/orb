/**
 * @fileOverview TimeRanges class
 * @license ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

hbbtv.objects.TimeRanges = (function() {
   const prototype = {};
   const privates = new WeakMap();

   Object.defineProperty(prototype, "length", {
      get() {
         return privates.get(this).ranges.length;
      }
   });

   prototype.start = function(index) {
      return privates.get(this).ranges[index].start;
   }

   prototype.end = function(index) {
      return privates.get(this).ranges[index].end;
   }

   function initialise(ranges) {
      privates.set(this, {
         ranges
      });
   }

   return {
      prototype: prototype,
      initialise: initialise
   };
})();

hbbtv.objects.createTimeRanges = function(ranges) {
   const timeRanges = Object.create(hbbtv.objects.TimeRanges.prototype);
   hbbtv.objects.TimeRanges.initialise.call(timeRanges, ranges);
   return timeRanges;
};