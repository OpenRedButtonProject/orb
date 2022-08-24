/**
 * @fileOverview OIPF configuration object.
 * See: {@link https://web.archive.org/web/20200219165053/http://www.oipf.tv/web-spec/volume5.html#application-oipfconfiguration} 
 * @license ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

hbbtv.objects.OipfConfiguration = (function() {
   const prototype = Object.create(HTMLObjectElement.prototype);
   const privates = new WeakMap();

   Object.defineProperty(prototype, "configuration", {
      get() {
         return privates.get(this).configuration;
      }
   });

   Object.defineProperty(prototype, "localSystem", {
      get() {
         return privates.get(this).localSystem;
      }
   });

   function initialise() {
      privates.set(this, {});
      const p = privates.get(this);
      p.configuration = hbbtv.objects.createConfiguration();
      p.localSystem = hbbtv.objects.createLocalSystem();
   }

   return {
      prototype: prototype,
      initialise: initialise
   }
})();

hbbtv.objects.upgradeToOipfConfiguration = function(object) {
   Object.setPrototypeOf(object, hbbtv.objects.OipfConfiguration.prototype);
   hbbtv.objects.OipfConfiguration.initialise.call(object);
}

hbbtv.objectManager.registerObject({
   name: "application/oipfconfiguration",
   mimeTypes: ["application/oipfconfiguration"],
   oipfObjectFactoryMethodName: "createConfigurationObject",
   upgradeObject: function(object) {
      hbbtv.objects.upgradeToOipfConfiguration(object);
   }
});