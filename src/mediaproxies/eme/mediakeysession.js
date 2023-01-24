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

   prototype.generateRequest = function(initDataType, initData) {
      return privates.get(this).proxy.callAsyncObserverMethod(
         MEDIA_KEY_SESSION_ID, "generateRequest", [initDataType, [...new Uint8Array(initData)]]
      );
   };

   prototype.addEventListener = function() { };

   function instantiate(proxy) {
      const obj = Object.create(prototype);
      privates.set(obj, { proxy });
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
