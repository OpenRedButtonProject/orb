/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 * <p>
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

package org.orbtv.tvbrowser;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class TvBrowserTypes {
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
    public static final int VK_FAST_FWD = 417;
    public static final int VK_REWIND = 412;
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
                       boolean hidden, int sourceId, String ipBroadcastId) {
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
            o.put("dsd", dsd);
            o.put("nid", nid);
            o.put("onid", onid);
            o.put("tsid", tsid);
            o.put("sid", sid);
            o.put("name", name);
            o.put("majorChannel", majorChannel);
            o.put("terminalChannel", terminalChannel);
            o.put("hidden", hidden);
            o.put("sourceID", sourceId);
            o.put("ipBroadcastID", ipBroadcastId);
            return o;
        }
    }

    public static class Component implements JSONSerializable {
        public int componentType;
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
        public static Component createVideoComponent(int componentTag, int pid, String encoding,
                                                     boolean encrypted, int aspectRatio, boolean active) {
            Component c = new Component();
            c.componentType = COMPONENT_TYPE_VIDEO;
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
        public static Component createAudioComponent(int componentTag, int pid, String encoding,
                                                     boolean encrypted, String language, boolean audioDescription, int audioChannels,
                                                     boolean active) {
            Component c = new Component();
            c.componentType = COMPONENT_TYPE_AUDIO;
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
        public static Component createSubtitleComponent(int componentTag, int pid, String encoding,
                                                        boolean encrypted, String language, boolean hearingImpaired, String label, boolean active) {
            Component c = new Component();
            c.componentType = COMPONENT_TYPE_SUBTITLE;
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
            o.put("componentTag", componentTag);
            o.put("pid", pid);
            o.put("type", componentType);
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

        ParentalRatingScheme(String name, List<ParentalRating> ratings) {
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


    static public <T extends TvBrowserTypes.JSONSerializable> JSONObject toJSONObject(T object) throws JSONException {
        if (object == null) {
            return null;
        } else {
            return object.toJSONObject();
        }
    }

    static public <T extends TvBrowserTypes.JSONSerializable> JSONArray toJSONArray(List<T> list) throws JSONException {
        JSONArray array = new JSONArray();
        if (list != null) {
            for (T item : list) {
                array.put(item.toJSONObject());
            }
        }
        return array;
    }
}
