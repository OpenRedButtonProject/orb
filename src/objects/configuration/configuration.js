/**
 * @fileOverview Configuration class
 * See: {@link https://web.archive.org/web/20200219165053/http://www.oipf.tv/web-spec/volume5.html#configuration-class} 
 * @license ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

hbbtv.objects.Configuration = (function() {
   const prototype = {};
   let gIsPendingAccessCallback = false;

   /**
    * Specifications:
    *
    * <p>
    *
    * @returns {}
    *
    * @name preferredAudioLanguage
    * @memberof Configuration#
    */
   Object.defineProperty(prototype, "preferredAudioLanguage", {
      get: hbbtv.bridge.configuration.getPreferredAudioLanguage
   });

   /**
    * Specifications:
    *
    * <p>
    *
    * @returns {}
    *
    * @name preferredSubtitleLanguage
    * @memberof Configuration#
    */
   Object.defineProperty(prototype, "preferredSubtitleLanguage", {
      get: hbbtv.bridge.configuration.getPreferredSubtitleLanguage,
   });

   /**
    * Specifications:
    *
    * <p>
    *
    * @returns {}
    *
    * @name preferredUILanguage
    * @memberof Configuration#
    */
   Object.defineProperty(prototype, "preferredUILanguage", {
      get: hbbtv.bridge.configuration.getPreferredUILanguage,
   });

   /**
    * Specifications:
    *
    * <p>
    *
    * @returns {}
    *
    * @name countryId
    * @memberof Configuration#
    */
   Object.defineProperty(prototype, "countryId", {
      get: hbbtv.bridge.configuration.getCountryId,
   });

   /**
    * Specifications:
    *
    * <p>
    *
    * @returns {}
    *
    * @name subtitlesEnabled
    * @memberof Configuration#
    */
   Object.defineProperty(prototype, "subtitlesEnabled", {
      get: hbbtv.bridge.configuration.getSubtitlesEnabled,
   });

   /**
    * Specifications:
    *
    * <p>
    *
    * @returns {}
    *
    * @name audioDescriptionEnabled
    * @memberof Configuration#
    */
   Object.defineProperty(prototype, "audioDescriptionEnabled", {
      get: hbbtv.bridge.configuration.getAudioDescriptionEnabled,
   });

   /**
    * Specifications:
    *
    * <p>
    *
    * @returns {}
    *
    * @name timeShiftSynchronized
    * @memberof Configuration#
    */
   Object.defineProperty(prototype, "timeShiftSynchronized", {
      get: function() {
         return false; // PVR
      }
   });

   /**
    * Specifications:
    *
    * <p>
    *
    * @returns {}
    *
    * @name dtt_network_ids
    * @memberof Configuration#
    */
   Object.defineProperty(prototype, "dtt_network_ids", {
      get: hbbtv.bridge.configuration.getDttNetworkIds,
   });

   /**
    * Specifications:
    *
    * <p>
    *
    * @returns {}
    *
    * @name deviceId
    * @memberof Configuration#
    */
   Object.defineProperty(prototype, "deviceId", {
      get: hbbtv.bridge.configuration.getDeviceId,
   });

   prototype.requestAccessToDistinctiveIdentifier = function(callback) {
      const wrapper = (event) => {
         // One shot event
         hbbtv.bridge.removeStrongEventListener("accesstodistinctiveidentifier", wrapper);
         callback(event.allowAccess);
         gIsPendingAccessCallback = false;
      };
      hbbtv.bridge.addStrongEventListener("accesstodistinctiveidentifier", wrapper);
      if (!gIsPendingAccessCallback) {
         gIsPendingAccessCallback = true;
         hbbtv.bridge.configuration.requestAccessToDistinctiveIdentifier();
      }
   }

   function initialise() {}

   return {
      prototype: prototype,
      initialise: initialise
   };
})();

hbbtv.objects.createConfiguration = function() {
   const configuration = Object.create(hbbtv.objects.Configuration.prototype);
   hbbtv.objects.Configuration.initialise.call(configuration);
   return configuration;
}