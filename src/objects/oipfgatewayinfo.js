/**
 * @fileOverview OIPF Gateway Info object
 * @license ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

hbbtv.objects.OipfGatewayInfo = (function() {
   const prototype = Object.create(HTMLObjectElement.prototype);
   const privates = new WeakMap();
   const gGarbageCollectionBlocked = new Set();

   /**
    * Specifications:
    *
    * <p>
    *
    * @returns {IsIGDiscovered}
    *
    * @name isIGDiscovered
    * @readonly
    * @memberof OipfGatewayInfo#
    */
   Object.defineProperty(prototype, "isIGDiscovered", {
      get: function() {
         return false;
      }
   });

   /**
    * Specifications:
    *
    * <p>
    *
    * @returns {IsAGDiscovered}
    *
    * @name isAGDiscovered
    * @readonly
    * @memberof OipfGatewayInfo#
    */
   Object.defineProperty(prototype, "isAGDiscovered", {
      get: function() {
         return false;
      }
   });

   /**
    * Specifications:
    *
    * <p>
    *
    * @returns {IsCSPGCIPlusDiscovered}
    *
    * @name isCSPGCIPlusDiscovered
    * @readonly
    * @memberof OipfGatewayInfo#
    */
   Object.defineProperty(prototype, "isCSPGCIPlusDiscovered", {
      get: function() {
         return hbbtv.drmManager.isCSPGCIPlusDiscovered();
      }
   });

   /**
    * Specifications:
    *
    * <p>
    *
    * @returns {IsCSPGDTCPDiscovered}
    *
    * @name isCSPGDTCPDiscovered
    * @readonly
    * @memberof OipfGatewayInfo#
    */
   Object.defineProperty(prototype, "isCSPGDTCPDiscovered", {
      get: function() {
         return false;
      }
   });

   /**
    * Specifications:
    *
    * <p>
    *
    * @returns {IgURL}
    *
    * @name igURL
    * @readonly
    * @memberof OipfGatewayInfo#
    */
   Object.defineProperty(prototype, "igURL", {
      get: function() {
         return "igURL";
      }
   });

   /**
    * Specifications:
    *
    * <p>
    *
    * @returns {AgURL}
    *
    * @name agURL
    * @readonly
    * @memberof OipfGatewayInfo#
    */
   Object.defineProperty(prototype, "agURL", {
      get: function() {
         return "agURL";
      }
   });

   /**
    * Specifications:
    *
    * <p>
    *
    * @returns {CspgDTCPURL}
    *
    * @name cspgDTCPURL
    * @readonly
    * @memberof OipfGatewayInfo#
    */
   Object.defineProperty(prototype, "cspgDTCPURL", {
      get: function() {
         return "cspgDTCPURL";
      }
   });

   /**
    * Specifications:
    *
    * <p>
    *
    * @returns {Interval}
    *
    * @name interval
    * @readonly
    * @memberof OipfGatewayInfo#
    */
   Object.defineProperty(prototype, "interval", {
      get: function() {
         return 30;
      }
   });


   /**
    * Specifications:
    *
    * <p>
    *
    * @returns {IsIGSupported}
    *
    * @name isIGSupported
    * @readonly
    * @memberof OipfGatewayInfo#
    */
   Object.defineProperty(prototype, "isIGSupported", {
      get: function() {
         return false;
      }
   });

   /**
    * Specifications:
    *
    * <p>
    *
    * @returns {IsAGSupported}
    *
    * @name isAGSupported
    * @readonly
    * @memberof OipfGatewayInfo#
    */
   Object.defineProperty(prototype, "isAGSupported", {
      get: function() {
         return false;
      }
   });

   /**
    * Specifications:
    *
    * <p>
    *
    * @returns {IsCSPGDTCPSupported}
    *
    * @name isCSPGDTCPSupported
    * @readonly
    * @memberof OipfGatewayInfo#
    */
   Object.defineProperty(prototype, "isCSPGDTCPSupported", {
      get: function() {
         return false;
      }
   });

   /**
    * Specifications:
    *
    * <p>
    *
    * @returns {CSPGCIPlusDRMType}
    *
    * @name CSPGCIPlusDRMType
    * @readonly
    * @memberof OipfGatewayInfo#
    */
   Object.defineProperty(prototype, "CSPGCIPlusDRMType", {
      get: function() {
         let result = [];
         const status = hbbtv.drmManager.getCSPGCIPlusStatus();
         if (status) {
            status.DRMSystemIDs.forEach((value) => {
               result.push(value.DRMSystemID);
            });
         }
         return result;
      }
   });

   prototype.isIGSupportedMethod = function(methodName) {
      return false;
   };

   // DOM level 1 event methods
   prototype.addEventListener = function(type, listener) {
      if (privates.get(this).eventDispatcher.addCountedEventListener(type, listener) > 0) {
         gGarbageCollectionBlocked.add(this);
      }
   };

   prototype.removeEventListener = function(type, listener) {
      if (privates.get(this).eventDispatcher.removeCountedEventListener(type, listener) == 0) {
         gGarbageCollectionBlocked.delete(this);
      }
   };

   // DOM level 0 event properties
   Object.defineProperty(prototype, "onDiscoverIG", {
      get() {
         return privates.get(this).onDiscoverIGDomLevel0;
      },
      set(listener) {
         const p = privates.get(this);
         if (p.onDiscoverIGDomLevel0) {
            this.removeEventListener("DiscoverIG", p.onDiscoverIGWrapper);
            p.onDiscoverIGWrapper = null;
         }
         p.onDiscoverIGDomLevel0 = listener;
         if (listener) {
            p.onDiscoverIGWrapper = (ev) => {
               listener(ev.channel, ev.errorState);
            };
            this.addEventListener("DiscoverIG", p.onDiscoverIGWrapper);
         }
      }
   });

   Object.defineProperty(prototype, "onDiscoverAG", {
      get() {
         return privates.get(this).onDiscoverAGDomLevel0;
      },
      set(listener) {
         const p = privates.get(this);
         if (p.onDiscoverAGDomLevel0) {
            this.removeEventListener("DiscoverAG", p.onDiscoverAGWrapper);
            p.onDiscoverAGWrapper = null;
         }
         p.onDiscoverAGDomLevel0 = listener;
         if (listener) {
            p.onDiscoverAGWrapper = (ev) => {
               listener(ev.channel, ev.errorState);
            };
            this.addEventListener("DiscoverAG", p.onDiscoverAGWrapper);
         }
      }
   });

   Object.defineProperty(prototype, "onDiscoverCSPGDTCP", {
      get() {
         return privates.get(this).onDiscoverCSPGDTCPDomLevel0;
      },
      set(listener) {
         const p = privates.get(this);
         if (p.onDiscoverCSPGDTCPDomLevel0) {
            this.removeEventListener("DiscoverCSPGDTCP", p.onDiscoverCSPGDTCPWrapper);
            p.onDiscoverCSPGDTCPWrapper = null;
         }
         p.onDiscoverCSPGDTCPDomLevel0 = listener;
         if (listener) {
            p.onDiscoverCSPGDTCPWrapper = (ev) => {
               listener(ev.channel, ev.errorState);
            };
            this.addEventListener("DiscoverCSPGDTCP", p.onDiscoverCSPGDTCPWrapper);
         }
      }
   });

   Object.defineProperty(prototype, "onDiscoverCSPGCIPlus", {
      get() {
         return privates.get(this).onDiscoverCSPGCIPlusDomLevel0;
      },
      set(listener) {
         const p = privates.get(this);
         if (p.onDiscoverCSPGCIPlusDomLevel0) {
            this.removeEventListener("DiscoverCSPGCIPlus", p.onDiscoverCSPGCIPlusWrapper);
            p.onDiscoverCSPGCIPlusWrapper = null;
         }
         p.onDiscoverCSPGCIPlusDomLevel0 = listener;
         if (listener) {
            p.onDiscoverCSPGCIPlusWrapper = (ev) => {
               listener(ev.channel, ev.errorState);
            };
            this.addEventListener("DiscoverCSPGCIPlus", p.onDiscoverCSPGCIPlusWrapper);
         }
      }
   });

   function dispatchDiscoverIGEvent() {
      const event = new Event("DiscoverIG");
      privates.get(this).eventDispatcher.dispatchEvent(event);
   }

   function dispatchDiscoverAGEvent() {
      const event = new Event("DiscoverAG");
      privates.get(this).eventDispatcher.dispatchEvent(event);
   }

   function dispatchDiscoverCSPGDTCPEvent() {
      const event = new Event("DiscoverCSPGDTCP");
      privates.get(this).eventDispatcher.dispatchEvent(event);
   }

   function dispatchDiscoverCSPGCIPlusEvent() {
      const event = new Event("DiscoverCSPGCIPlus");
      privates.get(this).eventDispatcher.dispatchEvent(event);
   }


   function initialise() {
      privates.set(this, {});
      const p = privates.get(this);
      p.eventDispatcher = new hbbtv.utils.EventDispatcher(this);
      hbbtv.drmManager.registerOipfGatewayInfo(this, dispatchDiscoverCSPGCIPlusEvent);
   }

   return {
      prototype: prototype,
      initialise: initialise
   };
})();

hbbtv.objects.upgradeToOipfGatewayInfo = function(object) {
   Object.setPrototypeOf(object, hbbtv.objects.OipfGatewayInfo.prototype);
   hbbtv.objects.OipfGatewayInfo.initialise.call(object);
};

hbbtv.objectManager.registerObject({
   name: "application/oipfgatewayinfo",
   mimeTypes: ["application/oipfgatewayinfo"],
   oipfObjectFactoryMethodName: "createGatewayInfoObject",
   upgradeObject: hbbtv.objects.upgradeToOipfGatewayInfo
});