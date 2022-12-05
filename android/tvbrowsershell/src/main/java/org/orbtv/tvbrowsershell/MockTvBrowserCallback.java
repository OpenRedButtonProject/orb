/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 * <p>
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

package org.orbtv.tvbrowsershell;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;

import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;
import android.util.SparseArray;
import android.view.KeyEvent;

import org.orbtv.tvbrowser.TvBrowser;
import org.orbtv.tvbrowser.TvBrowserTypes;
import org.orbtv.tvbrowser.TvBrowserCallback;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Vector;

public class MockTvBrowserCallback implements TvBrowserCallback {
    private static final String TAG = "MockTvBrowserCallback";

    private final MainActivity mMainActivity;
    private final MockDsmcc mMockDsmcc;
    private final String mManifest;
    Context mContext;
    private final Database mDatabase;
    private final MockHttpServer mServer;

    private TvBrowser.Session mSession = null;
    private TestSuiteRunner mTestSuiteRunner = null;
    private TestSuiteScenario mTestSuiteScenario;

    private final HashMap<Integer, Handler> mActiveSearchList = new HashMap<>();

    static SparseArray<Integer> mKeyMap;

    MockTvBrowserCallback(MainActivity mainActivity, MockDsmcc mockDsmcc, Bundle extras)
            throws Exception {
        mMainActivity = mainActivity;
        mMockDsmcc = mockDsmcc;
        if (extras != null) {
            mManifest = extras.getString("testsuite_manifest", "");
        } else {
            mManifest = "";
        }
        mContext = mainActivity.getApplicationContext();
        mDatabase = new Database(mContext, 1);
        mServer = new MockHttpServer(mContext);
        mTestSuiteRunner = new TestSuiteRunner(mContext, mManifest, mServer.getLocalHost(),
                new TestSuiteRunner.Callback() {
                    @Override
                    public void onTestSuiteStarted(String name, TestSuiteScenario scenario, String dsmccData, String action) {
                        mTestSuiteScenario = scenario;
                        mMockDsmcc.setDsmccData(dsmccData);
                        if (action.equals("simulate_successful_tune")) {
                            simulateSuccessfulTune();
                        }
                        Log.d("orb_automation_msg", "testsuite_started,name=" + name);
                    }

                    @Override
                    public void onTestSuiteFinished(String name, String report) {
                        Log.d("orb_automation_msg", "testsuite_finished,name=" + name + ",report=" + report);
                    }

                    @Override
                    public void onFinished() {
                        Log.d("orb_automation_msg", "finished");
                        System.exit(0);
                    }
                });
    }

    /**
     * This method is called once the session is ready to be called by the client and present HbbTV
     * applications.
     *
     * @param session The session (for convenience)
     */
    @Override
    public void onSessionReady(TvBrowser.Session session) {
        mSession = session;
        mTestSuiteRunner.run();
    }

    /**
     * Get immutable system information.
     *
     * @return Valid SystemInformation on success, otherwise invalid SystemInformation
     */
    @Override
    public TvBrowserTypes.SystemInformation getSystemInformation() {
        return new TvBrowserTypes.SystemInformation(
                true,
                "OBS",
                "Mock",
                BuildConfig.VERSION_NAME + " (" + BuildConfig.VERSION_CODE + ")",
                "0.0.0"
        );
    }

    /**
     * Gets a string containing languages to be used for audio playback, in order of preference.
     *
     * @return Comma separated string of languages (ISO 639-2 codes)
     */
    @Override
    public String getPreferredAudioLanguage() {
        return "eng,spa,gre";
    }

    /**
     * Gets a string containing languages to be used for subtitles, in order of preference.
     *
     * @return Comma separated string of languages (ISO 639-2 codes)
     */
    @Override
    public String getPreferredSubtitleLanguage() {
        return "en,eng,spa,gre";
    }

    /**
     * Gets a string containing languages to be used for the UI, in order of preference.
     *
     * @return Comma separated string of languages (ISO 639-2 codes)
     */
    @Override
    public String getPreferredUILanguage() {
        return "eng,spa,gre";
    }

    /**
     * Gets a string containing the three character country code identifying the country in which the
     * receiver is deployed.
     *
     * @return Country code (ISO 3166 alpha-3) string
     */
    @Override
    public String getCountryId() {
        return "GBR";
    }

    /**
     * Gets whether subtitles are enabled in the TV context. So HbbTV knows to start subtitle
     * components on channel change, for example.
     *
     * @return true if enabled, false otherwise
     */
    @Override
    public boolean getSubtitlesEnabled() {
        return true;
    }

    /**
     * Gets whether audio description is enabled in the TV context.
     *
     * @return true if enabled, false otherwise
     */
    @Override
    public boolean getAudioDescriptionEnabled() {
        return false;
    }

    /**
     * Get DTT network IDs.
     *
     * @return Array of DTT network IDs.
     */
    @Override
    public int[] getDttNetworkIds() {
        ArrayList<Integer> dttNetworkIds = new ArrayList<>();
        int[] result;

        Vector<TvBrowserTypes.Channel> channels = mTestSuiteScenario.getMockChannels();
        if (channels != null) {
            for (TvBrowserTypes.Channel mockChannel : channels) {
                if (((mockChannel.idType == 12) || (mockChannel.idType == 16)) && // TODO(library) Replace magic numbers
                        !dttNetworkIds.contains(mockChannel.nid)) {
                    dttNetworkIds.add(mockChannel.nid);
                }
            }
        }
        Collections.sort(dttNetworkIds);
        result = new int[dttNetworkIds.size()];
        for (int i = 0; i < dttNetworkIds.size(); i++) {
            result[i] = dttNetworkIds.get(i);
        }
        return result;
    }

    /**
     * Retrieves an array containing the supported Broadcast Delivery Systems (DVB_S, DVB_C, DVB_T,
     * DVB_C2, DVB_T2 or DVB_S2) as defined in Section 9.2, Table 15, under "UI Profile Name
     * Fragment".
     *
     * @return Array of delivery systems
     */
    @Override
    public String[] getSupportedDeliverySystems() {
        return new String[0];
    }

    /**
     * Override the default component selection of the terminal for the specified type.
     *
     * If id is empty, no component shall be selected for presentation (presentation is explicitly
     * disabled). Otherwise, the specified component shall be selected for presentation.
     *
     * If playback has already started, the presented component shall be updated.
     *
     * Default component selection shall be restored (revert back to the control of the terminal)
     * when: (1) the application terminates, (2) the channel is changed, (3) presentation has not
     * been explicitly disabled and the user selects another track in the terminal UI, or (4) the
     * restoreComponentSelection method is called.
     *
     * @param type Type of component selection to override (COMPONENT_TYPE_* code).
     * @param id A platform-defined component id or an empty string to disable presentation.
     */
    @Override
    public void overrideComponentSelection(int type, String id) {
        delaySelectComponent(type, false, id);
    }

    /**
     * Restore the default component selection of the terminal for the specified type.
     *
     * If playback has already started, the presented component shall be updated.
     *
     * @param type Type of component selection override to clear (COMPONENT_TYPE_* code).
     */
    @Override
    public void restoreComponentSelection(int type) {
        delaySelectComponent(type, true, null);
    }

    /**
     * Sets the presentation window of the DVB video. Values are in HbbTV 1280x720 coordinates.
     *
     * @param x Rectangle definition
     * @param y Rectangle definition
     * @param width Rectangle definition
     * @param height Rectangle definition
     */
    @Override
    public void setDvbVideoRectangle(int x, int y, int width, int height) {
        Log.v(TAG, "HbbTVClient.setDvbVideoRectangle(" + x + ", " + y + ", " + width + ", " + height + ")");
    }

    /**
     * Get the list of channels available.
     *
     * @return List of channels available
     */
    @Override
    public List<TvBrowserTypes.Channel> getChannelList() {
        return mTestSuiteScenario.getMockChannels();
    }

    /**
     * Returns the CCID of the current channel
     *
     * @return A CCID on success, an empty string otherwise
     */
    @Override
    public String getCurrentCcid() {
        TvBrowserTypes.Channel currentChannel = mTestSuiteScenario.getCurrentChannel();
        if (currentChannel == null) {
            return "";
        }
        return currentChannel.ccid;
    }

    /**
     * Find the channel with the given LCN and return its CCID.
     *
     * @param lcn LCN to find
     * @return A CCID on success, an empty string otherwise
     */
   /*@Override
   public String findCcidWithLcn(String lcn) {
      return null;
   }*/

    /**
     * Get the channel with the given CCID.
     *
     * @param ccid CCID for the required channel
     * @return Channel on success
     */
    @Override
    public TvBrowserTypes.Channel getChannel(String ccid) {
        // Naive implementation for mock
        Vector<TvBrowserTypes.Channel> channels = mTestSuiteScenario.getMockChannels();
        if (channels != null) {
            for (TvBrowserTypes.Channel mockChannel : channels) {
                if (mockChannel.ccid.equals(ccid)) {
                    return mockChannel;
                }
            }
        }
        return new TvBrowserTypes.Channel();
    }

    /**
     * Tune to specified channel. The implementation relies on the 'idType' parameter to
     * determine the valid fields that describe the channel. Possible idTypes are:
     *    ID_IPTV_SDS/ID_IPTV_URI - where 'ipBroadcastID' and 'sourceId' fields are valid
     *    other ID_.. values - where 'onid', 'tsid' and 'sid' fields are valid
     *    ID_DVB_SI_DIRECT - is supposed to be handled by setChannelByDsd()
     *
     * @param idType The type of channel
     * @param onid The original network ID for the required channel.
     * @param tsid The transport stream ID for the required channel.
     * @param sid The service ID for the required channel.
     * @param sourceID The ATSC source_ID of the channel.
     * @param ipBroadcastID The DVB textual service identifier of the IP broadcast service.
     * @param trickplay Ignore unless PVR functionality is supported (does not affect return)
     * @param contentAccessDescriptorURL May be required by DRM-protected IPTV broadcasts
     * @param quiet Channel change operation
     *              0 - normal tune
     *              1 - normal tune and no UI displayed
     *              2 - quiet tune (user does not know)
     *
     * @return negative value (e.g. TvBrowserTypes.CHANNEL_STATUS_CONNECTING) on success, or
     *         zero/positive value (see TvBrowserTypes.CHANNEL_STATUS_.. error values) on failure
     */
    @Override
    public int setChannelByTriplet(int idType, int onid, int tsid, int sid, int sourceID,
                                   String ipBroadcastID, boolean trickplay,
                                   String contentAccessDescriptorURL, int quiet) {
        if ((onid == -1) && (tsid == -1) && (sid == -1)) {
            return TvBrowserTypes.CHANNEL_STATUS_UNREALIZED;
        }
        if (idType == TvBrowserTypes.Channel.ID_IPTV_SDS ||
                idType == TvBrowserTypes.Channel.ID_DVB_SI_DIRECT) {
            // DSD and IPTV channels are not supported in mock implementation
            return TvBrowserTypes.CHANNEL_STATUS_NOT_SUPPORTED;
        }

        int status = TvBrowserTypes.CHANNEL_STATUS_UNKNOWN_CHANNEL;
        if (mTestSuiteScenario.selectChannel(onid, tsid, sid)) {
            status = TvBrowserTypes.CHANNEL_STATUS_CONNECTING;
            simulateSuccessfulTune();
        } else {
            // TODO simulateUnsuccessfulTune();
        }

        return status;
    }

    /**
     * Tune to specified channel using DSD.
     *
     * @param dsd DSD for the required channel.
     * @param sid SID for the required channel.
     * @param trickplay Ignore unless PVR functionality is supported (does not affect return)
     * @param contentAccessDescriptorURL May be required by DRM-protected IPTV broadcasts
     * @param quiet Channel change operation
     *              0 - normal tune
     *              1 - normal tune and no UI displayed
     *              2 - quiet tune (user does not know)
     *
     * @return negative value (e.g. TvBrowserTypes.CHANNEL_STATUS_CONNECTING) on success, or
     * zero/positive value (see TvBrowserTypes.CHANNEL_STATUS_.. error values) on failure
     */
    @Override
    public int setChannelByDsd(String dsd, int sid, boolean trickplay,
                               String contentAccessDescriptorURL, int quiet) {
        if ((dsd == null)) {
            return TvBrowserTypes.CHANNEL_STATUS_UNREALIZED;
        }

        // TODO Parse DSD
        int onid = 1;
        int tsid = 65283;
        int finalSid = 28186;

        int status = TvBrowserTypes.CHANNEL_STATUS_UNKNOWN_CHANNEL;
        if (mTestSuiteScenario.selectChannel(onid, tsid, sid)) {
            status = TvBrowserTypes.CHANNEL_STATUS_CONNECTING;
            simulateSuccessfulTune();
        } else {
            // TODO simulateUnsuccessfulTune();
        }

        return status;
    }

    /**
     * Get the list of programmes available for a channel.
     *
     * @param ccid CCID for the required channel
     * @return List of programmes available for the channel
     */
    @Override
    public List<TvBrowserTypes.Programme> getProgrammeList(String ccid) {
        long time = System.currentTimeMillis() / 1000L - 1;
        Vector<TvBrowserTypes.Programme> programmes = mTestSuiteScenario.getCurrentChannelProgrammes();
        if (programmes != null) {
            for (TvBrowserTypes.Programme mockProgramme : programmes) {
                mockProgramme.channelId = ccid;
                mockProgramme.startTime = time;
                time += 3600;
            }
        }
        return programmes;
    }

    /**
     * Get information about the present and following programmes on a channel.
     *
     * @param ccid CCID for the required channel
     * @return List of containing the present and following programmes, in that order
     */
    @Override
    public List<TvBrowserTypes.Programme> getPresentFollowingProgrammes(String ccid) {
        return null;
    }

    /**
     * Get the list of components available for a channel.
     *
     * @param ccid     CCID for the required channel
     * @param typeCode Required component type as defined by IHbbTVTypes.COMPONENT_TYPE_*
     * @return List of components available for the channel
     */
    @Override
    public List<TvBrowserTypes.Component> getComponentList(String ccid, int typeCode) {
        return mTestSuiteScenario.getCurrentChannelComponents();
    }

    /**
     * Get a private audio component in the selected channel.
     *
     * @param componentTag The component_tag of the component.
     *
     * @return The private component with the specified component_tag in the PMT of the currently
     *    selected broadcast channel; or null if unavailable or the component is not private (i.e.
     *    the stream type is audio, video or subtitle).
     *
     *    Mandatory properties: id, pid and encrypted. The id property shall be usable with the
     *    overrideComponentSelection method to select the component as an audio track. Other
     *    Component properties are not required.
     */
    @Override
    public TvBrowserTypes.Component getPrivateAudioComponent(String componentTag) {
        // TODO
        return null;
    }

    /**
     * Get a private video component in the selected channel.
     *
     * @param componentTag The component_tag of the component.
     *
     * @return The private component with the specified component_tag in the PMT of the currently
     *    selected broadcast channel; or null if unavailable or the component is not private (i.e.
     *    the stream type is audio, video or subtitle).
     *
     *    Mandatory properties: id, pid and encrypted. The id property shall be usable with the
     *    overrideComponentSelection method to select the component as an video track. Other
     *    Component properties are not required.
     */
    @Override
    public TvBrowserTypes.Component getPrivateVideoComponent(String componentTag) {
        // TODO
        return null;
    }

    /**
     * Experimental: Do we actually need this data (as in 1.5) or can we use a different interface?
     * <p>
     * Retrieves raw SI descriptor data with the defined descriptor tag id, and optionally the
     * extended descriptor tag id, for an event on a service.
     *
     * @param ccid          CCID for the required channel
     * @param eventId       Event ID for the required programme
     * @param tagId         Descriptor tag ID of data to be returned
     * @param extendedTagId Optional extended descriptor tag ID of data to be returned, or -1
     * @return The buffer containing the data. If there are multiple descriptors with the same
     * tag id then they will all be returned.
     */
    @Override
    public List<String> getSiDescriptorData(String ccid, String eventId, int tagId, int extendedTagId,
                                            int privateDataSpecifier) {
        ArrayList<String> siDescriptors = new ArrayList<>();
        byte[] b = {0x5f, 4, 0, 0, 0, 5};
        siDescriptors.add(new String(b));
        Log.v(TAG, "getSiDescriptorData " + ccid + " " + eventId + " " + tagId + " " + extendedTagId + " " + privateDataSpecifier);

        //Code to start generating all the possible events after getSiDescriptorData() is called.
      /*Log.v(TAG, "Starting periodic events");
      android.os.Handler handler = new android.os.Handler(Looper.getMainLooper());
      handler.postDelayed(
              new Runnable() {
                 int eventNum = 0;

                 public void run() {
                    Log.v(TAG, "HbbTVCLient.java Sending EVENT " + eventNum);
                    try {
                       switch (eventNum) {
                          case 0://ChannelStatusChanged
                             //public void sendChannelStatusChanged(int onetId, int transId, int servId, int statusCode, boolean permanentError) throws android.os.RemoteException;
                             mSession.sendChannelStatusChanged(0,1,2,3, true);
                             break;
                          case 1://ProgrammesChanged
                             mSession.sendProgrammesChanged();
                             break;
                          case 2://ParentalRatingChange
                             mSession.sendParentalRatingChange(true);
                             break;
                          case 3://ParentalRatingError
                             mSession.sendParentalRatingError();
                             break;
                          case 4://notifySelectedComponentChanged
                             mSession.sendSelectedComponentChanged(0);
                             break;
                          case 5://notifyComponentChanged
                             mSession.sendComponentChanged(0);
                             break;
                       }
                    } catch (RemoteException e) {
                       e.printStackTrace();
                    }
                    eventNum = (eventNum + 1) % 6;


                    handler.postDelayed(this, 2000);
                 }
              },
              2000);*/

        return siDescriptors;
    }

    /**
     * Retrieves the locked status of the specified channel. The correct implementation of this
     * function is not mandatory for HbbTV 1.2.1. It is used to implement the channel's locked
     * property as defined in OIPF vol. 5, section 7.13.11.2.
     *
     * @param ccid CCID of the required channel
     * @return true if parental control prevents the channel being viewed, e.g. when a PIN needs to
     * be entered by the user, false otherwise
     */
    @Override
    public boolean getChannelLocked(String ccid) {
        return false;
    }

    /**
     * Returns the current age set for parental control. 0 will be returned if parental control is
     * disabled or no age is set.
     * @return age in the range 4-18, or 0
     */
    @Override
    public int getParentalControlAge() {
        return 15;
    }

    /**
     * Returns the region set for parental control.
     * @return country using the 3-character code as specified in ISO 3166
     */
    @Override
    public String getParentalControlRegion() {
        return "GB";
    }

    /**
     * Called when the application at origin requests access to the distinctive identifier. The
     * client application should display a dialog for the user to allow or deny this and:
     *
     * 1. TvBrowser.notifyAccessToDistinctiveIdentifier should be called with the user choice.
     * 2. TvBrowserSessionCallback.getDistinctiveIdentifier should honour the user choice.
     *
     * The client application should allow the user to reset access from a settings menu. This shall
     * result in a new distinctive identifier being generated for an origin next time access is
     * allowed.
     *
     * The helper method TvBrowser.generateDistinctiveIdentifier may be used.
     *
     * Integrators should check 12.1.5 for requirements about distinctive identifiers.
     *
     * @param origin The origin of the application
     * @return true if access already granted, false otherwise
     */
    @Override
    public boolean requestAccessToDistinctiveIdentifier(final String origin) {
        // Early out if access already granted for this origin
        if (mDatabase.hasDistinctiveIdentifier(origin)) {
            return true;
        }
        mMainActivity.runOnUiThread(() -> {
            // Actual implementations may want to support other languages
            AlertDialog.Builder builder = new AlertDialog.Builder(mMainActivity)
                    .setMessage("Allow the application at \"" + origin + "\" access to a distinctive identifier?")
                    .setCancelable(false)
                    .setPositiveButton("ALLOW", new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int id) {
                            String identifier = TvBrowser.createDistinctiveIdentifier("00001",
                                    "secret", origin);
                            mDatabase.setDistinctiveIdentifier(origin, identifier);
                            mSession.onAccessToDistinctiveIdentifierDecided(origin, true);
                            dialog.cancel();
                        }
                    })
                    .setNegativeButton("DENY", new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int id) {
                            mDatabase.deleteDistinctiveIdentifier(origin);
                            mSession.onAccessToDistinctiveIdentifierDecided(origin, false);
                            dialog.cancel();
                        }
                    });
            builder.show();
        });
        return false;
    }

    /**
     * The distinctive identifier for origin or a distinctive identifier status string (for statuses
     * see TvBrowserTypes.DISTINCTIVE_IDENTIFIER_STATUS_*).
     *
     * Integrators should check 12.1.5 for requirements about distinctive identifiers.
     *
     * @param origin The origin of the requesting application
     * @return The distinctive identifier for origin or a distinctive identifier status string.
     */
    @Override
    public String getDistinctiveIdentifier(String origin) {
        if (mDatabase.hasDistinctiveIdentifier(origin)) {
            return mDatabase.getDistinctiveIdentifier(origin);
        }
        return TvBrowserTypes.DISTINCTIVE_IDENTIFIER_STATUS_REQUEST_REQUIRED;
    }

    /**
     * Enables the application developer to query information about the current memory available
     * to the application. This is used to help during application development to find application
     * memory leaks and possibly allow an application to make decisions related to its caching
     * strategy (e.g. for images).
     *
     *  @return The available memory to the application (in MBs) or -1 if the information is not available.
     */
    @Override
    public long getFreeMemory() {
        //TODO https://web.dev/monitor-total-page-memory-usage/
        return -1;
    }

    @Override
    public boolean startSearch(final TvBrowserTypes.Query q, int offset, int count, List<String> channelConstraints) {
        /* Keep a list with the active searches */
        Handler handler = new android.os.Handler(Looper.getMainLooper());
        handler.postDelayed(new Runnable() {
                                public void run() {
                                    Handler handler = mActiveSearchList.get(q.getQueryId());
                                    if (handler != null) {
                                        mSession.onMetadataSearchCompleted(q.getQueryId(), 0, mTestSuiteScenario.getCurrentChannelProgrammes(), offset, count);
                                        Log.i(TAG, "sendMetadataSearchEvent(" + q.getQueryId() + ", 0)");
                                        mActiveSearchList.remove(q.getQueryId());
                                    }
                                }
                            },
                2000);
        mActiveSearchList.put(q.getQueryId(), handler);
        return true;
    }

    @Override
    public boolean abortSearch(int queryId) {
        Handler handler = mActiveSearchList.get(queryId);
        if (handler != null) {
            mActiveSearchList.remove(queryId);
            handler.removeCallbacksAndMessages(null);
        }
        return true;
    }

    /**
     * Convert the Android key code to a TV Browser (TvBrowserTypes.VK_*) key code.
     *
     * @param androidKeyCode The Android key code (KeyEvent.KEYCODE_*)
     * @return The TV Browser (TvBrowserTypes.VK_*) key code.
     */
    @Override
    public int getTvBrowserKeyCode(int androidKeyCode) {
        return mKeyMap.get(androidKeyCode, TvBrowserTypes.VK_INVALID);
    }

    /**
     * Start TEMI timeline monitoring.
     *
     * @param componentTag The component tag of the temi timeline to monitor.
     * @param timelineId The timeline id of the temi timeline to monitor.
     *
     * @return The associated filter id upon success, -1 otherwise
     */
    @Override
    public int startTEMITimelineMonitoring(int componentTag, int timelineId) {
        return -1;
    }

    /**
     * Set whether presentation of any broadcast components must be suspended.
     *
     * @param presentationSuspended true if must be suspended, false otherwise
     */
    @Override
    public void setPresentationSuspended(boolean presentationSuspended) {
    }

    private void delaySelectComponent(final int componentType, final boolean restoreDefault,
                                      final String id) {
        new android.os.Handler(Looper.getMainLooper()).postDelayed(() -> {
            if (restoreDefault) {
                Vector<TvBrowserTypes.Component> components =
                        mTestSuiteScenario.getCurrentChannelComponents();
                if (components != null) {
                    // Restore to first in list
                    for (TvBrowserTypes.Component c : components) {
                        if (c.componentType == componentType) {
                            mTestSuiteScenario.selectComponent(componentType, c.id);
                            break;
                        }
                    }
                }
            } else {
                mTestSuiteScenario.selectComponent(componentType, id);
            }
            mSession.onComponentChanged(componentType);
            mSession.onSelectedComponentChanged(componentType);
            Log.i(TAG, "sendComponentChanged(" + componentType + ") and " +
                    "sendSelectedComponentChanged(" + componentType + ")");
        }, 50);
    }

    /**
     * TODO: comment
     */
    @Override
    public boolean stopTEMITimelineMonitoring(int timelineId) {
        return true;
    }

    /**
     * Finalises TEMI timeline monitoring
     *
     * @return true on success, false otherwise
     */
    @Override
    public boolean finaliseTEMITimelineMonitoring() {
        return false;
    }

    /**
     * Get current TEMI time.
     *
     * @param filterId
     *
     * @return current TEMI time, -1 if not available
     */
    @Override
    public long getCurrentTemiTime(int filterId) {
        return -1;
    }

    /**
     * Get current PTS time.
     *
     * @return current PTS time, -1 if not available
     */
    @Override
    public long getCurrentPtsTime() {
        return -1;
    }

    /**
     * Get the IP address that should be used for network services.
     *
     * @return An IP address.
     */
    @Override
    public String getHostAddress() {
        return mMainActivity.getHostAddress();
    }

    /**
     * Publish a test report (debug build only).
     *
     * @param testSuite A unique test suite name.
     * @param xml The XML test report.
     */
    @Override
    public void publishTestReport(String testSuite, String xml) {
        if (!BuildConfig.DEBUG) {
            return;
        }
        mTestSuiteRunner.onTestReportPublished(testSuite, xml);
    }

    private static byte[] getAssetBytes(Context context, String asset) {
        byte[] buffer;
        try {
            InputStream inputStream = context.getAssets().open(asset);
            int fileSize = inputStream.available();
            buffer = new byte[fileSize];
            inputStream.read(buffer);
            inputStream.close();
        } catch (IOException ex) {
            buffer = null;
        }
        return buffer;
    }

    private static String getAssetString(Context context, String asset) {
        String string;
        try {
            InputStream inputStream = context.getAssets().open(asset);
            StringBuilder stringBuilder = new StringBuilder();
            BufferedReader bufferedReader = new BufferedReader(new InputStreamReader(inputStream,
                    StandardCharsets.UTF_8));
            String line;
            while ((line = bufferedReader.readLine()) != null) {
                stringBuilder.append(line);
            }
            string = stringBuilder.toString();
            bufferedReader.close();
            inputStream.close();
        } catch (IOException ex) {
            string = null;
        }
        return string;
    }

    private void simulateSuccessfulTune() {
        TvBrowserTypes.Channel channel = mTestSuiteScenario.getCurrentChannel();
        if (channel != null) {
            mSession.onChannelStatusChanged(channel.onid, channel.tsid, channel.sid,
                    TvBrowserTypes.CHANNEL_STATUS_CONNECTING, false);
            new android.os.Handler(Looper.getMainLooper()).postDelayed(() -> {
                mSession.onChannelStatusChanged(channel.onid, channel.tsid, channel.sid,
                        TvBrowserTypes.CHANNEL_STATUS_PRESENTING, false);
                mSession.processAitSection(506, channel.sid, mTestSuiteScenario.getCurrentChannelAit());
            }, 50);
        } else {
            // TODO TUNED OFF
        }
    }

    static {
        mKeyMap = new SparseArray<Integer>();
        mKeyMap.put(KeyEvent.KEYCODE_PROG_RED, TvBrowserTypes.VK_RED);
        mKeyMap.put(KeyEvent.KEYCODE_PROG_GREEN, TvBrowserTypes.VK_GREEN);
        mKeyMap.put(KeyEvent.KEYCODE_PROG_YELLOW, TvBrowserTypes.VK_YELLOW);
        mKeyMap.put(KeyEvent.KEYCODE_PROG_BLUE, TvBrowserTypes.VK_BLUE);
        mKeyMap.put(KeyEvent.KEYCODE_DPAD_UP, TvBrowserTypes.VK_UP);
        mKeyMap.put(KeyEvent.KEYCODE_DPAD_DOWN, TvBrowserTypes.VK_DOWN);
        mKeyMap.put(KeyEvent.KEYCODE_DPAD_LEFT, TvBrowserTypes.VK_LEFT);
        mKeyMap.put(KeyEvent.KEYCODE_DPAD_RIGHT, TvBrowserTypes.VK_RIGHT);
        mKeyMap.put(KeyEvent.KEYCODE_ENTER, TvBrowserTypes.VK_ENTER);
        mKeyMap.put(KeyEvent.KEYCODE_DPAD_CENTER, TvBrowserTypes.VK_ENTER);
        mKeyMap.put(KeyEvent.KEYCODE_DEL, TvBrowserTypes.VK_BACK);
        mKeyMap.put(KeyEvent.KEYCODE_0, TvBrowserTypes.VK_0);
        mKeyMap.put(KeyEvent.KEYCODE_1, TvBrowserTypes.VK_1);
        mKeyMap.put(KeyEvent.KEYCODE_2, TvBrowserTypes.VK_2);
        mKeyMap.put(KeyEvent.KEYCODE_3, TvBrowserTypes.VK_3);
        mKeyMap.put(KeyEvent.KEYCODE_4, TvBrowserTypes.VK_4);
        mKeyMap.put(KeyEvent.KEYCODE_5, TvBrowserTypes.VK_5);
        mKeyMap.put(KeyEvent.KEYCODE_6, TvBrowserTypes.VK_6);
        mKeyMap.put(KeyEvent.KEYCODE_7, TvBrowserTypes.VK_7);
        mKeyMap.put(KeyEvent.KEYCODE_8, TvBrowserTypes.VK_8);
        mKeyMap.put(KeyEvent.KEYCODE_9, TvBrowserTypes.VK_9);
        mKeyMap.put(KeyEvent.KEYCODE_MEDIA_STOP, TvBrowserTypes.VK_STOP);
        mKeyMap.put(KeyEvent.KEYCODE_MEDIA_PLAY, TvBrowserTypes.VK_PLAY);
        mKeyMap.put(KeyEvent.KEYCODE_MEDIA_PAUSE, TvBrowserTypes.VK_PAUSE);
        mKeyMap.put(KeyEvent.KEYCODE_MEDIA_FAST_FORWARD, TvBrowserTypes.VK_FAST_FWD);
        mKeyMap.put(KeyEvent.KEYCODE_MEDIA_REWIND, TvBrowserTypes.VK_REWIND);
        mKeyMap.put(KeyEvent.KEYCODE_TV_TELETEXT, TvBrowserTypes.VK_TELETEXT);

        // Standard keyboard aliases for development
        mKeyMap.put(KeyEvent.KEYCODE_R, TvBrowserTypes.VK_RED);
        mKeyMap.put(KeyEvent.KEYCODE_G, TvBrowserTypes.VK_GREEN);
        mKeyMap.put(KeyEvent.KEYCODE_Y, TvBrowserTypes.VK_YELLOW);
        mKeyMap.put(KeyEvent.KEYCODE_B, TvBrowserTypes.VK_BLUE);
        mKeyMap.put(KeyEvent.KEYCODE_S, TvBrowserTypes.VK_STOP);
        mKeyMap.put(KeyEvent.KEYCODE_P, TvBrowserTypes.VK_PLAY);
        mKeyMap.put(KeyEvent.KEYCODE_Z, TvBrowserTypes.VK_PAUSE);
        mKeyMap.put(KeyEvent.KEYCODE_F, TvBrowserTypes.VK_FAST_FWD);
        mKeyMap.put(KeyEvent.KEYCODE_X, TvBrowserTypes.VK_REWIND);
        mKeyMap.put(KeyEvent.KEYCODE_T, TvBrowserTypes.VK_TELETEXT);
    }
}
