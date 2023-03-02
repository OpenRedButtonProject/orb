/**
 * @fileOverview OIPF AVSubtitleComponent class
 * @license ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

hbbtv.objects.AVSubtitleComponent = (function() {
   const prototype = {};
   const privates = new WeakMap();
   const publicProperties = [
      'componentTag',
      'pid',
      'type',
      'encoding',
      'encrypted',
      'language',
      'hearingImpaired',
      'label'
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
    * @memberof AVSubtitleComponent#
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
    * @memberof AVSubtitleComponent#
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
    * @memberof AVSubtitleComponent#
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
    * @memberof AVSubtitleComponent#
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
    * @memberof AVSubtitleComponent#
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
    * @returns {Language}
    *
    * @name language
    * @memberof AVSubtitleComponent#
    */
   Object.defineProperty(prototype, "language", {
      get: function() {
         const p = privates.get(this);
         return p.avComponentData.language;
      }
   });

   /**
    * Specifications:
    *
    * <p>
    * Security: none.
    *
    * @returns {HearingImpaired}
    *
    * @name hearingImpaired
    * @memberof AVSubtitleComponent#
    */
   Object.defineProperty(prototype, "hearingImpaired", {
      get: function() {
         const p = privates.get(this);
         return p.avComponentData.hearingImpaired;
      }
   });

   /**
    * Specifications:
    *
    * <p>
    * Security: none.
    *
    * @returns {Label}
    *
    * @name label
    * @memberof AVSubtitleComponent#
    */
   Object.defineProperty(prototype, "label", {
      get: function() {
         const p = privates.get(this);
         return p.avComponentData.label;
      }
   });

   // Initialise an instance of prototype
   function initialise(avSubtitleComponentData) {
      privates.set(this, {});
      const p = privates.get(this);
      p.avComponentData = avSubtitleComponentData; // Hold reference to caller's object
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

hbbtv.objects.createAVSubtitleComponent = function(avSubtitleComponentData) {
   // Create new instance of hbbtv.objects.AVSubtitleComponent.prototype
   const avSubtitleComponent = Object.create(hbbtv.objects.AVSubtitleComponent.prototype);
   hbbtv.objects.AVSubtitleComponent.initialise.call(avSubtitleComponent, avSubtitleComponentData);
   return avSubtitleComponent;
};