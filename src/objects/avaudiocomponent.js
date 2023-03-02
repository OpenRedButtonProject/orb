/**
 * @fileOverview OIPF AVAudioComponent class
 * @license ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

hbbtv.objects.AVAudioComponent = (function() {
   const prototype = {};
   const privates = new WeakMap();
   const publicProperties = [
      'componentTag',
      'pid',
      'type',
      'encoding',
      'encrypted',
      'language',
      'audioDescription',
      'audioChannels'
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
    * @memberof AVAudioComponent#
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
    * @memberof AVAudioComponent#
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
    * @memberof AVAudioComponent#
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
    * @memberof AVAudioComponent#
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
    * @memberof AVAudioComponent#
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
    * @memberof AVAudioComponent#
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
    * @returns {AudioDescription}
    *
    * @name audioDescription
    * @memberof AVAudioComponent#
    */
   Object.defineProperty(prototype, "audioDescription", {
      get: function() {
         const p = privates.get(this);
         return p.avComponentData.audioDescription;
      }
   });

   /**
    * Specifications:
    *
    * <p>
    * Security: none.
    *
    * @returns {AudioChannels}
    *
    * @name audioChannels
    * @memberof AVAudioComponent#
    */
   Object.defineProperty(prototype, "audioChannels", {
      get: function() {
         const p = privates.get(this);
         return p.avComponentData.audioChannels;
      }
   });

   // Initialise an instance of prototype
   function initialise(avAudioComponentData) {
      privates.set(this, {});

      const p = privates.get(this);
      p.avComponentData = avAudioComponentData; // Hold reference to caller's object
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

hbbtv.objects.createAVAudioComponent = function(avAudioComponentData) {
   // Create new instance of hbbtv.objects.AVAudioComponent.prototype
   const avAudioComponent = Object.create(hbbtv.objects.AVAudioComponent.prototype);
   hbbtv.objects.AVAudioComponent.initialise.call(avAudioComponent, avAudioComponentData);
   return avAudioComponent;
};