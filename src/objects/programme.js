/**
 * @fileOverview OIPF Programme class
 * See: {@link https://web.archive.org/web/20200219165053/http://www.oipf.tv/web-spec/volume5.html#programme-class}
 * @license ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

hbbtv.objects.Programme = (function() {
   const prototype = {};
   const privates = new WeakMap();

   hbbtv.utils.defineConstantProperties(prototype, {
      ID_TVA_CRID: 0,
      ID_DVB_EVENT: 1,
      ID_TVA_GROUP_CRID: 2
   });

   /**
    * Specifications:
    *
    * <p>
    *
    * @returns {ParentalRatings}
    *
    * @name parentalRatings
    * @readonly
    * @memberof Programme#
    */
   Object.defineProperty(prototype, "parentalRatings", {
      get: function() {
         return privates.get(this).programmeData.parentalRatings;
      }
   });

   /**
    * Specifications:
    *
    * <p>
    *
    * @returns {}
    *
    * @name name
    * @memberof Programme#
    */
   Object.defineProperty(prototype, "name", {
      get: function() {
         return privates.get(this).programmeData.name;
      },
      set: function(val) {
         privates.get(this).programmeData.name = val;
      }
   });

   /**
    * Specifications:
    *
    * <p>
    *
    * @returns {}
    *
    * @name description
    * @memberof Programme#
    */
   Object.defineProperty(prototype, "description", {
      get: function() {
         return privates.get(this).programmeData.description;
      },
      set: function(val) {
         privates.get(this).programmeData.description = val;
      }
   });

   /**
    * Specifications:
    *
    * <p>
    *
    * @returns {}
    *
    * @name longDescription
    * @memberof Programme#
    */
   Object.defineProperty(prototype, "longDescription", {
      get: function() {
         if (privates.get(this).programmeData.longDescription === undefined) {
            return privates.get(this).programmeData.description;
         } else {
            return privates.get(this).programmeData.longDescription;
         }
      },
      set: function(val) {
         privates.get(this).programmeData.longDescription = val;
      }
   });

   /**
    * Specifications:
    *
    * <p>
    *
    * @returns {}
    *
    * @name startTime
    * @memberof Programme#
    */
   Object.defineProperty(prototype, "startTime", {
      get: function() {
         return privates.get(this).programmeData.startTime;
      },
      set: function(val) {
         privates.get(this).programmeData.startTime = val;
      }
   });

   /**
    * Specifications:
    *
    * <p>
    *
    * @returns {}
    *
    * @name duration
    * @memberof Programme#
    */
   Object.defineProperty(prototype, "duration", {
      get: function() {
         return privates.get(this).programmeData.duration;
      },
      set: function(val) {
         privates.get(this).programmeData.duration = val;
      }
   });

   /**
    * Specifications:
    *
    * <p>
    *
    * @returns {}
    *
    * @name channelID
    * @memberof Programme#
    */
   Object.defineProperty(prototype, "channelID", {
      get: function() {
         return privates.get(this).programmeData.channelID;
      },
      set: function(val) {
         privates.get(this).programmeData.channelID = val;
      }
   });

   /**
    * Specifications:
    *
    * <p>
    *
    * @returns {}
    *
    * @name programmeID
    * @memberof Programme#
    */
   Object.defineProperty(prototype, "programmeID", {
      get: function() {
         return privates.get(this).programmeData.programmeID;
      },
      set: function(val) {
         privates.get(this).programmeData.programmeID = val;
      }
   });

   /**
    * Specifications:
    *
    * <p>
    *
    * @returns {}
    *
    * @name programmeIDType
    * @memberof Programme#
    */
   Object.defineProperty(prototype, "programmeIDType", {
      get: function() {
         return privates.get(this).programmeData.programmeIDType;
      },
      set: function(val) {
         privates.get(this).programmeData.programmeIDType = val;
      }
   });

   prototype.getSIDescriptors = function(descriptorTag, descriptorTagExtension = -1, privateDataSpecifier = 0) {
      const result = hbbtv.bridge.programme.getSIDescriptors(privates.get(this).programmeData.channelID,
         privates.get(this).programmeData.programmeID, descriptorTag, descriptorTagExtension,
         privateDataSpecifier);
      return (result.length > 0) ? hbbtv.objects.createCollection(result) : null;
   };

   // Initialise an instance of prototype
   function initialise(programmeData) {
      privates.set(this, {});
      const p = privates.get(this);
      p.programmeData = programmeData; // Hold reference to caller's object
   }

   // Private method to get a copy of the programme data
   function cloneProgrammeData() {
      return Object.assign({}, privates.get(this).programmeData);
   }

   /*prototype.toString = function() {
      return JSON.stringify(privates.get(this).programmeData);
   }*/

   return {
      prototype: prototype,
      initialise: initialise,
      cloneProgrammeData: cloneProgrammeData
   }
})();

hbbtv.objects.createProgramme = function(programmeData) {
   // Create new instance of hbbtv.objects.Programme.prototype
   const programme = Object.create(hbbtv.objects.Programme.prototype);
   hbbtv.objects.Programme.initialise.call(programme, programmeData);
   return programme;
};