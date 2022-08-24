/**
 * @fileOverview MediaElementObserver class
 * @license ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

hbbtv.objects.MediaElementObserver = (function() {
   const prototype = {};
   const privates = new WeakMap();
   const MEDIA_ELEMENT_EVENTS = ["seeked", "pause", "play", "abort", "ended", "ratechange", "error", "stalled", "waiting"];
   const ERROR_EVENTS = ["error"];

   hbbtv.utils.defineGetterProperties(prototype, {
      contentTime() {
         return privates.get(this).mediaObject.currentTime;
      },
      timelineSpeedMultiplier() {
         const mo = privates.get(this).mediaObject;
         return mo.ended || mo.paused ? 0 : mo.playbackRate;
      },
      contentTicks() {
         if (this.timeline) {
            return this.contentTime * this.timeline.timelineProperties.unitsPerSecond;
         } else {
            return NaN;
         }
      }
   });

   hbbtv.utils.defineGetterSetterProperties(prototype, {
      muted: {
         get() {
            return privates.get(this).mediaObject.muted;
         },
         set(val) {
            privates.get(this).mediaObject.muted = val;
         }
      },
      timeline: {
         get() {
            return privates.get(this).timeline;
         },
         set(val) {
            privates.get(this).timeline = val;
         }
      }
   });

   prototype.start = function() {
      const p = privates.get(this);
      let ret = true;
      if (p.mediaObject.error) {
         ret = false;
      } else if (!p.running) {
         for (const evt of MEDIA_ELEMENT_EVENTS) {
            p.mediaObject.addEventListener(evt, p.onMediaObjectEvent);
         }
         p.running = true;
         p.updateIntervalId = setInterval(() => {
            p.onMediaObjectEvent({
               type: "play"
            })
         }, 10000);
      }

      return ret;
   };

   prototype.stop = function() {
      const p = privates.get(this);
      if (p.running) {
         for (const evt of MEDIA_ELEMENT_EVENTS) {
            p.mediaObject.removeEventListener(evt, p.onMediaObjectEvent);
         }

         clearInterval(p.updateIntervalId);
         p.running = false;
      }
   }

   prototype.addEventListener = function(type, listener) {
      privates.get(this).eventTarget.addEventListener(type, listener);
   }

   prototype.removeEventListener = function(type, listener) {
      privates.get(this).eventTarget.removeEventListener(type, listener);
   }

   function onMediaObjectEvent(evt) {
      const p = privates.get(this);
      if (!ERROR_EVENTS.includes(evt.type)) {
         const event = new Event("MediaUpdated");
         event.data = {
            contentTime: this.contentTime,
            timelineSpeedMultiplier: this.timelineSpeedMultiplier
         };
         p.eventTarget.dispatchEvent(event);
      } else {
         p.eventTarget.dispatchEvent(new Event("Error"));
      }
   }

   function initialise(mediaObject) {
      privates.set(this, {
         mediaObject: mediaObject,
         eventTarget: document.createDocumentFragment(),
         onMediaObjectEvent: onMediaObjectEvent.bind(this),
         running: false
      });
   }

   return {
      prototype: prototype,
      initialise: initialise
   };
})();

hbbtv.objects.createMediaElementObserver = function(mediaObject) {
   const observer = Object.create(hbbtv.objects.MediaElementObserver.prototype);
   hbbtv.objects.MediaElementObserver.initialise.call(observer, mediaObject);
   return observer;
}