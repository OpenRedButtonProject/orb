/**
 * @fileOverview Installs a native candidate on initialisation.
 * @license ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

hbbtv.native = undefined;
hbbtv.nativeManager = (function() {
   let nativeCandidates = [];

   function initialise() {
      for (const native of nativeCandidates) {
         if (native.isNative()) {
            if (native.initialise !== undefined) {
               native.initialise();
            }
            hbbtv.native = native;
            console.debug("Installed native candidate " + hbbtv.native.name);
            return;
         }
      }
      console.error("Failed to install native candidate");
   };

   function registerNative(native) {
      nativeCandidates.push(native);
   }
   return {
      initialise: initialise,
      registerNative: registerNative
   };
})();