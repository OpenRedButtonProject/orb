/**
 * @fileOverview Debug polyfill
 * See: {@link https://web.archive.org/web/20200219165053/http://www.oipf.tv/web-spec/volume5.html#debug-print-api}
 * @license ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

(function() {
   if (typeof window.debug === 'undefined') {
      window.debug = function(arg) {
         console.debug(arg);
      };
   }
})();