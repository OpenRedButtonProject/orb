/**
 * @fileOverview OIPF AVComponent class
 * @license ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

hbbtv.objects.AVComponent = (function() {
   const prototype = {};
   const privates = new WeakMap();
   const publicProperties = [
      'componentTag',
      'pid',
      'type',
      'encoding',
      'encrypted'
   ];

   /**
    * Specifications:
    *
    * <p>
    * Security: none.
    *
    * @returns {ComponentTag}
    *
    * @name componentTag
    * @memberof AVComponent#
    */
   Object.defineProperty(prototype, "componentTag", {
      get: function() {
         const p = privates.get(this);
         return p.avComponentData.componentTag;
      }
   });

   /**
    * Specifications:
    *
    * <p>
    * Security: none.
    *
    * @returns {Pid}
    *
    * @name pid
    * @memberof AVComponent#
    */
   Object.defineProperty(prototype, "pid", {
      get: function() {
         const p = privates.get(this);
         return p.avComponentData.pid;
      }
   });

   /**
    * Specifications:
    *
    * <p>
    * Security: none.
    *
    * @returns {Type}
    *
    * @name type
    * @memberof AVComponent#
    */
   Object.defineProperty(prototype, "type", {
      get: function() {
         const p = privates.get(this);
         return p.avComponentData.type;
      }
   });

   /**
    * Specifications:
    *
    * <p>
    * Security: none.
    *
    * @returns {Encoding}
    *
    * @name encoding
    * @memberof AVComponent#
    */
   Object.defineProperty(prototype, "encoding", {
      get: function() {
         const p = privates.get(this);
         return p.avComponentData.encoding;
      }
   });

   /**
    * Specifications:
    *
    * <p>
    * Security: none.
    *
    * @returns {Encrypted}
    *
    * @name encrypted
    * @memberof AVComponent#
    */
   Object.defineProperty(prototype, "encrypted", {
      get: function() {
         const p = privates.get(this);
         return p.avComponentData.encrypted;
      }
   });

   // Initialise an instance of prototype
   function initialise(avComponentData, vb) {
      console.log("AVComponent initialise");
      privates.set(this, {});
      const p = privates.get(this);
      p.avComponentData = avComponentData; // Hold reference to caller's object
      p.vb = (vb && hbbtv.utils.HAS_WEAKREF_SUPPORT) ? new WeakRef(vb) : vb;
   }

   // Private method to get a copy of the AVComponent data
   /*function cloneAVComponentData() {
      return Object.assign({}, privates.get(this).avComponentData);
   }*/

   prototype.toString = function() {
      return JSON.stringify(privates.get(this).avComponentData, publicProperties);
   }

   return {
      prototype: prototype,
      initialise: initialise,
      //cloneAVComponentData: cloneAVComponentData
   }
})();

hbbtv.objects.createAVComponent = function(avComponentData) {
   // Create new instance of hbbtv.objects.Channel.prototype
   const avComponent = Object.create(hbbtv.objects.AVComponent.prototype);
   hbbtv.objects.AVComponent.initialise.call(avComponent, avComponentData);
   return avComponent;
};