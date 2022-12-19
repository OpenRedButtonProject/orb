/**
 * @fileOverview OIPF Application class
 * See: {@link https://web.archive.org/web/20200219165053/http://www.oipf.tv/web-spec/volume5.html#application-class} 
 * @license ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

hbbtv.objects.Application = (function() {
   const prototype = {};
   const privates = new WeakMap();

   prototype.createApplication = function(uri) {
      if (privates.get(this).disabled) {
         return null;
      }
      const url = new __URL(uri, document.location.href);
      if (hbbtv.bridge.manager.createApplication(url.href) === true) {
         return hbbtv.objects.createApplication({
            disabled: true
         });
      }
      return null;
   }

   prototype.destroyApplication = function() {
      if (privates.get(this).disabled) {
         return;
      }
      hbbtv.bridge.manager.destroyApplication();
   }

   prototype.show = function() {
      if (privates.get(this).disabled) {
         return;
      }
      try {
         hbbtv.bridge.manager.showApplication();
      } catch (e) {
         if (e.message !== "NotRunning") {
            throw (e);
         }
      }
   }

   prototype.hide = function() {
      if (privates.get(this).disabled) {
         return;
      }
      try {
         hbbtv.bridge.manager.hideApplication();
      } catch (e) {
         if (e.message !== "NotRunning") {
            throw (e);
         }
      }
   }

   Object.defineProperty(prototype, "privateData", {
      get() {
         return privates.get(this).privateData;
      }
   });

   function initialise(data) {
      privates.set(this, {});
      const p = privates.get(this);
      p.disabled = data.disabled;
      p.privateData = hbbtv.objects.createPrivateData({
         disabled: p.disabled
      });
   }

   return {
      prototype: prototype,
      initialise: initialise
   };
})();

hbbtv.objects.createApplication = function(data) {
   // Create new instance of hbbtv.objects.Application
   const application = Object.create(hbbtv.objects.Application.prototype);
   hbbtv.objects.Application.initialise.call(application, data);
   return application;
};