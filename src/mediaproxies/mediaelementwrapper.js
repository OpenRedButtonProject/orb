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
   const props = ["src","autoplay","controls","currentTime","playbackRate","volume","muted","loop","defaultMuted","crossOrigin","controlsList",
                  "defaultPlaybackRate","disableRemotePlayback","preservesPitch","srcObject","innerHTML"];
   const roProps = ["textTracks","audioTracks","videoTracks","paused","ended","currentSrc","buffered","error","duration",
                  "networkState","readyState","seekable"];
   const events = ["loadstart","progress","suspend","abort","error","emptied","stalled","loadedmetadata","loadeddata","canplay",
                  "canplaythrough","playing","waiting","seeking","seeked","ended","durationchange","timeupdate","play","pause",
                  "ratechange","resize","volumechange"];

   prototype.getAttribute = function(name) {
      if (props.includes(name)) {
         return privates.get(this).attributes[name];
      }
      return HTMLIFrameElement.prototype.getAttribute.apply(this, arguments);
   };

   prototype.setAttribute = function(name, value) {
      const p = privates.get(this);
      if (props.includes(name)) {
         if (p.attributes[name] !== value) {
            if (name === "src") {
               console.log("MediaElementWrapper: Setting iframe src attribute to '" + value + "'...");
               HTMLIFrameElement.prototype.setAttribute.call(this, name, value + (value.includes("?") ? "&" : "?") + ORB_PLAYER_MAGIC_SUFFIX);
            }
            p.attributes[name] = value;
            p.proxy.setRemoteObjectProperty("setAttribute", name, value);
         }
      }
      else {
         HTMLIFrameElement.prototype.setAttribute.call(this, name, value);
      }
   };

   prototype.removeAttribute = function(name) {
      const p = privates.get(this);
      if (props.includes(name)) {
         delete p.attributes.src;
         p.proxy.setRemoteObjectProperty("removeAttribute", name);
      }
      else {
         if (name === "src") {
            delete p.attributes.src;
            p.proxy.setRemoteObjectProperty("removeAttribute", name);
            console.log("MediaElementWrapper: Removing iframe src attribute...");
         }
         HTMLIFrameElement.prototype.removeAttribute.apply(this, arguments);
      }
   };

   prototype.appendChild = function(node) {
      if (node.nodeName) {
         if (node.nodeName.toLowerCase() === "source" && !this.src) {
            this.src = node.src;
         }
         else {
            privates.get(this).proxy.setRemoteObjectProperty("appendChild", node.outerHTML);
         }
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
               if (!this["on" + name] && callbacks.length === 1 ) {
                //  callProxyMethod.call(this, "addEventListener", [name]);
               }
            }
         }
         else {
            throw new TypeError("Failed to execute 'addEventListener' on 'EventTarget': parameter 2 is not of type 'Object'.");
         }
      }
      else {
         HTMLIFrameElement.prototype.addEventListener.apply(this, arguments);
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
                  if (!this["on" + name] && callbacks.length <= 0) {
                  //   callProxyMethod.call(this, "removeEventListener", [name]);
                  }
               }
            }
         }
         else {
            throw new TypeError("Failed to execute 'addEventListener' on 'EventTarget': parameter 2 is not of type 'Object'.");
         }
      }
      else {
         HTMLIFrameElement.prototype.addEventListener.apply(this, arguments);
      }
   }
   
   function makeMethod(name) {
      return function () {
         privates.get(this).proxy.setRemoteObjectProperty(name, ...Array.from(arguments).sort((a, b) => { return a - b; }));
      }
   }

   function makeAsyncMethod(name) {
      return function () {
         privates.get(this).proxy.setRemoteObjectProperty(name, ...Array.from(arguments).sort((a, b) => { return a - b; }));
         return Promise.resolve(); // TODO: need to fix this
      }
   }

   function initialise() {
      let p = privates.get(this);
      if (!p) {
         this.frameBorder = 0;
         Object.setPrototypeOf(this, prototype);
         privates.set(this, {
            attributes: { },
            properties: { 
               audioTracks: hbbtv.objects.createAudioTrackList(),
               videoTracks: hbbtv.objects.createVideoTrackList()
            },
            callbacks: { },
            proxy: hbbtv.objects.createIFrameObjectProxy(this, "video")
         });
         p = privates.get(this);

         HTMLIFrameElement.prototype.addEventListener.call(this, "load", () => {
            console.log("MediaElementWrapper: initialising iframe...");

             // TODO: find proper way of generating uuid
            p.proxy.initiateHandshake(Math.random(), this.contentWindow);
            p.properties.audioTracks.__orb_proxy__.initiateHandshake(Math.random(), this.contentWindow);
            p.properties.videoTracks.__orb_proxy__.initiateHandshake(Math.random(), this.contentWindow);
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
            if (p.properties[key] !== value) {
               if (key === "src") {
                  console.log("MediaElementWrapper: Setting iframe src property to '" + value + "'.");
                  srcOwnProperty.set.call(this, value + (value.includes("?") ? "&" : "?") + ORB_PLAYER_MAGIC_SUFFIX);
               }
               p.properties[key] = value;
               p.proxy.setRemoteObjectProperty(key, value);
            }
         },
         get() {
            return privates.get(this).properties[key];
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

   for (const key of events) {
      Object.defineProperty(prototype, "on" + key, {
         set(value) {
            const p = privates.get(this);
            if (value instanceof Object) {
               if (!p["on" + key] && (!p.callbacks[key] || p.callbacks[key].length <= 0)) {
                //  callProxyMethod.call(this, "addEventListener", [key]);
               }
               p["on" + key] = value;
            }
            else {
               if ((!p.callbacks[key] || p.callbacks[key].length <= 0)) {
               //   callProxyMethod.call(this, "removeEventListener", [key]);
               }
               p["on" + key] = null;
            }
         },
         get() {
            return privates.get(this)["on" + key];
         }
      });
   }

   return {
      initialise: initialise
   };
})();

hbbtv.objects.createMediaElementWrapper = function() {
   // Create iframe element with Document.prototype.createElement as
   // document.createElement is overrided in object manager
   const element = Document.prototype.createElement.call(document, "iframe");
   hbbtv.objects.MediaElementWrapper.initialise.call(element);
   return element;
};

hbbtv.objects.upgradeMediaElement = function(media) {
   const element = hbbtv.objects.createMediaElementWrapper();
   if (!media.getAttribute("src")) {
      for (const node of media.children) {
         if (node.nodeName && node.nodeName.toLowerCase() === "source") {
            element.src = node.src;
            break;
         }
      }
   }
   element.innerHTML = media.innerHTML;
   for (const key of media.getAttributeNames()) {
      element.setAttribute(key, media.getAttribute(key));
   }
   if (media.parentNode) {
      media.parentNode.insertBefore(element, media);
      media.parentNode.removeChild(media);
   }
   return element;
};