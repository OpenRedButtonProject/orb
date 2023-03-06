/**
 * @fileOverview MediaKeys class
 * @license ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */
hbbtv.objects.MediaKeys = (function() {
   const MEDIA_KEYS_ID = "MediaKeys";
   const privates = new WeakMap();
   const prototype = {};

   hbbtv.utils.defineGetterProperties(prototype, {
      orb_polyfilled() { return true; }
   });
   
   prototype.createSession = function(sessionType) {
      const proxy = privates.get(this).proxy;
      proxy.callObserverMethod(MEDIA_KEYS_ID, "createSession", [sessionType]);
      return hbbtv.objects.createMediaKeySession(proxy);
   };

   prototype.setServerCertificate = function(cert) {
      return privates.get(this).proxy.callAsyncObserverMethod(
         MEDIA_KEYS_ID, "setServerCertificate", [[...new Uint8Array(cert)]]
      );
   };

   prototype.orb_setIFrameProxy = function (proxy) {
      privates.get(this).proxy = proxy;
      proxy.registerObserver(MEDIA_KEYS_ID, this);
   };

   prototype.toJSON = function() {
      const mediaKeySystemAccess = privates.get(this).mediaKeySystemAccess;
      return { mediaKeySystemAccess: mediaKeySystemAccess.toJSON() };
   };

   function instantiate(mediaKeySystemAccess) {
      const obj = Object.create(prototype);
      privates.set(obj, { mediaKeySystemAccess });
      return obj;
   }

   return {
      instantiate
   };
})();

hbbtv.objects.createMediaKeys = function(mediaKeySystemAccess) {
   return hbbtv.objects.MediaKeys.instantiate(mediaKeySystemAccess);
};
