/**
 * @fileOverview MediaElementExtension class
 * @license ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

hbbtv.objects.MediaElementExtension = (function() {
   const privates = new WeakMap();
   const MEDIA_PROXY_ID = "HTMLMediaElement";
   const props = ["autoplay", "controls", "currentTime", "playbackRate", "volume", "muted", "loop", "defaultMuted", "crossOrigin", "controlsList",
      "defaultPlaybackRate", "disableRemotePlayback", "preservesPitch", "srcObject"
   ];

   function createPrototype() {
      const ORB_PLAYER_MAGIC_SUFFIX = "orb_player_magic_suffix";
      const prototype = Object.create(HTMLMediaElement.prototype);
      const methods = ["pause", "load"];
      const roProps = ["textTracks", "audioTracks", "videoTracks", "paused", "ended", "currentSrc", "error", "duration", "networkState", "readyState"];
      let lastMediaElement = undefined;

      prototype.getAttribute = function(name) {
         if (props.includes(name)) {
            return this[name];
         }
         return HTMLMediaElement.prototype.getAttribute.apply(this, arguments);
      };

      prototype.setAttribute = function(name, value) {
         if (props.includes(name)) {
            this[name] = value;
         } else {
            HTMLMediaElement.prototype.setAttribute.call(this, name, value);
         }
      };

      prototype.removeAttribute = function(name) {
         const p = privates.get(this);
         if (props.includes(name)) {
            delete p.videoDummy[name];
            p.iframeProxy.callObserverMethod(MEDIA_PROXY_ID, "removeAttribute", [name]);
         } else {
            HTMLMediaElement.prototype.removeAttribute.apply(this, arguments);
         }
      };

      prototype.play = function() {
         if (!this.__orb_addedToMediaSync__) { // check if the HTMLMediaElement is provided to MediaSynchroniser.addMediaObject() before we pause it
            if (lastMediaElement && lastMediaElement !== this && !lastMediaElement.paused) {
               lastMediaElement.pause();
            }
            lastMediaElement = this;
         }
         return privates.get(this).iframeProxy.callAsyncObserverMethod(MEDIA_PROXY_ID, "play");
      };

      prototype.appendChild = function(node) {
         privates.get(this).divDummy.appendChild(node);
      };

      prototype.insertBefore = function(newNode, otherNode) {
         privates.get(this).divDummy.insertBefore(newNode, otherNode);
      };

      prototype.removeChild = function(node) {
         privates.get(this).divDummy.removeChild(node);
      };

      function makeMethod(name) {
         return function() {
            privates.get(this).iframeProxy.callObserverMethod(MEDIA_PROXY_ID, name, Array.from(arguments).sort((a, b) => {
               return a - b;
            }));
         }
      }

      function resetProxySession() {
         const persistentProps = ["src", "autoplay", "controls", "playbackRate", "volume", "muted", "loop", "defaultMuted",
            "crossOrigin", "controlsList", "defaultPlaybackRate", "disableRemotePlayback", "preservesPitch"
         ];
         const p = privates.get(this);
         const properties = {};
         p.iframeProxy.invalidate();
         for (const key in p.videoDummy) {
            if (persistentProps.includes(key)) {
               properties[key] = p.videoDummy[key];
            }
         }
         p.iframeProxy.updateObserverProperties(MEDIA_PROXY_ID, properties);
      }

      // create the HTMLMediaElement's proxy methods
      for (const key of methods) {
         prototype[key] = makeMethod(key);
      }

      // create the HTMLMediaElement's proxy properties
      for (const key of props) {
         Object.defineProperty(prototype, key, {
            set(value) {
               const p = privates.get(this);
               if (p.videoDummy[key] !== value) {
                  p.videoDummy[key] = value;
                  p.iframeProxy.updateObserverProperties(MEDIA_PROXY_ID, {
                     [key]: value
                  });
               }
            },
            get() {
               return privates.get(this).videoDummy[key];
            }
         });
      }

      // mandatory step as setAttribute examines the props array
      // before setting a value to an attribute, and we need to be able
      // to change the src property that way as well
      props.push("src");

      Object.defineProperty(prototype, "src", {
         set(value) {
            const p = privates.get(this);
            if (p.videoDummy.src !== value) {
               p.videoDummy.src = value;
               if (value) {
                  console.log("MediaElementExtension: Setting iframe src property to '" + value + "'.");
                  resetProxySession.call(this);
                  p.iframe.src = value + (value.includes("?") ? "&" : "?") + ORB_PLAYER_MAGIC_SUFFIX;
               } else {
                  p.iframeProxy.updateObserverProperties(MEDIA_PROXY_ID, {
                     src: value
                  });
               }
            }
         },
         get() {
            return privates.get(this).videoDummy.src;
         }
      });

      const hiddenOwnProperty = Object.getOwnPropertyDescriptor(HTMLElement.prototype, "hidden");
      Object.defineProperty(prototype, "hidden", {
         set(value) {
            privates.get(this).iframe.hidden = value;
            hiddenOwnProperty.set.call(this, value);
         },
         get() {
            return hiddenOwnProperty.get.call(this);
         }
      });

      Object.defineProperty(prototype, "innerHTML", {
         set(value) {
            privates.get(this).divDummy.innerHTML = value;
         },
         get() {
            return privates.get(this).divDummy.innerHTML;
         }
      });

      for (const key of roProps) {
         Object.defineProperty(prototype, key, {
            get() {
               return privates.get(this).videoDummy[key];
            }
         });
      }

      return prototype;
   }

   // Mutation observer
   function addDocumentMutationIntercept() {
      const observer = new MutationObserver(function(mutationsList) {
         for (const mutation of mutationsList) {
            for (const node of mutation.removedNodes) {
               if (node.nodeName && (node.nodeName.toLowerCase() === "video" || node.nodeName.toLowerCase() === "audio")) {
                  const p = privates.get(node);
                  if (p && p.iframe.parentNode) {
                     p.iframe.parentNode.removeChild(p.iframe);
                  }
               }
            }
            for (const node of mutation.addedNodes) {
               if (node.nodeName && (node.nodeName.toLowerCase() === "video" || node.nodeName.toLowerCase() === "audio")) {
                  hbbtv.objects.upgradeMediaElement(node);
               }
            }
         }
      });
      const config = {
         childList: true,
         subtree: true
      };
      observer.observe(document.documentElement || document.body, config);
   }

   addDocumentMutationIntercept();
   const prototype = createPrototype();

   /**
    * Helper class to act as intermediate between MediaElementExtension and
    * IFrameMediaProxy, in order to prevent ping-pong effect when calling
    * a method or updating a property. In addition to that, updates
    * read-only properties of the media element from the other end, and when
    * requested with the MediaElementExtension, return those.
    */
   function VideoDummy(parent, iframeProxy) {
      this.audioTracks = hbbtv.objects.createAudioTrackList(iframeProxy);
      this.videoTracks = hbbtv.objects.createVideoTrackList(iframeProxy);
      this.textTracks = hbbtv.objects.createTextTrackList(parent, iframeProxy);
      this.dispatchEvent = function(e) {
         parent.dispatchEvent(e);
      };
      this.addTextTrack = function() {
         this.textTracks.orb_addTextTrack.apply(this.textTracks, arguments);
      };
   }

   function updateIFrameCSS() {
      const iframe = privates.get(this).iframe;
      const style = window.getComputedStyle(this);
      Array.from(style).forEach(
         key => iframe.style.setProperty(key, style.getPropertyValue(key), style.getPropertyPriority(key))
      );
      if (iframe.style.position !== "fixed") {
         const bounds = this.getBoundingClientRect();
         iframe.style.left = bounds.left;
         iframe.style.top = bounds.top;
         iframe.style.position = "absolute";
      }
   }

   function initialise() {
      let p = privates.get(this);
      if (!p) {
         const thiz = this;
         const iframeProxy = hbbtv.objects.createIFrameObjectProxy();
         const videoDummy = new VideoDummy(this, iframeProxy);

         // will be used as a placeholder to store the attributes of
         // the original video element, and after the custom prototype
         // is set, update all the stored properties with the original
         // values
         const initialProps = {};

         // will be used to intercept child addition/removal of
         // the actual video element
         const divDummy = document.createElement("div");

         // extract attributes and reset them before setting the custom prototype
         for (const key of this.getAttributeNames()) {
            if (props.includes(key)) {
               initialProps[key] = this.getAttribute(key) || true;
               this.removeAttribute(key);
            }
         }
         if (!initialProps.src) {
            for (const node of this.children) {
               if (node.nodeName && node.nodeName.toLowerCase() === "source") {
                  initialProps.src = node.src;
                  break;
               }
            }
         }
         initialProps.innerHTML = this.innerHTML;
         this.innerHTML = "";

         Object.setPrototypeOf(this, prototype);
         privates.set(this, {
            videoDummy,
            divDummy,
            iframeProxy,
            iframe: document.createElement("iframe")
         });

         p = privates.get(this);
         iframeProxy.registerObserver(MEDIA_PROXY_ID, videoDummy);

         p.iframe.frameBorder = 0;
         p.iframe.addEventListener("load", () => {
            if (thiz.src) {
               console.log("MediaElementExtension: initialising iframe with src", thiz.src + "...");

               iframeProxy.initiateHandshake(p.iframe.contentWindow)
                  .then(() => {
                     console.log("MediaElementExtension: iframe proxy handshake completed successfully");
                  });
            }
         });

         // whenever there is a change on the video element style,
         // update the iframe style as well
         const styleObserver = new MutationObserver(function() {
            updateIFrameCSS.call(thiz);
         });
         styleObserver.observe(this, {
            attributes: true,
            attributeFilter: ["style"]
         });

         // whenever there is a change in the childList of the divDummy,
         // check if the added/removed child is <source>, and if so
         // set/unset the iframe's src property
         const childListObserver = new MutationObserver(function(mutationsList) {
            for (const mutation of mutationsList) {
               for (const node of mutation.removedNodes) {
                  if (node.nodeName && node.nodeName.toLowerCase() === "source") {
                     thiz.removeAttribute("src");
                  }
               }
               for (const node of mutation.addedNodes) {
                  if (node.nodeName && node.nodeName.toLowerCase() === "source" && !thiz.src) {
                     thiz.src = node.src;
                  }
               }
            }
            iframeProxy.updateObserverProperties(MEDIA_PROXY_ID, {
               innerHTML: divDummy.innerHTML
            });
         });
         childListObserver.observe(divDummy, {
            childList: true
         });

         // update the new prototype with the values stored in initialProps
         for (const prop in initialProps) {
            this[prop] = initialProps[prop];
         }
         console.log("MediaElementExtension: initialised");
      }
      if (this.parentNode && !p.iframe.parentNode) {
         this.parentNode.appendChild(p.iframe);
         updateIFrameCSS.call(this);
      }
   }

   return {
      initialise: initialise
   };
})();

hbbtv.objects.upgradeMediaElement = function(media) {
   hbbtv.objects.MediaElementExtension.initialise.call(media);
};