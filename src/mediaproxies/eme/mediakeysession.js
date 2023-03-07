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

   /**
    * Specifications:
    *
    * <p>
    *
    * @returns {Closed}
    *
    * @name closed
    * @memberof MediaKeySession#
    */
   Object.defineProperty(prototype, "closed", {
      get: function() {
         return privates.get(this).proxy.callAsyncObserverMethod(MEDIA_KEY_SESSION_ID, "orb_closed");
      }
   });

   /**
    * Specifications:
    *
    * <p>
    *
    * @returns {Expiration}
    *
    * @name expiration
    * @memberof MediaKeySession#
    */
   Object.defineProperty(prototype, "expiration", {
      get: function() {
         return NaN;   // TODO
      }
   });

   /**
    * Specifications:
    *
    * <p>
    *
    * @returns {KeyStatuses}
    *
    * @name keyStatuses
    * @memberof MediaKeySession#
    */
   Object.defineProperty(prototype, "keyStatuses", {
      get: function() {
         return {};   // TODO
      }
   });

   /**
    * Specifications:
    *
    * <p>
    *
    * @returns {SessionId}
    *
    * @name sessionId
    * @memberof MediaKeySession#
    */
   Object.defineProperty(prototype, "sessionId", {
      get: function() {
         return "";   // TODO
      }
   });

   prototype.generateRequest = function(initDataType, initData) {
      return privates.get(this).proxy.callAsyncObserverMethod(
         MEDIA_KEY_SESSION_ID, "generateRequest", [initDataType, [...new Uint8Array(initData)]]
      );
   };

   prototype.update = function(licence) {
      return privates.get(this).proxy.callAsyncObserverMethod(
         MEDIA_KEY_SESSION_ID, "update", [[...new Uint8Array(licence)]]
      );
   };

   prototype.close = function() {
      return privates.get(this).proxy.callAsyncObserverMethod(MEDIA_KEY_SESSION_ID, "close");
   };

   prototype.load = function(sessionId) {
      return privates.get(this).proxy.callAsyncObserverMethod(MEDIA_KEY_SESSION_ID, "load", [sessionId]);
   };

   prototype.remove = function() {
      return privates.get(this).proxy.callAsyncObserverMethod(MEDIA_KEY_SESSION_ID, "remove");
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
