/**
 * @fileOverview AdioTrackList class
 * @license ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

hbbtv.objects.AudioTrackList = (function() {
   const prototype = {};
   const privates = new WeakMap();
   const events = ["change", "addtrack", "removetrack"];
   const evtTargetMethods = ["addEventListener", "removeEventListener", "dispatchEvent"];
   const AUDIO_TRACK_KEY = "AudioTrack_";

   Object.defineProperty(prototype, "length", {
      get() {
         return privates.get(this).trackList.length;
      }
   });

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

   function makeEventTargetMethod(name) {
      return function() {
         EventTarget.prototype[name].apply(privates.get(this).eventTarget, arguments);
      };
   }

   for (const func of evtTargetMethods) {
      prototype[func] = makeEventTargetMethod(func);
   }

   prototype.getTrackById = function(id) {
      for (const track of this) {
         if (track.id === id.toString()) {
            return track;
         }
      }
   }

   prototype[Symbol.iterator] = function*() {
      for (let i = 0; i < this.length; ++i) {
         yield this[i];
      }
   }

   prototype._setTrackList = function (trackList) {
      const p = privates.get(this);
      if (p.trackList) {
         for (let i = trackList.length; i < p.trackList.length; ++i) {
            p.proxy.unregisterObserver(AUDIO_TRACK_KEY + i);
            delete this[i];
         }
      }
      p.trackList = trackList;
      for (let i = 0; i < trackList.length; ++i) {
         this[i] = new AudioTrack(p.trackList, i, p.eventTarget, p.proxy);
      }
   }

   prototype._appendTrack = function (track) {
      const p = privates.get(this);
      p.trackList.push(track);
      this[p.trackList.length - 1] = new AudioTrack(p.trackList, p.trackList.length - 1, p.eventTarget, p.proxy);
      p.eventTarget.dispatchEvent(new TrackEvent("addtrack"));
   }

   prototype._removeTrackAtIndex = function (index) {
      const p = privates.get(this);
      if (index >= 0 && index < p.trackList.length) {
         for (let i = index; i < p.trackList.length - 1; i++) {
            this[i] = this[i + 1];
         }
         if (index < p.trackList.length) {
            delete this[p.trackList.length - 1];
         }
         p.trackList.splice(index, 1);
         p.eventTarget.dispatchEvent(new TrackEvent("removetrack"));
      }
   }

   function AudioTrack(allTracks, index, eventTarget, proxy) {
      proxy.registerObserver(AUDIO_TRACK_KEY + index, this);
      Object.defineProperty(this, "enabled", {
         get() {
            return allTracks[index].enabled;
         },
         set(value) {
            if (value !== allTracks[index].enabled) {
               if (value) {
                  for (let track of allTracks) {
                     track.enabled = false;
                  }
               }
               allTracks[index].enabled = value;
               proxy.updateObserverProperties(AUDIO_TRACK_KEY + index, {enabled: value});
               eventTarget.dispatchEvent(new Event("change"));
            }
         }
      });
      Object.defineProperty(this, "index", {
         value: allTracks[index].index,
         writable: false
      });
      Object.defineProperty(this, "id", {
         value: allTracks[index].id,
         writable: false
      });
      Object.defineProperty(this, "kind", {
         value: allTracks[index].kind,
         writable: false
      });
      Object.defineProperty(this, "label", {
         value: allTracks[index].label,
         writable: false
      });
      Object.defineProperty(this, "language", {
         value: allTracks[index].language,
         writable: false
      });
      Object.defineProperty(this, "numChannels", {
         value: allTracks[index].numChannels,
         writable: false
      });
      Object.defineProperty(this, "encoding", {
         value: allTracks[index].encoding,
         writable: false
      });
      Object.defineProperty(this, "encrypted", {
         value: allTracks[index].encrypted,
         writable: false
      });
   }

   function initialise(proxy) {
      const AUDIO_TRACK_LIST_KEY = "AudioTrackList";
      privates.set(this, {
         trackList: [],
         eventTarget: document.createDocumentFragment(),
         proxy: proxy
      });
      proxy.registerObserver(AUDIO_TRACK_LIST_KEY, this);
      
      // We create a new Proxy object which we return in order to avoid ping-pong calls
      // between the iframe and the main window when the user requests a property update
      // or a function call.
      const tracksProxy = new Proxy (this, {
         get: (target, property) => {
            if (typeof target[property] === "function") {
               if (!evtTargetMethods.includes(property)) {
                  return function() {
                     proxy.callObserverMethod(AUDIO_TRACK_LIST_KEY, property, Array.from(arguments).sort((a, b) => { return a - b; }));
                     return target[property].apply(target, arguments);
                  };
               }
               return target[property].bind(target);
            }
            return target[property];
         },
         set: (target, property, value) => {
            if (typeof target[property] !== "function") {
               proxy.updateObserverProperties(AUDIO_TRACK_LIST_KEY, {[property]: value});
            }
            target[property] = value;
            return true;
         }
      });
      return tracksProxy;
   }

   return {
      prototype: prototype,
      initialise: initialise,
   };
})();

hbbtv.objects.createAudioTrackList = function(proxy) {
   const trackList = Object.create(hbbtv.objects.AudioTrackList.prototype);
   return hbbtv.objects.AudioTrackList.initialise.call(trackList, proxy);
};
