/**
 * @fileOverview MediaKeySystemAccess class
 * @license ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */
hbbtv.objects.MediaKeySystemAccess = (function() {
   const privates = new WeakMap();
   const prototype = {};
   
   hbbtv.utils.defineGetterProperties(prototype, {
      keySystem() {
         return privates.get(this).keySystem;
      }
   });

   prototype.getConfiguration = function () {
      return privates.get(this).configuration;
   };

   prototype.toJSON = function() {
      const p = privates.get(this);
      return { keySystem: p.keySystem, configuration: p.configuration };
   };

   prototype.createMediaKeys = function () {
      return Promise.resolve(hbbtv.objects.createMediaKeys(this));
   };

   function instantiate(keySystem, configuration) {
      const obj = Object.create(prototype);
      Object.freeze(configuration);
      privates.set(obj, { keySystem, configuration });
      return obj;
   }

   return {
      instantiate
   };
})();

hbbtv.objects.createMediaKeySystemAccess = function(keySystem, configuration) {
   return hbbtv.objects.MediaKeySystemAccess.instantiate(keySystem, configuration);
};