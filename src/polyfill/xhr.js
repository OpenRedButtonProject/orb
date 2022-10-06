/**
 * @fileOverview XHR polyfill
 * @license ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

(function() {
   window.XMLHttpRequest = class extends window.XMLHttpRequest {
      getResponseHeader(headerName) {
         // Shall return null for dvb: protocol
         return new URL(this.responseURL).protocol === "dvb:" ? null :
            super.getResponseHeader(headerName);
      }
      getAllResponseHeaders() {
         // Shall return an empty string for dvb: protocol
         return new URL(this.responseURL).protocol === "dvb:" ? "" :
            super.getAllResponseHeaders();
      }
   };
})();