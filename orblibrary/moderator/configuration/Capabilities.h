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
#ifndef CAPABILITIES_H
#define CAPABILITIES_H

#include <string>
#include <vector>

namespace orb
{
/**
 * Representation of the media_profile element.
 * An AudioProfile type that describes an audio profile, valid combinations are as
 * defined by HBBTV 10.2.4.7 for the audioprofile element.
 */
struct AudioProfile
{
    /**
     * Name of profile (required).
     * Valid values as defined by OIPF DAE 9.3.11 for profile name.
     */
    std::string m_name;
    /**
     * MIME type of profile (required).
     * Valid values as defined by OIPF DAE 9.3.11 for profile type.
     */
    std::string m_type;
    /**
     * Space separated list of supported protocol names (optional, null to omit).
     * Valid values as defined by OIPF DAE 9.3.11 for profile transport and HBBTV 10.2.4.7.
     */
    std::string m_transport;
    /**
     * Space separated list of timeline types (optional, null to omit).
     * Valid values as defined by HBBTV 10.2.4.7 table 12a.
     */
    std::string m_syncTl;
    /**
     * Space separated list of DRM system IDs (optional, null to omit).
     * Valid values as defined by OIPF DAE 9.3.11 for profile DRMSystemID.
     */
    std::string m_drmSystemId;

}; // struct AudioProfile

/**
 * Representation of the video_profile element.
 * A VideoProfile type that describes a video profile, valid combinations are as
 * defined by HBBTV 10.2.4.7 for the video_profile element.
 */
struct VideoProfile : public AudioProfile
{
    /**
     * URI of HDR technology (optional, null to omit).
     * Valid values as defined by HBBTV 10.2.4.7 table 12b.
     */
    std::string m_hdr;

}; // struct VideoProfile

/**
 * Representation of the video_display_format element.
 * A VideoDisplayFormat type that describes a video display format, valid combinations are as
 * defined by HBBTV 10.2.4.7 for the video_display_format element.
 */
struct VideoDisplayFormat
{
    /**
     * Width of the video content (required).
     * Valid values as defined by HBBTV 10.2.4.7 for video_display_format name.
     */
    int m_width;
    /**
     * Height of the video content (required).
     * Valid values as defined by HBBTV 10.2.4.7 for video_display_format height.
     */
    int m_height;
    /**
     * Frame rate of the video content (required).
     * Valid values as defined by HBBTV 10.2.4.7 for video_display_format frame_rate.
     */
    int m_frameRate;
    /**
     * Bit depth of the video content (required).
     * Valid values as defined by HBBTV 10.2.4.7 for video_display_format bit_depth.
     */
    int m_bitDepth;
    /**
     * A space separated list of colorimetry strings (required).
     * Valid values as defined by HBBTV 10.2.4.7 for video_display_format colorimetry.
     */
    std::string m_colorimetry;

}; // struct VideoDisplayFormat

/**
 * Representation of the Capabilities object.
 * A Capabilities type that describes the current capabilities of the terminal.
 */
struct Capabilities
{
    /**
     * A list of HbbTV option strings supported by the terminal.
     * Valid values as defined by HBBTV 10.2.4.8 table 13.
     */
    std::vector<std::string> m_optionStrings;       // Required
    /**
     * A list of OIPF UI Profile Name Fragments supported by the terminal; this shall always include trick mode ("+TRICKMODE"),
     * any supported broadcast delivery systems (e.g. "+DVB_S") and no other values.
     * Valid values as defined by OIPF DAE table 16.
     */
    std::vector<std::string> m_profileNameFragments; // Required
    /**
     * A list of parental scheme names registered with the platform.
     * Valid values as defined by HBBTV 10.2.4.8 table 13.
     */
    std::vector<std::string> m_parentalSchemes;     // Required
    /**
     * A list of graphics performance levels supported by the terminal.
     * Valid values as defined by HBBTV 10.2.4.8 table 13.
     */
    std::vector<std::string> m_graphicsLevels;      // Optional
    /**
     * A list of URNs for each supported broadcast technology.
     * Valid values as defined by HBBTV 10.2.4.8 table 13.
     */
    std::vector<std::string> m_broadcastUrns;       // Optional
    /**
     * The current width of the primary display in centimetres.
     * Valid values as defined by HBBTV 10.2.4.8 table 13.
     */
    std::string m_displaySizeWidth;                 // Required
    /**
     * The current height of the primary display in centimetres.
     * Valid values as defined by HBBTV 10.2.4.8 table 13.
     */
    std::string m_displaySizeHeight;                // Required
    /**
     * The measurement type for display size.
     * Valid values as defined by HBBTV 10.2.4.8 table 13.
     */
    std::string m_displaySizeMeasurementType;       // Required
    /**
     * The current multi-channel audio capabilities
     * (required where terminals support multi-channel audio, null to omit).
     * Valid values as defined by HBBTV 10.2.4.7 for audio_system audio_output_format.
     */
    std::string m_audioOutputFormat;                // Optional
    /**
     * True when the terminal's audio outputs are operating in a pass-through
     * mode in which broadcast or broadband audio bitstreams are output directly
     * by the terminal without modification. False otherwise.
     * Valid values as defined by HBBTV 10.2.4.8 table 13.
     */
    bool m_passThroughStatus;                       // Optional
    /**
     * Minimum supported forward playback rate (required where terminals support
     * a playbackRate with a MediaSource object other than "1.0", null to omit).
     * Valid values as defined by HBBTV 10.2.4.8 table 13.
     */
    std::string m_html5MediaVariableRateMin;        // Optional
    /**
     * Maximum supported forward playback rate (required where terminals support
     * a playbackRate with a MediaSource object other than "1.0", null to omit).
     * Valid values as defined by HBBTV 10.2.4.8 table 13.
     */
    std::string m_html5MediaVariableRateMax;        // Optional
    /**
     * The URL of the JSON RPC WebSocket server
     * Valid values as defined by HBBTV 9.9.2.1
     * Discovery of the WebSocket Server.
     */
    std::string m_jsonRpcServerUrl;                 // Optional
    /**
     * The version of the JSON RPC WebSocket server
     * Valid values as defined by HBBTV 9.9.2.1
     * Discovery of the WebSocket Server.
     */
    std::string m_jsonRpcServerVersion;            // Optional
}; // struct Capabilities

} // namespace orb

#endif // CAPABILITIES_H