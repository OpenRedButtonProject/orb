/**
 * @fileOverview TextTrack class
 * @license ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */
 hbbtv.objects.TextTrack = (function() {
   const prototype = {};
   const privates = new WeakMap();
   const events = ["cuechange"];
   const evtTargetMethods = ["addEventListener", "removeEventListener", "dispatchEvent"];
   const roProps = ["kind", "label", "language", "activeCues", "cues", "id"];
   const props = ["isTTML", "isEmbedded", "inBandMetadataTrackDispatchType"];

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
      p.properties.cues.push(cue);
   };
   
   prototype.removeCue = function () {

   };

   function initialise(proxy, properties, index) {
      const TEXT_TRACK_KEY = "TextTrack_" + index;
      privates.set(this, {
         length: 0,
         eventTarget: document.createDocumentFragment(),
         proxy,
         properties,
         prefix: TEXT_TRACK_KEY
      });

      properties.cues = [];

      proxy.registerObserver(TEXT_TRACK_KEY, this);
      
      // We create a new Proxy object which we return in order to avoid ping-pong calls
      // between the iframe and the main window when the user requests a property update
      // or a function call.
      const tracksProxy = new Proxy (this, {
         get: (target, property) => {
            if (typeof target[property] === "function") {
               if (property !== "addEventListener" && property !== "removeEventListener" && property !== "dispatchEvent") {
                  return function() {
                     proxy.callObserverMethod(TEXT_TRACK_KEY, property, Array.from(arguments).sort((a, b) => { return a - b; }));
                     return target[property].apply(target, arguments);
                  };
               }
               return target[property].bind(target);
            }
            return target[property];
         },
         set: (target, property, value) => {
            if (typeof target[property] !== "function") {
               proxy.updateObserverProperties(TEXT_TRACK_KEY, {[property]: value});
            }
            target[property] = value;
            return true;
         }
      });
      return tracksProxy;
   }

   return {
      prototype: prototype,
      initialise: initialise
   };
})();

hbbtv.objects.createTextTrack = function(proxy, properties, index) {
   const track = Object.create(hbbtv.objects.TextTrack.prototype);
   return hbbtv.objects.TextTrack.initialise.call(track, proxy, properties, index);
};