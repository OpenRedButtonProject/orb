/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 * <p>
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package org.orbtv.orblibrary;

import android.util.Log;

import org.json.JSONObject;

import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

class MediaSwitcherManager {
    private static final String TAG = MediaSwitcherManager.class.getSimpleName();

    private final Object mLock = new Object();
    private final MediaSynchroniserManager mMediaSyncManager;

    private long mNativeManagerPointerField; // Reserved for native library
    // Track which timelines each switcher is monitoring
    private final Map<Integer, Set<String>> mSwitcherTimelines = new HashMap<>();

    MediaSwitcherManager(OrbSessionFactory.Configuration config, MediaSynchroniserManager mediaSyncManager) {
        mMediaSyncManager = mediaSyncManager;
        mNativeManagerPointerField = 0;
        jniInitialise();
    }

    /**
     * Create a new MediaSwitcher instance.
     *
     * @return The ID of the new MediaSwitcher instance.
     */
    public int createMediaSwitcher() {
        synchronized (mLock) {
            return jniCreateMediaSwitcher();
        }
    }

    /**
     * Destroy a MediaSwitcher instance.
     *
     * @param id The ID of the MediaSwitcher instance.
     */
    public void destroyMediaSwitcher(int id) {
        synchronized (mLock) {
            // Stop monitoring all timelines for this switcher
            Set<String> timelines = mSwitcherTimelines.remove(id);
            if (timelines != null) {
                for (String timelineSelector : timelines) {
                    // Only stop if no other switcher is monitoring this timeline
                    boolean otherSwitcherMonitoring = false;
                    for (Map.Entry<Integer, Set<String>> entry : mSwitcherTimelines.entrySet()) {
                        if (entry.getKey() != id && entry.getValue().contains(timelineSelector)) {
                            otherSwitcherMonitoring = true;
                            break;
                        }
                    }
                    if (!otherSwitcherMonitoring) {
                        mMediaSyncManager.stopTimelineMonitoring(timelineSelector, false);
                    }
                }
            }
            jniDestroyMediaSwitcher(id);
        }
    }

    /**
     * Request a media switch.
     *
     * @param id The ID of the MediaSwitcher instance.
     * @param params The switch parameters as a JSONObject.
     * @return True if the switch request was accepted.
     */
    public boolean switchMediaPresentation(int id, JSONObject params) {
        synchronized (mLock) {
            return jniSwitchMediaPresentation(id, params.toString());
        }
    }

    /**
     * Start monitoring a timeline for a switch.
     * Delegates to MediaSynchroniserManager to avoid duplication.
     *
     * @param id The ID of the MediaSwitcher instance.
     * @param timelineSelector The timeline selector.
     * @param timelineSource True if timeline is from originalMediaObject (true = master, false = other).
     * @return True if monitoring started successfully.
     */
    public boolean startTimelineMonitoring(int id, String timelineSelector, boolean timelineSource) {
        synchronized (mLock) {
            // Check if this timeline is already being monitored
            boolean alreadyMonitoring = false;
            for (Set<String> timelines : mSwitcherTimelines.values()) {
                if (timelines.contains(timelineSelector)) {
                    alreadyMonitoring = true;
                    break;
                }
            }

            // If not already monitoring, start monitoring via MediaSynchroniserManager
            if (!alreadyMonitoring) {
                // timelineSource: true means timeline is from originalMediaObject (master),
                // false means from newMediaObject (other)
                boolean isMaster = timelineSource;
                if (!mMediaSyncManager.startTimelineMonitoring(timelineSelector, isMaster)) {
                    return false;
                }
            }

            // Track this timeline for this switcher
            Set<String> timelines = mSwitcherTimelines.get(id);
            if (timelines == null) {
                timelines = new HashSet<>();
                mSwitcherTimelines.put(id, timelines);
            }
            timelines.add(timelineSelector);

            return true;
        }
    }

    /**
     * Stop monitoring a timeline.
     * Delegates to MediaSynchroniserManager to avoid duplication.
     *
     * @param id The ID of the MediaSwitcher instance.
     * @param timelineSelector The timeline selector.
     */
    public void stopTimelineMonitoring(int id, String timelineSelector) {
        synchronized (mLock) {
            Set<String> timelines = mSwitcherTimelines.get(id);
            if (timelines != null) {
                timelines.remove(timelineSelector);
                // Only stop monitoring if no other switcher is monitoring this timeline
                boolean otherSwitcherMonitoring = false;
                for (Map.Entry<Integer, Set<String>> entry : mSwitcherTimelines.entrySet()) {
                    if (entry.getKey() != id && entry.getValue().contains(timelineSelector)) {
                        otherSwitcherMonitoring = true;
                        break;
                    }
                }
                if (!otherSwitcherMonitoring) {
                    mMediaSyncManager.stopTimelineMonitoring(timelineSelector, false);
                }
            }
        }
    }

    /**
     * Get the current time on a timeline.
     * Delegates to MediaSynchroniserManager to avoid duplication.
     *
     * @param timelineSelector The timeline selector.
     * @return The current time in ticks, or -1 if unavailable.
     */
    public long getTimelineCurrentTime(String timelineSelector) {
        synchronized (mLock) {
            // Delegate to MediaSynchroniserManager which already has the infrastructure
            return mMediaSyncManager.getContentTime(timelineSelector);
        }
    }

    /**
     * Release resources.
     */
    public void releaseResources() {
        synchronized (mLock) {
            // Stop monitoring all timelines for all switchers
            Set<String> allTimelines = new HashSet<>();
            for (Set<String> timelines : mSwitcherTimelines.values()) {
                allTimelines.addAll(timelines);
            }
            for (String timelineSelector : allTimelines) {
                mMediaSyncManager.stopTimelineMonitoring(timelineSelector, false);
            }
            mSwitcherTimelines.clear();
            jniReleaseResources();
        }
    }

    private native void jniInitialise();

    private native int jniCreateMediaSwitcher();

    private native void jniDestroyMediaSwitcher(int id);

    private native boolean jniSwitchMediaPresentation(int id, String paramsJson);

    private native void jniReleaseResources();
}
