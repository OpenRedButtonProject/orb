/**
 * @fileOverview keyset class
 * See: {@link https://web.archive.org/web/20200219165053/http://www.oipf.tv/web-spec/volume5.html#keyset-class} 
 * @license ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

hbbtv.objects.KeySet = (function() {
   const prototype = {};
   const privates = new WeakMap();

   hbbtv.utils.defineConstantProperties(prototype, {
      RED: 0x1,
      GREEN: 0x2,
      YELLOW: 0x4,
      BLUE: 0x8,
      NAVIGATION: 0x10,
      VCR: 0x20,
      SCROLL: 0x40,
      INFO: 0x80,
      NUMERIC: 0x100,
      ALPHA: 0x200,
      OTHER: 0x400
   });

   Object.defineProperty(prototype, "value", {
      get() {
         if (privates.get(this).disabled) {
            return 0;
         }
         return hbbtv.bridge.manager.getKeyValues();
      }
   });

   Object.defineProperty(prototype, "otherKeys", {
      get() {
         return [];
      }
   });

   Object.defineProperty(prototype, "maximumValue", {
      get() {
         return hbbtv.bridge.manager.getKeyMaximumValue();
      }
   });

   Object.defineProperty(prototype, "maximumOtherKeys", {
      get() {
         return 0;
      }
   });

   Object.defineProperty(prototype, "supportsPointer", {
      get() {
         return false; // Not currently supported.
      }
   });

   prototype.setValue = function(value, otherKeys) {
      if (privates.get(this).disabled) {
         return 0;
      }
      return hbbtv.bridge.manager.setKeyValue(value, otherKeys);
   }

   prototype.getKeyIcon = function(code) {
      if (privates.get(this).disabled) {
         return "";
      }
      return hbbtv.bridge.manager.getKeyIcon(code);
   }

   function initialise(data) {
      privates.set(this, {});
      const p = privates.get(this);
      p.disabled = data.disabled;
   }

   return {
      prototype: prototype,
      initialise: initialise
   };
})();

hbbtv.objects.createKeySet = function(data) {
   const keyset = Object.create(hbbtv.objects.KeySet.prototype);
   hbbtv.objects.KeySet.initialise.call(keyset, data);
   return keyset;
}