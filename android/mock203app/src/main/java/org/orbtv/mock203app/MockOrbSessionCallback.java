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

package org.orbtv.mock203app;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;

import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;
import android.util.SparseArray;
import android.view.KeyEvent;

import org.orbtv.orblibrary.IOrbSession;
import org.orbtv.orblibrary.OrbSessionFactory;
import org.orbtv.orblibrary.IOrbSessionCallback;
import org.orbtv.orbpolyfill.BridgeTypes;

import java.io.BufferedInputStream;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;
import java.nio.charset.StandardCharsets;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardOpenOption;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Vector;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;

public class MockOrbSessionCallback implements IOrbSessionCallback {
    private static final String TAG = "MockTvBrowserCallback";

    private final MainActivity mMainActivity;
    private final String mManifest;
    Context mContext;
    private final Database mDatabase;
    private String mBasePath;
    private String mDsmPath = "";
    private final HashMap<Integer, Handler> mActiveSubscriptionList;
    private final MockHttpServer mServer;

    private IOrbSession mSession = null;
    private int mVolume = 100;
    private TestSuiteRunner mTestSuiteRunner = null;
    private TestSuiteScenario mTestSuiteScenario;

    private final HashMap<Integer, Handler> mActiveSearchList = new HashMap<>();

    static SparseArray<Integer> mKeyMap;

    MockOrbSessionCallback(MainActivity mainActivity, Bundle extras)
            throws Exception {
        mMainActivity = mainActivity;
        if (extras != null) {
            mManifest = extras.getString("testsuite_manifest", "");
        } else {
            mManifest = "";
        }
        mContext = mainActivity.getApplicationContext();
        mDatabase = new Database(mContext, 1);
        mBasePath = mContext.getDataDir().getPath() + "/dsmcc/";
        File base = new File(mBasePath);
        if (base.exists()) {
            Utils.recursiveDelete(base);
        }
        mActiveSubscriptionList = new HashMap<>();
        mServer = new MockHttpServer(mContext);
        mTestSuiteRunner = new TestSuiteRunner(mContext, mManifest, mServer.getLocalHost(),
                new TestSuiteRunner.Callback() {
                    @Override
                    public void onTestSuiteStarted(String name, TestSuiteScenario scenario, String dsmccData, String action) {
                        mTestSuiteScenario = scenario;
                        setDsmccData(dsmccData);
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
    public void onSessionReady(IOrbSession session) {
        mSession = session;
        mTestSuiteRunner.run();
    }

    /**
     * Get the current capabilities of the terminal.
     *
     * @return A Capabilities object.
     */
    @Override
    public BridgeTypes.Capabilities getCapabilities() {
        // TODO These are mock values, instead obtain from system/broadcast software.

        List<String> optionStrings = new ArrayList<>();
        //optionStrings.add("+PVR");
        optionStrings.add("+DRM");

        List<String> profileNameFragments = new ArrayList<>();
        profileNameFragments.add("+TRICKMODE"); // +ITV_KEYS is inherited from the base profile
        profileNameFragments.add("+DVB_T");
        profileNameFragments.add("+DVB_T2");
        profileNameFragments.add("+DVB_S");
        profileNameFragments.add("+DVB_S2");

        List<String> parentalSchemes = new ArrayList<>();
        parentalSchemes.add("dvb-si");

        List<String> graphicsLevels = new ArrayList<>();
        graphicsLevels.add("urn:hbbtv:graphics:performance:level1");
        graphicsLevels.add("urn:hbbtv:graphics:performance:level2");

        List<String> broadcastUrns = new ArrayList<>();
        broadcastUrns.add("urn:dvb:broadcast:ird:video:25_Hz_H.264_AVC_HDTV_IRD");
        broadcastUrns.add("urn:dvb:broadcast:ird:video:30_Hz_H.264_AVC_HDTV_IRD");
        broadcastUrns.add("urn:dvb:broadcast:ird:video:50_Hz_H.264_AVC_HDTV_IRD");
        broadcastUrns.add("urn:dvb:broadcast:ird:video:60_Hz_H.264_AVC_HDTV_IRD");
        broadcastUrns.add("urn:dvb:broadcast:ird:video:50_Hz_HEVC_HDTV_8-bit_IRD");
        broadcastUrns.add("urn:dvb:broadcast:ird:video:60_Hz_HEVC_HDTV_8-bit_IRD");
        //broadcastUrns.add("urn:dvb:broadcast:ird:video:50_Hz_HEVC_HDTV_10-bit_IRD");
        //broadcastUrns.add("urn:dvb:broadcast:ird:video:60_Hz_HEVC_HDTV_10-bit_IRD");
        //broadcastUrns.add("urn:dvb:broadcast:ird:video:HEVC_UHDTV_IRD");
        //broadcastUrns.add("urn:dvb:broadcast:ird:video:HEVC_HDR_UHDTV_IRD_using_HLG10");
        //broadcastUrns.add("urn:dvb:broadcast:ird:video:HEVC_HDR_UHDTV_IRD_using_PQ10");
        //broadcastUrns.add("urn:dvb:broadcast:ird:video:HEVC_HDR_HFR_UHDTV_IRD_using_HLG10");
        //broadcastUrns.add("urn:dvb:broadcast:ird:video:HEVC_HDR_HFR_UHDTV_IRD_using_PQ10");
        broadcastUrns.add("urn:dvb:broadcast:ird:audio:MPEG-1_and_MPEG-2_backwards_compatible");
        broadcastUrns.add("urn:dvb:broadcast:ird:audio:AC-3_and_enhanced_AC-3");
        broadcastUrns.add("urn:dvb:broadcast:ird:audio:MPEG-4_AAC_family");
        //broadcastUrns.add("urn:dvb:broadcast:ird:audio:DTS");
        //broadcastUrns.add("urn:dvb:broadcast:ird:audio:AC-4_channel_based");
        //broadcastUrns.add("urn:dvb:broadcast:ird:audio:AC-4_channel_based_immersive_personalized");
        //broadcastUrns.add("urn:dvb:broadcast:ird:audio:MPEG-H");

        String displaySizeWidth = "71"; // Mock 32" TV
        String displaySizeHeight = "40"; // Mock 32" TV
        String displaySizeMeasurementType = "built-in"; // hdmi-accurate, hdmi-other
        String audioOutputFormat = "stereo"; // multichannel, multichannel-preferred
        boolean passThroughStatus = false;
        String html5MediaVariableRateMin = "0.5";
        String html5MediaVariableRateMax = "5.0";

        return new BridgeTypes.Capabilities(
                optionStrings,
                profileNameFragments,
                broadcastUrns,
                parentalSchemes,
                graphicsLevels,
                displaySizeWidth,
                displaySizeHeight,
                displaySizeMeasurementType,
                audioOutputFormat,
                passThroughStatus,
                html5MediaVariableRateMin,
                html5MediaVariableRateMax
        );
    }

    /**
     * Get a list of audio profiles supported by the terminal, as defined by HBBTV 10.2.4.7 for
     * the audio_profile element.
     *
     * @return A list of audio profiles supported by the terminal.
     */
    @Override
    public List<BridgeTypes.AudioProfile> getAudioProfiles() {
        List<BridgeTypes.AudioProfile> profiles = new ArrayList<>();
        profiles.add(new BridgeTypes.AudioProfile(
                "MPEG1_L3",
                "audio/mpeg",
                null,
                null,
                null
        ));
        profiles.add(new BridgeTypes.AudioProfile(
                "HEAAC",
                "audio/mp4",
                null,
                null,
                null
        ));
        profiles.add(new BridgeTypes.AudioProfile(
                "MP4_HEAAC",
                "audio/mp4",
                "dash",
                "dash_pr",
                null
        ));

        // For terminals that support E-AC3 audio:
        profiles.add(new BridgeTypes.AudioProfile(
                "MP4_E-AC3",
                "audio/mp4",
                null,
                null,
                null
        ));
        profiles.add(new BridgeTypes.AudioProfile(
                "MP4_E-AC3",
                "audio/mp4",
                "dash",
                "dash_pr",
                null
        ));

        return profiles;
    }

    /**
     * Get a list of video profiles supported by the terminal, as defined by HBBTV 10.2.4.7 for
     * the video_profile element.
     *
     * @return A list of video profiles supported by the terminal.
     */
    @Override
    public List<BridgeTypes.VideoProfile> getVideoProfiles() {
        List<BridgeTypes.VideoProfile> profiles = new ArrayList<>();
        profiles.add(new BridgeTypes.VideoProfile(
                "MP4_AVC_SD_25_HEAAC",
                "video/mp4",
                "dash",
                "dash_pr",
                null,
                null
        ));
        profiles.add(new BridgeTypes.VideoProfile(
                "MP4_AVC_HD_25_HEAAC",
                "video/mp4",
                "dash",
                "dash_pr",
                null,
                null
        ));
        profiles.add(new BridgeTypes.VideoProfile(
                "MP4_AVC_SD_25_HEAAC_EBUTTD",
                "video/mp4",
                "dash",
                "dash_pr",
                null,
                null
        ));
        profiles.add(new BridgeTypes.VideoProfile(
                "MP4_AVC_HD_25_HEAAC_EBUTTD",
                "video/mp4",
                "dash",
                "dash_pr",
                null,
                null
        ));
        profiles.add(new BridgeTypes.VideoProfile(
                "TS_AVC_SD_25_HEAAC",
                "video/mpeg",
                null,
                "temi",
                null,
                null
        ));
        profiles.add(new BridgeTypes.VideoProfile(
                "TS_AVC_HD_25_HEAAC",
                "video/mpeg",
                null,
                "temi",
                null,
                null
        ));
        profiles.add(new BridgeTypes.VideoProfile(
                "MP4_AVC_SD_25_HEAAC",
                "video/mp4",
                null,
                null,
                null,
                null
        ));
        profiles.add(new BridgeTypes.VideoProfile(
                "MP4_AVC_HD_25_HEAAC",
                "video/mp4",
                null,
                null,
                null,
                null
        ));

        // For terminals that support E-AC3 audio:
        profiles.add(new BridgeTypes.VideoProfile(
                "TS_AVC_SD_25_E-AC3",
                "video/mpeg",
                null,
                "temi",
                null,
                null
        ));
        profiles.add(new BridgeTypes.VideoProfile(
                "TS_AVC_HD_25_E-AC3",
                "video/mpeg",
                null,
                "temi",
                null,
                null
        ));
        profiles.add(new BridgeTypes.VideoProfile(
                "MP4_AVC_SD_25_E-AC3",
                "video/mp4",
                null,
                null,
                null,
                null
        ));
        profiles.add(new BridgeTypes.VideoProfile(
                "MP4_AVC_HD_25_E-AC3",
                "video/mp4",
                null,
                null,
                null,
                null
        ));
        profiles.add(new BridgeTypes.VideoProfile(
                "MP4_AVC_SD_25_E-AC3",
                "video/mp4",
                "dash",
                "dash_pr",
                null,
                null
        ));
        profiles.add(new BridgeTypes.VideoProfile(
                "MP4_AVC_HD_25_E-AC3",
                "video/mp4",
                "dash",
                "dash_pr",
                null,
                null
        ));
        profiles.add(new BridgeTypes.VideoProfile(
                "MP4_AVC_SD_25_E-AC3_EBUTTD",
                "video/mp4",
                "dash",
                "dash_pr",
                null,
                null
        ));
        profiles.add(new BridgeTypes.VideoProfile(
                "MP4_AVC_HD_25_E-AC3_EBUTTD",
                "video/mp4",
                "dash",
                "dash_pr",
                null,
                null
        ));

        // TODO UHD

        return profiles;
    }

    /**
     * If the terminal supports UHD, get a list that describes the highest quality video format
     * the terminal supports, as defined by HBBTV 10.2.4.7 for the video_display_format element;
     * otherwise get an empty list.
     *
     * Note: If the terminal changes its display format based on the content being played,
     * multiple elements may be included in the list when multiple frame rate families are usable
     * or the highest resolution does not support each highest quality parameter.
     *
     * @return A list that describes the highest quality video format.
     */
    @Override
    public List<BridgeTypes.VideoDisplayFormat> getVideoDisplayFormats() {
        return new ArrayList<>(); // TODO UHD
    }

    /**
     * Get the current number of additional media streams containing SD video accompanied by audio
     * that can be decoded and presented by an A/V control object or HTML5 media element.
     *
     * @return The current number of additional media streams. If the value is non-zero, then a
     *    call to play an A/V control object, HTML5 media element or video/broadcast object shall
     *    not fail due to lack of resources for SD media.
     */
    @Override
    public int getExtraSDVideoDecodes() {
        return 1; // TODO
    }

    /**
     * Get the current number of additional media streams containing HD video accompanied by audio
     * that can be decoded and presented by an A/V control object or HTML5 media element.
     *
     * @return The current number of additional media streams. If the value is non-zero, then a
     *    call to play an A/V control object, HTML5 media element or video/broadcast object shall
     *    not fail due to lack of resources for HD media.
     */
    @Override
    public int getExtraHDVideoDecodes() {
        return 1; // TODO
    }

    /**
     * Get the current number of additional media streams containing UHD video accompanied by
     * audio that can be decoded and presented by an A/V control object or HTML5 media element.
     *
     * @return The current number of additional media streams. If the value is non-zero, then a
     *    call to play an A/V control object, HTML5 media element or video/broadcast object shall
     *    not fail due to lack of resources for UHD media.
     */
    @Override
    public int getExtraUHDVideoDecodes() {
        return 0; // TODO UHD
    }

    /**
     * Get immutable system information.
     *
     * @return Valid SystemInformation on success, otherwise invalid SystemInformation
     */
    @Override
    public BridgeTypes.SystemInformation getSystemInformation() {
        return new BridgeTypes.SystemInformation(
                true,
                "OBS",
                "Mock",
                "build version? ()",
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

        Vector<BridgeTypes.Channel> channels = mTestSuiteScenario.getMockChannels();
        if (channels != null) {
            for (BridgeTypes.Channel mockChannel : channels) {
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
    public List<BridgeTypes.Channel> getChannelList() {
        return mTestSuiteScenario.getMockChannels();
    }

    /**
     * Returns the CCID of the current channel
     *
     * @return A CCID on success, an empty string otherwise
     */
    @Override
    public String getCurrentCcid() {
        BridgeTypes.Channel currentChannel = mTestSuiteScenario.getCurrentChannel();
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
    public BridgeTypes.Channel getChannel(String ccid) {
        // Naive implementation for mock
        Vector<BridgeTypes.Channel> channels = mTestSuiteScenario.getMockChannels();
        if (channels != null) {
            for (BridgeTypes.Channel mockChannel : channels) {
                if (mockChannel.ccid.equals(ccid)) {
                    return mockChannel;
                }
            }
        }
        return new BridgeTypes.Channel();
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
     * @return negative value (e.g. BridgeTypes.CHANNEL_STATUS_CONNECTING) on success, or
     *         zero/positive value (see BridgeTypes.CHANNEL_STATUS_.. error values) on failure
     */
    @Override
    public int setChannelByTriplet(int idType, int onid, int tsid, int sid, int sourceID,
                                   String ipBroadcastID, boolean trickplay,
                                   String contentAccessDescriptorURL, int quiet) {
        if ((onid == -1) && (tsid == -1) && (sid == -1)) {
            return BridgeTypes.CHANNEL_STATUS_UNREALIZED;
        }
        if (idType == BridgeTypes.Channel.ID_IPTV_SDS ||
                idType == BridgeTypes.Channel.ID_DVB_SI_DIRECT) {
            // DSD and IPTV channels are not supported in mock implementation
            return BridgeTypes.CHANNEL_STATUS_NOT_SUPPORTED;
        }

        int status = BridgeTypes.CHANNEL_STATUS_UNKNOWN_CHANNEL;
        if (mTestSuiteScenario.selectChannel(onid, tsid, sid)) {
            status = BridgeTypes.CHANNEL_STATUS_CONNECTING;
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
     * @return negative value (e.g. BridgeTypes.CHANNEL_STATUS_CONNECTING) on success, or
     * zero/positive value (see BridgeTypes.CHANNEL_STATUS_.. error values) on failure
     */
    @Override
    public int setChannelByDsd(String dsd, int sid, boolean trickplay,
                               String contentAccessDescriptorURL, int quiet) {
        if ((dsd == null)) {
            return BridgeTypes.CHANNEL_STATUS_UNREALIZED;
        }

        // TODO Parse DSD
        int onid = 1;
        int tsid = 65283;
        int finalSid = 28186;

        int status = BridgeTypes.CHANNEL_STATUS_UNKNOWN_CHANNEL;
        if (mTestSuiteScenario.selectChannel(onid, tsid, sid)) {
            status = BridgeTypes.CHANNEL_STATUS_CONNECTING;
            simulateSuccessfulTune();
        } else {
            // TODO simulateUnsuccessfulTune();
        }

        return status;
    }

    /**
     * @param ccid                       The CCID of the channel to set.
     * @param trickplay                  True if the application has optionally hinted trickplay resources are
     *                                   required; or false otherwise. Does not affect the success of this operation.
     * @param contentAccessDescriptorURL Optionally, additional information for DRM-protected IPTV
     *                                   broadcasts; or an empty string otherwise.
     * @param quiet                      Type of channel change: 0 for normal; 1 for normal, no UI; 2 for quiet (HbbTV
     *                                   A.2.4.3.2).
     * @return A CHANNEL_STATUS_* code (on success, the code has a value less than 0).
     */
    @Override
    public int setChannelToCcid(String ccid, boolean trickplay, String contentAccessDescriptorURL, int quiet) {
        BridgeTypes.Channel ch = getChannel(ccid);
        return setChannelByTriplet(
                ch.idType,
                ch.onid,
                ch.tsid,
                ch.sid,
                ch.sourceId,
                ch.ipBroadcastId,
                trickplay,
                contentAccessDescriptorURL,
                quiet);
    }

    /**
     * Get the list of programmes available for a channel.
     *
     * @param ccid CCID for the required channel
     * @return List of programmes available for the channel
     */
    @Override
    public List<BridgeTypes.Programme> getProgrammeList(String ccid) {
        long time = System.currentTimeMillis() / 1000L - 1;
        Vector<BridgeTypes.Programme> programmes = mTestSuiteScenario.getCurrentChannelProgrammes();
        if (programmes != null) {
            for (BridgeTypes.Programme mockProgramme : programmes) {
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
    public List<BridgeTypes.Programme> getPresentFollowingProgrammes(String ccid) {
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
    public List<BridgeTypes.Component> getComponentList(String ccid, int typeCode) {
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
    public BridgeTypes.Component getPrivateAudioComponent(String componentTag) {
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
    public BridgeTypes.Component getPrivateVideoComponent(String componentTag) {
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
                            String identifier = OrbSessionFactory.createDistinctiveIdentifier("00001",
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
     * see BridgeTypes.DISTINCTIVE_IDENTIFIER_STATUS_*).
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
        return BridgeTypes.DISTINCTIVE_IDENTIFIER_STATUS_REQUEST_REQUIRED;
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
    public boolean startSearch(final BridgeTypes.Query q, int offset, int count, List<String> channelConstraints) {
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
     * Convert the Android key code to a TV Browser (BridgeTypes.VK_*) key code.
     *
     * @param androidKeyCode The Android key code (KeyEvent.KEYCODE_*)
     * @return The TV Browser (BridgeTypes.VK_*) key code.
     */
    @Override
    public int getTvBrowserKeyCode(int androidKeyCode) {
        return mKeyMap.get(androidKeyCode, BridgeTypes.VK_INVALID);
    }

    /**
     * Called when the active key set and optional other keys are changed.
     *
     * @param keySet Key set (a bitwise mask of constants, as defined by HbbTV).
     * @param otherKeys Optional other keys.
     */
    @Override
    public void onKeySetChanged(int keySet, int[] otherKeys) {
        // Not used
    }

    /**
     * Notify that the application status is changed.
     *
     * @param status The application status.
     */
    @Override
    public void onApplicationStatusChanged(ApplicationStatus status) {
        // Not used
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

    /**
     * Returns the actual volume level set.
     *
     * @return Integer value between 0 up to and including 100 to indicate volume level.
     */
    public int getVolume() {
        return mVolume;
    }

    /**
     * Adjusts the volume of the currently playing media to the volume as indicated by volume.
     *
     * @param volume Integer value between 0 up to and including 100 to indicate volume level.
     * @return true if the volume has changed. false if the volume has not changed.
     */
    public boolean setVolume(int volume) {
        if (mVolume == volume) {
            return false;
        }
        mVolume = volume;
        return true;
    }

    private void delaySelectComponent(final int componentType, final boolean restoreDefault,
                                      final String id) {
        new android.os.Handler(Looper.getMainLooper()).postDelayed(() -> {
            if (restoreDefault) {
                Vector<BridgeTypes.Component> components =
                        mTestSuiteScenario.getCurrentChannelComponents();
                if (components != null) {
                    // Restore to first in list
                    for (BridgeTypes.Component c : components) {
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
     * Get the list of supported DRM System IDs currently available. Once called,
     * the caller can track the availability changes by listening to
     * onDRMSystemStatusChange events. DRM System ID can enter the following states:
     *    - 0 READY, fully initialised and ready
     *    - 1 UNKNOWN, no longer available
     *    - 2 INITIALISING, initialising and not ready to communicate
     *    - 3 ERROR, in error state
     * @return List of supported DRM System IDs currently available.
     */
    @Override
    public List<BridgeTypes.DRMSystemStatus> getSupportedDRMSystemIDs() {
        ArrayList<BridgeTypes.DRMSystemStatus> result = new ArrayList<>();
        //String drmSystem, ArrayList<String> drmSystemIDs, int status,
        //String protectionGateways, String supportedFormats
      /*result.add(new BridgeTypes.DRMSystemStatus("test", null, 2, //INITIALISING
"ci+ dtcp-ip", "TS_BBTS TTS_BBTS MP4_PDCF"));
      Handler handler = new android.os.Handler(Looper.getMainLooper());
      handler.postDelayed(new Runnable() {
                             public void run() {
                                ArrayList<String> drmSystemIds1 = new ArrayList<>();
                                ArrayList<String> drmSystemIds2 = new ArrayList<>();
                                drmSystemIds1.add("urn:dvb:casystemid:4096");
                                drmSystemIds2.add("urn:dvb:casystemid:19188");
                                mSession.onDRMSystemStatusChange("test", drmSystemIds1,0 ,// READY
                                        "ci+ dtcp-ip", "TS_BBTS TTS_BBTS MP4_PDCF");
                                mSession.onDRMSystemStatusChange("urn:dvb:casystemid:19188", drmSystemIds2,0, // READY
                                        "ci+ dtcp-ip", "TS_BBTS TTS_BBTS MP4_PDCF");
                             }
                          },
              6000);*/

        return result;
    }

    /**
     * Checks the availability of a valid license for playing a protected content item.
     *
     * @param DRMPrivateData DRM proprietary private data
     * @param DRMSystemID ID of the DRM System
     * @return true if there is a valid license available that may allow playing the content
     */
    @Override
    public boolean canPlayContent(String DRMPrivateData, String DRMSystemID) {
        return true;
    }

    /**
     * Checks the availability of a valid license for recording a protected content item.
     *
     * @param DRMPrivateData DRM proprietary private data
     * @param DRMSystemID ID of the DRM System
     * @return true if there is a valid license available locally that may allow recording the content
     */
    @Override
    public boolean canRecordContent(String DRMPrivateData, String DRMSystemID) {
        return true;
    }

    /**
     * Send message to the DRM system.
     *
     * @param msgId unique ID to identify the message, to be passed as the 'msgID'
     *        argument for onDRMMessageResult
     * @param msgType message type as defined by the DRM system
     * @param msg message to be provided to the underlying DRM system
     * @param drmSystemID ID of the DRM System
     * @param block Whether the function needs to block until the reply is received
     */
    @Override
    public String sendDRMMessage(String msgId, String msgType, String msg, String drmSystemID, boolean block) {
      /*Handler handler = new android.os.Handler(Looper.getMainLooper());
      handler.postDelayed(new Runnable() {
                             public void run() {
                                mSession.onDRMMessageResult(msgId, msg + "ECHO", 0);
                             }
                          }, 3000);
      handler.postDelayed(new Runnable() {
                             public void run() {
                                mSession.onDRMSystemMessage(msg + "ECHO2", "test");
                             }
                          }, 6000);*/
        return null;
    }

    /**
     * Set the DRM system, that the terminal shall use for playing protected broadband content.
     *
     * @param DRMSystemID ID of the DRM System
     * @return false if the terminal is unable to set the specified DRM system as requested,
     *         true otherwise
     */
    @Override
    public boolean setActiveDRM(String DRMSystemID) {
        return true;
    }

    /**
     * Request file from DSM-CC
     *
     * @param url DVB Url of requested file
     * @param requestId ID of request (returned to DsmccClient.receiveContent)
     */
    @Override
    public boolean requestDsmccDvbContent(String url, int requestId) {
        if (mDsmPath.isEmpty()) {
            return false;
        }
        Uri uri = Uri.parse(url);
        String path = mDsmPath + "/" + uri.getAuthority() + uri.getPath();
        Log.d(TAG, "Get mock content: " + path);
        File file = new File(path);
        if (file.exists()) {
            if (file.isDirectory()) {
                StringBuilder sb = new StringBuilder();
                File[] files = file.listFiles();
                if (files != null) {
                    for (File child : files) {
                        if (sb.length() != 0) sb.append(",");
                        sb.append(child.getName());
                    }
                }
                mSession.onDsmccReceiveContent(requestId, ByteBuffer.wrap(sb.toString().getBytes()));
                return true;
            } else {
                Path p = Paths.get(path);
                try {
                    FileChannel channel = FileChannel.open(p, StandardOpenOption.READ);
                    ByteBuffer buffer = ByteBuffer.allocate((int) channel.size());
                    channel.read(buffer);
                    buffer.flip();
                    channel.close();
                    mSession.onDsmccReceiveContent(requestId, buffer);
                    return true;
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }
        return false;
    }

    /**
     * Release resources for DSM-CC file request
     *
     * @param requestId ID of request
     */
    @Override
    public void closeDsmccDvbContent(int requestId) {

    }

    /**
     * Subscribe to DSM-CC Stream Event with URL and event name
     *
     * @param url DVB Url of event object
     * @param name Name of stream event
     * @param listenId ID of subscriber
     */
    @Override
    public boolean subscribeDsmccStreamEventName(String url, String name, int listenId) {
        Handler handler = sendStreamEvents(url, name, listenId, -1);
        mActiveSubscriptionList.put(listenId, handler);
        return true;
    }

    /**
     * Subscribe to DSM-CC Stream Event with component tag and event ID
     *
     * @param name Name of stream event
     * @param componentTag Component tag for stream event
     * @param eventId Event Id of stream event
     * @param listenId ID of subscriber
     */
    @Override
    public boolean subscribeDsmccStreamEventId(String name, int componentTag, int eventId, int listenId) {
        Handler handler = sendStreamEvents("url", name, listenId, componentTag);
        mActiveSubscriptionList.put(listenId, handler);
        return true;
    }

    /**
     * Subscribe to DSM-CC Stream Event with component tag and event ID
     *
     * @param listenId ID of subscriber
     */
    @Override
    public void unsubscribeDsmccStreamEvent(int listenId) {
        Handler handler = mActiveSubscriptionList.get(listenId);
        if (handler != null) {
            handler.removeCallbacksAndMessages(null);
        }
        mActiveSubscriptionList.remove(listenId);
    }

    /**
     * Publish a test report.
     *
     * @param testSuite A unique test suite name.
     * @param xml The XML test report.
     */
    @Override
    public void publishTestReport(String testSuite, String xml) {
        mTestSuiteRunner.onTestReportPublished(testSuite, xml);
    }

    /**
     * Show the soft keyboard.
     */
    @Override
    public void showSoftKeyboard() { }

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
        BridgeTypes.Channel channel = mTestSuiteScenario.getCurrentChannel();
        if (channel != null) {
            mSession.onChannelStatusChanged(channel.onid, channel.tsid, channel.sid,
                    BridgeTypes.CHANNEL_STATUS_CONNECTING, false);
            new android.os.Handler(Looper.getMainLooper()).postDelayed(() -> {
                mSession.onChannelStatusChanged(channel.onid, channel.tsid, channel.sid,
                        BridgeTypes.CHANNEL_STATUS_PRESENTING, false);
                mSession.processAitSection(506, channel.sid, mTestSuiteScenario.getCurrentChannelAit());
            }, 50);
        } else {
            // TODO TUNED OFF
        }
    }

    private Handler sendStreamEvents(String url, String name, int listenId, int componentTag) {
        Handler handler = new android.os.Handler(Looper.getMainLooper());
        handler.postDelayed(new Runnable() {
            private int count = 0;

            public void run() {
                Handler handler = mActiveSubscriptionList.get(listenId);
                if (handler != null) {
                    String data, text, status;
                    if (!url.endsWith("/xxx") && (componentTag != 29)) {
                        status = "trigger";
                        switch (count) {
                            case 0:
                                data = "48656c6c6f204862625456";
                                text = "Hello HbbTV";
                                break;
                            case 1:
                                data = "54657374206576656e7420c3a4c3b6c3bc21";
                                text = "Test event äöü!";
                                break;
                            case 2:
                            default:
                                data = "cafebabe0008090a0d101fff";
                                text = "\n\r";
                                break;
                        }
                        count++;
                        if (count != 3) {
                            handler.postDelayed(this, 2000);
                        }
                    } else {
                        data = "";
                        text = "";
                        status = "error";
                    }
                    mSession.onDsmccReceiveStreamEvent(listenId, name, data, text, status, null);
                }
            }
        }, 2000);
        return handler;
    }

    private void setDsmccData(String dsmccData) {
        if (dsmccData.isEmpty()) {
            return;
        }
        mDsmPath = unpackMockDsmcc(mContext, dsmccData);
        Log.d(TAG, "Using path: " + mDsmPath);
    }

    private String unpackMockDsmcc(Context context, String dsmccData) {
        String path = mBasePath + dsmccData.replace("/", "_") + "/";
        File dsmdir = new File(path);
        if (!dsmdir.exists()) {
            Log.i(TAG, "Unpacking Mock Dsmcc to: " + path);
            try {
                InputStream is = context.getAssets().open("tests/" + dsmccData);
                ZipInputStream zis = new ZipInputStream(new BufferedInputStream(is));
                String filename;
                ZipEntry ze;
                byte[] buffer = new byte[1024];
                int count;
                Log.i(TAG, "unpackMockDsmcc zip avail: " + zis.available());

                while ((ze = zis.getNextEntry()) != null) {
                    filename = ze.getName();
                    Log.i(TAG, "Dsmcc file: " + filename);
                    if (ze.isDirectory()) {
                        File fmd = new File(path + filename);
                        if (!fmd.mkdirs()) {
                            Log.e(TAG, "unpackMockDsmcc mkdirs FAILED: " + path + filename);
                            break;
                        }
                    } else {
                        FileOutputStream fout = new FileOutputStream(path + filename);
                        while ((count = zis.read(buffer)) != -1) {
                            fout.write(buffer, 0, count);
                        }
                        fout.close();
                        zis.closeEntry();
                    }
                }
                zis.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
        return dsmdir.getPath();
    }

    static {
        mKeyMap = new SparseArray<Integer>();
        mKeyMap.put(KeyEvent.KEYCODE_PROG_RED, BridgeTypes.VK_RED);
        mKeyMap.put(KeyEvent.KEYCODE_PROG_GREEN, BridgeTypes.VK_GREEN);
        mKeyMap.put(KeyEvent.KEYCODE_PROG_YELLOW, BridgeTypes.VK_YELLOW);
        mKeyMap.put(KeyEvent.KEYCODE_PROG_BLUE, BridgeTypes.VK_BLUE);
        mKeyMap.put(KeyEvent.KEYCODE_DPAD_UP, BridgeTypes.VK_UP);
        mKeyMap.put(KeyEvent.KEYCODE_DPAD_DOWN, BridgeTypes.VK_DOWN);
        mKeyMap.put(KeyEvent.KEYCODE_DPAD_LEFT, BridgeTypes.VK_LEFT);
        mKeyMap.put(KeyEvent.KEYCODE_DPAD_RIGHT, BridgeTypes.VK_RIGHT);
        mKeyMap.put(KeyEvent.KEYCODE_ENTER, BridgeTypes.VK_ENTER);
        mKeyMap.put(KeyEvent.KEYCODE_DPAD_CENTER, BridgeTypes.VK_ENTER);
        mKeyMap.put(KeyEvent.KEYCODE_DEL, BridgeTypes.VK_BACK);
        mKeyMap.put(KeyEvent.KEYCODE_0, BridgeTypes.VK_0);
        mKeyMap.put(KeyEvent.KEYCODE_1, BridgeTypes.VK_1);
        mKeyMap.put(KeyEvent.KEYCODE_2, BridgeTypes.VK_2);
        mKeyMap.put(KeyEvent.KEYCODE_3, BridgeTypes.VK_3);
        mKeyMap.put(KeyEvent.KEYCODE_4, BridgeTypes.VK_4);
        mKeyMap.put(KeyEvent.KEYCODE_5, BridgeTypes.VK_5);
        mKeyMap.put(KeyEvent.KEYCODE_6, BridgeTypes.VK_6);
        mKeyMap.put(KeyEvent.KEYCODE_7, BridgeTypes.VK_7);
        mKeyMap.put(KeyEvent.KEYCODE_8, BridgeTypes.VK_8);
        mKeyMap.put(KeyEvent.KEYCODE_9, BridgeTypes.VK_9);
        mKeyMap.put(KeyEvent.KEYCODE_MEDIA_STOP, BridgeTypes.VK_STOP);
        mKeyMap.put(KeyEvent.KEYCODE_MEDIA_PLAY, BridgeTypes.VK_PLAY);
        mKeyMap.put(KeyEvent.KEYCODE_MEDIA_PAUSE, BridgeTypes.VK_PAUSE);
        mKeyMap.put(KeyEvent.KEYCODE_MEDIA_FAST_FORWARD, BridgeTypes.VK_FAST_FWD);
        mKeyMap.put(KeyEvent.KEYCODE_MEDIA_REWIND, BridgeTypes.VK_REWIND);
        mKeyMap.put(KeyEvent.KEYCODE_TV_TELETEXT, BridgeTypes.VK_TELETEXT);

        // Standard keyboard aliases for development
        mKeyMap.put(KeyEvent.KEYCODE_R, BridgeTypes.VK_RED);
        mKeyMap.put(KeyEvent.KEYCODE_G, BridgeTypes.VK_GREEN);
        mKeyMap.put(KeyEvent.KEYCODE_Y, BridgeTypes.VK_YELLOW);
        mKeyMap.put(KeyEvent.KEYCODE_B, BridgeTypes.VK_BLUE);
        mKeyMap.put(KeyEvent.KEYCODE_S, BridgeTypes.VK_STOP);
        mKeyMap.put(KeyEvent.KEYCODE_P, BridgeTypes.VK_PLAY);
        mKeyMap.put(KeyEvent.KEYCODE_Z, BridgeTypes.VK_PAUSE);
        mKeyMap.put(KeyEvent.KEYCODE_F, BridgeTypes.VK_FAST_FWD);
        mKeyMap.put(KeyEvent.KEYCODE_X, BridgeTypes.VK_REWIND);
        mKeyMap.put(KeyEvent.KEYCODE_T, BridgeTypes.VK_TELETEXT);
    }
}
