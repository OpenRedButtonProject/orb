/**
 * @fileOverview OIPF video/broadcast object.
 * See: {@link https://web.archive.org/web/20200219165053/http://www.oipf.tv/web-spec/volume5.html#video-broadcast}
 * @license ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

hbbtv.objects.VideoBroadcast = (function() {
   const prototype = Object.create(HTMLObjectElement.prototype);
   const privates = new WeakMap();
   let gActiveStateOwner = null;
   let gBroadbandAvInUse = false;
   const gGarbageCollectionBlocked = new Set();

   const gObjectFinalizedWhileActive = hbbtv.utils.createFinalizationRegistry(() => {
      setVideoRectangle(0, 0, 1280, 720, true, false);
      hbbtv.bridge.broadcast.setPresentationSuspended(gBroadbandAvInUse);
      gActiveStateOwner = null;
   });

   hbbtv.utils.defineConstantProperties(prototype, {
      PLAY_STATE_UNREALIZED: 0,
      PLAY_STATE_CONNECTING: 1,
      PLAY_STATE_PRESENTING: 2,
      PLAY_STATE_STOPPED: 3,
      COMPONENT_TYPE_VIDEO: 0,
      COMPONENT_TYPE_AUDIO: 1,
      COMPONENT_TYPE_SUBTITLE: 2,
      ERROR_TUNER_UNAVAILABLE: 2,
      ERROR_UNKNOWN_CHANNEL: 5,
      POSITION_START: 0,
      POSITION_CURRENT: 1,
      POSITION_END: 2,

      CHANNEL_STATUS_UNREALIZED: -4,
      CHANNEL_STATUS_PRESENTING: -3,
      CHANNEL_STATUS_CONNECTING: -2,
      CHANNEL_STATUS_CONNECTING_RECOVERY: -1,
      CHANNEL_STATUS_WRONG_TUNER: 0,
      CHANNEL_STATUS_NO_SIGNAL: 1,
      CHANNEL_STATUS_TUNER_IN_USE: 2,
      CHANNEL_STATUS_PARENTAL_LOCKED: 3,
      CHANNEL_STATUS_ENCRYPTED: 4,
      CHANNEL_STATUS_UNKNOWN_CHANNEL: 5,
      CHANNEL_STATUS_INTERRUPTED: 6,
      CHANNEL_STATUS_RECORDING_IN_PROGRESS: 7,
      CHANNEL_STATUS_CANNOT_RESOLVE_URI: 8,
      CHANNEL_STATUS_INSUFFICIENT_BANDWIDTH: 9,
      CHANNEL_STATUS_CANNOT_BE_CHANGED: 10,
      CHANNEL_STATUS_INSUFFICIENT_RESOURCES: 11,
      CHANNEL_STATUS_CHANNEL_NOT_IN_TS: 12,
      CHANNEL_STATUS_UNKNOWN_ERROR: 100,
   });

   /* readonly properties */
   hbbtv.utils.defineGetterProperties(prototype, {
      currentChannel() {
         if (!privates.get(this).isBroadcastRelated) {
            /* ETSI TS 102 796 V1.6.1 A.2.26 */
            return null;
         }
         const channel = privates.get(this).currentChannelData;
         return channel;
      },
      fullScreen() {
         noRestrictionSecurityCheck();
         return privates.get(this).fullScreen;
      },
      playState() {
         noRestrictionSecurityCheck();
         return privates.get(this).playState;
      },

      /** Extensions to video/broadcast for recording and timeshift.
       * Broadcast-independent applications: shall throw a "Security Error" */
      currentTimeShiftMode() {
         const p = privates.get(this);
         mandatoryBroadcastRelatedSecurityCheck(p);
         return undefined; // TODO
      },
      maxOffset() {
         const p = privates.get(this);
         mandatoryBroadcastRelatedSecurityCheck(p);
         return undefined; // TODO
      },
      playbackOffset() {
         const p = privates.get(this);
         mandatoryBroadcastRelatedSecurityCheck(p);
         return undefined; // TODO
      },
      playPosition() {
         const p = privates.get(this);
         mandatoryBroadcastRelatedSecurityCheck(p);
         return undefined; // TODO
      },
      playSpeed() {
         const p = privates.get(this);
         mandatoryBroadcastRelatedSecurityCheck(p);
         return undefined; // TODO
      },
      playSpeeds() {
         const p = privates.get(this);
         mandatoryBroadcastRelatedSecurityCheck(p);
         return undefined; // TODO
      },
      recordingState() {
         const p = privates.get(this);
         mandatoryBroadcastRelatedSecurityCheck(p);
         return undefined; // TODO
      },

      /** Extensions to video.broadcast for access to EIT p/f.
       *  Broadcast-independent applications: shall throw a "Security Error" */
      programmes() {
         const p = privates.get(this);
         mandatoryBroadcastRelatedSecurityCheck(p);
         if (!p.currentChannelProgrammes) {
            let programmes = hbbtv.bridge.broadcast.getProgrammes(p.currentChannelData.ccid);
            programmes.forEach(function(item, index) {
               item.parentalRatings = hbbtv.objects.createParentalRatingCollection(item.parentalRatings);
               programmes[index] = hbbtv.objects.createProgramme(item);
            });
            p.currentChannelProgrammes = programmes;
         }
         return hbbtv.objects.createCollection(p.currentChannelProgrammes);
      }
   });

   hbbtv.utils.defineGetterSetterProperties(prototype, {
      data: {
         get: function() {
            noRestrictionSecurityCheck();
            return "";
         },
         set: function(val) {
            noRestrictionSecurityCheck();
         }
      },
      height: {
         get: function() {
            noRestrictionSecurityCheck();
            return this.offsetHeight;
         },
         set: function(val) {
            noRestrictionSecurityCheck();
            if (this.fullScreen) {
               throw new TypeError('"height" is read-only');
            } else {
               this.style.height = val;
            }
         }
      },
      timeShiftMode: {
         get: function() {
            const p = privates.get(this);
            mandatoryBroadcastRelatedSecurityCheck(p);
            return undefined; // TODO
         },
         set: function(val) {
            const p = privates.get(this);
            mandatoryBroadcastRelatedSecurityCheck(p);
         }
      },
      width: {
         get: function() {
            noRestrictionSecurityCheck();
            return this.offsetWidth;
         },
         set: function(val) {
            noRestrictionSecurityCheck();
            if (this.fullScreen) {
               throw new TypeError('"height" is read-only');
            } else {
               this.style.width = val;
            }
         }
      },
   });

   prototype.getChannelConfig = function() {
      const p = privates.get(this);
      mandatoryBroadcastRelatedSecurityCheck(p);
      return p.channelConfig;
   };

   prototype.bindToCurrentChannel = function() {
      const p = privates.get(this);
      mandatoryBroadcastRelatedSecurityCheck(p);
      if (p.playState === this.PLAY_STATE_UNREALIZED || p.playState === this.PLAY_STATE_STOPPED) {
         let tmpChannelData;
         try {
            tmpChannelData = hbbtv.objects.createChannel(hbbtv.bridge.broadcast.getCurrentChannel());
         } catch (e) {
            if (e.name === 'SecurityError') {
               console.log('bindToCurrentChannel, unexpected condition: app appears broadcast-independent.');
            }
            throw (e);
         }
         if (tmpChannelData !== false) {
            if (acquireActiveState.call(this)) {
               console.log("Control DVB presentation!");
               hbbtv.bridge.broadcast.setPresentationSuspended(false);
               hbbtv.holePuncher.setBroadcastVideoObject(this);

               let wasPlayStateStopped = false;
               if (p.playState === this.PLAY_STATE_UNREALIZED) {
                  /* DAE vol5 Table 8 state transition #7 */
                  p.playState = this.PLAY_STATE_PRESENTING;
               } else {
                  /* PLAY_STATE_STOPPED */
                  /* DAE vol5 Table 8 state transition #17 with HbbTV 2.0.3 modification */
                  p.playState = this.PLAY_STATE_CONNECTING;
                  wasPlayStateStopped = true;
               }
               addBridgeEventListeners.call(this);
               dispatchPlayStateChangeEvent.call(this, p.playState);
               if (wasPlayStateStopped) {
                  /* For PLAY_STATE_STOPPED: extra step to go into Presenting State */
                  /* DAE vol5 Table 8 state transition #17 with HbbTV 2.0.3 modification. */
                  p.playState = this.PLAY_STATE_PRESENTING;
                  dispatchPlayStateChangeEvent.call(this, p.playState);
               }
            } else {
               if (p.playState === this.PLAY_STATE_STOPPED) {
                  /* DAE vol5 Table 8 state transition #17 with HbbTV 2.0.3 modification */
                  dispatchPlayStateChangeEvent.call(this, p.playState);
               } else {
                  /* DAE vol5 Table 8 state transition #8 - binding fails */
                  unregisterAllStreamEventListeners(p);
                  p.playState = this.PLAY_STATE_UNREALIZED;
                  dispatchPlayStateChangeEvent.call(this, p.playState, this.ERROR_TUNER_UNAVAILABLE);
               }
            }
         } else {
            /* DAE vol5 Table 8 state transition #8 - no channel being presented */
            unregisterAllStreamEventListeners(p);
            p.playState = this.PLAY_STATE_UNREALIZED;
            dispatchPlayStateChangeEvent.call(this, p.playState, this.ERROR_UNKNOWN_CHANNEL);
         }
      }
      if (p.currentChannelData) {
         return p.currentChannelData;
      }
   };

   prototype.createChannelObject = function(idType, dsdOrOnid, sidOrTsid, sid, sourceID, ipBroadcastID) {
      noRestrictionSecurityCheck();
      if (idType == 13) {
         // createChannelObject(idType, dsd, tsid)
         return hbbtv.objects.createChannel({
            idType: idType,
            dsd: dsdOrOnid,
            sid: sidOrTsid
         });
      } else {
         // createChannelObject(idType, onid, tsid, sid, sourceID, ipBroadcastID)
         let channel = {
            idType: idType,
            onid: dsdOrOnid,
            tsid: sidOrTsid,
            sid: sid,
            sourceID: sourceID,
            ipBroadcastID: ipBroadcastID,
         };
         const p = privates.get(this);
         if (p.isBroadcastRelated) {
            let foundChannel = (p.channelConfig) ? p.channelConfig.channelList.findChannel(channel) : null;
            if (foundChannel !== null) {
               return foundChannel;
            }
         }
         return hbbtv.objects.createChannel(channel);
      }
   };

   prototype.setChannel = function(channel, trickplay = false, contentAccessDescriptorURL = "", quiet = 0) {
      const p = privates.get(this);
      let releaseOnError = false;
      // TODO Check state transitions table. Disallow if not connecting or presenting or stopped.
      if (channel === null) {
         this.release();
         setIsBroadcastRelated.call(this, false);
         const errorState = hbbtv.bridge.broadcast.setChannelToNull(trickplay, contentAccessDescriptorURL, quiet);
         if (errorState < 0) {
            dispatchChannelChangeSucceededEvent.call(this, null);
         }
         return;
      }

      // Acquire active state if required
      if (p.playState === this.PLAY_STATE_UNREALIZED || p.playState === this.PLAY_STATE_STOPPED) {
         if (!acquireActiveState.call(this)) {
            hbbtv.bridge.broadcast.setPresentationSuspended(false);
            /* DAE vol5 Table 8 state transition #2 & #6 - no suitable tuner is available */
            dispatchChannelChangeErrorEvent.call(this, channel, this.ERROR_TUNER_UNAVAILABLE);
            return;
         }
         addBridgeEventListeners.call(this);
         releaseOnError = true;
      }
      // Change channel
      p.isTransitioningToBroadcastRelated = true;
      let errorState = 0;
      if (channel.idType == 13) { // ID_DVB_SI_DIRECT
         if (channel.dsd !== undefined && channel.sid !== undefined) {
            errorState = hbbtv.bridge.broadcast.setChannelToDsd(
               hbbtv.utils.base64Encode(channel.dsd),
               channel.sid,
               trickplay,
               contentAccessDescriptorURL,
               quiet);
         }
      } else {
         if (channel.ccid !== undefined) {
            errorState = hbbtv.bridge.broadcast.setChannelToCcid(
               channel.ccid,
               trickplay,
               contentAccessDescriptorURL,
               quiet);
         } else {
            if (channel.onid !== undefined && channel.tsid !== undefined
                  && channel.sid !== undefined) {
               errorState = hbbtv.bridge.broadcast.setChannelToTriplet(
                  channel.idType,
                  channel.onid,
                  channel.tsid,
                  channel.sid,
                  (typeof channel.sourceID !== 'undefined') ? channel.sourceID : -1,
                  (typeof channel.ipBroadcastID !== 'undefined') ? channel.ipBroadcastID : "",
                  trickplay,
                  contentAccessDescriptorURL,
                  quiet);
            }
         }
      }

      if (errorState >= 0) {
         p.isTransitioningToBroadcastRelated = false;
         if (releaseOnError) {
            removeBridgeEventListeners.call(this);
            hbbtv.holePuncher.setBroadcastVideoObject(null);
            releaseActiveState();
         }
         /* DAE vol5 Table 8 state transition #2 - combination of channel properties is invalid */
         /*                                      - channel type is not supported */
         dispatchChannelChangeErrorEvent.call(this, channel, errorState);
         return;
      }

      if (p.isBroadcastRelated && (quiet !== 2)) {
         try {
            p.currentChannelData = hbbtv.objects.createChannel(hbbtv.bridge.broadcast.getCurrentChannel());
            if (p.channelConfig === null) {
               p.channelConfig = hbbtv.objects.createChannelConfig();
            }
         } catch (e) {
            if (e.name === 'SecurityError') {
               console.log('setChannel, unexpected condition: app appears broadcast-independent.');
            }
            throw (e);
         }
      }

      /* DAE vol5 Table 8 state transition #1 */
      unregisterAllStreamEventListeners(p);
      p.playState = this.PLAY_STATE_CONNECTING;
      p.waitingPlayStateConnectingConfirm = true;
      dispatchPlayStateChangeEvent.call(this, p.playState);
   };

   function removeBridgeEventListeners() {
      const p = privates.get(this);
      if (p.onChannelStatusChanged != null) {
         hbbtv.bridge.removeWeakEventListener("ChannelStatusChanged", p.onChannelStatusChanged);
         p.onChannelStatusChanged = null;
      }
      if (p.onProgrammesChanged != null) {
         hbbtv.bridge.removeWeakEventListener("ProgrammesChanged", p.onProgrammesChanged);
         p.onProgrammesChanged = null;
      }
      if (p.onParentalRatingChange != null) {
         hbbtv.bridge.removeWeakEventListener("ParentalRatingChange", p.onParentalRatingChange);
         p.onParentalRatingChange = null;
      }
      if (p.onParentalRatingError != null) {
         hbbtv.bridge.removeWeakEventListener("ParentalRatingError", p.onParentalRatingError);
         p.onParentalRatingError = null;
      }
      if (p.onSelectedComponentChanged != null) {
         hbbtv.bridge.removeWeakEventListener("SelectedComponentChanged", p.onSelectedComponentChanged);
         p.onSelectedComponentChanged = null;
      }
      if (p.onComponentChanged != null) {
         hbbtv.bridge.removeWeakEventListener("ComponentChanged", p.onComponentChanged);
         p.onComponentChanged = null;
      }
      if (p.onStreamEvent != null) {
         hbbtv.bridge.removeWeakEventListener("StreamEvent", p.onStreamEvent);
         p.onStreamEvent = null;
      }
      if (p.onTransitionedToBroadcastRelated != null) {
         hbbtv.bridge.removeWeakEventListener("TransitionedToBroadcastRelated", p.onTransitionedToBroadcastRelated);
         p.onTransitionedToBroadcastRelated = null;
      }
   }

   function addBridgeEventListeners() {
      const p = privates.get(this);
      if (!p.onChannelStatusChanged) {
         p.onChannelStatusChanged = (event) => {
            const p = privates.get(this);
            console.log("Received ChannelStatusChanged (" +
               event.onetId + "," +
               event.transId + "," +
               event.servId + "), status: " + event.statusCode + " playState: " + p.playState);
            if (p.playState == this.PLAY_STATE_CONNECTING) {
               switch (event.statusCode) {
                  case this.CHANNEL_STATUS_PRESENTING:
                     /* DAE vol5 Table 8 state transition #9 */
                     hbbtv.holePuncher.setBroadcastVideoObject(this);
                     p.playState = this.PLAY_STATE_PRESENTING;
                     dispatchChannelChangeSucceededEvent.call(this, p.currentChannelData);
                     dispatchPlayStateChangeEvent.call(this, p.playState);
                     break;

                  case this.CHANNEL_STATUS_CONNECTING:
                     if (p.currentChannelData == null ||
                        event.servId != p.currentChannelData.sid ||
                        event.onetId != p.currentChannelData.onid ||
                        event.transId != p.currentChannelData.tsid) {
                        try {
                           p.currentChannelData = hbbtv.objects.createChannel(hbbtv.bridge.broadcast.getCurrentChannelForEvent());
                        } catch (e) {
                           if (e.name === 'SecurityError') {
                              console.log('Unexpected condition: app appears broadcast-independent.');
                           }
                           throw(e);
                        }
                     }
                     if (p.waitingPlayStateConnectingConfirm) {
                        console.log("waitingPlayStateConnectingConfirm TRUE. Ignore event");
                     } else {
                        /* DAE vol5 Table 8 state transition #10, or possibly, a user initiated channel change */
                        /* Terminal connected to the broadcast or IP multicast stream but presentation blocked */
                        p.playState = this.PLAY_STATE_CONNECTING;
                        dispatchChannelChangeSucceededEvent.call(this, p.currentChannelData);
                        dispatchPlayStateChangeEvent.call(this, p.playState);
                     }
                     break;

                  case this.CHANNEL_STATUS_CONNECTING_RECOVERY:
                     /* DAE vol5 Table 8 state transition #11 */
                     /* Recovery from transient error */
                     p.playState = this.PLAY_STATE_PRESENTING;
                     dispatchPlayStateChangeEvent.call(this, p.playState);
                     break;

                  default:
                     if (event.permanentError) {
                        /* DAE vol5 Table 8 state transition #13 */
                        unregisterAllStreamEventListeners(p);
                        p.playState = this.PLAY_STATE_UNREALIZED;
                        dispatchPlayStateChangeEvent.call(this, p.playState);
                     } /* else DAE vol5 Table 8 state transition #2 */
                     dispatchChannelChangeErrorEvent.call(this, p.currentChannelData, event.statusCode);
               }
               p.waitingPlayStateConnectingConfirm = false;
            } else if (p.playState == this.PLAY_STATE_PRESENTING) {
               if (event.permanentError) {
                  /* DAE vol5 Table 8 state transition #16A */
                  unregisterAllStreamEventListeners(p);
                  p.playState = this.PLAY_STATE_UNREALIZED;
                  dispatchPlayStateChangeEvent.call(this, p.playState);
               } else if (event.statusCode == this.CHANNEL_STATUS_CONNECTING) {
                  /* Possibly a user initiated channel change (with app not bound to service) */
                  p.playState = this.PLAY_STATE_CONNECTING;
                  if (p.currentChannelData == null ||
                     event.servId != p.currentChannelData.sid ||
                     event.onetId != p.currentChannelData.onid ||
                     event.transId != p.currentChannelData.tsid) {
                     try {
                        p.currentChannelData = hbbtv.objects.createChannel(hbbtv.bridge.broadcast.getCurrentChannelForEvent());
                     } catch (e) {
                        if (e.name === 'SecurityError') {
                           console.log('onChannelStatusChanged, unexpected condition: app appears broadcast-independent.');
                        }
                        throw (e);
                     }
                     dispatchChannelChangeSucceededEvent.call(this, p.currentChannelData);
                  }
                  dispatchPlayStateChangeEvent.call(this, p.playState);
               } else /* temporary error */ {
                  /* DAE vol5 Table 8 state transition #15 */
                  p.playState = this.PLAY_STATE_CONNECTING;
                  dispatchPlayStateChangeEvent.call(this, p.playState);
               }
            } else if ((p.playState == this.PLAY_STATE_STOPPED) && (event.permanentError)) {
               /* DAE vol5 Table 8 state transition #16B */
               unregisterAllStreamEventListeners(p);
               p.playState = this.PLAY_STATE_UNREALIZED;
               dispatchPlayStateChangeEvent.call(this, p.playState);
            } else {
               console.log("Unhandled state transition. Current playState " + p.playState + ", event:");
               console.log(event);
            }
         };

         hbbtv.bridge.addWeakEventListener("ChannelStatusChanged", p.onChannelStatusChanged);
      }

      if (!p.onProgrammesChanged) {
         p.onProgrammesChanged = (event) => {
            console.log("Received ProgrammesChanged");
            console.log(event);
            dispatchProgrammesChanged.call(this);
         };

         hbbtv.bridge.addWeakEventListener("ProgrammesChanged", p.onProgrammesChanged);
      }

      if (!p.onParentalRatingChange) {
         p.onParentalRatingChange = (event) => {
            console.log("Received ParentalRatingChange");
            console.log(event);
            dispatchParentalRatingChange.call(this, event.contentID, event.ratings, event.DRMSystemID, event.blocked);
         };

         hbbtv.bridge.addWeakEventListener("ParentalRatingChange", p.onParentalRatingChange);
      }

      if (!p.onParentalRatingError) {
         p.onParentalRatingError = (event) => {
            console.log("Received ParentalRatingError");
            console.log(event);
            dispatchParentalRatingError.call(this, event.contentID, event.ratings, event.DRMSystemID);
         };

         hbbtv.bridge.addWeakEventListener("ParentalRatingError", p.onParentalRatingError);
      }

      if (!p.onSelectedComponentChanged) {
         p.onSelectedComponentChanged = (event) => {
            const p = privates.get(this);
            try {
               p.currentChannelComponents = null;
               dispatchSelectedComponentChanged.call(this, event.componentType);
            } catch (e) {
               if (e.name === 'SecurityError') {
                  console.log('onSelectedComponentChanged, unexpected condition: app appears broadcast-independent.');
               }
               throw (e);
            }
         };

         hbbtv.bridge.addWeakEventListener("SelectedComponentChanged", p.onSelectedComponentChanged);
      }

      if (!p.onComponentChanged) {
         p.onComponentChanged = (event) => {
            const p = privates.get(this);
            /* Update internal state */
            try {
               p.currentChannelComponents = null;
               dispatchComponentChanged.call(this, event.componentType);
            } catch (e) {
               if (e.name === 'SecurityError') {
                  console.log('onComponentChanged, unexpected condition: app appears broadcast-independent.');
               }
               throw (e);
            }
         };

         hbbtv.bridge.addWeakEventListener("ComponentChanged", p.onComponentChanged);
      }

      if (!p.onStreamEvent) {
         p.onStreamEvent = (event) => {
            console.log("Received StreamEvent");
            console.log(event);
            dispatchStreamEvent.call(this, event.id, event.name, event.data, event.text, event.status);
         };

         hbbtv.bridge.addWeakEventListener("StreamEvent", p.onStreamEvent);
      }

      if (!p.onTransitionedToBroadcastRelated) {
         p.onTransitionedToBroadcastRelated = (event) => {
            const p = privates.get(this);
            if (p.isTransitioningToBroadcastRelated) {
               p.isTransitioningToBroadcastRelated = false;
               setIsBroadcastRelated.call(this, true);
               try {
                  p.currentChannelData = hbbtv.objects.createChannel(hbbtv.bridge.broadcast.getCurrentChannel());
                  if (p.channelConfig === null) {
                     p.channelConfig = hbbtv.objects.createChannelConfig();
                  }
               } catch (e) {
                  if (e.name === 'SecurityError') {
                     console.log('onTransitionedToBroadcastRelated, unexpected condition: app appears broadcast-independent.');
                  }
                  throw (e);
               }
            }
         };
      }

      hbbtv.bridge.addWeakEventListener("TransitionedToBroadcastRelated", p.onTransitionedToBroadcastRelated);
   }

   function cycleChannel(delta) {
      const p = privates.get(this);
      if (!p.isBroadcastRelated) {
         throw new DOMException('', 'SecurityError');
      }
      if ((p.playState === this.PLAY_STATE_UNREALIZED) || (p.channelConfig.channelList.length < 2)) {
         dispatchChannelChangeErrorEvent.call(this, p.currentChannelData, this.CHANNEL_STATUS_CANNOT_BE_CHANGED);
         return;
      }
      let i;
      for (i = 0; i < p.channelConfig.channelList.length; i++) {
         if (p.channelConfig.channelList.item(i).ccid === p.currentChannelData.ccid) {
            /* DAE vol5 Table 8 state transition #3 happens in setChannel() */
            let n = p.channelConfig.channelList.length;
            this.setChannel(p.channelConfig.channelList.item((i + delta + n) % n));
            return;
         }
      }
      if (p.playState === this.PLAY_STATE_CONNECTING) {
         /* DAE vol5 Table 8 state transition #4 */
         unregisterAllStreamEventListeners(p);
         p.playState = this.PLAY_STATE_UNREALIZED;
         /* Note: playState is updated first, so it is already correct for the ChannelChangeErrorEvent */
         dispatchChannelChangeErrorEvent.call(this, p.currentChannelData, this.CHANNEL_STATUS_CANNOT_BE_CHANGED);
         dispatchPlayStateChangeEvent.call(this, p.playState, this.CHANNEL_STATUS_CANNOT_BE_CHANGED);
      } else {
         /* Either Presenting or Stopped */
         /* DAE vol5 Table 8 state transition #5 */
         dispatchChannelChangeErrorEvent.call(this, p.currentChannelData, this.CHANNEL_STATUS_CANNOT_BE_CHANGED);
      }
   }

   /** Broadcast-independent applications: shall throw a "Security Error" */
   function mandatoryBroadcastRelatedSecurityCheck(p) {
      if (!p.isBroadcastRelated) {
         throw new DOMException('', 'SecurityError');
      }
   }

   /** Broadcast-independent applications: shall have no restrictions */
   function noRestrictionSecurityCheck() {
      /* noop */
   };

   prototype.prevChannel = function() {
      const p = privates.get(this);
      mandatoryBroadcastRelatedSecurityCheck(p);
      cycleChannel.call(this, -1);
   };

   prototype.nextChannel = function() {
      const p = privates.get(this);
      mandatoryBroadcastRelatedSecurityCheck(p);
      cycleChannel.call(this, 1);
   };

   prototype.setFullScreen = function(val) {
      const p = privates.get(this);
      /** Broadcast-independent applications: setFullScreen() shall have no effect */
      if (!p.isBroadcastRelated) {
         return;
      }
      p.fullScreen = val;
      hbbtv.holePuncher.notifyFullScreenChanged(this);
      dispatchFullScreenChangeEvent.call(this);
   };

   prototype.release = function() {
      const p = privates.get(this);
      /** Broadcast-independent applications: release() shall have no effect */
      if (!p.isBroadcastRelated) {
         return; // TODO Really?
      }
      removeBridgeEventListeners.call(this);
      if (p.playState !== this.PLAY_STATE_UNREALIZED) {
         /* DAE vol5 Table 8 state transition #12 */
         p.currentChannelData = null;
         p.currentChannelProgrammes = null;
         p.currentChannelComponents = null;
         unregisterAllStreamEventListeners(p);
         p.playState = this.PLAY_STATE_UNREALIZED;
         hbbtv.holePuncher.setBroadcastVideoObject(null);
         releaseActiveState.call(this);
         dispatchPlayStateChangeEvent.call(this, p.playState);
         /* TODO: If app has modified the set of components, they continue to be presented */
      }
   };

   prototype.stop = function() {
      const p = privates.get(this);
      /** Broadcast-independent applications: stop() shall have no effect */
      if (!p.isBroadcastRelated) {
         return;
      }
      if (p.playState === this.PLAY_STATE_CONNECTING || p.playState === this.PLAY_STATE_PRESENTING) {
         /* DAE vol5 Table 8 state transition #14 */
         p.playState = this.PLAY_STATE_STOPPED;
         removeBridgeEventListeners.call(this);
         hbbtv.holePuncher.setBroadcastVideoObject(null);
         hbbtv.bridge.broadcast.setPresentationSuspended(true);
         dispatchPlayStateChangeEvent.call(this, p.playState);
      }
   };

   // DOM level 1 event methods
   prototype.addEventListener = function(type, listener) {
      noRestrictionSecurityCheck();
      if (privates.get(this).eventDispatcher.addCountedEventListener(type, listener) > 0) {
         gGarbageCollectionBlocked.add(this);
      }
   };

   prototype.removeEventListener = function(type, listener) {
      noRestrictionSecurityCheck();
      if (privates.get(this).eventDispatcher.removeCountedEventListener(type, listener) == 0) {
         gGarbageCollectionBlocked.delete(this);
      }
   };

   prototype.getComponents = function(componentType) {
      const p = privates.get(this);
      mandatoryBroadcastRelatedSecurityCheck(p);
      if (!p.currentChannelComponents) {
         p.currentChannelComponents = getFormattedComponents(p.currentChannelData.ccid);
      }
      let result;
      if ((componentType === null) || (componentType === undefined)) {
         result = p.currentChannelComponents.filter(component => {
            return (!component.hidden);
         });
      } else {
         result = p.currentChannelComponents.filter(component => {
            return ((component.type === componentType) && !component.hidden);
         });
      }
      return avComponentArrayToCollection.call(this, result);
   };

   prototype.getCurrentActiveComponents = function(componentType) {
      const p = privates.get(this);
      mandatoryBroadcastRelatedSecurityCheck(p);
      if (!p.currentChannelComponents) {
         p.currentChannelComponents = getFormattedComponents(p.currentChannelData.ccid);
      }
      if ((componentType === null) || (componentType === undefined)) {
         let result = p.currentChannelComponents.filter(component => {
            return component.active;
         });
         return avComponentArrayToCollection.call(this, result);
      } else {
         let result = p.currentChannelComponents.filter(component => {
            return (component.type === componentType) && component.active;
         });
         return avComponentArrayToCollection.call(this, result);
      }
   };

   prototype.selectComponent = function(component) {
      const p = privates.get(this);
      mandatoryBroadcastRelatedSecurityCheck(p);
      if (!p.currentChannelComponents) {
         p.currentChannelComponents = getFormattedComponents(p.currentChannelData.ccid);
      }
      let foundComponent = null;
      if ((!p.currentChannelComponents) || (p.currentChannelComponents.length === 0)) {
         throw new DOMException('', 'InvalidStateError');
      }
      /* HbbTV 2.0.3 A.1: The selectComponent() and unselectComponent() methods shall be asynchronous. */
      if (!isNaN(component)) {
         foundComponent = p.currentChannelComponents.find(item => {
            return ((item.type === component) && (item.default));
         });
      } else {
         foundComponent = p.currentChannelComponents.find(item => {
            return compareComponents.call(this, component, item);
         });
      }
      if (foundComponent) { // && (!foundComponent.active)) {//Note: we can't rely on active to avoid repetition as it won't be updated until events with the result are received
         if (foundComponent.type == this.COMPONENT_TYPE_AUDIO) {
            hbbtv.bridge.broadcast.selectComponent(foundComponent.type, foundComponent.pid,
               foundComponent.language);
         } else {
            hbbtv.bridge.broadcast.selectComponent(foundComponent.type, foundComponent.pid, "");
         }
      }
   };

   prototype.unselectComponent = function(component) {
      const p = privates.get(this);
      mandatoryBroadcastRelatedSecurityCheck(p);
      if (!p.currentChannelComponents) {
         p.currentChannelComponents = getFormattedComponents(p.currentChannelData.ccid);
      }
      let foundComponent = null;
      /* HbbTV 2.0.3 A.1: The selectComponent() and unselectComponent() methods shall be asynchronous. */
      if (!isNaN(component)) {
         foundComponent = p.currentChannelComponents.find(item => {
            return ((item.type === component) && item.default);
         });
      } else {
         if (((component.type === 1) && (hbbtv.bridge.configuration.getPreferredAudioLanguage() != null)) || /* Return to the default preferred audio language. */
            ((component.type === 2) && (hbbtv.bridge.configuration.getPreferredSubtitleLanguage() != null))) {
            /* Return to the default preferred subtitle language */
            let defaultComponent = p.currentChannelComponents.find(item => {
               return ((component.type === item.type) && item.default);
            });
            if (defaultComponent) { // && !defaultComponent.active) {//Note: we can't rely on active to avoid repetition as it won't be updated until events with the result are received
               hbbtv.bridge.broadcast.selectComponent(defaultComponent.type, defaultComponent.pid, "");
            }
            return;
         } else {
            foundComponent = p.currentChannelComponents.find(item => {
               return compareComponents.call(this, component, item);
            });
         }
      }
      if (foundComponent) { // && foundComponent.active) {//Note: we can't rely on active to avoid repetition as it won't be updated until events with the result are received
         hbbtv.bridge.broadcast.unselectComponent(
            foundComponent.type,
            foundComponent.pid
         );
      }
   };

   function raiseStreamEventError(name, listener) {
      const event = new Event("StreamEvent");
      Object.assign(event, {
         name: name,
         data: "",
         text: "",
         status: "error"
      });
      hbbtv.utils.runOnMainLooper(this, function() {
         listener(event);
      });
   }

   function registerStreamEventListener(p, targetURL, eventName, listener, componentTag = -1, streamEventId = -1) {
      //Check whether it is already added
      const streamEventID = getStreamEventID(targetURL, eventName);
      const streamEventInternalID = p.streamEventListenerIdMap.get(streamEventID);
      console.log("add Stream Event Listener with: " + targetURL + ", on event:" + eventName);
      if (streamEventInternalID) {
         const streamEventListeners = p.streamEventListenerMap.get(streamEventInternalID);
         if (streamEventListeners) {
            if (!streamEventListeners.includes(listener)) {
               streamEventListeners.push(listener);
            }
         } else {
            console.error("Unconsistent state, " + streamEventID + "(" + streamEventInternalID + ") has no listeners.");
         }
      } else {
         let resultId = hbbtv.bridge.broadcast.addStreamEventListener(targetURL, eventName, componentTag, streamEventId);
         if (resultId == -1) {
            console.error("Failed to add Stream Event Listener with: " + targetURL + ", on event:" + eventName);
            raiseStreamEventError(eventName, listener);
         } else {
            p.streamEventListenerIdMap.set(streamEventID, resultId);
            p.streamEventListenerMap.set(resultId, new Array(listener));
         }
      }
   }

   function unregisterAllStreamEventListeners(p) {
      p.streamEventListenerMap.clear();
      p.streamEventListenerIdMap.forEach(function(value, key) {
         hbbtv.bridge.broadcast.removeStreamEventListener(value);
      });
      p.streamEventListenerIdMap.clear();
   }

   prototype.addStreamEventListener = function(targetURL, eventName, listener) {
      /* Extensions to video/broadcast for synchronization: No security restrictions specified */
      const p = privates.get(this);
      if ((p.playState == this.PLAY_STATE_PRESENTING) || (p.playState == this.PLAY_STATE_STOPPED)) {
         if (targetURL.startsWith("dvb://")) {
            registerStreamEventListener(p, targetURL, eventName, listener);
         } else {
            let found = false;
            let componentTag, streamEventId;
            let request = new XMLHttpRequest();
            request.addEventListener("loadend", () => {
               if (request.status == 200) {
                  let dsmcc_objects = request.responseXML.getElementsByTagName("dsmcc:dsmcc_object");
                  if (dsmcc_objects.length == 0) {
                     dsmcc_objects = request.responseXML.getElementsByTagName("dsmcc_object");
                  }
                  for (let x = 0;
                     (x < dsmcc_objects.length) && !found; x++) {
                     let stream_events = dsmcc_objects[x].getElementsByTagName("dsmcc:stream_event");
                     if (stream_events.length == 0) {
                        stream_events = dsmcc_objects[x].getElementsByTagName("stream_event");
                     }
                     for (var y = 0; y < stream_events.length; y++) {
                        let streamEventName = stream_events[y].getAttribute('dsmcc:stream_event_name');
                        if (streamEventName == null) {
                           streamEventName = stream_events[y].getAttribute('stream_event_name');
                        }
                        if (streamEventName === eventName) {
                           found = true;
                           componentTag = dsmcc_objects[x].getAttribute("dsmcc:component_tag");
                           if (componentTag == null) {
                              componentTag = dsmcc_objects[x].getAttribute("component_tag");
                           }
                           streamEventId = stream_events[y].getAttribute('dsmcc:stream_event_id');
                           if (streamEventId == null) {
                              streamEventId = stream_events[y].getAttribute('stream_event_id');
                           }
                           break;
                        }
                     }
                  }
               }

               if (found) {
                  registerStreamEventListener(p, targetURL, eventName, listener, componentTag, streamEventId);
               } else {
                  console.error("Failed to find Stream Event for: " + targetURL + ", on event:" + eventName +
                     ", request status: " + request.status);
                  raiseStreamEventError(eventName, listener);
               }
            });
            request.open("GET", targetURL);
            request.send();
         }
      }
   };

   prototype.removeStreamEventListener = function(targetURL, eventName, listener) {
      /* Extensions to video/broadcast for synchronization: No security restrictions specified */
      const p = privates.get(this);
      const streamEventID = getStreamEventID(targetURL, eventName);
      const streamEventInternalID = p.streamEventListenerIdMap.get(streamEventID);
      if (streamEventInternalID) {
         const streamEventListeners = p.streamEventListenerMap.get(streamEventInternalID);
         if (streamEventListeners) {
            const index = streamEventListeners.indexOf(listener);
            if (index != -1) {
               streamEventListeners.splice(index, 1);
               if (streamEventListeners.length == 0) {
                  p.streamEventListenerIdMap.delete(streamEventID);
                  p.streamEventListenerMap.delete(streamEventInternalID);
                  hbbtv.bridge.broadcast.removeStreamEventListener(streamEventInternalID);
               }
            }
         } else {
            console.error("Unconsistent state, " + streamEventID + "(" + streamEventInternalID + ") has no listeners.");
         }
      } else {
         console.error("Unexisting Stream Event Listener " + streamEventID);
      }
   };

   prototype.recordNow = function() {
      const p = privates.get(this);
      mandatoryBroadcastRelatedSecurityCheck(p);
      // TODO
   };

   prototype.stopRecording = function() {
      const p = privates.get(this);
      mandatoryBroadcastRelatedSecurityCheck(p);
      // TODO
   };

   function acquireActiveState() {
      if (gActiveStateOwner !== null) {
         const owner = hbbtv.utils.weakDeref(gActiveStateOwner)
         if (owner !== this) {
            return false;
         }
      }
      gActiveStateOwner = hbbtv.utils.weakRef(this);
      gObjectFinalizedWhileActive.register(this);
      return true;
   }

   function releaseActiveState() {
      if (gActiveStateOwner !== null) {
         const owner = hbbtv.utils.weakDeref(gActiveStateOwner)
         if (owner === this) {
            gObjectFinalizedWhileActive.unregister(this);
            hbbtv.bridge.broadcast.setPresentationSuspended(gBroadbandAvInUse);
            gActiveStateOwner = null;
         }
      }
   }

   // DOM level 0 event properties
   Object.defineProperty(prototype, "onfocus", {
      get() {
         return privates.get(this).onfocusDomLevel0;
      },
      set(listener) {
         const p = privates.get(this);
         if (p.onfocusDomLevel0) {
            this.removeEventListener("focus", p.onfocusDomLevel0);
         }
         p.onfocusDomLevel0 = listener;
         if (listener) {
            this.addEventListener("focus", p.onfocusDomLevel0);
         }
      }
   });

   Object.defineProperty(prototype, "onblur", {
      get() {
         return privates.get(this).onblurDomLevel0;
      },
      set(listener) {
         const p = privates.get(this);
         if (p.onblurDomLevel0) {
            this.removeEventListener("blur", p.onblurDomLevel0);
         }
         p.onblurDomLevel0 = listener;
         if (listener) {
            this.addEventListener("blur", p.onblurDomLevel0);
         }
      }
   });

   Object.defineProperty(prototype, "onFullScreenChange", {
      get() {
         return privates.get(this).onFullScreenChangeDomLevel0;
      },
      set(listener) {
         const p = privates.get(this);
         if (p.onFullScreenChangeDomLevel0) {
            this.removeEventListener("FullScreenChange", p.onFullScreenChangeDomLevel0);
         }
         p.onFullScreenChangeDomLevel0 = listener;
         if (listener) {
            this.addEventListener("FullScreenChange", p.onFullScreenChangeDomLevel0);
         }
      }
   });

   Object.defineProperty(prototype, "onChannelChangeError", {
      get() {
         return privates.get(this).onChannelChangeErrorDomLevel0;
      },
      set(listener) {
         const p = privates.get(this);
         if (p.onChannelChangeErrorDomLevel0) {
            this.removeEventListener("ChannelChangeError", p.onChannelChangeErrorWrapper);
            p.onChannelChangeErrorWrapper = null;
         }
         p.onChannelChangeErrorDomLevel0 = listener;
         if (listener) {
            p.onChannelChangeErrorWrapper = (ev) => {
               listener(ev.channel, ev.errorState);
            };
            this.addEventListener("ChannelChangeError", p.onChannelChangeErrorWrapper);
         }
      }
   });

   Object.defineProperty(prototype, "onChannelChangeSucceeded", {
      get() {
         return privates.get(this).onChannelChangeSucceededDomLevel0;
      },
      set(listener) {
         const p = privates.get(this);
         if (p.onChannelChangeSucceededDomLevel0) {
            this.removeEventListener("ChannelChangeSucceeded", p.onChannelChangeSucceededWrapper);
            p.onChannelChangeSucceededWrapper = null;
         }
         p.onChannelChangeSucceededDomLevel0 = listener;
         if (listener) {
            p.onChannelChangeSucceededWrapper = (ev) => {
               listener(ev.channel);
            };
            this.addEventListener("ChannelChangeSucceeded", p.onChannelChangeSucceededWrapper);
         }
      }
   });

   Object.defineProperty(prototype, "onPlayStateChange", {
      get() {
         return privates.get(this).onPlayStateChangeDomLevel0;
      },
      set(listener) {
         const p = privates.get(this);
         if (p.onPlayStateChangeDomLevel0) {
            this.removeEventListener("PlayStateChange", p.onPlayStateChangeWrapper);
            p.onPlayStateChangeWrapper = null;
         }
         p.onPlayStateChangeDomLevel0 = listener;
         if (listener) {
            p.onPlayStateChangeWrapper = (ev) => {
               listener(ev.state, ev.error);
            };
            this.addEventListener("PlayStateChange", p.onPlayStateChangeWrapper);
         }
      }
   });

   Object.defineProperty(prototype, "onProgrammesChanged", {
      get() {
         return privates.get(this).onProgrammesChangedDomLevel0;
      },
      set(listener) {
         const p = privates.get(this);
         if (p.onProgrammesChangedDomLevel0) {
            this.removeEventListener("ProgrammesChanged", p.onProgrammesChangedDomLevel0);
         }
         p.onProgrammesChangedDomLevel0 = listener;
         if (listener) {
            this.addEventListener("ProgrammesChanged", p.onProgrammesChangedDomLevel0);
         }
      }
   });

   Object.defineProperty(prototype, "onParentalRatingChange", {
      get() {
         return privates.get(this).onParentalRatingChangeDomLevel0;
      },
      set(listener) {
         const p = privates.get(this);
         if (p.onParentalRatingChangeDomLevel0) {
            this.removeEventListener("ParentalRatingChange", p.onParentalRatingChangeWrapper);
            p.onParentalRatingChangeWrapper = null;
         }
         p.onParentalRatingChangeDomLevel0 = listener;
         if (listener) {
            p.onParentalRatingChangeWrapper = (ev) => {
               listener(ev.contentID, ev.ratings, ev.DRMSystemID, ev.blocked);
            };
            this.addEventListener("ParentalRatingChange", p.onParentalRatingChangeWrapper);
         }
      }
   });

   Object.defineProperty(prototype, "onParentalRatingError", {
      get() {
         return privates.get(this).onParentalRatingErrorDomLevel0;
      },
      set(listener) {
         const p = privates.get(this);
         if (p.onParentalRatingErrorDomLevel0) {
            this.removeEventListener("ParentalRatingError", p.onParentalRatingErrorWrapper);
            p.onParentalRatingErrorWrapper = null;
         }
         p.onParentalRatingErrorDomLevel0 = listener;
         if (listener) {
            p.onParentalRatingErrorWrapper = (ev) => {
               listener(ev.contentID, ev.ratings, ev.DRMSystemID);
            };
            this.addEventListener("ParentalRatingError", p.onParentalRatingErrorWrapper);
         }
      }
   });

   Object.defineProperty(prototype, "onSelectedComponentChanged", {
      get() {
         return privates.get(this).onSelectedComponentChangedDomLevel0;
      },
      set(listener) {
         const p = privates.get(this);
         if (p.onSelectedComponentChangedDomLevel0) {
            this.removeEventListener("SelectedComponentChanged", p.onSelectedComponentChangedWrapper);
            p.onSelectedComponentChangedWrapper = null;
         }
         p.onSelectedComponentChangedDomLevel0 = listener;
         if (listener) {
            p.onSelectedComponentChangedWrapper = (ev) => {
               listener(ev.componentType);
            };
            this.addEventListener("SelectedComponentChanged", p.onSelectedComponentChangedWrapper);
         }
      }
   });

   Object.defineProperty(prototype, "onComponentChanged", {
      get() {
         return privates.get(this).onComponentChangedDomLevel0;
      },
      set(listener) {
         const p = privates.get(this);
         if (p.onComponentChangedDomLevel0) {
            this.removeEventListener("ComponentChanged", p.onComponentChangedWrapper);
            p.onComponentChangedWrapper = null;
         }
         p.onComponentChangedDomLevel0 = listener;
         if (listener) {
            p.onComponentChangedWrapper = (ev) => {
               listener(ev.componentType);
            };
            this.addEventListener("ComponentChanged", p.onComponentChangedWrapper);
         }
      }
   });

   function dispatchFocusEvent() {
      noRestrictionSecurityCheck();
      const event = new Event("focus");
      privates.get(this).eventDispatcher.dispatchEvent(event);
   }

   function dispatchBlurEvent() {
      noRestrictionSecurityCheck();
      const event = new Event("blur");
      privates.get(this).eventDispatcher.dispatchEvent(event);
   }

   function dispatchFullScreenChangeEvent() {
      noRestrictionSecurityCheck();
      const event = new Event("FullScreenChange");
      privates.get(this).eventDispatcher.dispatchEvent(event);
   }

   function dispatchChannelChangeErrorEvent(channel, errorState) {
      noRestrictionSecurityCheck();
      const event = new Event("ChannelChangeError");
      Object.assign(event, {
         channel: channel,
         errorState: errorState
      });
      privates.get(this).eventDispatcher.dispatchEvent(event);
   }

   function dispatchChannelChangeSucceededEvent(channel) {
      noRestrictionSecurityCheck();
      const event = new Event("ChannelChangeSucceeded");
      Object.assign(event, {
         channel: channel
      });
      privates.get(this).eventDispatcher.dispatchEvent(event);
   }

   function dispatchPlayStateChangeEvent(state, error) {
      noRestrictionSecurityCheck();
      const event = new Event("PlayStateChange");
      Object.assign(event, {
         state: state,
         error: error
      });
      privates.get(this).eventDispatcher.dispatchEvent(event);
   }

   function dispatchProgrammesChanged() {
      const p = privates.get(this);
      mandatoryBroadcastRelatedSecurityCheck(p);
      const event = new Event("ProgrammesChanged");
      p.eventDispatcher.dispatchEvent(event);
   }

   function dispatchParentalRatingChange(contentID, ratings, DRMSystemID, blocked) {
      const p = privates.get(this);
      mandatoryBroadcastRelatedSecurityCheck(p);
      const event = new Event("ParentalRatingChange");
      Object.assign(event, {
         contentID: contentID,
         ratings: ratings,
         DRMSystemID: DRMSystemID,
         blocked: blocked
      });
      p.eventDispatcher.dispatchEvent(event);
   }

   function dispatchParentalRatingError(contentID, ratings, DRMSystemID) {
      const p = privates.get(this);
      mandatoryBroadcastRelatedSecurityCheck(p);
      const event = new Event("ParentalRatingError");
      Object.assign(event, {
         contentID: contentID,
         ratings: ratings,
         DRMSystemID: DRMSystemID
      });
      p.eventDispatcher.dispatchEvent(event);
   }

   function dispatchSelectedComponentChanged(componentType) {
      const p = privates.get(this);
      mandatoryBroadcastRelatedSecurityCheck(p);
      const event = new Event("SelectedComponentChanged");
      Object.assign(event, {
         componentType: componentType
      });
      p.eventDispatcher.dispatchEvent(event);
      const event2 = new Event("SelectedComponentChange");
      Object.assign(event2, {
         componentType: componentType
      });
      p.eventDispatcher.dispatchEvent(event2);
   }

   function dispatchComponentChanged(componentType) {
      const p = privates.get(this);
      mandatoryBroadcastRelatedSecurityCheck(p);
      const event = new Event("ComponentChanged");
      Object.assign(event, {
         componentType: componentType
      });
      p.eventDispatcher.dispatchEvent(event);
   }

   function dispatchStreamEvent(id, name, data, text, status) {
      const p = privates.get(this);
      mandatoryBroadcastRelatedSecurityCheck(p);
      const event = new Event("StreamEvent");
      Object.assign(event, {
         name: name,
         data: data,
         text: text,
         status: status
      });
      const listeners = p.streamEventListenerMap.get(id);
      if (listeners) {
         listeners.forEach(listener => hbbtv.utils.runOnMainLooper(this, function() {
            listener(event);
         }));
         if (status === "error") {
            p.streamEventListenerMap.delete(id);
            for (let [key, value] of p.streamEventListenerIdMap) {
               if (value === id) {
                  p.streamEventListenerIdMap.delete(key);
                  break;
               }
            }
         }
      }
   }

   function initialise() {
      /* TODO: the video/broadcast embedded object (...) SHALL adhere to the tuner related security
       * requirements in section 10.1.3.1. => TLS handshake through a valid X.509v3 certificate */
      privates.set(this, {});
      const p = privates.get(this);
      p.eventDispatcher = new hbbtv.utils.EventDispatcher(this);
      /* Associates targetURL::eventName with internal ID */
      p.streamEventListenerIdMap = new Map();
      /* Associates internal ID with registered listeners */
      p.streamEventListenerMap = new Map();
      p.playState = this.PLAY_STATE_UNREALIZED;
      p.waitingPlayStateConnectingConfirm = false;
      p.fullScreen = false;
      p.x = 0;
      p.y = 0;
      p.width = 1280;
      p.height = 720;
      this.widescreen = true; // TODO
      p.display_none = (this.style.display === 'none');

      // The application either starts off broadcast-independent or becomes broadcast-independent
      // when setChannel(null, ...) is called on the realized v/b object. Here we set we are
      // broadcast-related unless getCurrentChannel() throws SecurityError.
      p.currentChannelData = null;
      p.channelConfig = null;
      p.isTransitioningToBroadcastRelated = false;
      setIsBroadcastRelated.call(this, true);
      try {
         p.currentChannelData = hbbtv.objects.createChannel(hbbtv.bridge.broadcast.getCurrentChannel());
         p.channelConfig = hbbtv.objects.createChannelConfig();
      } catch (e) {
         if (e.name === 'SecurityError') {
            p.currentChannelData = null;
            setIsBroadcastRelated.call(this, false);
         } else {
            throw (e);
         }
      }
   }

   function setIsBroadcastRelated(value) {
      const p = privates.get(this);
      p.isBroadcastRelated = value;
      if (p.channelConfig !== null) {
         hbbtv.objects.ChannelConfig.setIsBroadcastRelated.call(p.channelConfig, value);
      }
   }

   function avComponentArrayToCollection(avArray) {
      let result = [];
      avArray.forEach(function(item, index) {
         switch (item.type) {
            case this.COMPONENT_TYPE_VIDEO:
               result[index] = hbbtv.objects.createAVVideoComponent(item);
               break;
            case this.COMPONENT_TYPE_AUDIO:
               result[index] = hbbtv.objects.createAVAudioComponent(item);
               break;
            case this.COMPONENT_TYPE_SUBTITLE:
               result[index] = hbbtv.objects.createAVSubtitleComponent(item);
               break;
            default:
               result[index] = hbbtv.objects.createAVComponent(item);
         }
      }, this);
      return hbbtv.objects.createCollection(result, this);
   }

   function getFormattedComponents(ccid) {
      let components = hbbtv.bridge.broadcast.getComponents(ccid, -1);
      components.forEach(function(item, index) {
         /* Hide internal status properties */
         Object.defineProperty(item, "active", {
            enumerable: false
         });
         Object.defineProperty(item, "default", {
            enumerable: false
         });
         Object.defineProperty(item, "hidden", {
            enumerable: false
         });
         if (item.aspectRatio !== undefined) {
            if (item.aspectRatio === 0) {
               item.aspectRatio = 1.33; // 4:3
            } else {
               item.aspectRatio = 1.78; // 16:9
            }
         }

      });
      return components;
   }

   function compareComponents(a, b) {
      if (a.type === b.type &&
         a.componentTag === b.componentTag &&
         a.pid === b.pid &&
         a.encoding === b.encoding &&
         a.encrypted === b.encrypted) {
         switch (a.type) {
            case this.COMPONENT_TYPE_VIDEO: {
               return a.aspectRatio === b.aspectRatio;
            }
            case this.COMPONENT_TYPE_AUDIO: {
               return a.language === b.language &&
                  a.audioDescription === b.audioDescription &&
                  a.audioChannels === b.audioChannels;
            }
            case this.COMPONENT_TYPE_SUBTITLE: {
               return a.language === b.language &&
                  a.hearingImpaired === b.hearingImpaired &&
                  a.label === b.label;
            }
         }
      }
      return false;
   }

   function getStreamEventID(targetURL, eventName) {
      return targetURL + "::" + eventName;
   }

   function notifyBroadbandAvInUse(broadbandAvInUse) {
      // This is a "static" method that does not use a "this" variable. Call this method like:
      // hbbtv.objects.VideoBroadcast.notifyBroadbandAvInUse(true).
      const hasRealizedObject = gActiveStateOwner !== null &&
         hbbtv.utils.weakDeref(gActiveStateOwner) !== null;
      if (gBroadbandAvInUse !== broadbandAvInUse) {
         gBroadbandAvInUse = broadbandAvInUse;
         if (hasRealizedObject) {
            // It's up to the realized object whether we suspsend broadcast presentation or not.
         } else {
            hbbtv.bridge.broadcast.setPresentationSuspended(broadbandAvInUse);
         }
      }
   }

   return {
      prototype: prototype,
      notifyBroadbandAvInUse: notifyBroadbandAvInUse,
      initialise: initialise
   };
})();

hbbtv.objects.upgradeToVideoBroadcast = function(object) {
   /* TODO: Support only one active v/b object HbbTV 2.0.3 A.2.4.2 */
   Object.setPrototypeOf(object, hbbtv.objects.VideoBroadcast.prototype);
   hbbtv.objects.VideoBroadcast.initialise.call(object);
};

hbbtv.objectManager.registerObject({
   name: "video/broadcast",
   mimeTypes: ["video/broadcast"],
   oipfObjectFactoryMethodName: "createVideoBroadcastObject",
   upgradeObject: function(object) {
      hbbtv.objects.upgradeToVideoBroadcast(object);
   }
});
