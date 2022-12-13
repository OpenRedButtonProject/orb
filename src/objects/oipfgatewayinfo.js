/**
 * @fileOverview OIPF Gateway Info object.
 * See: {@link https://web.archive.org/web/20200219165053/http://www.oipf.tv/web-spec/volume5.html#application-oipfgatewayinfo} 
 * @preserve Copyright (c) Ocean Blue Software Ltd.
 * @license MIT (see LICENSE for full license)
 */
/*jshint esversion: 6 */

hbbtv.objects.OipfGatewayInfo = (function() {
   const prototype = Object.create(HTMLObjectElement.prototype);
   const privates = new WeakMap();
   const gGarbageCollectionBlocked = new Set();

   const Result = Object.freeze({
    STATUS_READY:        0,
    STATUS_UNKNOWN:      1,
    STATUS_INITIALISING: 2,
    STATUS_ERROR:        3,
    STATUS_DECODING:     4,
   });

   /* readonly properties */
   hbbtv.utils.defineGetterProperties(prototype, {
      isIGDiscovered() {
         return false;
      },
      isAGDiscovered() {
         return false;
      },
      isCSPGCIPlusDiscovered() {
         return privates.get(this).isCSPGCIPlusDiscovered;
      },
      isCSPGDTCPDiscovered() {
         return false;
      },
      igURL() {
         return "igURL";
      },
      agURL() {
         return "agURL";
      },
      cspgDTCPURL() {
         return "cspgDTCPURL";
      },
      interval() {
         return 30;
      },
      isIGSupported() {
         return false;
      },
      isAGSupported() {
         return false;
      },
      isCSPGCIPlusSupported() {
         return true;
      },
      isCSPGDTCPSupported() {
         return false;
      },
      CSPGCIPlusDRMType() {
         return null;
      },

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

   // Internal listeners
   function addBridgeEventListeners() {
      const p = privates.get(this);

      p.onDRMSystemStatusChange = (event) => {
         const p = privates.get(this);
         if (p.drmSystemIdStatusMap.has(event.DRMSystemID)) {
            if (event.status === Result.STATUS_UNKNOWN) {
               p.drmSystemIdStatusMap.delete(event.DRMSystemID);
            } else {
               p.drmSystemIdStatusMap.set(event.DRMSystemID, event.status);
            }
         } else if (event.status !== Result.STATUS_UNKNOWN) {
            p.drmSystemIdStatusMap.set(event.DRMSystemID, event.status);
         }
         if (isCIPlus(event)) {
            p.isCSPGCIPlusDiscovered = ((event.status === Result.STATUS_READY) || (event.status === Result.STATUS_DECODING));
            if ((event.status === Result.STATUS_READY) || (event.status === Result.STATUS_UNKNOWN)) {
               dispatchDiscoverCSPGCIPlusEvent.call(this);
            }
         }
      };
      hbbtv.bridge.addWeakEventListener("DRMSystemStatusChange", p.onDRMSystemStatusChange);
   }

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

      p.isCSPGCIPlusDiscovered = false;

      /* Associates DRMSystemID with status */
      p.drmSystemIdStatusMap = new Map();
      let sysIds = hbbtv.bridge.drm.getSupportedDRMSystemIDs();
      sysIds.forEach(element => {
         p.drmSystemIdStatusMap.set(element.DRMSystemID, element.status);
         if (((element.status === Result.STATUS_READY) || (element.status === Result.STATUS_DECODING)) && isCIPlus(element)) p.isCSPGCIPlusDiscovered = true;
      });

      addBridgeEventListeners.call(this);
   }
   
   function isCIPlus(element) {
      return element.DRMSystemID.startsWith("urn:dvb:casystemid:") && element.protectionGateways.includes("ci+");
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
