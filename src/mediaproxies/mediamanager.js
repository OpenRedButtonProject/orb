/**
 * @fileOverview Monitors the document and certain JavaScript mechanisms for objects. Builds or 
 * upgrades those objects if there is a registered handler.
 * @license ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */
hbbtv.mediaManager = (function() {
   let objectHandlers = {};
   let fallbackHandlers = undefined;
   const mediaProxy = hbbtv.objects.createIFrameObjectProxy();

   function initialise() {
      addSourceManipulationIntercept();
      addMutationIntercept();

      const __play = HTMLMediaElement.prototype.play;

      // we override play() for the DASH playback as we end up receiving
      // Uncaught (in promise) DOMException: The play() request was interrupted by a new load request.
      // when calling play() immediately after setting the src attribute
      HTMLMediaElement.prototype.play = function() {
         const thiz = this;
         return new Promise((resolve, reject) => {
            if (thiz.readyState < 2) {
               const playFcn = function() {
                  thiz.removeEventListener("loadeddata", playFcn);
                  __play.call(thiz).then(resolve, reject);
               };
               thiz.addEventListener("loadeddata", playFcn, true);
            } else {
               __play.call(thiz).then(resolve, reject);
            }
         });
      }
   }

   function registerObject(handlers) {
      objectHandlers[handlers.getName()] = handlers;
      if (handlers.getName() === "native") {
         fallbackHandlers = handlers;
      }
   }

   function getHandlerByContentType(type) {
      for (const key in objectHandlers) {
         const supportedTypes = objectHandlers[key].getSupportedContentTypes();
         for (const t of supportedTypes) {
            if (type.includes(t)) {
               return objectHandlers[key];
            }
         }
      }
      return undefined;
   }

   function setObjectHandler(nextHandler, src) {
      this.__objectType = nextHandler.getName();
      nextHandler.initialise(this, src);
   }

   function upgradeObject(src) {
      const object = this;
      let objType = object.__objectType;
      if (!src) {
         return Promise.reject("Playback source is not defined.");
      } else if ((object.getAttribute("src") === src && objType) || src.toLowerCase().startsWith("blob:")) {
         return Promise.resolve();
      }

      // consider playback anchors with url
      let ext = src.split("#")[0];
      ext = ext.split("?")[0];
      // first check each objectHandler supported extensions
      if (ext) {
         ext = ext.split(".").pop().toLowerCase();
         console.log("MediaManager: Checking extension support for ." + ext + "...");
         for (const key in objectHandlers) {
            if (objectHandlers[key].getSupportedExtensions().indexOf(ext) >= 0) {
               setObjectHandler.call(object, objectHandlers[key], src);
               return Promise.resolve();
            }
         }
      }

      // if no supported extension is found, request the content-type of the source
      const request = new XMLHttpRequest();
      request.open('HEAD', src);
      request.send();
      return new Promise((resolve, reject) => {
         request.onabort = request.onerror = () => {
            reject("An error occurred while requesting the content type.");
         };
         request.onload = () => {
            try {
               const contentType = request
                  .getAllResponseHeaders()
                  .split('\n')
                  .find(header => header.toLowerCase().startsWith('content-type'))
                  .split(':')[1]
                  .trim();

               console.log("MediaManager: Requested content of type " + contentType);
               const nextHandler = getHandlerByContentType(contentType);
               if (nextHandler) {
                  setObjectHandler.call(object, nextHandler, src);
                  resolve();
               } else {
                  reject("Failed to find a registered playback proxy for the content type " + contentType);
               }
            } catch (e) {
               reject(e);
            }
         };
      })
   }

   function upgradeToFallback(node, err) {
      console.warn("MediaManager: Failed to upgrade object. Fallback to native proxy. Error: " + err);
      if (fallbackHandlers) {
         node.__objectType = fallbackHandlers.getName();
         fallbackHandlers.initialise(node, node.src);
      }
   }

   function addSourceManipulationIntercept() {
      const ownProperty = Object.getOwnPropertyDescriptor(HTMLMediaElement.prototype, "src");
      Object.defineProperty(HTMLMediaElement.prototype, "src", {
         set(val) {
            this.setAttribute("src", val);
         },
         get() {
            return ownProperty.get.call(this);
         }
      });

      HTMLMediaElement.prototype.setAttribute = function(name, value) {
         if (name === "src" && !this.__objectType) {
            const thiz = this;
            console.log("MediaManager: intercepted src manipulation. new src: " + value);
            upgradeObject.call(this, value).catch(e => upgradeToFallback(thiz, e));
         }
         Element.prototype.setAttribute.apply(this, arguments);
      };
   }

   function registerObservers(media) {
      const MEDIA_PROXY_ID = "HTMLMediaElement";
      mediaProxy.registerObserver(MEDIA_PROXY_ID, media);

      const audioTracks = hbbtv.objects.createAudioTrackList(mediaProxy);
      const videoTracks = hbbtv.objects.createVideoTrackList(mediaProxy);
      const textTracks = hbbtv.objects.createTextTrackList(media, mediaProxy);

      Object.defineProperty(media, "audioTracks", {
         value: audioTracks,
         writable: false
      });
      Object.defineProperty(media, "videoTracks", {
         value: videoTracks,
         writable: false
      });
      Object.defineProperty(media, "textTracks", {
         value: textTracks,
         writable: false
      });

      const genericEvents = [
         "loadstart", "progress", "suspend", "abort", "error", "emptied", "stalled", "canplay",
         "canplaythrough", "playing", "waiting", "seeking", "seeked", "resize", "__obs_onerror__"
      ];
      const genericHandler = (e) => {
         mediaProxy.dispatchEvent(MEDIA_PROXY_ID, e);
      };
      const propsUpdateCallback = function(e) {
         const props = {};
         const keys = ["currentTime", "playbackRate", "volume", "muted", "loop", "defaultMuted", "defaultPlaybackRate", "disableRemotePlayback",
            "preservesPitch", "paused", "ended", "currentSrc", "error", "duration", "networkState", "readyState"
         ];
         for (const key of keys) {
            props[key] = media[key];
         }
         mediaProxy.updateObserverProperties(MEDIA_PROXY_ID, props);
         mediaProxy.dispatchEvent(MEDIA_PROXY_ID, e);
         console.log("iframe: update properties", props);
      };
      const makeCallback = function(property) {
         return function(e) {
            mediaProxy.updateObserverProperties(MEDIA_PROXY_ID, {
               [property]: media[property]
            });
            mediaProxy.dispatchEvent(MEDIA_PROXY_ID, e);
         }
      };
      for (const evt of genericEvents) {
         media.addEventListener(evt, genericHandler, true);
      }
      media.addEventListener("loadeddata", propsUpdateCallback, true);
      media.addEventListener("play", propsUpdateCallback, true);
      media.addEventListener("ended", propsUpdateCallback, true);
      media.addEventListener("pause", propsUpdateCallback, true);
      media.addEventListener("durationchanged", makeCallback("duration"), true);
      media.addEventListener("ratechange", makeCallback("playbackRate"), true);
      media.addEventListener("volumechange", makeCallback("volume"), true);
      media.addEventListener("timeupdate", (() => {
         const cb = makeCallback("currentTime");
         // will be used to prevent the event from being dispatched too frequently
         // from the iframe to the main window, in order to improve performance
         let counter = 0;
         return function(e) {
            if (counter++ % 5 === 0) {
               cb(e);
            }
         };
      })(), true);
      media.addTextTrack = function() {
         return textTracks.orb_addTextTrack.apply(textTracks, arguments);
      };
   }

   // Mutation observer
   function addMutationIntercept() {
      const observer = new MutationObserver(function(mutationsList) {
         for (const mutation of mutationsList) {
            for (const node of mutation.addedNodes) {
               if (node.nodeName && node.nodeName.toLowerCase() === "video" || node.nodeName.toLowerCase() === "audio") {
                  registerObservers(node);
               }
            }
         }
      });

      observer.observe(document.documentElement || document.body, {
         childList: true,
         subtree: true
      });
   }

   return {
      initialise: initialise,
      registerObject: registerObject
   };
})();