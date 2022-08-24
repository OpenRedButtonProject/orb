/**
 * @fileOverview LocalSystem class
 * See: {@link https://web.archive.org/web/20200219165053/http://www.oipf.tv/web-spec/volume5.html#localsystem-class} 
 * @license ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

hbbtv.objects.LocalSystem = (function() {
   const prototype = {};
   let gLocalSystem = null;

   Object.defineProperty(prototype, "modelName", {
      get() {
         return getLocalSystem().modelName;
      }
   });

   Object.defineProperty(prototype, "vendorName", {
      get() {
         return getLocalSystem().vendorName;
      }
   });

   Object.defineProperty(prototype, "softwareVersion", {
      get() {
         return getLocalSystem().softwareVersion;
      }
   });

   Object.defineProperty(prototype, "hardwareVersion", {
      get() {
         return getLocalSystem().hardwareVersion;
      }
   });

   function initialise() {
   }

   function getLocalSystem() {
      if (gLocalSystem === null) {
         gLocalSystem = hbbtv.bridge.configuration.getLocalSystem();
      }
      return gLocalSystem;
   }

   return {
      prototype: prototype,
      initialise: initialise
   }
})();

hbbtv.objects.createLocalSystem = function() {
   const localSystem = Object.create(hbbtv.objects.LocalSystem.prototype);
   hbbtv.objects.LocalSystem.initialise.call(localSystem);
   return localSystem;
}