/**
 * @fileOverview Window close polyfill
 * @license ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

(function() {
   // Override window.close()
   window.close = function() {
      hbbtv.bridge.manager.destroyApplication();
   };
})();