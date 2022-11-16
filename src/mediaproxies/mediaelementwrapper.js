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
   const MEDIA_PROXY_ID = "HTMLMediaElement";
   const methods = ["pause","load","canPlayType","captureStream","fastSeek"];
   const asyncMethods = ["setMediaKeys","setSinkId"];
   const props = ["src","autoplay","controls","currentTime","playbackRate","volume","muted","loop","defaultMuted","crossOrigin","controlsList",
                  "defaultPlaybackRate","disableRemotePlayback","preservesPitch","srcObject"];
   const roProps = ["textTracks","audioTracks","videoTracks","paused","ended","currentSrc","buffered","error","duration","networkState","readyState","seekable",
                  "HAVE_CURRENT_DATA","HAVE_ENOUGH_DATA","HAVE_FUTURE_DATA","HAVE_METADATA","HAVE_NOTHING","NETWORK_EMPTY","NETWORK_IDLE","NETWORK_LOADING",
                  "NETWORK_NO_SOURCE"];
   const events = ["loadstart","progress","suspend","abort","error","emptied","stalled","loadedmetadata","loadeddata","canplay",
                  "canplaythrough","playing","waiting","seeking","seeked","ended","durationchange","timeupdate","play","pause",
                  "ratechange","resize","volumechange"];
   let lastMediaElement = undefined;

   prototype.getAttribute = function(name) {
      if (props.includes(name)) {
         return this[name];
      }
      return HTMLIFrameElement.prototype.getAttribute.apply(this, arguments);
   };

   prototype.setAttribute = function(name, value) {
      const p = privates.get(this);
      if (props.includes(name)) {
         this[name] = value;
      }
      else {
         HTMLIFrameElement.prototype.setAttribute.call(this, name, value);
      }
   };

   prototype.removeAttribute = function(name) {
      const p = privates.get(this);
      if (props.includes(name)) {
         delete p.videoDummy[name];
         p.proxy.callObserverMethod("removeAttribute", [name]);
      }
      else {
         HTMLIFrameElement.prototype.removeAttribute.apply(this, arguments);
      }
   };

   prototype.appendChild = function(node) {
      privates.get(this).divDummy.appendChild(node);
   };

   prototype.play = function() {
      if (!this.__added_to_media_sync__) { // check if the HTMLMediaElement is provided to MediaSynchroniser.addMediaObject() before we pause it
         if (lastMediaElement && lastMediaElement !== this && !lastMediaElement.paused) {
            lastMediaElement.pause();
         }
         lastMediaElement = this;
      }
      return privates.get(this).proxy.callAsyncObserverMethod(MEDIA_PROXY_ID, "play");
   };
   
   function makeMethod(name) {
      return function () {
         privates.get(this).proxy.callObserverMethod(MEDIA_PROXY_ID, name, Array.from(arguments).sort((a, b) => { return a - b; }));
      }
   }

   function makeAsyncMethod(name) {
      return function () {
         return privates.get(this).proxy.callAsyncObserverMethod(MEDIA_PROXY_ID, name, Array.from(arguments).sort((a, b) => { return a - b; }));
      }
   }

   function resetProxySession() {
      const persistentProps = ["src","autoplay","controls","playbackRate","volume","muted","loop","defaultMuted",
                              "crossOrigin","controlsList","defaultPlaybackRate","disableRemotePlayback","preservesPitch"];
      const p = privates.get(this);
      const properties = { };
      p.proxy.invalidate();
      for (const key of persistentProps) {
         if (p.videoDummy[key] !== undefined) {
            properties[key] = p.videoDummy[key];
         }
      }
      p.proxy.updateObserverProperties(MEDIA_PROXY_ID, properties);
   }

   /**
    * Helper class to act as intermediate between MediaElementWrapper and
    * IFrameMediaProxy, in order to prevent ping-pong effect when calling
    * a method or updating a property. In addition to that, updates
    * read-only properties of the media element from the other end, and when
    * requested with the MediaElementWrapper, return those.
    */
   function VideoDummy(parent, proxy) {
      this.HAVE_CURRENT_DATA = HTMLMediaElement.HAVE_CURRENT_DATA;
      this.HAVE_ENOUGH_DATA = HTMLMediaElement.HAVE_ENOUGH_DATA;
      this.HAVE_FUTURE_DATA = HTMLMediaElement.HAVE_FUTURE_DATA;
      this.HAVE_METADATA = HTMLMediaElement.HAVE_METADATA;
      this.HAVE_NOTHING = HTMLMediaElement.HAVE_NOTHING;
      this.NETWORK_EMPTY = HTMLMediaElement.NETWORK_EMPTY;
      this.NETWORK_IDLE = HTMLMediaElement.NETWORK_IDLE;
      this.NETWORK_LOADING = HTMLMediaElement.NETWORK_LOADING;
      this.NETWORK_NO_SOURCE = HTMLMediaElement.NETWORK_NO_SOURCE;
      this.audioTracks = hbbtv.objects.createAudioTrackList(proxy);
      this.videoTracks = hbbtv.objects.createVideoTrackList(proxy);
      this.textTracks = hbbtv.objects.createTextTrackList(proxy);
      this.dispatchEvent = function(e) {
         parent.dispatchEvent(e);
      };
      this.addTextTrack = function () {
         this.textTracks._addTextTrack.apply(this.textTracks, arguments);
      };
   }

   function initialise() {
      let p = privates.get(this);
      if (!p) {
         const thiz = this;
         this.frameBorder = 0;
         Object.setPrototypeOf(this, prototype);
         const proxy = hbbtv.objects.createIFrameObjectProxy();
         privates.set(this, {
            videoDummy: new VideoDummy(thiz, proxy),
            divDummy: document.createElement("div"), // will be used to manage append and remove Child on the iframe
            proxy: proxy,
         });
         p = privates.get(this);
         p.proxy.registerObserver(MEDIA_PROXY_ID, p.videoDummy);

         HTMLIFrameElement.prototype.addEventListener.call(this, "load", () => {
            if (this.src) {
               console.log("MediaElementWrapper: initialising iframe with src", this.src + "...");

               p.proxy.initiateHandshake(this.contentWindow)
               .then(() => { console.log("MediaElementWrapper: iframe proxy handshake completed successfully");});
            }
         });
         
         const observer = new MutationObserver(function(mutationsList) {
            for (const mutation of mutationsList) {
               for (const node of mutation.addedNodes) {
                  if (node.nodeName && node.nodeName.toLowerCase() === "source" && !thiz.src) {
                     thiz.src = node.src;
                  }
               }
            }
            p.proxy.updateObserverProperties(MEDIA_PROXY_ID, {innerHTML: p.divDummy.innerHTML});
         });
   
         observer.observe(p.divDummy, {
            childList: true
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
               if (key === "src" && value) {
                  console.log("MediaElementWrapper: Setting iframe src property to '" + value + "'.");
                  resetProxySession.call(this);
                  srcOwnProperty.set.call(this, value + (value.includes("?") ? "&" : "?") + ORB_PLAYER_MAGIC_SUFFIX);
               }
               else {
                  p.proxy.updateObserverProperties(MEDIA_PROXY_ID, {[key]: value});
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
         set(callback) {
            const p = privates.get(this);
            if (p["on" + key]) {
               this.removeEventListener(key, p["on" + key]);
            }

            if (callback instanceof Object) {
               p["on" + key] = callback;
               if (callback) {
                  this.addEventListener(key, callback);
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

   Object.defineProperty(prototype, "innerHTML", {
      set(value) {
         privates.get(this).divDummy.innerHTML = value;
      },
      get() {
         return privates.get(this).divDummy.innerHTML;
      }
   });

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
      element[key] = media.getAttribute(key);
   }
   if (media.parentNode) {
      media.parentNode.insertBefore(element, media);
      media.parentNode.removeChild(media);
   }
   return element;
};
