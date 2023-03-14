/**
 * @fileOverview MediaElementTsClient class
 * @license ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

hbbtv.objects.MediaElementTsClient = (function() {
   const prototype = {};
   const privates = new WeakMap();

   hbbtv.utils.defineGetterSetterProperties(prototype, {
      correlationTimestamp: {
         get() {
            return privates.get(this).correlationTimestamp;
         },
         set(val) {
            const p = privates.get(this);
            p.correlationTimestamp = val;
            onMasterMediaUpdated.call(this, {
               data: {
                  contentTime: p.masterMediaObserver.contentTime
               }
            });
         }
      }
   });

   prototype.addEventListener = function(type, listener) {
      privates.get(this).eventTarget.addEventListener(type, listener);
   };

   prototype.removeEventListener = function(type, listener) {
      privates.get(this).eventTarget.removeEventListener(type, listener);
   };

   prototype.destroy = function() {
      const p = privates.get(this);
      p.masterMediaObserver.removeEventListener("MediaUpdated", p.onMasterMediaUpdated);
      p.masterMediaObserver.removeEventListener("Error", p.onFailureToPresentMedia);
      p.mediaObject.removeEventListener("ended", p.onFailureToPresentMedia);
      delete p.mediaObject.__orb_addedToMediaSync__;
      Object.setPrototypeOf(p.mediaObject, p.moPrototype);
      clearInterval(p.pollIntervalId);
      privates.delete(this);
   };

   function onMasterMediaUpdated(e) {
      const p = privates.get(this);
      if (!isNaN(e.data.contentTime)) {
         const targetTime = e.data.contentTime + (p.correlationTimestamp.tlvOther - p.correlationTimestamp.tlvMaster);
         let canSeek = false;
         for (let i = 0; i < p.mediaObject.buffered.length; ++i) {
            if (targetTime >= p.mediaObject.buffered.start(i) && targetTime <= p.mediaObject.buffered.end(i)) {
               canSeek = true;
               break;
            }
         }
         if (canSeek) {
            checkMediaSync.call(this, targetTime);
         } else if (p.canSyncWithMaster) {
            p.canSyncWithMaster = false;
            dispatchErrorEvent(p.eventTarget, 1); // insufficient buffer size (transient)
         }
      }
   }

   function checkMediaSync(contentTime) {
      const p = privates.get(this);
      if (contentTime >= 0 && !(p.mediaObject.ended || p.mediaObject.readyState < HTMLMediaElement.HAVE_CURRENT_DATA)) {
         if (p.masterMediaObserver.timelineSpeedMultiplier == 0) {
            p.moPrototype.pause.call(p.mediaObject);
         } else {
            console.log(contentTime, p.mediaObject.currentTime);
            if (p.mediaObject.paused) {
               p.moPrototype.play.call(p.mediaObject);
            }
            const ownProperty = Object.getOwnPropertyDescriptor(p.moPrototype, "playbackRate");
            if (ownProperty) {
               ownProperty.set.call(p.mediaObject, p.masterMediaObserver.timelineSpeedMultiplier);
            }
         }

         if (Math.abs(contentTime - p.mediaObject.currentTime) > p.tolerance / 1000.0) {
            const ownProperty = Object.getOwnPropertyDescriptor(p.moPrototype, "currentTime");
            if (ownProperty) {
               ownProperty.set.call(p.mediaObject, contentTime + p.tolerance / 1000.0);
               console.log("Synchronised tlvOther with tlvMaster");
            }
         }
         if (!p.canSyncWithMaster) {
            p.canSyncWithMaster = true;
            p.eventTarget.dispatchEvent(new Event("SyncNowAchievable"));
         }
      } else {
         p.moPrototype.pause.call(p.mediaObject);
         if (p.canSyncWithMaster) {
            p.canSyncWithMaster = false;
            dispatchErrorEvent(p.eventTarget, 11); // failed to synchronise media (transient)
         }
      }
   }

   function onFailureToPresentMedia() {
      const p = privates.get(this);
      if (p.canSyncWithMaster) {
         p.canSyncWithMaster = false;
         dispatchErrorEvent(p.eventTarget, 2); // failed to present media (transient)
      }
   }

   function dispatchErrorEvent(eventTarget, errorCode) {
      let evt = new Event("Error");
      evt.errorCode = errorCode;
      eventTarget.dispatchEvent(evt);
   }

   function initialise(mediaObject, correlationTimestamp, tolerance, multiDecoderMode, masterMediaObserver) {
      privates.set(this, {
         mediaObject: mediaObject,
         tolerance: tolerance,
         multiDecoderMode: multiDecoderMode,
         masterMediaObserver: masterMediaObserver,
         canSyncWithMaster: true,
         correlationTimestamp: correlationTimestamp,
         eventTarget: document.createDocumentFragment(),
         moPrototype: Object.getPrototypeOf(mediaObject),
         onMasterMediaUpdated: onMasterMediaUpdated.bind(this),
         onFailureToPresentMedia: onFailureToPresentMedia.bind(this),
         pollIntervalId: setInterval(() => {
            checkMediaSync.call(this, masterMediaObserver.contentTime);
         }, 2000)
      });

      function dispatchErrorEvent9() {
         if (p.canSyncWithMaster) {
            p.canSyncWithMaster = false;
            dispatchErrorEvent(p.eventTarget, 9); // not in suitable state to synchronise media (transient)
         }
      }

      const p = privates.get(this);

      const moPrototypeOverride = Object.create(p.moPrototype);
      moPrototypeOverride.pause = () => {
         dispatchErrorEvent9();
         p.moPrototype.pause.call(this);
      };
      moPrototypeOverride.play = () => {
         setTimeout(dispatchErrorEvent9, 0); // defer the error until play() returns
         return p.moPrototype.play.call(mediaObject);
      };
      hbbtv.utils.defineGetterSetterProperties(moPrototypeOverride, {
         currentTime: {
            get() {
               const ownProperty = Object.getOwnPropertyDescriptor(p.moPrototype, "currentTime");
               return ownProperty ? ownProperty.get.call(mediaObject) : undefined;
            },
            set(value) {
               dispatchErrorEvent9();
               const ownProperty = Object.getOwnPropertyDescriptor(p.moPrototype, "currentTime");
               if (ownProperty) {
                  ownProperty.set.call(mediaObject, value);
               }
            }
         },
         playbackRate: {
            get() {
               const ownProperty = Object.getOwnPropertyDescriptor(p.moPrototype, "playbackRate");
               return ownProperty ? ownProperty.get.call(mediaObject) : undefined;
            },
            set(value) {
               dispatchErrorEvent9();
               const ownProperty = Object.getOwnPropertyDescriptor(p.moPrototype, "playbackRate");
               if (ownProperty) {
                  ownProperty.set.call(mediaObject, value);
               }
            }
         }
      });

      Object.setPrototypeOf(mediaObject, moPrototypeOverride);
      mediaObject.__orb_addedToMediaSync__ = true;

      masterMediaObserver.addEventListener("MediaUpdated", p.onMasterMediaUpdated);
      masterMediaObserver.addEventListener("Error", p.onFailureToPresentMedia);
      mediaObject.addEventListener("ended", p.onFailureToPresentMedia);
      checkMediaSync.call(this, masterMediaObserver.contentTime);
   }

   return {
      prototype: prototype,
      initialise: initialise
   };
})();

hbbtv.objects.createMediaElementTsClient = function(mediaObject, timelineSelector, correlationTimestamp, tolerance, multiDecoderMode, masterMediaObserver) {
   const client = Object.create(hbbtv.objects.MediaElementTsClient.prototype);
   hbbtv.objects.MediaElementTsClient.initialise.call(client, mediaObject, timelineSelector, correlationTimestamp, tolerance, multiDecoderMode, masterMediaObserver);
   return client;
}