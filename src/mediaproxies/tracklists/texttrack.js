/**
 * @fileOverview TextTrack class
 * @license ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */
 hbbtv.objects.TextTrack = (function() {
   const prototype = Object.create(TextTrack.prototype);
   const privates = new WeakMap();
   const events = ["cuechange"];
   const evtTargetMethods = ["addEventListener", "removeEventListener", "dispatchEvent"];
   const roProps = ["kind", "label", "language", "activeCues", "cues", "id"];
   const props = ["isTTML", "isEmbedded", "inBandMetadataTrackDispatchType"];
   const TRACK_MODE_SHOWING = "showing";
   const TRACK_MODE_HIDDEN = "hidden";
   const TRACK_MODE_DISABLED = "disabled";

   for (const key of events) {
      Object.defineProperty(prototype, "on" + key, {
         set(callback) {
            const p = privates.get(this);
            if (p["on" + key]) {
               p.eventTarget.removeEventListener(key, p["on" + key]);
            }

            if (callback instanceof Object) {
               p["on" + key] = callback;
               if (callback) {
                  p.eventTarget.addEventListener(key, callback);
               }
            }
            else {
               p["on" + key] = null;
            }
         },
         get() {
            return privates.get(this)["on" + key];
         }
      });
   }

   for (const key of roProps) {
      Object.defineProperty(prototype, key, {
         get() {
            return privates.get(this).properties[key];
         }
      });
   }

   for (const key of props) {
      Object.defineProperty(prototype, key, {
         set(value) {
            privates.get(this).properties[key] = value;
         },
         get() {
            return privates.get(this).properties[key];
         }
      });
   }

   Object.defineProperty(prototype, "mode", {
      set(value) {
         const p = privates.get(this);
         if (p.properties.mode !== value && [TRACK_MODE_DISABLED, TRACK_MODE_HIDDEN, TRACK_MODE_SHOWING].includes(value)) {
            p.properties.mode = value;
            if (value === TRACK_MODE_SHOWING) {
               p.onTimeUpdate();
               if (!p.invalidated) {
                  p.mediaElement.addEventListener("timeupdate", p.onTimeUpdate, true);
               }
            }
            else {
               p.properties.activeCues.orb_clear();
               p.mediaElement.removeEventListener("timeupdate", p.onTimeUpdate, true);
            }
            p.mediaElement.textTracks.dispatchEvent(new Event("change"));
         }
      },
      get() {
         return privates.get(this).properties.mode;
      }
   });

   Object.defineProperty(prototype, "default", {
      set(value) {
         privates.get(this).properties.default = !!value;
         if (value) {
            this.mode = TRACK_MODE_SHOWING;
         }
      },
      get() {
         return privates.get(this).properties.default;
      }
   });

   function makeEventTargetMethod(name) {
      return function() {
         EventTarget.prototype[name].apply(privates.get(this).eventTarget, arguments);
      };
   }
   
   for (const func of evtTargetMethods) {
      prototype[func] = makeEventTargetMethod(func);
   }

   prototype.addCue = function (cue) {
      const p = privates.get(this);
      p.properties.cues.orb_addCue(cue);
   };
   
   prototype.removeCue = function (cue) {
      const p = privates.get(this);
      p.properties.cues.orb_removeCue(cue);
      p.properties.activeCues.orb_removeCue(cue);
   };

   prototype.orb_invalidate = function () {
      const p = privates.get(this);
      p.proxy.unregisterObserver(p.observerId);
      p.activeCues.orb_clear();
      p.cues.orb_clear();
      p.mediaElement.removeEventListener("timeupdate", p.onTimeUpdate, true);
      p.invalidated = true;
   };

   function initialise(mediaElement, proxy, id, kind, label, language) {
      const thiz = this;
      const observerId = "TextTrack_" + id;
      const properties = {
         id,
         kind,
         label,
         language,
         cues: hbbtv.objects.createTextTrackCueList(),
         activeCues: hbbtv.objects.createTextTrackCueList(),
         mode: TRACK_MODE_DISABLED,
         default: false
      };
      privates.set(this, {
         length: 0,
         eventTarget: document.createDocumentFragment(),
         onTimeUpdate: (e) => {
            const time = mediaElement.currentTime;
            let changed = false;
            for (let i = properties.activeCues.length - 1; i >= 0; --i) {
               const cue = properties.activeCues[i];
               if (cue.endTime < time || cue.startTime > time) {
                  properties.activeCues.orb_removeCueAt(i);
                  if (typeof cue.onexit === "function") {
                     cue.onexit();
                  }
                  changed = true;
               }
            }
            for (const cue of properties.cues) {
               if (cue.endTime >= time && cue.startTime <= time && properties.activeCues.orb_indexOf(cue) < 0) {
                  properties.activeCues.orb_addCue(cue);
                  if (typeof cue.onenter === "function") {
                     cue.onenter();
                  }
                  changed = true;
               }
            }
            if (changed) {
               thiz.dispatchEvent(new Event("cuechange"));
            }
         },
         properties,
         mediaElement,
         observerId,
         proxy
      });

      proxy.registerObserver(observerId, this);
      
      // We create a new Proxy object which we return in order to avoid ping-pong calls
      // between the iframe and the main window when the user requests a property update
      // or a function call.
      const trackProxy = new Proxy (this, {
         get: (target, property) => {
            if (typeof target[property] === "function") {
               if (!evtTargetMethods.includes(property)) {
                  return function() {
                     if (property !== "addCue") {
                        proxy.callObserverMethod(observerId, property, Array.from(arguments).sort((a, b) => { return a - b; }));
                     }
                     else {
                        const cueObj = { };
                        for (const key in arguments[0]) {
                           if (typeof arguments[0] !== "function") {
                              cueObj[key] = arguments[0][key];
                           }
                        }
                        proxy.callObserverMethod(observerId, property, [cueObj]);
                     }
                     return target[property].apply(target, arguments);
                  };
               }
               return target[property].bind(target);
            }
            return target[property];
         },
         set: (target, property, value) => {
            if (typeof target[property] !== "function") {
               proxy.updateObserverProperties(observerId, {[property]: value});
            }
            target[property] = value;
            return true;
         }
      });
      return trackProxy;
   }

   return {
      prototype,
      initialise
   };
})();

hbbtv.objects.createTextTrack = function(mediaElement, proxy, index, kind, label, language) {
   const track = Object.create(hbbtv.objects.TextTrack.prototype);
   return hbbtv.objects.TextTrack.initialise.call(track, mediaElement, proxy, index, kind, label, language);
};