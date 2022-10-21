/**
 * @fileOverview MediaElementWrapper class
 * @license ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

 hbbtv.objects.MediaElementWrapper = (function() {
   const prototype = Object.create(HTMLIFrameElement.prototype);
   const privates = new WeakMap();
   const methods = ["pause","load","canPlayType","captureStream","fastSeek"];
   const asyncMethods = ["play","setMediaKeys","setSinkId"];
   const props = ["src","autoplay","controls","currentTime","playbackRate","volume","muted","loop","defaultMuted","crossOrigin","controlsList",
                  "defaultPlaybackRate","disableRemotePlayback","preservesPitch","srcObject",
                  "onloadstart","onprogress","onsuspend","onabort","onerror","onemptied","onstalled","onloadedmetadata","onloadeddata","oncanplay",
                  "canplaythrough","onplaying","onwaiting","onseeking","onseeked","onended","ondurationchange","ontimeupdate","onplay","onpause",
                  "ratechange","onresize","onvolumechange"];
   const roProps = ["textTracks","audioTracks","videoTracks","paused","ended","currentSrc","buffered","error","duration",
                  "networkState","readyState","seekable"];

   prototype.getAttribute = function(name) {
      if (props.includes(name)) {
         const p = privates.get(this);
         if (p.mediaElement) {
            return p.mediaElement.getAttribute(name);
         }
         return p.attributes[name];
      }
      return Element.prototype.getAttribute.call(this, name);
   };

   prototype.setAttribute = function(name, value) {
      if (props.includes(name)) {
         const p = privates.get(this);
         p.attributes[name] = value;
         if (p.mediaElement) {
            p.mediaElement.setAttribute(name, value);
         }
      }
      else {
         Element.prototype.setAttribute.call(this, name, value);
      }
   };

   prototype.addEventListener = function (name, callback) {
      const p = privates.get(this);
      if (p.mediaElement) {
         p.mediaElement.addEventListener(name, callback);
      }
      else {
         if (!p.events[name]) {
            p.events[name] = [];
         }
         p.events[name].push(callback);
      }
   }

   prototype.removeEventListener = function (name, callback) {
      const p = privates.get(this);
      if (p.mediaElement) {
         p.mediaElement.removeEventListener(name, callback);
      }
      else {
         const callbacks = p.events[name];
         if (callbacks) {
            const index = callbacks.indexOf(callback);
            if (index >= 0) {
               callbacks.splice(index, 1);
               if (callbacks.length <= 0) {
                  delete p.events[name];
               }
            }
         }
      }
   }

   function makeMethod(name) {
      return function () {
         let result;
         const p = privates.get(this);
         if (p.mediaElement) {
            result = p.mediaElement[name](arguments);
         }
         return result;
      }
   }

   function makeAsyncMethod(name) {
      return function() {
         const thiz = this;
         const p = privates.get(this);
         if (p.mediaElement) {
            return p.mediaElement[name](arguments);
         }
         return new Promise((resolve) => {
            function deferMethod() {
               Element.prototype.removeEventListener.call(thiz, "load", deferMethod);
               resolve(p.mediaElement[name](arguments));
            }
            Element.prototype.addEventListener.call(thiz, "load", deferMethod);
         });
      }
   }

   function initialise(tagName) {
      let p = privates.get(this);
      if (!p) {
         this.frameBorder = 0;
         Object.setPrototypeOf(this, prototype);
         privates.set(this, {
            attributes: { },
            properties: { },
            events: { }
         });
         p = privates.get(this);

         function onload() {
            console.log("MediaElementWrapper: iframe loaded");
            Element.prototype.removeEventListener.call(this, "load", onload);
            this.contentDocument.body.style.margin = 0;
            p.mediaElement = this.contentDocument.createElement(tagName);
            p.mediaElement.style.cssText = "width:100%;height:100%";

            // set the properties and events that have been requested
            // before the iframe was loaded
            for (const key in p.attributes) {
               p.mediaElement.setAttribute(key, p.attributes[key]);
            }
            for (const key in p.properties) {
               p.mediaElement[key] = p.properties[key];
            }
            for (const key in p.events) {
               for (const cb of p.events[key]) {
                  p.mediaElement.addEventListener(key, cb);
               }
            }
            p.events = { };
            this.contentDocument.body.appendChild(p.mediaElement);
         }
         Element.prototype.addEventListener.call(this, "load", onload);
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
            if (p.mediaElement) {
               p.mediaElement[key] = value;
            }
         },
         get() {
            const p = privates.get(this);
            if (p.mediaElement) {
               return p.mediaElement[key];
            }
            return p.properties[key];
         }
      });
   }
   
   for (const key of roProps) {
      Object.defineProperty(prototype, key, {
         get() {
            const p = privates.get(this);
            if (p.mediaElement) {
               return p.mediaElement[key];
            }
            return undefined;
         }
      });
   }

   return {
      initialise: initialise
   };
})();

hbbtv.objects.upgradeMediaElement = function(media) {
   // Create iframe element with Document.prototype.createElement as
   // document.createElement is overrided in object manager
   const element = Document.prototype.createElement.call(document, "iframe");
   element.src = media.src + "orb_player_magic_suffix";
   hbbtv.objects.MediaElementWrapper.initialise.call(element, media.tagName);
   for (const key of media.getAttributeNames()) {
      element.setAttribute(key, media.getAttribute(key));
   }
   if (media.parentNode) {
      media.parentNode.insertBefore(element, media);
      media.parentNode.removeChild(media);
   }
   return element;
}