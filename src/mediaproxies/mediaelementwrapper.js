/**
 * @fileOverview MediaElementWrapper class
 * @license ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

 hbbtv.objects.MediaElementWrapper = (function() {
   const prototype = Object.create(HTMLIFrameElement.prototype);
   const privates = new WeakMap();
   const srcOwnProperty = Object.getOwnPropertyDescriptor(HTMLIFrameElement.prototype, "src");
   const ORB_PLAYER_MAGIC_SUFFIX = "orb_player_magic_suffix";
   const methods = ["pause","load","canPlayType","captureStream","fastSeek"];
   const asyncMethods = ["play","setMediaKeys","setSinkId"];
   const props = ["autoplay","controls","currentTime","playbackRate","volume","muted","loop","defaultMuted","crossOrigin","controlsList",
                  "defaultPlaybackRate","disableRemotePlayback","preservesPitch","srcObject"]
   const roProps = ["textTracks","audioTracks","videoTracks","paused","ended","currentSrc","buffered","error","duration",
                  "networkState","readyState","seekable"];
   const events = ["loadstart","progress","suspend","abort","error","emptied","stalled","loadedmetadata","loadeddata","canplay",
                  "canplaythrough","playing","waiting","seeking","seeked","ended","durationchange","timeupdate","play","pause",
                  "ratechange","resize","volumechange"];
   const iframes = { };
   let iframeCounter = 0;
   let callCounter = 0;

   prototype.getAttribute = function(name) {
      const p = privates.get(this);
      if (name === "src") {
         return p.attributes.src;
      }
      if (props.includes(name)) {
         if (p.id !== -1) {
            const result = (async function () { await requestProperty.call(this, "getAttribute", [name]) }).call(this);
            return result;
         }
         return p.attributes[name];
      }
      return HTMLIFrameElement.prototype.getAttribute.call(this, name);
   };

   prototype.setAttribute = function(name, value) {
      const p = privates.get(this);
      if (props.includes(name)) {
         p.attributes[name] = value;
         if (p.id !== -1) {
            requestProperty.call(this, "setAttribute", [name, value]);
         }
      }
      else {
         if (name === "src") {
            delete iframes[p.id];
            p.id = -1;
            p.attributes.src = value;
            console.log("MediaElementWrapper: Setting iframe src attribute to '" + value + "'.");
            value += (value.includes("?") ? "&" : "?") + ORB_PLAYER_MAGIC_SUFFIX;
         }
         HTMLIFrameElement.prototype.setAttribute.call(this, name, value);
      }
   };

   prototype.addEventListener = function (name, callback) {
      if (events.includes(name)) {
         if (callback instanceof Object) {
            const p = privates.get(this);
            let callbacks = p.callbacks[name];
            if (!callbacks) {
               callbacks = p.callbacks[name] = [];
            }
            if (callbacks.indexOf(callback) < 0) {
               callbacks.push(callback);
               if (!this["on" + name] && callbacks.length === 1 && p.id !== -1) {
                  requestProperty.call(this, "addEventListener", [name]);
               }
            }
         }
         else {
            throw new TypeError("Failed to execute 'addEventListener' on 'EventTarget': parameter 2 is not of type 'Object'.");
         }
      }
      else {
         HTMLIFrameElement.prototype.addEventListener.call(this, arguments);
      }
   }

   prototype.removeEventListener = function (name, callback) {
      if (events.includes(name)) {
         if (callback instanceof Object) {
            const p = privates.get(this);
            const callbacks = p.callbacks[name];
            if (callbacks) {
               const index = callbacks.indexOf(callback);
               if (index >= 0) {
                  callbacks.splice(index, 1);
                  if (!this["on" + name] && callbacks.length <= 0 && p.id !== -1) {
                     requestProperty.call(this, "removeEventListener", [name]);
                  }
               }
            }
         }
         else {
            throw new TypeError("Failed to execute 'addEventListener' on 'EventTarget': parameter 2 is not of type 'Object'.");
         }
      }
      else {
         HTMLIFrameElement.prototype.addEventListener.call(this, arguments);
      }
   }

   function requestProperty(name, args = []) {
      const thiz = this;
      const iframeId = privates.get(this).id;
      const callId = callCounter++;
      this.contentWindow.postMessage(JSON.stringify({callId:callId, name:name, args:args}), '*');
      return new Promise((resolve, reject) => {
         function deferResult(e) {
            window.removeEventListener("message", deferResult);
            clearInterval(intervalId);
            try {
               const msg = JSON.parse(e.data);
               if (iframeId === msg.iframeId && msg.callId === callId) {
                  console.log("MediaElementWrapper: received message from iframe:", msg);
                  resolve(msg.result);
               }
            }
            catch (e) {
               reject(e);
            }
         }
         const intervalId = setTimeout(() => {
            window.removeEventListener("message", deferResult);
            reject("MediaElementWrapper: Timeout was reached while calling '" + name + "'");
         }, 10000);
         window.addEventListener("message", deferResult);
      });
   }

   function makeMethod(name) {
      return async function () {
         return await requestProperty.call(this, name, Array.from(arguments).sort((a, b) => { return a - b; }));
      }
   }

   function makeAsyncMethod(name) {
      return function () {
         return requestProperty.call(this, name, Array.from(arguments).sort((a, b) => { return a - b; }));
      }
   }

   function initialise() {
      let p = privates.get(this);
      if (!p) {
         this.frameBorder = 0;
         Object.setPrototypeOf(this, prototype);
         privates.set(this, {
            id: -1,
            attributes: { },
            properties: { },
            callbacks: { }
         });
         p = privates.get(this);

         HTMLIFrameElement.prototype.addEventListener.call(this, "load", () => {
            p.id = iframeCounter++;
            iframes[p.id] = this;
            console.log("MediaElementWrapper: initialising iframe with id", p.id);
            requestProperty.call(this, "initialise", [p.id]);

            // set the properties and events that have been requested
            // before the iframe was loaded
            for (const key in p.attributes) {
               requestProperty.call(this, "setAttribute", [key, p.attributes[key]]);
            }
            for (const key in p.properties) {
               requestProperty.call(this, key, [p.properties[key]]);
            }
            for (const key in p.callbacks) {
               requestProperty.call(this, "addEventListener", [key]);
            }
            for (const key of events) {
               if (p["on" + key] && (!p.callbacks[key] || p.callbacks[key].length <= 0)) {
                  requestProperty.call(this, "addEventListener", [key]);
               }
            }
         });
         console.log("MediaElementWrapper: initialised");
      }
      else {
         console.log("MediaElementWrapper: already initialised");
      }
   }
   
   // create the HTMLMediaElement's proxy methods
   for (const key of methods) {
      prototype[key] = makeMethod(key);
   }
   
   for (const key of asyncMethods) {
      prototype[key] = makeAsyncMethod(key);
   }
   
   // create the HTMLMediaElement's proxy properties
   for (const key of props) {
      Object.defineProperty(prototype, key, {
         set(value) {
            const p = privates.get(this);
            p.properties[key] = value;
            if (p.id !== -1) {
               requestProperty.call(this, key, [value]);
            }
         },
         get() {
            const p = privates.get(this);
            let result;
            if (p.id !== -1) {
               result = (async function () { await requestProperty.call(this, key); }).call(this);
            }
            else {
               result = p.properties[key];
            }
            return result;
         }
      });
   }
   
   for (const key of roProps) {
      Object.defineProperty(prototype, key, {
         get() {
            let result;
            if (privates.get(this).id !== -1) {
               result = (async function () { await requestProperty.call(this, key); }).call(this);
            }
            return result;
         }
      });
   }

   for (const key of events) {
      Object.defineProperty(prototype, "on" + key, {
         set(value) {
            const p = privates.get(this);
            if (value instanceof Object) {
               if (!p["on" + key] && (!p.callbacks[key] || p.callbacks[key].length <= 0) && p.id !== -1) {
                  requestProperty.call(this, "addEventListener", [key]);
               }
               p["on" + key] = value;
            }
            else {
               if ((!p.callbacks[key] || p.callbacks[key].length <= 0) && p.id !== -1) {
                  requestProperty.call(this, "removeEventListener", [key]);
               }
               p["on" + key] = null;
            }
         },
         get() {
            return privates.get(this)["on" + key];
         }
      });
   }

   Object.defineProperty(prototype, "src", {
      set(value) {
         const p = privates.get(this);
         delete iframes[p.id];
         p.id = -1;
         p.properties.src = value;
         console.log("MediaElementWrapper: Setting iframe src property to '" + value + "'.");
         srcOwnProperty.set.call(this, value + (value.includes("?") ? "&" : "?") + ORB_PLAYER_MAGIC_SUFFIX);
      },
      get() {
         return privates.get(this).properties.src;
      }
   });

   // callback for events received from the iframes
   window.addEventListener("message", (e) => {
      const msg = JSON.parse(e.data);
      if (msg.callId === -1 && iframes[msg.iframeId]) {
         const p = privates.get(iframes[msg.iframeId]);
         if (p) {
            const evt = new Event(msg.type);
            iframes[msg.iframeId].dispatchEvent(evt);
            if (p.callbacks[msg.type]) {
               for (const cb of p.callbacks[msg.type]) {
                  cb(evt);
               }
            }
            if (iframes[msg.iframeId]["on" + msg.type]) {
               iframes[msg.iframeId]["on" + msg.type](evt);
            }
         }
      }
   });

   return {
      initialise: initialise
   };
})();

hbbtv.objects.upgradeMediaElement = function(media) {
   // Create iframe element with Document.prototype.createElement as
   // document.createElement is overrided in object manager
   const element = Document.prototype.createElement.call(document, "iframe");
   hbbtv.objects.MediaElementWrapper.initialise.call(element);
   element.innerHTML = media.innerHTML;
   for (const key of media.getAttributeNames()) {
      element.setAttribute(key, media.getAttribute(key));
   }
   if (media.parentNode) {
      media.parentNode.insertBefore(element, media);
      media.parentNode.removeChild(media);
   }
   return element;
}