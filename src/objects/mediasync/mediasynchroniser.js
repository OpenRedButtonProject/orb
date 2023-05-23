/**
 * @fileOverview application/hbbtvMediaSynchroniser object
 * @license ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

hbbtv.objects.MediaSynchroniser = (function() {
    const prototype = Object.create(HTMLObjectElement.prototype);
    const privates = new WeakMap();
    let mediaSyncs = {};
    let lastMediaSync = undefined;
    const MEDIA_ERROR_LOOKUP = {
        [MediaError.MEDIA_ERR_ABORTED]: 16,
        [MediaError.MEDIA_ERR_DECODE]: 16,
        [MediaError.MEDIA_ERR_NETWORK]: 14,
        [MediaError.MEDIA_ERR_SRC_NOT_SUPPORTED]: 16,
    };

    window.addEventListener('beforeunload', () => {
        for (let id in mediaSyncs) {
            hbbtv.bridge.mediaSync.destroy(id);
        }
    });

    hbbtv.utils.defineGetterProperties(prototype, {
        lastError() {
            return privates.get(this).lastError;
        },
        lastErrorSource() {
            return privates.get(this).lastErrorSource;
        },
        nrOfSlaves() {
            const nr = hbbtv.bridge.mediaSync.nrOfSlaves(privates.get(this).id);
            return nr == -1 ? null : nr;
        },
        interDeviceSyncEnabled() {
            return hbbtv.bridge.mediaSync.interDeviceSyncEnabled(privates.get(this).id);
        },
        maxBroadbandStreamsWithBroadcast() {
            return 1;
        },
        maxBroadbandStreamsNoBroadcast() {
            return 1;
        },
        currentTime() {
            const p = privates.get(this);
            if (!p.mediaObserver) {
                return NaN;
            }
            return p.mediaObserver.contentTime;
        },
    });

    hbbtv.utils.defineGetterSetterProperties(prototype, {
        contentIdOverride: {
            get() {
                return hbbtv.bridge.mediaSync.getContentIdOverride(privates.get(this).id);
            },
            set(val) {
                hbbtv.bridge.mediaSync.setContentIdOverride(privates.get(this).id, val || '');
            },
        },
    });

    prototype.initMediaSynchroniser = async function(mediaObject, timelineSelector) {
        const p = privates.get(this);
        const isBroadcast = mediaObject.getAttribute('__mimeType') === 'video/broadcast';
        if (p.inPermanentErrorState) {
            dispatchErrorEvent.call(this, 13, null); // in permanent error state (transient)
        } else if (lastMediaSync === this) {
            dispatchErrorEvent.call(this, 17, null); // already initialised (transient)
        } else if (
            !isBroadcast &&
            (mediaObject.readyState < HTMLMediaElement.HAVE_CURRENT_DATA || mediaObject.ended)
        ) {
            setToPermanentErrorState.call(this);
            dispatchErrorEvent.call(this, 16, mediaObject); // mediaobject not in suitable state (permanent)
        } else if (!hbbtv.bridge.mediaSync.initialise(p.id, isBroadcast)) {
            console.warn('MediaSynchroniser: Failed to initialise Media Sync object.'); // this should never happen
            setToPermanentErrorState.call(this);
            dispatchErrorEvent.call(this, 13, null); // in permanent error state (transient)
        } else if (!hbbtv.bridge.mediaSync.startTimelineMonitoring(p.id, timelineSelector, true)) {
            setToPermanentErrorState.call(this);
            dispatchErrorEvent.call(this, 15, null); // unavailable/unsupported timeline selector (permanent)
        } else {
            function refreshContentId() {
                let params = [];
                return mediaObject
                    .orb_getCurrentPeriod()
                    .then((curPeriod) => {
                        if (curPeriod) {
                            if (curPeriod.id) {
                                params.push('period=' + curPeriod.id);
                            }
                            if (curPeriod.ciAncillaryData) {
                                params.push(
                                    'period_ci_ancillary=' + curPeriod.ciAncillaryData.__text
                                );
                            }
                        }
                        return mediaObject.orb_getCiAncillaryData();
                    })
                    .then((ciAncData) => {
                        if (ciAncData) {
                            params.push('mpd_ci_ancillary=' + ciAncData);
                        }
                    })
                    .finally(() => {
                        p.contentId = mediaObject.src;
                        if (params.length > 0) {
                            p.contentId += '#' + params.join('&');
                        }
                    });
            }

            p.timelineUnavailableHandler = (e) => {
                console.log(
                    "MediaSynchroniser: Timeline with selector '" +
                    e.timelineSelector +
                    "' is unavailable."
                );
                if (
                    e.timelineSelector === timelineSelector &&
                    setToPermanentErrorState.call(this)
                ) {
                    dispatchErrorEvent.call(this, 15, mediaObject); // unsupported timeline selector (permanent)
                }
            };

            p.timelineAvailableHandler = (e) => {
                console.log(
                    "MediaSynchroniser: Timeline with selector '" +
                    e.timeline.timelineSelector +
                    "' is now available."
                );
                if (e.timeline.timelineSelector === timelineSelector) {
                    if (p.mediaObserver) {
                        p.mediaObserver.timeline = e.timeline;
                    }
                    if (!p.synchroniserInitialised) {
                        p.synchroniserInitialised = true;
                        dispatchEvent.call(this, 'SynchroniserInitialised');
                    }
                }
            };
            hbbtv.bridge.addWeakEventListener('TimelineUnavailable', p.timelineUnavailableHandler);
            hbbtv.bridge.addWeakEventListener('TimelineAvailable', p.timelineAvailableHandler);

            if (lastMediaSync && !privates.get(lastMediaSync).inPermanentErrorState) {
                // invalidate previously initilised media synchroniser
                setToPermanentErrorState.call(lastMediaSync);
                dispatchErrorEvent.call(lastMediaSync, 18, mediaObject); // replaced by a new media synchroniser (permanent)
            }

            p.masterMediaObject = mediaObject;

            lastMediaSync = this;

            if (isBroadcast) {
                if (
                    mediaObject.playState ===
                    hbbtv.objects.BroadcastObserver.prototype.PLAY_STATE_UNREALIZED ||
                    mediaObject.playState ===
                    hbbtv.objects.BroadcastObserver.prototype.PLAY_STATE_STOPPED
                ) {
                    errorHandler();
                    return;
                }
                p.mediaObserver = hbbtv.objects.createBroadcastObserver(mediaObject);
            } else {
                p.mediaObserver = hbbtv.objects.createMediaElementObserver(mediaObject);
                await refreshContentId();
                const mrsUrl = await extractMrsUrl(mediaObject);
                hbbtv.bridge.mediaSync.updateCssCiiProperties(
                    p.id,
                    p.contentId,
                    mediaObject.readyState >= HTMLMediaElement.HAVE_CURRENT_DATA ?
                    'okay' :
                    'transitioning',
                    'final',
                    mrsUrl
                );
            }

            if (p.mediaObserver.start()) {
                let relIndex = timelineSelector.indexOf(':rel:');
                let curPeriod = undefined;
                let timelines = {};

                p.onPeriodChangedHandler = async (e) => {
                    if (curPeriod && e.data.id !== curPeriod) {
                        //make available timeline based on e.data.id
                        let currentTimelineSelector = timelineSelector.replace(
                            curPeriod,
                            e.data.id
                        );
                        relIndex = currentTimelineSelector.indexOf(':rel:');

                        if (relIndex >= 0) {
                            console.warn(
                                'MediaSynchroniser: DASH period id changed from ' +
                                curPeriod +
                                ' to ' +
                                e.data.id
                            );
                            hbbtv.bridge.mediaSync.setTimelineAvailability(
                                p.id,
                                timelines[curPeriod],
                                false,
                                0,
                                0
                            );
                            curPeriod = currentTimelineSelector
                                .substring(relIndex + 5)
                                .split(':')[1];
                            if (curPeriod) {
                                if (timelines[curPeriod] !== currentTimelineSelector) {
                                    timelines[curPeriod] = currentTimelineSelector;
                                    // in case timelineSelector of master media changes (i.e. period change)
                                    timelineSelector = currentTimelineSelector;
                                    hbbtv.bridge.mediaSync.startTimelineMonitoring(
                                        p.id,
                                        currentTimelineSelector,
                                        true
                                    );
                                }
                                hbbtv.bridge.mediaSync.setTimelineAvailability(
                                    p.id,
                                    currentTimelineSelector,
                                    true,
                                    p.mediaObserver.contentTicks,
                                    p.mediaObserver.timelineSpeedMultiplier
                                );
                            }
                        }
                        await refreshContentId();
                        const mrsUrl = await extractMrsUrl(p.masterMediaObject);
                        hbbtv.bridge.mediaSync.updateCssCiiProperties(
                            p.id,
                            p.contentId,
                            p.masterMediaObject.readyState >= HTMLMediaElement.HAVE_CURRENT_DATA ?
                            'okay' :
                            'transitioning',
                            'final',
                            mrsUrl
                        );
                    }
                };

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
                            hbbtv.bridge.mediaSync.stopTimelineMonitoring(
                                p.id,
                                timelines[pid],
                                true
                            );
                            delete timelines[pid];
                        }
                    }
                };

                p.mediaObserver.addEventListener('MediaUpdated', mediaUpdatedHandler);
                p.mediaObserver.addEventListener('Error', errorHandler);

                if (relIndex >= 0) {
                    mediaObject.addEventListener(
                        '__orb_onstreamupdated__',
                        p.onStreamUpdatedHandler
                    );
                    mediaObject.addEventListener(
                        '__orb_onperiodchanged__',
                        p.onPeriodChangedHandler
                    );
                    curPeriod = timelineSelector.substring(relIndex + 5).split(':')[1];
                    if (curPeriod) {
                        const curPeriodInfo = await mediaObject.orb_getCurrentPeriod();
                        if (curPeriodInfo && curPeriodInfo.id !== curPeriod) {
                            // while starting mediasync, dash is streaming with a different timelineselector
                            hbbtv.bridge.mediaSync.stopTimelineMonitoring(
                                p.id,
                                timelineSelector,
                                true
                            );
                        } else {
                            timelines[curPeriod] = timelineSelector;
                            hbbtv.bridge.mediaSync.setTimelineAvailability(
                                p.id,
                                timelineSelector,
                                true,
                                p.mediaObserver.contentTicks,
                                p.mediaObserver.timelineSpeedMultiplier
                            );
                        }
                    } else {
                        hbbtv.bridge.mediaSync.setTimelineAvailability(
                            p.id,
                            timelineSelector,
                            true,
                            p.mediaObserver.contentTicks,
                            p.mediaObserver.timelineSpeedMultiplier
                        );
                    }
                } else {
                    hbbtv.bridge.mediaSync.setTimelineAvailability(
                        p.id,
                        timelineSelector,
                        true,
                        p.mediaObserver.contentTicks,
                        p.mediaObserver.timelineSpeedMultiplier
                    );
                }
            } else {
                errorHandler();
            }
        }
    };

    prototype.addMediaObject = function(
        mediaObject,
        timelineSelector,
        correlationTimestamp,
        tolerance,
        multiDecoderMode
    ) {
        const p = privates.get(this);
        if (correlationTimestamp === undefined) {
            correlationTimestamp = {
                tlvMaster: 0,
                tlvOther: 0,
            };
        }
        if (isNaN(tolerance) || tolerance < 0) {
            tolerance = 0;
        }

        if (p.inPermanentErrorState) {
            dispatchErrorEvent.call(this, 13, null); // in permanent error state (transient)
        } else if (lastMediaSync !== this) {
            dispatchErrorEvent.call(this, 7, null); // not yet initialised (transient)
        } else if (p.masterMediaObject === mediaObject || p.mediaObjects.has(mediaObject)) {
            dispatchErrorEvent.call(this, 4, mediaObject); // media object already added (transient)
        } else if (0) {
            // TODO: check streams combination support
            dispatchErrorEvent.call(this, 20, mediaObject); // unsupported combination of streams (transient)
        } else if (
            correlationTimestamp === null ||
            isNaN(correlationTimestamp.tlvMaster) ||
            isNaN(correlationTimestamp.tlvOther)
        ) {
            dispatchErrorEvent.call(this, 5, mediaObject); // invalid correlation timestamp (transient)
        } else if (mediaObject.error) {
            dispatchErrorEvent.call(this, 2, mediaObject); // failure in presentation of the media object (transient)
        } else if (mediaObject.readyState < HTMLMediaElement.HAVE_CURRENT_DATA) {
            dispatchErrorEvent.call(this, 9, mediaObject); // not in suitable state for sync (transient)
        } else if (!hbbtv.bridge.mediaSync.startTimelineMonitoring(p.id, timelineSelector, false)) {
            dispatchErrorEvent.call(this, 3, mediaObject); // unavailable/unsupported timeline selector (transient)
        } else {
            p.mediaObjects.add(mediaObject);

            if (mediaObject.getAttribute('__mimeType') !== 'video/broadcast') {
                privates.set(mediaObject, {
                    tsClient: hbbtv.objects.createMediaElementTsClient(
                        mediaObject,
                        timelineSelector,
                        correlationTimestamp,
                        tolerance,
                        multiDecoderMode,
                        p.mediaObserver,
                        p.id
                    ),
                    onAudioTrackChanged: onAudioTrackChanged.bind(mediaObject),
                    timelineSelector: timelineSelector,
                });
            } else {
                // TODO: implement TS client for broadcast if needed
            }
            const priv = privates.get(mediaObject);
            mediaObject.audioTracks.addEventListener('change', priv.onAudioTrackChanged);
            priv.tsClient.addEventListener('SyncNowAchievable', () =>
                dispatchEvent.call(this, 'SyncNowAchievable', {
                    mediaObject: mediaObject,
                })
            );
            priv.tsClient.addEventListener('Error', (e) => {
                dispatchErrorEvent.call(this, e.errorCode, mediaObject);
                if (e.errorCode === 2 || e.errorCode === 9) {
                    this.removeMediaObject(mediaObject);
                }
            });
            dispatchEvent.call(this, 'MediaObjectAdded', {
                mediaObject: mediaObject,
            });
        }
    };

    prototype.removeMediaObject = function(mediaObject) {
        const p = privates.get(this);

        if (p.inPermanentErrorState) {
            dispatchErrorEvent.call(this, 13, null); // in permanent error state (transient)
        } else if (lastMediaSync !== this) {
            dispatchErrorEvent.call(this, 7, null); // not yet initialised (transient)
        } else if (mediaObject === p.masterMediaObject) {
            setToPermanentErrorState.call(this);
            dispatchErrorEvent.call(this, 18, mediaObject); // replaced by a new media synchroniser (permanent)
        } else if (!p.mediaObjects.has(mediaObject)) {
            dispatchErrorEvent.call(this, 8, mediaObject); // media object was not found (transient)
        } else {
            const priv = privates.get(mediaObject);
            mediaObject.audioTracks.removeEventListener('change', priv.onAudioTrackChanged);
            hbbtv.bridge.mediaSync.stopTimelineMonitoring(p.id, priv.timelineSelector, false);
            priv.tsClient.destroy();
            p.mediaObjects.delete(mediaObject);
            privates.delete(mediaObject);
            p.mediaObserver.muted = false;
            console.log('MediaSynchroniser: Removed media object from media synchroniser.');
        }
    };

    prototype.updateCorrelationTimestamp = function(mediaObject, correlationTimestamp) {
        const p = privates.get(this);
        if (correlationTimestamp === undefined) {
            correlationTimestamp = {
                tlvMaster: 0,
                tlvOther: 0,
            };
        }

        if (lastMediaSync !== this) {
            dispatchErrorEvent.call(this, 7, null); // not yet initialised (transient)
        } else if (p.inPermanentErrorState) {
            dispatchErrorEvent.call(this, 13, null); // in permanent error state (transient)
        } else if (!p.mediaObjects.has(mediaObject)) {
            dispatchErrorEvent.call(this, 8, mediaObject); // media object has not been added (transient)
        } else if (
            !correlationTimestamp ||
            isNaN(correlationTimestamp.tlvMaster) ||
            isNaN(correlationTimestamp.tlvOther)
        ) {
            dispatchErrorEvent.call(this, 5, mediaObject); // invalid correlation timestamp (transient)
        } else {
            privates.get(mediaObject).tsClient.correlationTimestamp = correlationTimestamp;
        }
    };

    prototype.enableInterDeviceSync = function(callback) {
        const p = privates.get(this);
        const thiz = this;

        const enableInterDeviceSync = function() {
            if (!p.inPermanentErrorState) {
                if (typeof callback === 'function') {
                    const cb = (e) => {
                        hbbtv.bridge.removeWeakEventListener('InterDeviceSyncEnabled', cb);
                        if (e.id == p.id) {
                            callback();
                        }
                    };
                    hbbtv.bridge.addWeakEventListener('InterDeviceSyncEnabled', cb);
                }
                hbbtv.bridge.mediaSync.enableInterDeviceSync(p.id);
                console.log('MediaSynchroniser: Enabled inter-device synchronisation.');
            }
            thiz.removeEventListener('SynchroniserInitialised', enableInterDeviceSync);
        };

        if (p.inPermanentErrorState) {
            dispatchErrorEvent.call(this, 13, null); // in permanent error state (transient)
        } else if (lastMediaSync !== this) {
            dispatchErrorEvent.call(this, 7, null); // not yet initialised (transient)
        } else {
            if (p.synchroniserInitialised) {
                enableInterDeviceSync();
            } else {
                this.addEventListener('SynchroniserInitialised', enableInterDeviceSync);
            }
        }
    };

    prototype.disableInterDeviceSync = function(callback) {
        const p = privates.get(this);
        const thiz = this;

        const disableInterDeviceSync = function() {
            if (!p.inPermanentErrorState) {
                if (typeof callback === 'function') {
                    const cb = (e) => {
                        hbbtv.bridge.removeWeakEventListener('InterDeviceSyncDisabled', cb);
                        if (e.id == p.id) {
                            callback();
                        }
                    };
                    hbbtv.bridge.addWeakEventListener('InterDeviceSyncDisabled', cb);
                }
                hbbtv.bridge.mediaSync.disableInterDeviceSync(p.id);
                console.log('MediaSynchroniser: Disabled inter-device synchronisation.');
            }
            thiz.removeEventListener('SynchroniserInitialised', disableInterDeviceSync);
        };

        if (p.inPermanentErrorState) {
            dispatchErrorEvent.call(this, 13, null); // in permanent error state (transient)
        } else if (lastMediaSync !== this) {
            dispatchErrorEvent.call(this, 7, null); // not yet initialised (transient)
        } else {
            if (p.synchroniserInitialised) {
                disableInterDeviceSync();
            } else {
                this.addEventListener('SynchroniserInitialised', disableInterDeviceSync);
            }
        }
    };

    prototype.addEventListener = function(type, listener) {
        privates.get(this).eventTarget.addEventListener(type, listener);
    };

    prototype.removeEventListener = function(type, listener) {
        privates.get(this).eventTarget.removeEventListener(type, listener);
    };

    async function mediaUpdatedHandler(e) {
        const p = privates.get(lastMediaSync);
        if (p.masterMediaObject.getAttribute('__mimeType') !== 'video/broadcast') {
            const mrsUrl = await extractMrsUrl(p.masterMediaObject);
            hbbtv.bridge.mediaSync.updateCssCiiProperties(
                p.id,
                p.contentId,
                p.masterMediaObject.readyState >= HTMLMediaElement.HAVE_CURRENT_DATA ?
                'okay' :
                'transitioning',
                'final',
                mrsUrl
            );
            if (p.mediaObserver.timeline && p.mediaObserver.timeline.timelineSelector) {
                hbbtv.bridge.mediaSync.setContentTimeAndSpeed(
                    p.id,
                    p.mediaObserver.timeline.timelineSelector,
                    p.mediaObserver.contentTicks,
                    p.mediaObserver.timelineSpeedMultiplier
                );
            }
        }
    }

    async function errorHandler() {
        const p = privates.get(lastMediaSync);
        if (p.masterMediaObject.getAttribute('__mimeType') !== 'video/broadcast') {
            const mrsUrl = await extractMrsUrl(p.masterMediaObject);
            hbbtv.bridge.mediaSync.updateCssCiiProperties(
                p.id,
                p.contentId,
                'fault',
                'final',
                mrsUrl
            );
        }
        if (setToPermanentErrorState.call(lastMediaSync)) {
            if (p.masterMediaObject.getAttribute('__mimeType') !== 'video/broadcast') {
                console.log('MediaSynchroniser: Media element error:', p.masterMediaObject.error);
                if (
                    p.masterMediaObject.error &&
                    MEDIA_ERROR_LOOKUP[p.masterMediaObject.error.code] !== undefined
                ) {
                    dispatchErrorEvent.call(
                        lastMediaSync,
                        MEDIA_ERROR_LOOKUP[p.masterMediaObject.error.code],
                        p.masterMediaObject
                    );
                } else {
                    dispatchErrorEvent.call(lastMediaSync, 16, p.masterMediaObject); // not in suitable state for sync (permanent)
                }
            } else {
                dispatchErrorEvent.call(lastMediaSync, 16, p.masterMediaObject); // not in suitable state for sync (permanent)
            }
        }
    }

    function extractMrsUrl(mediaObject) {
        return mediaObject.orb_getMrsUrl().then((mrsUrl) => (mrsUrl ? mrsUrl : ''));
    }

    function onAudioTrackChanged() {
        let muteMasterMedia = false;
        for (const audioTrack of this.audioTracks) {
            if (audioTrack.enabled) {
                muteMasterMedia = true;
                break;
            }
        }
        privates.get(lastMediaSync).mediaObserver.muted = muteMasterMedia;
    }

    function setToPermanentErrorState() {
        const p = privates.get(this);
        let ret = false;
        if (!p.inPermanentErrorState) {
            if (p.mediaObserver) {
                p.mediaObserver.removeEventListener('MediaUpdated', mediaUpdatedHandler);
                p.mediaObserver.removeEventListener('Error', errorHandler);
                p.mediaObserver.stop();
                p.mediaObserver = undefined;
            }
            if (p.masterMediaObject) {
                p.masterMediaObject.removeEventListener(
                    '__orb_onstreamupdated__',
                    p.onStreamUpdatedHandler
                );
                p.masterMediaObject.removeEventListener(
                    '__orb_onperiodchanged__',
                    p.onPeriodChangedHandler
                );
            }
            for (const mediaObject of p.mediaObjects) {
                const priv = privates.get(mediaObject);
                priv.tsClient.destroy();
                mediaObject.audioTracks.removeEventListener('change', priv.onAudioTrackChanged);
                privates.delete(mediaObject);
            }
            p.mediaObjects.clear();
            if (p.timelineUnavailableHandler) {
                hbbtv.bridge.removeWeakEventListener(
                    'TimelineUnavailable',
                    p.timelineUnavailableHandler
                );
            }
            if (p.timelineAvailableHandler) {
                hbbtv.bridge.removeWeakEventListener(
                    'TimelineAvailable',
                    p.timelineAvailableHandler
                );
            }
            p.inPermanentErrorState = true;

            hbbtv.bridge.mediaSync.destroy(p.id);
            delete mediaSyncs[p.id];

            ret = true;
        }
        return ret;
    }

    // helper function for dispatching events
    function dispatchEvent(event, contextInfo) {
        console.log("Dispatched '" + event + "' event.");
        const evt = new Event(event);
        if (contextInfo) {
            Object.assign(evt, contextInfo);
        }
        privates.get(this).eventTarget.dispatchEvent(evt);
        hbbtv.utils.runOnMainLooper(this, function() {
            if (this['on' + event]) {
                if (contextInfo) {
                    this['on' + event](...Object.values(contextInfo));
                } else {
                    this['on' + event]();
                }
            }
        });
    }

    // helper function for error events
    function dispatchErrorEvent(lastError, lastErrorSource) {
        const p = privates.get(this);
        p.lastError = lastError; // media object already added (transient)
        p.lastErrorSource = lastErrorSource;
        dispatchEvent.call(this, 'Error', {
            lastError: p.lastError,
            lastErrorSource: p.lastErrorSource,
        });
        console.log('MediaSynchroniser: Last error:', lastError);
    }

    function initialise() {
        privates.set(this, {
            eventTarget: document.createDocumentFragment(),
            mediaObjects: new Set(),
            inPermanentErrorState: false,
            id: hbbtv.bridge.mediaSync.instantiate(),
            lastError: null,
            lastErrorSource: null,
        });
        mediaSyncs[privates.get(this).id] = this;
    }

    return {
        prototype: prototype,
        initialise: initialise,
    };
})();

hbbtv.objects.upgradeToMediaSynchroniser = function(object) {
    Object.setPrototypeOf(object, hbbtv.objects.MediaSynchroniser.prototype);
    hbbtv.objects.MediaSynchroniser.initialise.call(object);
};

hbbtv.objectManager.registerObject({
    name: 'application/hbbtvMediaSynchroniser',
    mimeTypes: ['application/hbbtvmediasynchroniser'],
    oipfObjectFactoryMethodName: 'createMediaSynchroniser',
    upgradeObject: function(object) {
        hbbtv.objects.upgradeToMediaSynchroniser(object);
    },
});