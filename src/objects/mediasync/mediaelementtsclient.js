/**
 * @fileOverview MediaElementTsClient class
 * @license ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

hbbtv.objects.MediaElementTsClient = (function() {
   const prototype = {};
   const privates = new WeakMap();
   const TIME_UPDATE_FREQUENCY = 250;

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
      hbbtv.bridge.removeWeakEventListener("TimelineUnavailable", p.onTimelineUnavailable);
      hbbtv.bridge.removeWeakEventListener("TimelineAvailable", p.onTimelineAvailable);
      p.masterMediaObserver.removeEventListener("MediaUpdated", p.onMasterMediaUpdated);
      p.masterMediaObserver.removeEventListener("Error", p.onFailureToPresentMedia);
      p.mediaObject.removeEventListener("ended", p.onFailureToPresentMedia);
      p.mediaObject.addEventListener("__orb_onstreamupdated__", p.onStreamUpdatedHandler);
      p.mediaObject.addEventListener("__orb_onperiodchanged__", p.onPeriodChangedHandler);
      delete p.mediaObject.__orb_addedToMediaSync__;
      Object.setPrototypeOf(p.mediaObject, p.moPrototype);
      clearInterval(p.pollIntervalId);
      privates.delete(this);
   };

   function onMasterMediaUpdated(e) {
      const p = privates.get(this);
      if (!isNaN(e.data.contentTime) && p.timeline) {
         const targetTime = e.data.contentTime + ((p.correlationTimestamp.tlvOther / p.timeline.timelineProperties.tickRate) - (p.correlationTimestamp.tlvMaster / p.masterMediaObserver.tickRate));
         let canSeek = false;
         for (let i = 0; i < p.mediaObject.buffered.length; ++i) {
            if (targetTime >= p.mediaObject.buffered.start(i) && targetTime <= p.mediaObject.buffered.end(i)) {
               canSeek = true;
               break;
            }
         }
         if (canSeek) {
            checkMediaSync.call(this, targetTime);
         } else {
            dispatchErrorEvent.call(this, 1); // insufficient buffer size (transient)
         }
      }
   }

   function checkMediaSync(contentTime) {
      const p = privates.get(this);
      if (p.mediaObject.readyState >= HTMLMediaElement.HAVE_METADATA && contentTime >= 0 && contentTime < p.mediaObject.duration) {
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
         if (p.lastError) {
            p.lastError = 0;
            p.eventTarget.dispatchEvent(new Event("SyncNowAchievable"));
         }
      } else {
         p.moPrototype.pause.call(p.mediaObject);
         dispatchErrorEvent.call(this, 11); // failed to synchronise media (transient)
      }
   }

   function onFailureToPresentMedia() {
      dispatchErrorEvent.call(this, 2); // failed to present media (transient)
   }

   function onTimelineAvailable(e) {
      const p = privates.get(this);
      if (e.timeline.timelineSelector === p.timelineSelector) {
         p.timeline = e.timeline;
         const thiz = this;
         function pollMediaSync() {
            checkMediaSync.call(thiz, p.masterMediaObserver.contentTime + ((p.correlationTimestamp.tlvOther / p.timeline.timelineProperties.unitsPerSecond) - (p.correlationTimestamp.tlvMaster / p.masterMediaObserver.tickRate)));
         }
         if (!p.pollIntervalId) {
            p.pollIntervalId = setInterval(pollMediaSync, 2000);
         }
         pollMediaSync();
      }
   };

   function onTimelineUnavailable(e) {
      const p = privates.get(this);
      if (e.timelineSelector === p.timelineSelector) {
         clearInterval(p.pollIntervalId);
         p.pollIntervalId = undefined;
         p.timeline = undefined;
         dispatchErrorEvent.call(this, 3);
      }
   };

   function dispatchErrorEvent(errorCode) {
      const p = privates.get(this);
      if (p.lastError !== errorCode) {
         let evt = new Event("Error");
         p.lastError = evt.errorCode = errorCode;
         p.eventTarget.dispatchEvent(evt);
      }
   }

   async function initialise(mediaObject, timelineSelector, correlationTimestamp, tolerance, multiDecoderMode, masterMediaObserver, mediaSyncId) {
       // add the time update frequency to tolerance to
       // prevent hicup effect when checking difference with master media
      tolerance += TIME_UPDATE_FREQUENCY;
      
      privates.set(this, {
         mediaObject: mediaObject,
         tolerance: tolerance,
         multiDecoderMode: multiDecoderMode,
         masterMediaObserver: masterMediaObserver,
         lastError: 0,
         timelineSelector: timelineSelector,
         correlationTimestamp: correlationTimestamp,
         eventTarget: document.createDocumentFragment(),
         moPrototype: Object.getPrototypeOf(mediaObject),
         onMasterMediaUpdated: onMasterMediaUpdated.bind(this),
         onFailureToPresentMedia: onFailureToPresentMedia.bind(this),
         onTimelineUnavailable: onTimelineUnavailable.bind(this),
         onTimelineAvailable: onTimelineAvailable.bind(this)
      });

      const p = privates.get(this);

      const moPrototypeOverride = Object.create(p.moPrototype);
      moPrototypeOverride.pause = () => {
         dispatchErrorEvent.call(this, 9); // not in suitable state to synchronise media (transient)
         p.moPrototype.pause.call(this);
      };
      moPrototypeOverride.play = () => {
         const res = p.moPrototype.play.call(mediaObject);
         dispatchErrorEvent.call(this, 9); // not in suitable state to synchronise media (transient)
         return res;
      };
      hbbtv.utils.defineGetterSetterProperties(moPrototypeOverride, {
         currentTime: {
            get() {
               const ownProperty = Object.getOwnPropertyDescriptor(p.moPrototype, "currentTime");
               return ownProperty ? ownProperty.get.call(mediaObject) : undefined;
            },
            set(value) {
               dispatchErrorEvent.call(this, 9); // not in suitable state to synchronise media (transient)
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
               dispatchErrorEvent.call(this, 9); // not in suitable state to synchronise media (transient)
               const ownProperty = Object.getOwnPropertyDescriptor(p.moPrototype, "playbackRate");
               if (ownProperty) {
                  ownProperty.set.call(mediaObject, value);
               }
            }
         }
      });

      Object.setPrototypeOf(mediaObject, moPrototypeOverride);
      mediaObject.__orb_addedToMediaSync__ = true;

      hbbtv.bridge.addWeakEventListener("TimelineUnavailable", p.onTimelineUnavailable);
      hbbtv.bridge.addWeakEventListener("TimelineAvailable", p.onTimelineAvailable);
      masterMediaObserver.addEventListener("MediaUpdated", p.onMasterMediaUpdated);
      masterMediaObserver.addEventListener("Error", p.onFailureToPresentMedia);
      mediaObject.addEventListener("ended", p.onFailureToPresentMedia);

      // DASH timelines
      let relIndex = timelineSelector.indexOf(":rel:");
      let curPeriod = undefined;
      let timelines = {};

      p.onPeriodChangedHandler = async (e) => {
         if (curPeriod && e.data.id !== curPeriod) {
            //make available timeline based on e.data.id
            let currentTimelineSelector = timelineSelector.replace(curPeriod, e.data.id);
            relIndex = currentTimelineSelector.indexOf(":rel:");

            if (relIndex >= 0) {
               console.warn("MediaElementTSClient: DASH period id changed from " + curPeriod + " to " + e.data.id + ". Stopping timeline monitoring.");

               hbbtv.bridge.mediaSync.setTimelineAvailability(mediaSyncId, timelines[curPeriod], false, 0, 0);
               curPeriod = currentTimelineSelector.substring(relIndex + 5).split(":")[1];
               if (curPeriod) {
                  if (timelines[curPeriod] !== currentTimelineSelector) {
                     timelines[curPeriod] = currentTimelineSelector;
                     hbbtv.bridge.mediaSync.startTimelineMonitoring(mediaSyncId, currentTimelineSelector, false);
                  }
                  hbbtv.bridge.mediaSync.setTimelineAvailability(mediaSyncId, currentTimelineSelector, true, p.timeline ? mediaObject.currentTime * p.timeline.timelineProperties.unitsPerSecond : NaN, mediaObject.ended || mediaObject.paused ? 0 : mediaObject.playbackRate);
               }
            }
            //await refreshContentId();
            //const mrsUrl = await extractMrsUrl(p.masterMediaObject);
            //hbbtv.bridge.mediaSync.updateCssCiiProperties(mediaSyncId, p.contentId, p.masterMediaObject.readyState >= HTMLMediaElement.HAVE_CURRENT_DATA ? "okay" : "transitioning", "final", mrsUrl);
         }
      }

      p.onStreamUpdatedHandler = async (e) => {
         const periods = await mediaObject.orb_getPeriods();
         if (periods) {
            let periodIds = Object.keys(timelines);
            periodIds = periodIds.filter((pid) => {
               for (const period of periods) {
                  if (period.id === pid) {
                     return false;
                  }
               }
               return true;
            });
            for (const pid of periodIds) {
               hbbtv.bridge.mediaSync.stopTimelineMonitoring(mediaSyncId, timelines[pid], false);
               delete timelines[pid];
            }
         }
      }
      if (relIndex >= 0) {
         mediaObject.addEventListener("__orb_onstreamupdated__", p.onStreamUpdatedHandler);
         mediaObject.addEventListener("__orb_onperiodchanged__", p.onPeriodChangedHandler);
         curPeriod = timelineSelector.substring(relIndex + 5).split(":")[1];
         if (curPeriod) {
            const curPeriodInfo = await mediaObject.orb_getCurrentPeriod();
            if (curPeriodInfo && curPeriodInfo.id !== curPeriod) {
               // while starting mediasync, dash is streaming with a different timelineselector
               hbbtv.bridge.mediaSync.stopTimelineMonitoring(mediaSyncId, timelineSelector, false);
            } else {
               timelines[curPeriod] = timelineSelector;
               hbbtv.bridge.mediaSync.setTimelineAvailability(mediaSyncId, timelineSelector, true, NaN, mediaObject.ended || mediaObject.paused ? 0 : mediaObject.playbackRate);
            }
         } else {
            hbbtv.bridge.mediaSync.setTimelineAvailability(mediaSyncId, timelineSelector, true, NaN, mediaObject.ended || mediaObject.paused ? 0 : mediaObject.playbackRate);
         }
      }
      else if (!timelineSelector.includes(":temi:")) {
         hbbtv.bridge.mediaSync.setTimelineAvailability(mediaSyncId, timelineSelector, true, NaN, mediaObject.ended || mediaObject.paused ? 0 : mediaObject.playbackRate);
      }
   }

   return {
      prototype: prototype,
      initialise: initialise
   };
})();

hbbtv.objects.createMediaElementTsClient = function() {
   const client = Object.create(hbbtv.objects.MediaElementTsClient.prototype);
   hbbtv.objects.MediaElementTsClient.initialise.apply(client, arguments);
   return client;
}