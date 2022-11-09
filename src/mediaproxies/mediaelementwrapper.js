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
   const roProps = ["textTracks","audioTracks","videoTracks","paused","ended","currentSrc","buffered","error","duration","networkState","readyState","seekable",
                  "HAVE_CURRENT_DATA","HAVE_ENOUGH_DATA","HAVE_FUTURE_DATA","HAVE_METADATA","HAVE_NOTHING","NETWORK_EMPTY","NETWORK_IDLE","NETWORK_LOADING",
                  "NETWORK_NO_SOURCE"];
   const events = ["loadstart","progress","suspend","abort","error","emptied","stalled","loadedmetadata","loadeddata","canplay",
                  "canplaythrough","playing","waiting","seeking","seeked","ended","durationchange","timeupdate","play","pause",
                  "ratechange","resize","volumechange"];

   prototype.getAttribute = function(name) {
      if (props.includes(name)) {
         return privates.get(this).videoDummy._attributes[name];
      }
      return HTMLIFrameElement.prototype.getAttribute.apply(this, arguments);
   };

   prototype.setAttribute = function(name, value) {
      const p = privates.get(this);
      if (props.includes(name)) {
         if (p.videoDummy._attributes[name] !== value) {
            p.videoDummy._attributes[name] = value;
            if (name === "src") {
               console.log("MediaElementWrapper: Setting iframe src attribute to '" + value + "'...");
               resetProxySession.call(this);
               HTMLIFrameElement.prototype.setAttribute.call(this, name, value + (value.includes("?") ? "&" : "?") + ORB_PLAYER_MAGIC_SUFFIX);
            }
            else {
               p.proxy.callMethod("setAttribute", [name, value]);
            }
         }
      }
      else {
         HTMLIFrameElement.prototype.setAttribute.call(this, name, value);
      }
   };

   prototype.removeAttribute = function(name) {
      const p = privates.get(this);
      if (props.includes(name)) {
         delete p.videoDummy._attributes.src;
         p.proxy.callMethod("removeAttribute", name);
      }
      else {
         if (name === "src") {
            delete p.videoDummy._attributes.src;
            p.proxy.callMethod("removeAttribute", name);
            console.log("MediaElementWrapper: Removing iframe src attribute...");
         }
         HTMLIFrameElement.prototype.removeAttribute.apply(this, arguments);
      }
   };

   prototype.appendChild = function(node) {
      if (node.nodeName && node.nodeName.toLowerCase() === "source" && !this.src) {
         this.src = node.src;
      }
      this.innerHTML += node.outerHTML;
   };
   
   function makeMethod(name) {
      return function () {
         privates.get(this).proxy.callMethod(name, Array.from(arguments).sort((a, b) => { return a - b; }));
      }
   }

   function makeAsyncMethod(name) {
      return function () {
         return privates.get(this).proxy.callAsyncMethod(name, Array.from(arguments).sort((a, b) => { return a - b; }));
      }
   }

   function resetProxySession() {
      const persistentProps = ["src","autoplay","controls","playbackRate","volume","muted","loop","defaultMuted",
                              "crossOrigin","controlsList","defaultPlaybackRate","disableRemotePlayback","preservesPitch"];
      const p = privates.get(this);
      const properties = { };
      p.proxy.invalidate();
      p.videoDummy.audioTracks.__orb_proxy__.invalidate();
      p.videoDummy.videoTracks.__orb_proxy__.invalidate();
      for (const attr in p.videoDummy._attributes) {
         p.proxy.callMethod("setAttribute", [attr, p.videoDummy._attributes[attr]]);
      }
      for (const key of persistentProps) {
         if (p.videoDummy[key] !== undefined) {
            properties[key] = p.videoDummy[key];
         }
      }
      p.proxy.setRemoteObjectProperties(properties);
   }

   /**
    * Helper class to act as intermediate between MediaElementWrapper and
    * IFrameMediaProxy, in order to prevent ping-pong effect when calling
    * a method or updating a property. In addition to that, updates
    * read-only properties of the media element from the other end, and when
    * requested with the MediaElementWrapper, return those.
    */
   function VideoDummy(parent) {
      this._attributes = { };
      this.audioTracks = hbbtv.objects.createAudioTrackList();
      this.videoTracks = hbbtv.objects.createVideoTrackList();
      this.dispatchEvent = function(e) {
         parent.dispatchEvent(e);
         if (typeof parent["on" + e.type] === "function") {
            parent["on" + e.type](e);
         }
      };
   }

   function initialise() {
      let p = privates.get(this);
      if (!p) {
         this.frameBorder = 0;
         Object.setPrototypeOf(this, prototype);
         const videoDummy = new VideoDummy(this);
         privates.set(this, {
            videoDummy: videoDummy,
            proxy: hbbtv.objects.createIFrameObjectProxy(videoDummy, "video")
         });
         p = privates.get(this);

         HTMLIFrameElement.prototype.addEventListener.call(this, "load", () => {
            if (this.src) {
               console.log("MediaElementWrapper: initialising iframe with src", this.src + "...");

               // TODO: find proper way of generating uuid
               p.proxy.initiateHandshake(Math.random(), this.contentWindow);
               p.videoDummy.audioTracks.__orb_proxy__.initiateHandshake(Math.random(), this.contentWindow);
               p.videoDummy.videoTracks.__orb_proxy__.initiateHandshake(Math.random(), this.contentWindow);
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
            if (p.videoDummy[key] !== value) {
               p.videoDummy[key] = value;
               if (key === "src") {
                  console.log("MediaElementWrapper: Setting iframe src property to '" + value + "'.");
                  resetProxySession.call(this);
                  srcOwnProperty.set.call(this, value + (value.includes("?") ? "&" : "?") + ORB_PLAYER_MAGIC_SUFFIX);
               }
               else {
                  p.proxy.setRemoteObjectProperties({[key]: value});
               }
            }
         },
         get() {
            return privates.get(this).videoDummy[key];
         }
      });
   }
   
   for (const key of roProps) {
      Object.defineProperty(prototype, key, {
         get() {
            return privates.get(this).videoDummy[key];
         }
      });
   }

   for (const key of events) {
      Object.defineProperty(prototype, "on" + key, {
         set(value) {
            const p = privates.get(this);
            if (value instanceof Object) {
               p["on" + key] = value;
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