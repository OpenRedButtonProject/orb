/**
 * @fileOverview OIPF AVVideoComponent class
 * @license ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

hbbtv.objects.AVVideoComponent = (function() {
   const prototype = {};
   const privates = new WeakMap();
   const publicProperties = [
      'componentTag',
      'pid',
      'type',
      'encoding',
      'encrypted',
      'aspectRatio'
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
    * @memberof AVVideoComponent#
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
    * @memberof AVVideoComponent#
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
    * @memberof AVVideoComponent#
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
    * @memberof AVVideoComponent#
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
    * @memberof AVVideoComponent#
    */
   Object.defineProperty(prototype, "encrypted", {
      get: function() {
         const p = privates.get(this);
         return p.avComponentData.encrypted;
      }
   });

   /**
    * Specifications:
    *
    * <p>
    * Security: none.
    *
    * @returns {AspectRatio}
    *
    * @name aspectRatio
    * @memberof AVVideoComponent#
    */
   Object.defineProperty(prototype, "aspectRatio", {
      get: function() {
         const p = privates.get(this);
         let aspectRatio = p.avComponentData.aspectRatio;
         if (aspectRatio !== undefined) {
            if (aspectRatio === 0) {
               aspectRatio = 1.33; // 4:3
            } else {
               aspectRatio = 1.78; // 16:9
            }
         }
         return aspectRatio;
      }
   });

   // Initialise an instance of prototype
   function initialise(avVideoComponentData, vb) {
      privates.set(this, {});
      const p = privates.get(this);
      p.avComponentData = avVideoComponentData; // Hold reference to caller's object
   }

   function getId() {
      const p = privates.get(this);
      return p.avComponentData.id;
   }

   prototype.toString = function() {
      return JSON.stringify(privates.get(this).avComponentData, publicProperties);
   }

   return {
      prototype: prototype,
      initialise: initialise,
      getId: getId
   }
})();

hbbtv.objects.createAVVideoComponent = function(avVideoComponentData) {
   // Create new instance of hbbtv.objects.AVVideoComponent.prototype
   const avVideoComponent = Object.create(hbbtv.objects.AVVideoComponent.prototype);
   hbbtv.objects.AVVideoComponent.initialise.call(avVideoComponent, avVideoComponentData);
   return avVideoComponent;
};