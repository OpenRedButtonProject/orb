/**
 * @fileOverview Service configuration and point of entry for initialisation.
 * @license ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

hbbtv.core = (function() {
   function initialise() {
      hbbtv.nativeManager.initialise();
      hbbtv.bridge.initialise();
      hbbtv.objectManager.initialise();
      hbbtv.mediaManager.initialise();
   };
   return {
      initialise: initialise
   };
})();