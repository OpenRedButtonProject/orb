/**
 * @fileOverview MediaError class
 * @license ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

hbbtv.objects.MediaError = (function() {
   const prototype = {};
   const privates = new WeakMap();
   const constants = ["MEDIA_ERR_ABORTED", "MEDIA_ERR_NETWORK", "MEDIA_ERR_DECODE", "MEDIA_ERR_SRC_NOT_SUPPORTED"];
   const roProps = ["code", "message"];

   for (const key of constants) {
      Object.defineProperty(prototype, key, {
         value: MediaError[key],
         writable: false
      });
   }

   for (const key of roProps) {
      Object.defineProperty(prototype, key, {
         get() {
            return privates.get(this)[key];
         }
      });
   }

   function initialise(code, message) {
      privates.set(this, {
         code,
         message
      });
   }

   return {
      prototype: prototype,
      initialise: initialise
   };
})();

hbbtv.objects.createMediaError = function(code, message) {
   const mediaError = Object.create(hbbtv.objects.MediaError.prototype);
   hbbtv.objects.MediaError.initialise.call(mediaError, code, message);
   return mediaError;
};