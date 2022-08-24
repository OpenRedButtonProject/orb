/**
 * @fileOverview VideoTrackList class
 * @license ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

hbbtv.objects.VideoTrackList = (function() {
   const prototype = {};
   const privates = new WeakMap();

   Object.defineProperty(prototype, "selectedIndex", {
      get() {
         for (const track of this) {
            if (track.selected) {
               return track.id;
            }
         }
         return -1;
      }
   });

   Object.defineProperty(prototype, "length", {
      get() {
         return privates.get(this).length;
      }
   });

   Object.defineProperty(prototype, "onchange", {
      get() {
         return privates.get(this).onchange;
      },
      set(callback) {
         const p = privates.get(this);
         if (p.onchange) {
            p.eventTarget.removeEventListener("change", p.onchange);
         }
         p.onchange = callback;
         if (callback) {
            p.eventTarget.addEventListener("change", callback);
         }
      }
   });

   prototype[Symbol.iterator] = function*() {
      for (let i = 0; i < this.length; ++i) {
         yield this[i];
      }
   }

   prototype.getTrackById = function(id) {
      for (const track of this) {
         if (track.id === id.toString()) {
            return track;
         }
      }
   }

   prototype.addEventListener = function(event, listener) {
      privates.get(this).eventTarget.addEventListener(event, listener);
   }

   prototype.removeEventListener = function(event, listener) {
      privates.get(this).eventTarget.removeEventListener(event, listener);
   }

   function VideoTrack(props, eventTarget) {
      Object.defineProperty(this, "selected", {
         get() {
            return props.selected;
         },
         set(value) {
            if (value !== props.selected) {
               props.selected = value;
               eventTarget.dispatchEvent(new Event("change"));
            }
         }
      });
      Object.defineProperty(this, "index", {
         value: props.index,
         writable: false
      });
      Object.defineProperty(this, "id", {
         value: props.id,
         writable: false
      });
      Object.defineProperty(this, "kind", {
         value: props.kind,
         writable: false
      });
      Object.defineProperty(this, "label", {
         value: props.label,
         writable: false
      });
      Object.defineProperty(this, "language", {
         value: props.language,
         writable: false
      });
      Object.defineProperty(this, "encoding", {
         value: props.encoding,
         writable: false
      });
      Object.defineProperty(this, "encrypted", {
         value: props.encrypted,
         writable: false
      });
   }

   function initialise(trackList) {
      privates.set(this, {});
      const p = privates.get(this);
      p.length = trackList.length;
      p.eventTarget = document.createDocumentFragment();
      for (let i = 0; i < trackList.length; ++i) {
         this[i] = new VideoTrack(trackList[i], p.eventTarget);
      }
      Object.freeze(this);
   }

   return {
      prototype: prototype,
      initialise: initialise
   };
})();

hbbtv.objects.createVideoTrackList = function(trackList) {
   const videoTrackList = Object.create(hbbtv.objects.VideoTrackList.prototype);
   hbbtv.objects.VideoTrackList.initialise.call(videoTrackList, trackList);
   return videoTrackList;
}
