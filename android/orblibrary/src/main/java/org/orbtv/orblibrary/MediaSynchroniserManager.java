/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 * <p>
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

package org.orbtv.orblibrary;

import org.json.JSONException;
import org.orbtv.orbpolyfill.BridgeTypes;

import java.util.List;

class MediaSynchroniserManager {
    private static final String TAG = MediaSynchroniserManager.class.getSimpleName();

    private SessionCallback mSessionCallback;
    private final Object mLock = new Object();

    private long mNativeManagerPointerField; // Reserved for native library

    MediaSynchroniserManager(OrbSessionFactory.Configuration config) {
        mSessionCallback = null;
        jniInitialise(config.mediaSyncCiiPort, config.mediaSyncWcPort, config.mediaSyncTsPort);
    }

    interface SessionCallback {
        /**
         * Tell the bridge
         *
         * @param
         * @param
         */
        public void dispatchTimelineAvailableEvent(BridgeTypes.Timeline timeline);

        /**
         * Tell the bridge
         *
         * @param
         * @param
         */
        public void dispatchTimelineUnavailableEvent(String timelineSelector);

        /**
         * Tell the bridge
         *
         * @param
         * @param
         */
        public void dispatchInterDeviceSyncEnabled(int mediaSyncId);

        /**
         * Tell the bridge
         *
         * @param
         * @param
         */
        public void dispatchInterDeviceSyncDisabled(int mediaSyncId);

        /**
         * Tell the tv browser callback
         *
         * @param
         * @param
         */
        public int startTEMITimelineMonitoring(int componentTag, int timelineId);

        /**
         * Tell the tv browser callback
         *
         * @param
         * @param
         */
        public boolean stopTEMITimelineMonitoring(int filterId);

        /**
         * Tell the tv browser callback
         *
         * @param
         * @param
         */
        public long getCurrentPtsTime();

        /**
         * Tell the tv browser callback
         *
         * @param
         * @param
         */
        public long getCurrentTemiTime(int filterId);
    }

    public void setSessionCallback(SessionCallback sessionCallback) {
        synchronized (mLock) {
            mSessionCallback = sessionCallback;
        }
    }

    public void updateBroadcastContentStatus(int onetId, int transId, int servId, int statusCode,
                                             boolean permanentError, List<BridgeTypes.Programme> programmes) {

        boolean pError = false;
        long startTime = 0;
        long duration = 0;
        String programmeId = "";
        String hexOnetId = Integer.toHexString(onetId);
        String hexTransId = Integer.toHexString(transId);
        String hexServId = Integer.toHexString(servId);
        if (programmes != null) {
            for (BridgeTypes.Programme programme : programmes) {
                if (programme != null && programme.programmeId != null && !programme.programmeId.isEmpty()) {
                    String[] ids = programme.programmeId.split(";");
                    if (ids.length > 0) {
                        ids = ids[0].replace("dvb://", "").split("\\.");
                        if (ids.length == 3 && ids[0].equalsIgnoreCase(hexOnetId) &&
                            ids[1].equalsIgnoreCase(hexTransId) && ids[2].equalsIgnoreCase(hexServId)) {
                            programmeId = programme.programmeId.trim();
                            startTime = programme.startTime;
                            duration = programme.duration;
                            break;
                        }
                    }
                }
            }
        }

        if (statusCode >= BridgeTypes.CHANNEL_STATUS_NOT_SUPPORTED) {
            pError = permanentError;
        }

        switch (statusCode) {
            case BridgeTypes.CHANNEL_STATUS_PRESENTING:
            case BridgeTypes.CHANNEL_STATUS_CONNECTING:
            case BridgeTypes.CHANNEL_STATUS_NOT_SUPPORTED:
            case BridgeTypes.CHANNEL_STATUS_NO_SIGNAL:
            case BridgeTypes.CHANNEL_STATUS_TUNER_IN_USE:
            case BridgeTypes.CHANNEL_STATUS_PARENTAL_LOCKED:
            case BridgeTypes.CHANNEL_STATUS_ENCRYPTED:
            case BridgeTypes.CHANNEL_STATUS_UNKNOWN_CHANNEL:
            case BridgeTypes.CHANNEL_STATUS_INTERRUPTED:
            case BridgeTypes.CHANNEL_STATUS_RECORDING_IN_PROGRESS:
            case BridgeTypes.CHANNEL_STATUS_CANNOT_RESOLVE_URI:
            case BridgeTypes.CHANNEL_STATUS_INSUFFICIENT_BANDWIDTH:
            case BridgeTypes.CHANNEL_STATUS_CANNOT_BE_CHANGED:
            case BridgeTypes.CHANNEL_STATUS_INSUFFICIENT_RESOURCES:
            case BridgeTypes.CHANNEL_STATUS_CHANNEL_NOT_IN_TS:
            case BridgeTypes.CHANNEL_STATUS_UNKNOWN_ERROR:
                jniUpdateDvbInfo(onetId,
                        transId,
                        servId,
                        pError,
                        statusCode == BridgeTypes.CHANNEL_STATUS_PRESENTING,
                        programmeId,
                        startTime,
                        duration);
                break;
            default:
                break;
        }
    }

    public int createMediaSynchroniser() {
        return jniCreateMediaSynchroniser();
    }

    public boolean initialiseMediaSynchroniser(int id, boolean isMasterBroadcast) {
        return jniInitialiseMediaSynchroniser(id, isMasterBroadcast);
    }

    public void destroyMediaSynchroniser(int id) {
        jniDestroyMediaSynchroniser(id);
    }

    public boolean enableInterDeviceSync(String ipAddr) {
        return jniEnableInterDeviceSync(ipAddr);
    }

    public void disableInterDeviceSync() {
        jniDisableInterDeviceSync();
    }

    public int nrOfSlaves(int id) {
        return jniNrOfSlaves(id);
    }

    public boolean interDeviceSyncEnabled(int id) {
        return jniInterDeviceSyncEnabled(id);
    }

    public String getContentIdOverride(int id) {
        return jniGetContentIdOverride(id);
    }

    public void setContentIdOverride(int id, String cid) {
        jniSetContentIdOverride(id, cid);
    }

    public boolean startTimelineMonitoring(String timelineSelector, boolean isMaster) {
        return jniStartTimelineMonitoring(timelineSelector, isMaster);
    }

    public void stopTimelineMonitoring(String timelineSelector, boolean forceStop) {
        jniStopTimelineMonitoring(timelineSelector, forceStop);
    }

    public void setContentTimeAndSpeed(String timelineSelector, long contentTime, double speed) {
        jniSetContentTimeAndSpeed(timelineSelector, contentTime, speed);
    }

    public long getContentTime(String timelineSelector) {
        return jniGetContentTime(timelineSelector);
    }

    public void updateCssCiiProperties(String contentId, String presentationStatus, String contentIdStatus, String mrsUrl) {
        jniUpdateCssCiiProperties(contentId, presentationStatus, contentIdStatus, mrsUrl);
    }

    public void setTEMITimelineAvailability(int filterId, boolean isAvailable, long currentTime, long timescale, double speed) {
        jniSetTEMITimelineAvailability(filterId, isAvailable, currentTime, timescale, speed);
    }

    public boolean setTimelineAvailability(int id, String timelineSelector, boolean isAvailable, long ticks, double speed) {
        return jniSetTimelineAvailability(id, timelineSelector, isAvailable, ticks, speed);
    }

    public void releaseResources() {
        jniReleaseResources();
    }

    private void jniCbDispatchTimelineAvailableEvent(String timelineSelector, long unitsPerSecond) {
        synchronized (mLock) {
            if (mSessionCallback != null) {
                try {
                    BridgeTypes.Timeline timeline = new BridgeTypes.Timeline(timelineSelector);
                    if (timeline.unitsPerSecond == null) { // just in case it is not set in the timeline selector
                        timeline.unitsPerSecond = unitsPerSecond;
                    }
                    mSessionCallback.dispatchTimelineAvailableEvent(timeline);
                } catch (JSONException e) {
                    e.printStackTrace();
                }
            }
        }
    }

    private void jniCbDispatchTimelineUnavailableEvent(String timelineSelector) {
        synchronized (mLock) {
            if (mSessionCallback != null) {
                mSessionCallback.dispatchTimelineUnavailableEvent(timelineSelector);
            }
        }
    }

    private void jniCbDispatchInterDeviceSyncEnabled(int mediaSyncId) {
        synchronized (mLock) {
            if (mSessionCallback != null) {
                mSessionCallback.dispatchInterDeviceSyncEnabled(mediaSyncId);
            }
        }
    }

    private void jniCbDispatchInterDeviceSyncDisabled(int mediaSyncId) {
        synchronized (mLock) {
            if (mSessionCallback != null) {
                mSessionCallback.dispatchInterDeviceSyncDisabled(mediaSyncId);
            }
        }
    }

    private int jniCbStartTEMITimelineMonitoring(int componentTag, int timelineId) {
        int result = -1;
        synchronized (mLock) {
            if (mSessionCallback != null) {
                result = mSessionCallback.startTEMITimelineMonitoring(componentTag, timelineId);
            }
        }
        return result;
    }

    private boolean jniCbStopTEMITimelineMonitoring(int filterId) {
        boolean result = false;
        synchronized (mLock) {
            if (mSessionCallback != null) {
                result = mSessionCallback.stopTEMITimelineMonitoring(filterId);
            }
        }
        return result;
    }

    private long jniCbGetCurrentPtsTime() {
        long result = -1;
        synchronized (mLock) {
            if (mSessionCallback != null) {
                result = mSessionCallback.getCurrentPtsTime();
            }
        }
        return result;
    }

    private long jniCbGetCurrentTemiTime(int filterId) {
        long result = -1;
        synchronized (mLock) {
            if (mSessionCallback != null) {
                result = mSessionCallback.getCurrentTemiTime(filterId);
            }
        }
        return result;
    }

    private native int jniCreateMediaSynchroniser();

    private native void jniInitialise(int ciiPort, int wcPort, int tsPort);

    private native boolean jniInitialiseMediaSynchroniser(int id, boolean isMasterBroadcast);

    private native void jniDestroyMediaSynchroniser(int id);

    private native boolean jniEnableInterDeviceSync(String ipAddr);

    private native void jniDisableInterDeviceSync();

    private native int jniNrOfSlaves(int id);

    private native boolean jniInterDeviceSyncEnabled(int id);

    private native String jniGetContentIdOverride(int id);

    private native void jniSetContentIdOverride(int id, String cid);

    private native boolean jniStartTimelineMonitoring(String timeline, boolean isMaster);

    private native void jniStopTimelineMonitoring(String timelineSelector, boolean forceStop);

    private native void jniUpdateCssCiiProperties(String contentId, String presentationStatus, String contentIdStatus, String mrsUrl);

    private native boolean jniSetContentTimeAndSpeed(String timelineSelector, long contentTime, double speed);

    private native long jniGetContentTime(String timelineSelector);

    private native boolean jniSetTEMITimelineAvailability(int filterId, boolean isAvailable, long currentTime, long timescale, double speed);

    private native boolean jniSetTimelineAvailability(int id, String timelineSelector, boolean isAvailable, long ticks, double speed);

    private native void jniUpdateDvbInfo(int onetId, int transId, int servId, boolean permanentError, boolean presenting, String progarammeId, long startTime, long duration);

    private native void jniReleaseResources();
}