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

package org.orbtv.orbpolyfill;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.util.ArrayList;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class BridgeTypes {
    public interface JSONSerializable {
        JSONObject toJSONObject() throws JSONException;
    }

    // Codes used to indicate the status of a channel.
    public static final int CHANNEL_STATUS_UNREALIZED = -4;
    public static final int CHANNEL_STATUS_PRESENTING = -3;
    public static final int CHANNEL_STATUS_CONNECTING = -2;
    public static final int CHANNEL_STATUS_RECOVERING = -1;
    // Channel change errors - See OIPF DAE spec section 7.13.1.2 onChannelChangeError table
    public static final int CHANNEL_STATUS_NOT_SUPPORTED = 0;
    public static final int CHANNEL_STATUS_NO_SIGNAL = 1;
    public static final int CHANNEL_STATUS_TUNER_IN_USE = 2;
    public static final int CHANNEL_STATUS_PARENTAL_LOCKED = 3;
    public static final int CHANNEL_STATUS_ENCRYPTED = 4;
    public static final int CHANNEL_STATUS_UNKNOWN_CHANNEL = 5;
    public static final int CHANNEL_STATUS_INTERRUPTED = 6;
    public static final int CHANNEL_STATUS_RECORDING_IN_PROGRESS = 7;
    public static final int CHANNEL_STATUS_CANNOT_RESOLVE_URI = 8;
    public static final int CHANNEL_STATUS_INSUFFICIENT_BANDWIDTH = 9;
    public static final int CHANNEL_STATUS_CANNOT_BE_CHANGED = 10;
    public static final int CHANNEL_STATUS_INSUFFICIENT_RESOURCES = 11;
    public static final int CHANNEL_STATUS_CHANNEL_NOT_IN_TS = 12;
    public static final int CHANNEL_STATUS_UNKNOWN_ERROR = 100;

    // Codes used to indicate the video aspect ratio.
    public static final int ASPECT_RATIO_4_3 = 0;
    public static final int ASPECT_RATIO_16_9 = 1;

    // Codes used to indicate component types.
    public static final int COMPONENT_TYPE_ANY = -1;
    public static final int COMPONENT_TYPE_VIDEO = 0;
    public static final int COMPONENT_TYPE_AUDIO = 1;
    public static final int COMPONENT_TYPE_SUBTITLE = 2;

    // Codes used to indicate the type of sub-window to use for the overlay.
    public static final int OVERLAY_TYPE_PANEL = 1; // Above the attached window. Used by the dummy app
    public static final int OVERLAY_TYPE_MEDIA_OVERLAY = 2; // Above the media window. Used with TV Input Framework

    // Codes used to indicate a key press.
    public static final int VK_INVALID = 0;
    public static final int VK_RED = 403;
    public static final int VK_GREEN = 404;
    public static final int VK_YELLOW = 405;
    public static final int VK_BLUE = 406;
    public static final int VK_LEFT = 37;
    public static final int VK_UP = 38;
    public static final int VK_RIGHT = 39;
    public static final int VK_DOWN = 40;
    public static final int VK_ENTER = 13;
    public static final int VK_BACK = 461;
    public static final int VK_0 = 48;
    public static final int VK_1 = 49;
    public static final int VK_2 = 50;
    public static final int VK_3 = 51;
    public static final int VK_4 = 52;
    public static final int VK_5 = 53;
    public static final int VK_6 = 54;
    public static final int VK_7 = 55;
    public static final int VK_8 = 56;
    public static final int VK_9 = 57;
    public static final int VK_STOP = 413;
    public static final int VK_PLAY = 415;
    public static final int VK_PAUSE = 19;
    public static final int VK_PLAY_PAUSE = 402;
    public static final int VK_FAST_FWD = 417;
    public static final int VK_REWIND = 412;
    public static final int VK_RECORD = 416;
    public static final int VK_TELETEXT = 459;

    // Codes used to indicate search status. See OIPF spec vol 5 DAE, section 7.12.1.1
    public static final int SEARCH_STATUS_COMPLETED = 0;
    public static final int SEARCH_STATUS_ABORTED = 3;
    public static final int SEARCH_STATUS_NO_RESOURCE = 4;

    // Status strings used for distinctive identifier
    public static final String DISTINCTIVE_IDENTIFIER_STATUS_REQUEST_REQUIRED = "#1";

    public static class Channel implements JSONSerializable {
        public boolean valid;
        public String ccid;
        public String name;
        public String dsd;
        public String ipBroadcastId;
        public int channelType;
        public int idType;
        public int majorChannel;
        public int terminalChannel;
        public int nid;
        public int onid;
        public int tsid;
        public int sid;
        public boolean hidden;
        public int sourceId;
        public List<Channel> serviceInstances;
        public int currentInstanceIndex;

        // channelType and idType values. See OIPF spec vol 5 DAE, section 7.13.11.1
        public static final int TYPE_TV = 0;
        public static final int TYPE_RADIO = 1;
        public static final int TYPE_OTHER = 2;
        public static final int TYPE_ALL = 128;
        public static final int TYPE_DATA = 256;
        public static final int ID_ANALOG = 0;
        public static final int ID_DVB_C = 10;
        public static final int ID_DVB_S = 11;
        public static final int ID_DVB_T = 12;
        public static final int ID_DVB_SI_DIRECT = 13;
        public static final int ID_DVB_C2 = 14;
        public static final int ID_DVB_S2 = 15;
        public static final int ID_DVB_T2 = 16;
        public static final int ID_DVB_I = 50;
        public static final int ID_DVB_DASH = 51;
        public static final int ID_ISDB_C = 20;
        public static final int ID_ISDB_S = 21;
        public static final int ID_ISDB_T = 22;
        public static final int ID_ATSC_T = 30;
        public static final int ID_IPTV_SDS = 40;
        public static final int ID_IPTV_URI = 41;

        public Channel() {
            this.valid = false;
        } // TODO(library) Remove valid and support returning null

        /**
         * Channel data structure.
         *
         * @param valid
         * @param ccid
         * @param name
         * @param dsd
         * @param channelType
         * @param idType
         * @param majorChannel
         * @param terminalChannel
         * @param nid
         * @param onid
         * @param tsid
         * @param sid
         * @param hidden
         * @param sourceId
         * @param ipBroadcastId
         */
        public Channel(boolean valid, String ccid, String name, String dsd, int channelType,
                       int idType, int majorChannel, int terminalChannel, int nid, int onid, int tsid, int sid,
                       boolean hidden, int sourceId, String ipBroadcastId, List<Channel> serviceInstances,
                       int currentInstanceIndex) {
            this.valid = valid;
            this.ccid = ccid;
            this.name = name;
            this.dsd = dsd;
            this.channelType = channelType;
            this.idType = idType;
            this.majorChannel = majorChannel;
            this.terminalChannel = terminalChannel;
            this.nid = nid;
            this.onid = onid;
            this.tsid = tsid;
            this.sid = sid;
            this.hidden = hidden;
            this.sourceId = sourceId;
            this.ipBroadcastId = ipBroadcastId;
            this.serviceInstances = serviceInstances;
            this.currentInstanceIndex = currentInstanceIndex;
        }

        /**
         * Get the data structure as a JSONObject.
         *
         * @return The data structure as a JSONObject.
         * @throws JSONException
         */
        public JSONObject toJSONObject() throws JSONException {
            JSONObject o = new JSONObject();
            o.put("valid", valid);
            o.put("channelType", channelType);
            o.put("idType", idType);
            o.put("ccid", ccid);
            if (dsd != null && !dsd.isEmpty()) {
                o.put("dsd", dsd);
            }
            o.put("onid", onid);
            if (nid != 0) {
                o.put("nid", nid);
            }
            o.put("tsid", tsid);
            o.put("sid", sid);
            o.put("name", name);
            o.put("majorChannel", majorChannel);
            if (terminalChannel != -1) {
                o.put("terminalChannel", terminalChannel);
            }
            o.put("hidden", hidden);
            o.put("sourceID", sourceId);
            o.put("ipBroadcastID", ipBroadcastId);
            if (serviceInstances != null) {
                JSONArray instances = new JSONArray();
                for (int i = 0; i < serviceInstances.size(); ++i) {
                    instances.put(i, serviceInstances.get(i).toJSONObject());
                }
                o.put("serviceInstances", instances);
                if (currentInstanceIndex >= 0 && currentInstanceIndex < serviceInstances.size()) {
                    o.put("currentInstanceIndex", currentInstanceIndex);
                }
            }
            return o;
        }
    }

    public static class Component implements JSONSerializable {
        public int componentType;
        public String id; // Platform-defined ID that is usable with overrideComponentSelection
        public int componentTag;
        public int pid;
        public String encoding;
        public int aspectRatio; // Video - ASPECT_RATIO_...
        public String language; // Audio and subtitle
        public boolean audioDescription; // Audio
        public int audioChannels; // Audio
        public boolean hearingImpaired; // Subtitle
        public boolean encrypted;
        public boolean active;
        public boolean hidden;
        public String label; // Subtitle

        private Component() {
        }

        /**
         * Create Component data structure for video.
         *
         * @param componentTag
         * @param pid
         * @param encoding
         * @param encrypted
         * @param aspectRatio ASPECT_RATIO_4_3 or ASPECT_RATIO_16_9
         * @param active
         * @return
         */
        public static Component createVideoComponent(String id, int componentTag, int pid,
                                                     String encoding, boolean encrypted, int aspectRatio, boolean active) {
            Component c = new Component();
            c.componentType = COMPONENT_TYPE_VIDEO;
            c.id = id;
            c.componentTag = componentTag;
            c.pid = pid;
            c.encoding = encoding;
            c.encrypted = encrypted;
            c.aspectRatio = aspectRatio;
            c.active = active;
            c.hidden = false;
            return c;
        }

        /**
         * Create Component data structure for audio.
         *
         * @param componentTag
         * @param pid
         * @param encoding
         * @param encrypted
         * @param language
         * @param audioDescription
         * @param audioChannels
         * @param active
         * @return
         */
        public static Component createAudioComponent(String id, int componentTag, int pid,
                                                     String encoding, boolean encrypted, String language, boolean audioDescription,
                                                     int audioChannels, boolean active) {
            Component c = new Component();
            c.componentType = COMPONENT_TYPE_AUDIO;
            c.id = id;
            c.componentTag = componentTag;
            c.pid = pid;
            c.encoding = encoding;
            c.encrypted = encrypted;
            c.language = language;
            c.audioDescription = audioDescription;
            c.audioChannels = audioChannels;
            c.active = active;
            c.hidden = false;
            return c;
        }

        /**
         * Create Component data structure for subtitles.
         *
         * @param componentTag
         * @param pid
         * @param encoding
         * @param encrypted
         * @param language
         * @param hearingImpaired
         * @param label
         * @param active
         * @return
         */
        public static Component createSubtitleComponent(String id, int componentTag, int pid,
                                                        String encoding, boolean encrypted, String language, boolean hearingImpaired, String label,
                                                        boolean active) {
            Component c = new Component();
            c.componentType = COMPONENT_TYPE_SUBTITLE;
            c.id = id;
            c.componentTag = componentTag;
            c.pid = pid;
            c.encoding = encoding;
            c.encrypted = encrypted;
            c.language = language;
            c.hearingImpaired = hearingImpaired;
            c.label = label;
            c.active = active;
            c.hidden = false;
            return c;
        }

        /**
         * Get the data structure as a JSONObject.
         *
         * @return The data structure as a JSONObject.
         * @throws JSONException
         */
        public JSONObject toJSONObject() throws JSONException {
            JSONObject o = new JSONObject();
            o.put("type", componentType);
            o.put("id", id);
            o.put("componentTag", componentTag);
            o.put("pid", pid);
            o.put("encoding", encoding);
            o.put("encrypted", encrypted);
            switch (componentType) {
                case COMPONENT_TYPE_VIDEO: {
                    o.put("aspectRatio", aspectRatio);
                    break;
                }
                case COMPONENT_TYPE_AUDIO: {
                    o.put("language", language);
                    o.put("audioDescription", audioDescription);
                    o.put("audioChannels", audioChannels);
                    break;
                }
                case COMPONENT_TYPE_SUBTITLE: {
                    o.put("language", language);
                    o.put("hearingImpaired", hearingImpaired);
                    o.put("label", label);
                    break;
                }
                default: {
                    // Ignore
                }
            }
            o.put("active", active);
            if (hidden) {
                o.put("hidden", true);
            }
            return o;
        }
    }

    public static class ParentalRating implements JSONSerializable {
        public String name;
        public String scheme;
        public int value;
        public int labels;
        public String region;

        /**
         * ParentalRating data structure.
         *
         * @param name
         * @param scheme
         * @param value
         * @param labels
         * @param region
         */
        public ParentalRating(String name, String scheme, int value, int labels, String region) {
            this.name = name;
            this.scheme = scheme;
            this.value = value;
            this.labels = labels;
            this.region = region;
        }

        /**
         * Get the data structure as a JSONObject.
         *
         * @return The data structure as a JSONObject.
         * @throws JSONException
         */
        public JSONObject toJSONObject() throws JSONException {
            JSONObject o = new JSONObject();
            o.put("name", name);
            o.put("scheme", scheme);
            o.put("value", value);
            o.put("labels", labels);
            o.put("region", region);
            return o;
        }
    }

    public static class Programme implements JSONSerializable {
        public String programmeId;
        public int programmeIdType;
        public String name;
        public String description;
        public String longDescription;
        public long startTime;
        public long duration;
        public String channelId;
        public List<ParentalRating> parentalRatings;

        // programmeIdType values according to OIPF spec vol 5 DAE, section 7.10.2.1
        public static final int ID_TVA_CRID = 0;
        public static final int ID_DVB_EVENT = 1;
        public static final int ID_TVA_GROUP_CRID = 2;

        /**
         * Programme data structure.
         *
         * @param name
         * @param programmeId
         * @param programmeIdType
         * @param description
         * @param longDescription
         * @param startTime
         * @param duration
         * @param channelId
         * @param parentalRatings
         */
        public Programme(String name, String programmeId, int programmeIdType, String description,
                         String longDescription, long startTime, long duration, String channelId,
                         List<ParentalRating> parentalRatings) {
            this.name = name;
            this.programmeId = programmeId;
            this.programmeIdType = programmeIdType;
            this.description = description;
            this.longDescription = longDescription;
            this.startTime = startTime;
            this.duration = duration;
            this.channelId = channelId;
            this.parentalRatings = parentalRatings;
        }

        /**
         * Get the data structure as a JSONObject.
         *
         * @return The data structure as a JSONObject.
         * @throws JSONException
         */
        public JSONObject toJSONObject() throws JSONException {
            JSONObject o = new JSONObject();
            o.put("programmeID", programmeId);
            o.put("programmeIDType", programmeIdType);
            o.put("name", name);
            o.put("description", description);
            o.put("longDescription", longDescription);
            o.put("startTime", startTime);
            o.put("duration", duration);
            o.put("channelID", channelId);
            JSONArray a = new JSONArray();
            for (ParentalRating element : parentalRatings) {
                a.put(element.toJSONObject());
            }
            o.put("parentalRatings", a);
            return o;
        }
    }

    public static class SystemInformation implements JSONSerializable {
        public boolean valid;
        public String vendorName;
        public String modelName;
        public String softwareVersion;
        public String hardwareVersion;

        /**
         * SystemInformation data structure.
         *
         * @param valid
         * @param vendorName
         * @param modelName
         * @param softwareVersion
         * @param hardwareVersion
         */
        public SystemInformation(boolean valid, String vendorName, String modelName,
                                 String softwareVersion, String hardwareVersion) {
            this.valid = valid;
            this.vendorName = vendorName;
            this.modelName = modelName;
            this.softwareVersion = softwareVersion;
            this.hardwareVersion = hardwareVersion;
        }

        /**
         * Get the data structure as a JSONObject.
         *
         * @return The data structure as a JSONObject.
         * @throws JSONException
         */
        public JSONObject toJSONObject() throws JSONException {
            JSONObject o = new JSONObject();
            o.put("valid", valid);
            o.put("vendorName", vendorName);
            o.put("modelName", modelName);
            o.put("softwareVersion", softwareVersion);
            o.put("hardwareVersion", hardwareVersion);
            return o;
        }
    }

    // TODO(library) Can this be simplified or is not really a type - create TvBrowserQuery class?
    public static class Query {
        private static final String TAG = "Query";

        private int mQueryId;

        // Operation values
        public static final int OP_ID = 0;
        public static final int OP_AND = 1;
        public static final int OP_OR = 2;
        public static final int OP_NOT = 3;

        // Comparison values
        public static final int CMP_EQUAL = 0;
        public static final int CMP_NOT_EQL = 1;
        public static final int CMP_MORE = 2;
        public static final int CMP_MORE_EQL = 3;
        public static final int CMP_LESS = 4;
        public static final int CMP_LESS_EQL = 5;
        public static final int CMP_CONTAINS = 6; // String - case insensitive matches

        private int mOperation;
        private Query mOperator1;
        private Query mOperator2;

        private String mField;
        private int mComparison;
        private String mValue;

        void commonQuery(JSONObject in) {
            try {
                mComparison = -1;
                mQueryId = (in.has("queryId")) ? in.getInt("queryId") : -1;
                if (in.has("operation")) {
                    switch (in.getString("operation")) {
                        case "IDENTITY":
                            mOperation = OP_ID;
                            break;
                        case "AND":
                            mOperation = OP_AND;
                            break;
                        case "OR":
                            mOperation = OP_OR;
                            break;
                        case "NOT":
                            mOperation = OP_NOT;
                            break;
                    }
                    JSONArray arguments = in.getJSONArray("arguments");
                    if (arguments.length() > 0) {
                        mOperator1 = new Query((JSONObject) arguments.get(0));
                        if (arguments.length() > 1) {
                            mOperator2 = new Query((JSONObject) arguments.get(1));
                        }
                    }
                } else {
                    mOperation = OP_ID;
                }
                if (mOperation == OP_ID) {
                    mField = in.getString("field");
                    mComparison = in.getInt("comparison");
                    mValue = in.getString("value");
                }
            } catch (JSONException e) {
                e.printStackTrace();
            }
        }

        public Query(JSONObject in) {
            commonQuery(in);
        }

        public Query(String query) {
            JSONObject in;
            try {
                in = new JSONObject(query);
                commonQuery(in);
            } catch (JSONException e) {
                e.printStackTrace();
            }
        }

        public Query and(Query operator2) {
            mOperation = OP_AND;
            mOperator2 = operator2;
            return this;
        }

        public Query or(Query operator2) {
            mOperation = OP_OR;
            mOperator2 = operator2;
            return this;
        }

        public Query not() {
            mOperation = OP_NOT;
            return this;
        }

        public int describeContents() {
            return 0;
        }

        public int getQueryId() {
            return mQueryId;
        }

        public int getOperation() {
            return mOperation;
        }

        public Query getOperator1() {
            return mOperator1;
        }

        public Query getOperator2() {
            return mOperator2;
        }

        public String getField() {
            return mField;
        }

        public int getComparison() {
            return mComparison;
        }

        public String getValue() {
            return mValue;
        }

        /**
         * @return String describing the query object
         */
        public String toString() {
            String result = "";
            if (mQueryId != -1) {
                result += "Query_" + mQueryId + " ";
            }
            result += "(";
            switch (mOperation) {
                case OP_AND: {
                    if (mOperator1 != null) {
                        result += mOperator1.toString();
                    }
                    result += ".AND.";
                    if (mOperator2 != null) {
                        result += mOperator2.toString();
                    }
                    break;
                }
                case OP_OR: {
                    if (mOperator1 != null) {
                        result += mOperator1.toString();
                    }
                    result += ".OR.";
                    if (mOperator2 != null) {
                        result += mOperator2.toString();
                    }
                    break;
                }
                case OP_NOT: {
                    result += " NOT.";
                    if (mOperator1 != null) {
                        result += mOperator1.toString();
                    }
                    break;
                }
                case OP_ID: {
                    result += mField + getCompareString() + "'" + mValue + "'";
                    break;
                }
            }
            result += ")";
            return result;
        }

        private String getCompareString() {
            switch (mComparison) {
                case CMP_EQUAL: {
                    return " == ";
                }
                case CMP_NOT_EQL: {
                    return " != ";
                }
                case CMP_MORE: {
                    return " > ";
                }
                case CMP_MORE_EQL: {
                    return " >= ";
                }
                case CMP_LESS: {
                    return " < ";
                }
                case CMP_LESS_EQL: {
                    return " <= ";
                }
                case CMP_CONTAINS: {
                    return " Ct ";
                }
            }
            return "";
        }
    }

    public static class ParentalRatingScheme implements JSONSerializable {
        public String name;
        public List<ParentalRating> ratings;

        public ParentalRatingScheme(String name, List<ParentalRating> ratings) {
            this.name = name;
            this.ratings = ratings;
        }

        @Override
        public JSONObject toJSONObject() throws JSONException {
            JSONObject o = new JSONObject();
            o.put("name", name);
            o.put("ratings", toJSONArray(ratings));
            return o;
        }
    }

    public static class Timeline implements JSONSerializable {
        public String timelineSelector;
        public Long unitsPerSecond;

        public Timeline(String timelineSelector) throws JSONException {
            this.unitsPerSecond = null;
            if (timelineSelector != null) {
                Pattern pattern = Pattern.compile("(?<=:timeline:).*");
                Matcher matcher = pattern.matcher(timelineSelector);
                if (matcher.find()) {
                    String[] selectorParts = matcher.group().split(":");
                    if (selectorParts.length > 0) {
                        this.timelineSelector = timelineSelector;
                        switch (selectorParts[0]) {
                            case "temi":
                                return;
                            case "html-media-timeline":
                                this.unitsPerSecond = 1000L;
                                return;
                            case "pts":
                                this.unitsPerSecond = 90000L;
                                return;
                            case "mpd":
                                if (selectorParts.length > 3) {
                                    unitsPerSecond = Long.parseLong(selectorParts[3]);
                                } else {
                                    unitsPerSecond = 1000L;
                                }
                                return;
                            default:
                                break;
                        }
                    }
                }
            }
            throw new JSONException("Invalid timeline selector!!!");
        }

        @Override
        public JSONObject toJSONObject() throws JSONException {
            JSONObject o = new JSONObject();
            JSONObject props = new JSONObject();
            o.put("timelineSelector", timelineSelector);
            props.put("unitsPerSecond", unitsPerSecond);
            props.put("unitsPerTick", 1);
            o.put("timelineProperties", props);
            return o;
        }
    }

    public static class DRMSystemStatus implements JSONSerializable {
        private static final String TAG = "DRMSystemStatus";

        public String drmSystem;
        public ArrayList<String> drmSystemIDs;
        public int status;
        public String protectionGateways;
        public String supportedFormats;

        /**
         * DRMSystemStatus data structure.
         *
         * @param drmSystemID ID of the DRM System
         * @param status status status of the indicated DRM system. Possible states:
         *              - 0 READY, fully initialised and ready
         *              - 1 UNKNOWN, no longer available
         *              - 2 INITIALISING, initialising and not ready to communicate
         *              - 3 ERROR, in error state
         * @param protectionGateways space separated list of zero or more CSP Gateway types that are
         *                           capable of supporting the DRM system
         * @param supportedFormats space separated list of zero or more supported file and/or
         *                         container formats by the DRM system
         */
        public DRMSystemStatus(String drmSystem, ArrayList<String> drmSystemIDs, int status,
                               String protectionGateways, String supportedFormats) {
            this.drmSystem = drmSystem;
            this.drmSystemIDs = drmSystemIDs;
            this.status = status;
            this.protectionGateways = protectionGateways;
            this.supportedFormats = supportedFormats;
        }

        /**
         * Get the data structure as a JSONObject.
         *
         * @return The data structure as a JSONObject.
         * @throws JSONException
         */
        @Override
        public JSONObject toJSONObject() throws JSONException {
            JSONObject o = new JSONObject();
            o.put("DRMSystem", drmSystem);
            o.put("DRMSystemIDs", new JSONArray(drmSystemIDs));
            o.put("status", status);
            o.put("protectionGateways", protectionGateways);
            o.put("supportedFormats", supportedFormats);
            return o;
        }
    }

    public static class Capabilities implements JSONSerializable {
        public List<String> optionStrings;
        public List<String> profileNameFragments;
        public List<String> parentalSchemes;
        public List<String> graphicsLevels; // Optional
        public List<String> broadcastUrns; // Optional
        public String displaySizeWidth;
        public String displaySizeHeight;
        public String displaySizeMeasurementType;
        public String audioOutputFormat; // Optional
        public boolean passThroughStatus; // Optional
        public String html5MediaVariableRateMin; // Optional
        public String html5MediaVariableRateMax; // Optional
        public String jsonRpcServerUrl; // Auto populated by ORB bridge
        public String jsonRpcServerVersion; // Auto populated by ORB bridge

        /**
         * Create a capabilities type that describes the current capabilities of the terminal.
         *
         * @param optionStrings A list of HbbTV option strings supported by the terminal.
         * Valid values as defined by HBBTV 10.2.4.8 table 13.
         * @param profileNameFragments A list of OIPF UI Profile Name Fragments supported by the
         * terminal; this shall always include trick mode ("+TRICKMODE"), any supported broadcast
         * delivery systems (e.g. "+DVB_S") and no other values.
         * Valid values as defined by OIPF DAE table 16.
         * @param parentalSchemes A list of parental scheme names registered with the platform.
         * Valid values are usable as scheme names with other parental APIs.
         * @param graphicsLevels A list of graphics performance levels supported by the terminal
         * (required if any of the graphics levels are supported, null to omit).
         * Valid values as defined by HBBTV 10.2.4.7 table 12c.
         * @param broadcastUrns A list of URNs for each supported broadcast technology (required if
         * any broadcast delivery is supported, null to omit).
         * Valid values as defined in HBBTV 10.2.4.7 for broadcast element value.
         * @param displaySizeWidth The current width of the primary display in centimetres.
         * Valid values as defined by HBBTV 10.2.4.7 for display_size width.
         * @param displaySizeHeight The current height of the primary display in centimetres.
         * Valid values as defined by HBBTV 10.2.4.7 for display_size height.
         * @param displaySizeMeasurementType The measurement type.
         * Valid values as defined by HBBTV 10.2.4.7 for display_size measurement_type.
         * @param audioOutputFormat The current multi-channel audio capabilities (required where
         * terminals support multi-channel audio, null to omit).
         * Valid values as defined by HBBTV 10.2.4.7 for audio_system audio_output_format.
         * @param passThroughStatus True when the terminal's audio outputs are operating in a
         * pass-through mode in which broadcast or broadband audio bitstreams are output directly by
         * the terminal without modification. False otherwise.
         * @param html5MediaVariableRateMin Minimum supported forward playback rate (required where
         * terminals support a playbackRate with a MediaSource object other than "1.0", null to omit).
         * Valid values as defined by HBBTV 10.2.4.7 for html5_media_variable_rate min.
         * @param html5MediaVariableRateMax Maximum supported forward playback rate (required where
         * terminals support a playbackRate with a MediaSource object other than "1.0", null to omit).
         * Valid values as defined by HBBTV 10.2.4.7 for html5_media_variable_rate max.
         * @throws IllegalArgumentException Thrown to indicate an invalid argument was provided.
         */
        public Capabilities(
                List<String> optionStrings,
                List<String> profileNameFragments,
                List<String> broadcastUrns,
                List<String> parentalSchemes,
                List<String> graphicsLevels,
                String displaySizeWidth,
                String displaySizeHeight,
                String displaySizeMeasurementType,
                String audioOutputFormat,
                boolean passThroughStatus,
                String html5MediaVariableRateMin,
                String html5MediaVariableRateMax) throws IllegalArgumentException {
            if (optionStrings == null) {
                throw new IllegalArgumentException("Argument 'optionStrings' cannot be null.");
            }
            this.optionStrings = optionStrings;
            if (profileNameFragments == null) {
                throw new IllegalArgumentException("Argument 'profileNameFragments' cannot be null.");
            }
            this.profileNameFragments = profileNameFragments;
            if (parentalSchemes == null) {
                throw new IllegalArgumentException("Argument 'parentalSchemes' cannot be null.");
            }
            this.parentalSchemes = parentalSchemes;
            this.graphicsLevels = graphicsLevels;
            this.broadcastUrns = broadcastUrns;
            if (displaySizeWidth == null) {
                throw new IllegalArgumentException(
                        "Argument 'displaySizeWidth' cannot be null.");
            }
            this.displaySizeWidth = displaySizeWidth;
            if (displaySizeHeight == null) {
                throw new IllegalArgumentException(
                        "Argument 'displaySizeHeight' cannot be null.");
            }
            this.displaySizeHeight = displaySizeHeight;
            if (displaySizeMeasurementType == null) {
                throw new IllegalArgumentException(
                        "Argument 'displaySizeMeasurementType' cannot be null.");
            }
            this.displaySizeMeasurementType = displaySizeMeasurementType;
            this.audioOutputFormat = audioOutputFormat;
            this.passThroughStatus = passThroughStatus;
            this.html5MediaVariableRateMin = html5MediaVariableRateMin;
            this.html5MediaVariableRateMax = html5MediaVariableRateMax;
        }

        public JSONObject toJSONObject() throws JSONException {
            JSONObject o = new JSONObject();
            o.put("optionStrings", new JSONArray(optionStrings));
            o.put("profileNameFragments", new JSONArray(profileNameFragments));
            o.put("parentalSchemes", new JSONArray(parentalSchemes));
            if (graphicsLevels != null) {
                o.put("graphicsLevels", new JSONArray(graphicsLevels));
            }
            if (broadcastUrns != null) {
                o.put("broadcastUrns", new JSONArray(broadcastUrns));
            }
            o.put("displaySizeWidth", displaySizeWidth);
            o.put("displaySizeHeight", displaySizeHeight);
            o.put("displaySizeMeasurementType", displaySizeMeasurementType);
            if (audioOutputFormat != null) {
                o.put("audioOutputFormat", audioOutputFormat);
                o.put("passThroughStatus", passThroughStatus);
            }
            if (html5MediaVariableRateMin != null) {
                o.put("html5MediaVariableRateMin", html5MediaVariableRateMin);
            }
            if (html5MediaVariableRateMax != null) {
                o.put("html5MediaVariableRateMax", html5MediaVariableRateMax);
            }
            if (jsonRpcServerUrl != null) {
                o.put("jsonRpcServerUrl", jsonRpcServerUrl);
            }
            if (jsonRpcServerVersion != null) {
                o.put("jsonRpcServerVersion", jsonRpcServerVersion);
            }
            return o;
        }
    }

    public static class AudioProfile implements JSONSerializable {
        public String name;
        public String type;
        public String transport; // Optional
        public String syncTl; // Optional
        public String drmSystemId; // Optional

        /**
         * Create an AudioProfile type that describes an audio profile, valid combinations are as
         * defined by HBBTV 10.2.4.7 for the audio_profile element.
         *
         * @param name Name of profile (required).
         * Valid values as defined by OIPF DAE 9.3.11 for audio_profile name.
         * @param type MIME type of profile (required).
         * Valid values as defined by OIPF DAE 9.3.11 for audio_profile type.
         * @param transport Space separated list of supported protocol names (optional, null to omit).
         * Valid values as defined by OIPF DAE 9.3.11 for audio_profile transport and HBBTV 10.2.4.7.
         * @param syncTl Space separated list of timeline types (optional, null to omit).
         * Valid values as defined by HBBTV 10.2.4.7 table 12a.
         * @param drmSystemId Space separated list of DRM system IDs (optional, null to omit).
         * Valid values as defined by OIPF DAE 9.3.11 for audio_profile DRMSystemID.
         * @throws IllegalArgumentException Thrown to indicate an invalid argument was provided.
         */
        public AudioProfile(String name, String type, String transport, String syncTl,
                            String drmSystemId) throws IllegalArgumentException {
            if (name == null) {
                throw new IllegalArgumentException("Argument 'name' cannot be null.");
            }
            this.name = name;
            if (type == null) {
                throw new IllegalArgumentException("Argument 'type' cannot be null.");
            }
            this.type = type;
            this.transport = transport;
            this.syncTl = syncTl;
            this.drmSystemId = drmSystemId;
        }

        public JSONObject toJSONObject() throws JSONException {
            JSONObject o = new JSONObject();
            o.put("name", name);
            o.put("type", type);
            if (transport != null) {
                o.put("transport", transport);
            }
            if (syncTl != null) {
                o.put("syncTl", syncTl);
            }
            if (drmSystemId != null) {
                o.put("drmSystemId", drmSystemId);
            }
            return o;
        }
    }

    public static class VideoProfile implements JSONSerializable {
        public String name;
        public String type;
        public String transport; // Optional
        public String syncTl; // Optional
        public String drmSystemId; // Optional
        public String hdr; // Optional

        /**
         * Create a VideoProfile type that describes a video profile, valid combinations are as
         * defined by HBBTV 10.2.4.7 for the video_profile element.
         *
         * @param name Name of profile (required).
         * Valid values as defined by OIPF DAE 9.3.11 for video_profile name.
         * @param type MIME type of profile (required).
         * Valid values as defined by OIPF DAE 9.3.11 for video_profile type.
         * @param transport Space separated list of supported protocol names (optional, null to omit).
         * Valid values as defined by OIPF DAE 9.3.11 for video_profile transport and HBBTV 10.2.4.7.
         * @param syncTl Space separated list of timeline types (optional, null to omit).
         * Valid values as defined by HBBTV 10.2.4.7 table 12a.
         * @param drmSystemId Space separated list of DRM system IDs (optional, null to omit).
         * Valid values as defined by OIPF DAE 9.3.11 for video_profile DRMSystemID.
         * @param hdr URI of HDR technology (optional, null to omit).
         * Valid values as defined by HBBTV 10.2.4.7 table 12b.
         * @throws IllegalArgumentException Thrown to indicate an invalid argument was provided.
         */
        public VideoProfile(String name, String type, String transport, String syncTl,
                            String drmSystemId, String hdr) throws IllegalArgumentException {
            if (name == null) {
                throw new IllegalArgumentException("Argument 'name' cannot be null.");
            }
            this.name = name;
            if (type == null) {
                throw new IllegalArgumentException("Argument 'type' cannot be null.");
            }
            this.type = type;
            this.transport = transport;
            this.syncTl = syncTl;
            this.drmSystemId = drmSystemId;
            this.hdr = hdr;
        }

        public JSONObject toJSONObject() throws JSONException {
            JSONObject o = new JSONObject();
            o.put("name", name);
            o.put("type", type);
            if (transport != null) {
                o.put("transport", transport);
            }
            if (syncTl != null) {
                o.put("syncTl", syncTl);
            }
            if (drmSystemId != null) {
                o.put("drmSystemId", drmSystemId);
            }
            if (hdr != null) {
                o.put("hdr", hdr);
            }
            return o;
        }
    }

    public static class VideoDisplayFormat implements JSONSerializable {
        public int width;
        public int height;
        public int frameRate;
        public int bitDepth;
        public String colorimetry;

        /**
         * Create a VideoDisplayFormat type that describes a video display format, valid combinations
         * are as defined by HBBTV 10.2.4.7 for the video_display_format element.
         *
         * @param width Width of the video content (required).
         * Valid values as defined by HBBTV 10.2.4.7 for video_display_format name.
         * @param height Height of the video content (required).
         * Valid values as defined by HBBTV 10.2.4.7 for video_display_format height.
         * @param frameRate Frame rate of the video content (required).
         * Valid values as defined by HBBTV 10.2.4.7 for video_display_format frame_rate.
         * @param bitDepth Bit depth of the video content (required).
         * Valid values as defined by HBBTV 10.2.4.7 for video_display_format bit_depth.
         * @param colorimetry A space separated list of colorimetry strings (required).
         * Valid values as defined by HBBTV 10.2.4.7 for video_display_format colorimetry.
         * @throws IllegalArgumentException Thrown to indicate an invalid argument was provided.
         */
        public VideoDisplayFormat(
                int width,
                int height,
                int frameRate,
                int bitDepth,
                String colorimetry) throws IllegalArgumentException {
            this.width = width;
            this.height = height;
            this.frameRate = frameRate;
            this.bitDepth = bitDepth;
            if (colorimetry == null) {
                throw new IllegalArgumentException("Argument 'colorimetry' cannot be null.");
            }
            this.colorimetry = colorimetry;
        }

        @Override
        public JSONObject toJSONObject() throws JSONException {
            JSONObject o = new JSONObject();
            o.put("width", width);
            o.put("height", height);
            o.put("frameRate", frameRate);
            o.put("bitDepth", bitDepth);
            o.put("colorimetry", colorimetry);
            return o;
        }
    }

    static public <T extends BridgeTypes.JSONSerializable> JSONObject toJSONObject(T object)
            throws JSONException {
        if (object == null) {
            return null;
        } else {
            return object.toJSONObject();
        }
    }

    static public <T extends BridgeTypes.JSONSerializable> JSONArray toJSONArray(List<T> list)
            throws JSONException {
        JSONArray array = new JSONArray();
        if (list != null) {
            for (T item : list) {
                array.put(item.toJSONObject());
            }
        }
        return array;
    }

    public static class DASHEvent implements JSONSerializable {
        public String id;
        public double startTime;
        public double duration;
        public String contentEncoding;

        public DASHEvent(String id, double startTime, double duration, String contentEncoding) {
            this.id = id;
            this.startTime = startTime;
            this.duration = duration;
            this.contentEncoding = contentEncoding;
        }

        @Override
        public JSONObject toJSONObject() throws JSONException {
            JSONObject o = new JSONObject();
            if (id != null) {
                o.put("id", id);
            }
            else {
                o.put("id", "");
            }
            if (contentEncoding != null) {
                o.put("contentEncoding", contentEncoding);
            }
            else {
                o.put("contentEncoding", "string");
            }
            if (startTime >= 0.0) {
                o.put("startTime", startTime);
            }
            if (duration >= 0.0) {
                o.put("duration", duration);
            }
            return o;
        }
    }
}
