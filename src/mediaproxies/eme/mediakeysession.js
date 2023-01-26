/**
 * @fileOverview MediaKeySession class
 * @license ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */
hbbtv.objects.MediaKeySession = (function() {
   const MEDIA_KEY_SESSION_ID = "MediaKeySession";
   const privates = new WeakMap();
   const prototype = {};
   const events = ["keystatuseschange", "message"];
   const evtTargetMethods = ["addEventListener", "removeEventListener", "dispatchEvent"];

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
            } else {
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

   prototype.generateRequest = function(initDataType, initData) {
      return privates.get(this).proxy.callAsyncObserverMethod(
         MEDIA_KEY_SESSION_ID, "generateRequest", [initDataType, [...new Uint8Array(initData)]]
      );
   };

   function instantiate(proxy) {
      const obj = Object.create(prototype);
      privates.set(obj, { 
         proxy,
         eventTarget: document.createDocumentFragment()
      });
      proxy.registerObserver(MEDIA_KEY_SESSION_ID, obj);
      return obj;
   }

   return {
      instantiate
   };
})();

hbbtv.objects.createMediaKeySession = function(iframeProxy) {
   return hbbtv.objects.MediaKeySession.instantiate(iframeProxy);
};
