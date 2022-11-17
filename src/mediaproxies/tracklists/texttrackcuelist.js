/**
 * @fileOverview TextTrackCueList class
 * @license ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */
 hbbtv.objects.TextTrackCueList = (function() {
   const prototype = Object.create(TextTrackCueList.prototype);
   const privates = new WeakMap();

   Object.defineProperty(prototype, "length", {
      get() {
         return privates.get(this).length;
      }
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

   prototype.orb_addCue = function (cue) {
      for (const c of this) {
         if (cue === c) {
            return;
         }
      }
      const p = privates.get(this);
      this[p.length] = cue;
      ++p.length;
   };

   prototype.orb_removeCue = function (cue) {
      const p = privates.get(this);
      let found = false;
      for (let i = 0; i < p.length; ++i) {
         if (found) {
            this[i - 1] = this[i];
         }
         else if (cue === this[i]){
            found = true;
         }
      }
      if (found) {
         delete this[--p.length];
      }
   };

   prototype.orb_removeCueAt = function (index) {
      const p = privates.get(this);
      for (let i = index + 1; i < p.length; ++i) {
         this[i - 1] = this[i];
      }
      if (index >= 0 && index < p.length) {
         delete this[--p.length];
      }
   };

   prototype.orb_indexOf = function (cue) {
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
         length: 0
      });
      return this;
   };

   return {
      prototype: prototype,
      initialise: initialise
   };
})();

hbbtv.objects.createTextTrackCueList = function() {
   const cueList = Object.create(hbbtv.objects.TextTrackCueList.prototype);
   return hbbtv.objects.TextTrackCueList.initialise.call(cueList);
};
