/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
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
#pragma once

#include <string>
#include <vector>

namespace orb
{
/**
 * Representation of the audio_profile element.
 */
struct AudioProfile
{
    std::string m_name;       // Required
    std::string m_type;       // Required
    std::string m_transport;  // Optional
    std::string m_syncTl;     // Optional
    std::string m_drmSystemId; // Optional

    /**
     * Create an AudioProfile type that describes an audio profile, valid combinations are as
     * defined by HBBTV 10.2.4.7 for the audio_profile element.
     *
     * @param name        Name of profile (required).
     *                    Valid values as defined by OIPF DAE 9.3.11 for audio_profile name.
     * @param type        MIME type of profile (required).
     *                    Valid values as defined by OIPF DAE 9.3.11 for audio_profile type.
     * @param transport   Space separated list of supported protocol names (optional, null to omit).
     *                    Valid values as defined by OIPF DAE 9.3.11 for audio_profile transport and HBBTV 10.2.4.7.
     * @param syncTl      Space separated list of timeline types (optional, null to omit).
     *                    Valid values as defined by HBBTV 10.2.4.7 table 12a.
     * @param drmSystemId Space separated list of DRM system IDs (optional, null to omit).
     *                    Valid values as defined by OIPF DAE 9.3.11 for audio_profile DRMSystemID.
     */
    AudioProfile(
        std::string name,
        std::string type,
        std::string transport,
        std::string syncTl,
        std::string drmSystemId)
        : m_name(name)
        , m_type(type)
        , m_transport(transport)
        , m_syncTl(syncTl)
        , m_drmSystemId(drmSystemId)
    {
    }
}; // struct AudioProfile

/**
 * Representation of the video_profile element.
 */
struct VideoProfile
{
    std::string m_name;       // Required
    std::string m_type;       // Required
    std::string m_transport;  // Optional
    std::string m_syncTl;     // Optional
    std::string m_drmSystemId; // Optional
    std::string m_hdr;        // Optional

    /**
     * Create a VideoProfile type that describes a video profile, valid combinations are as
     * defined by HBBTV 10.2.4.7 for the video_profile element.
     *
     * @param name        Name of profile (required).
     *                    Valid values as defined by OIPF DAE 9.3.11 for video_profile name.
     * @param type        MIME type of profile (required).
     *                    Valid values as defined by OIPF DAE 9.3.11 for video_profile type.
     * @param transport   Space separated list of supported protocol names (optional, null to omit).
     *                    Valid values as defined by OIPF DAE 9.3.11 for video_profile transport and HBBTV 10.2.4.7.
     * @param syncTl      Space separated list of timeline types (optional, null to omit).
     *                    Valid values as defined by HBBTV 10.2.4.7 table 12a.
     * @param drmSystemId Space separated list of DRM system IDs (optional, null to omit).
     *                    Valid values as defined by OIPF DAE 9.3.11 for video_profile DRMSystemID.
     * @param hdr         URI of HDR technology (optional, null to omit).
     *                    Valid values as defined by HBBTV 10.2.4.7 table 12b.
     */
    VideoProfile(std::string name,
                 std::string type,
                 std::string transport,
                 std::string syncTl,
                 std::string drmSystemId,
                 std::string hdr)
        : m_name(name)
        , m_type(type)
        , m_transport(transport)
        , m_syncTl(syncTl)
        , m_drmSystemId(drmSystemId)
        , m_hdr(hdr)
    {
    }
}; // struct VideoProfile

/**
 * Representation of the video_display_format element.
 */
struct VideoDisplayFormat
{
    int m_width;
    int m_height;
    int m_frameRate;
    int m_bitDepth;
    std::string m_colorimetry;

    /**
     * Create a VideoDisplayFormat type that describes a video display format, valid combinations
     * are as defined by HBBTV 10.2.4.7 for the video_display_format element.
     *
     * @param width       Width of the video content (required).
     *                    Valid values as defined by HBBTV 10.2.4.7 for video_display_format name.
     * @param height      Height of the video content (required).
     *                    Valid values as defined by HBBTV 10.2.4.7 for video_display_format height.
     * @param frameRate   Frame rate of the video content (required).
     *                    Valid values as defined by HBBTV 10.2.4.7 for video_display_format frame_rate.
     * @param bitDepth    Bit depth of the video content (required).
     *                    Valid values as defined by HBBTV 10.2.4.7 for video_display_format bit_depth.
     * @param colorimetry A space separated list of colorimetry strings (required).
     *                    Valid values as defined by HBBTV 10.2.4.7 for video_display_format colorimetry.
     */
    VideoDisplayFormat(
        int width,
        int height,
        int frameRate,
        int bitDepth,
        std::string colorimetry)
        : m_width(width)
        , m_height(height)
        , m_frameRate(frameRate)
        , m_bitDepth(bitDepth)
        , m_colorimetry(colorimetry)
    {
    }
}; // struct VideoDisplayFormat

/**
 * Representation of the Capabilities object.
 */
struct Capabilities
{
    std::vector<std::string> m_optionStrings;       // Required
    std::vector<std::string> m_profileNameFragments; // Required
    std::vector<std::string> m_parentalSchemes;     // Required
    std::vector<std::string> m_graphicsLevels;      // Optional
    std::vector<std::string> m_broadcastUrns;       // Optional
    std::string m_displaySizeWidth;                 // Required
    std::string m_displaySizeHeight;                // Required
    std::string m_displaySizeMeasurementType;       // Required
    std::string m_audioOutputFormat;                // Optional
    bool m_passThroughStatus;                       // Optional
    std::string m_html5MediaVariableRateMin;        // Optional
    std::string m_html5MediaVariableRateMax;        // Optional

    /**
     * Create a capabilities type that describes the current capabilities of the terminal.
     *
     * @param optionStrings              A list of HbbTV option strings supported by the terminal.
     *                                   Valid values as defined by HBBTV 10.2.4.8 table 13.
     * @param profileNameFragments       A list of OIPF UI Profile Name Fragments supported by the
     *                                   terminal; this shall always include trick mode ("+TRICKMODE"),
     *                                   any supported broadcast delivery systems (e.g. "+DVB_S") and
     *                                   no other values.
     *                                   Valid values as defined by OIPF DAE table 16.
     * @param parentalSchemes            A list of parental scheme names registered with the platform.
     *                                   Valid values are usable as scheme names with other parental APIs.
     * @param graphicsLevels             A list of graphics performance levels supported by the terminal
     *                                   (required if any of the graphics levels are supported, null to omit).
     *                                   Valid values as defined by HBBTV 10.2.4.7 table 12c.
     * @param broadcastUrns              A list of URNs for each supported broadcast technology
     *                                   (required if any broadcast delivery is supported, null to omit).
     *                                   Valid values as defined in HBBTV 10.2.4.7 for broadcast
     *                                   element value.
     * @param displaySizeWidth           The current width of the primary display in centimetres.
     *                                   Valid values as defined by HBBTV 10.2.4.7 for display_size width.
     * @param displaySizeHeight          The current height of the primary display in centimetres.
     *                                   Valid values as defined by HBBTV 10.2.4.7 for display_size height.
     * @param displaySizeMeasurementType The measurement type.
     *                                   Valid values as defined by HBBTV 10.2.4.7 for display_size
     *                                   measurement_type.
     * @param audioOutputFormat          The current multi-channel audio capabilities (required where
     *                                   terminals support multi-channel audio, null to omit).
     *                                   Valid values as defined by HBBTV 10.2.4.7 for audio_system
     *                                   audio_output_format.
     * @param passThroughStatus          True when the terminal's audio outputs are operating in a
     *                                   pass-through mode in which broadcast or broadband audio
     *                                   bitstreams are output directly by the terminal without
     *                                   modification. False otherwise.
     * @param html5MediaVariableRateMin  Minimum supported forward playback rate (required where
     *                                   terminals support a playbackRate with a MediaSource object
     *                                   other than "1.0", null to omit).
     *                                   Valid values as defined by HBBTV 10.2.4.7 for
     *                                   html5_media_variable_rate min.
     * @param html5MediaVariableRateMax  Maximum supported forward playback rate (required where
     *                                   terminals support a playbackRate with a MediaSource object
     *                                   other than "1.0", null to omit).
     *                                   Valid values as defined by HBBTV 10.2.4.7 for
     *                                   html5_media_variable_rate max.
     */
    Capabilities(
        std::vector<std::string> optionStrings,
        std::vector<std::string> profileNameFragments,
        std::vector<std::string> parentalSchemes,
        std::vector<std::string> graphicsLevels,
        std::vector<std::string> broadcastUrns,
        std::string displaySizeWidth,
        std::string displaySizeHeight,
        std::string displaySizeMeasurementType,
        std::string audioOutputFormat,
        bool passThroughStatus,
        std::string html5MediaVariableRateMin,
        std::string html5MediaVariableRateMax)
        : m_optionStrings(optionStrings)
        , m_profileNameFragments(profileNameFragments)
        , m_parentalSchemes(parentalSchemes)
        , m_graphicsLevels(graphicsLevels)
        , m_broadcastUrns(broadcastUrns)
        , m_displaySizeWidth(displaySizeWidth)
        , m_displaySizeHeight(displaySizeHeight)
        , m_displaySizeMeasurementType(displaySizeMeasurementType)
        , m_audioOutputFormat(audioOutputFormat)
        , m_passThroughStatus(passThroughStatus)
        , m_html5MediaVariableRateMin(html5MediaVariableRateMin)
        , m_html5MediaVariableRateMax(html5MediaVariableRateMax)
    {
    }
}; // struct Capabilities
} // namespace orb