#include "PlatformAndroid.h"
#include "log.hpp"
#include "ConfigurationUtil.h"

namespace orb
{
const int STATUS_OK = -1;

AndroidPlatform::AndroidPlatform(ApplicationType apptype) : mAppType(apptype)
{
    LOGI("AndroidPlatform constructor");
    mChannelList.push_back(GenerateChannel(1, 2, 3, "Channel Sintel"));
    mChannelList.push_back(GenerateChannel(4, 5, 6, "Channel BigBuckBunny"));
    mChannelList.push_back(GenerateChannel(7, 8, 9, "Channel TearsOfSteel"));
    mCurrentChannel = std::make_shared<Channel>();
}

Channel AndroidPlatform::GenerateChannel(int onid, int tsid, int sid, std::string name)
{
    Channel channel;
    channel.SetName(name);
    channel.SetCcid("dvb://" + std::to_string(onid) + "." + std::to_string(tsid) + "." + std::to_string(sid));
    channel.SetChannelType(Channel::CHANNEL_TYPE_TV);
    channel.SetIdType(Channel::CHANNEL_ID_IPTV_URI);
    channel.SetMajorChannel(1);
    channel.SetTerminalChannel(1);
    channel.SetOnid(onid);
    channel.SetNid(onid); // nid is the same as onid for IP channel
    channel.SetTsId(tsid);
    channel.SetSid(sid);
    channel.SetIpBroadcastId(std::to_string(onid) + "." + std::to_string(tsid) + "." + std::to_string(sid));
    return channel;
}

AndroidPlatform::~AndroidPlatform()
{
    LOGI("AndroidPlatform destructor");
}

std::shared_ptr<Capabilities> AndroidPlatform::Configuration_GetCapabilities()
{
    LOGI("AndroidPlatform Configuration_GetCapabilities");
    // TODO: This is a mock implementation.
    std::shared_ptr<Capabilities> capabilities = std::make_shared<Capabilities>();

    std::vector<std::string> optionStrings;
    optionStrings.push_back("+PVR");
    optionStrings.push_back("+DRM");

    std::vector<std::string> profileNameFragments;
    profileNameFragments.push_back("+TRICKMODE"); // +ITV_KEYS is inherited from the base profile
    profileNameFragments.push_back("+DVB_T");
    profileNameFragments.push_back("+DVB_T2");
    profileNameFragments.push_back("+DVB_S");
    profileNameFragments.push_back("+DVB_S2");

    std::vector<std::string> parentalSchemes;
    parentalSchemes.push_back("dvb-si");

    std::vector<std::string> graphicsLevels;
    graphicsLevels.push_back("urn:hbbtv:graphics:performance:level1");
    graphicsLevels.push_back("urn:hbbtv:graphics:performance:level2");

    std::vector<std::string> broadcastUrns;
    broadcastUrns.push_back("urn:dvb:broadcast:ird:video:25_Hz_H.264_AVC_HDTV_IRD");
    broadcastUrns.push_back("urn:dvb:broadcast:ird:video:30_Hz_H.264_AVC_HDTV_IRD");
    broadcastUrns.push_back("urn:dvb:broadcast:ird:video:50_Hz_H.264_AVC_HDTV_IRD");
    broadcastUrns.push_back("urn:dvb:broadcast:ird:video:60_Hz_H.264_AVC_HDTV_IRD");
    broadcastUrns.push_back("urn:dvb:broadcast:ird:video:50_Hz_HEVC_HDTV_8-bit_IRD");
    broadcastUrns.push_back("urn:dvb:broadcast:ird:video:60_Hz_HEVC_HDTV_8-bit_IRD");

    broadcastUrns.push_back("urn:dvb:broadcast:ird:audio:MPEG-1_and_MPEG-2_backwards_compatible");
    broadcastUrns.push_back("urn:dvb:broadcast:ird:audio:AC-3_and_enhanced_AC-3");
    broadcastUrns.push_back("urn:dvb:broadcast:ird:audio:MPEG-4_AAC_family");

    std::string displaySizeWidth = "71"; // Mock 32" TV
    std::string displaySizeHeight = "40"; // Mock 32" TV
    std::string displaySizeMeasurementType = "built-in"; // hdmi-accurate, hdmi-other
    std::string audioOutputFormat = "stereo"; // multichannel, multichannel-preferred
    bool passThroughStatus = false;
    std::string html5MediaVariableRateMin = "0.5";
    std::string html5MediaVariableRateMax = "5.0";

    capabilities->m_optionStrings = optionStrings;
    capabilities->m_profileNameFragments = profileNameFragments;
    capabilities->m_parentalSchemes = parentalSchemes;
    capabilities->m_graphicsLevels = graphicsLevels;
    capabilities->m_broadcastUrns = broadcastUrns;
    capabilities->m_displaySizeWidth = displaySizeWidth;
    capabilities->m_displaySizeHeight = displaySizeHeight;
    capabilities->m_displaySizeMeasurementType = displaySizeMeasurementType;
    capabilities->m_audioOutputFormat = audioOutputFormat;
    capabilities->m_passThroughStatus = passThroughStatus;
    capabilities->m_html5MediaVariableRateMin = html5MediaVariableRateMin;
    capabilities->m_html5MediaVariableRateMax = html5MediaVariableRateMax;
    // HbbTV port is 8910, OpApp port is 8911
    int port = ConfigurationUtil::getJsonRpcServerPort(mAppType);
    capabilities->m_jsonRpcServerUrl = ConfigurationUtil::getJsonRpcServerUrl(port);
    capabilities->m_jsonRpcServerVersion = ConfigurationUtil::getJsonRpcServerVersion();
    return capabilities;
}

std::vector<AudioProfile> AndroidPlatform::Configuration_GetAudioProfiles()
{
    LOGI("AndroidPlatform Configuration_GetAudioProfiles");
    std::vector<AudioProfile> audioProfiles;
    audioProfiles.push_back(ConfigurationUtil::createAudioProfile("MPEG1_L3", "audio/mpeg", "", "", ""));
    audioProfiles.push_back(ConfigurationUtil::createAudioProfile("HEAAC", "audio/mp4", "", "", ""));
    audioProfiles.push_back(ConfigurationUtil::createAudioProfile("MP4_HEAAC", "audio/mp4", "dash", "dash_pr", ""));
    audioProfiles.push_back(ConfigurationUtil::createAudioProfile("MP4_E-AC3", "audio/mp4", "", "", ""));
    audioProfiles.push_back(ConfigurationUtil::createAudioProfile("MP4_E-AC3", "audio/mp4", "dash", "dash_pr", ""));
    return audioProfiles;
}

std::vector<VideoProfile> AndroidPlatform::Configuration_GetVideoProfiles()
{
    LOGI("AndroidPlatform Configuration_GetVideoProfiles");
    std::vector<VideoProfile> videoProfiles;
    videoProfiles.push_back(ConfigurationUtil::createVideoProfile("MP4_AVC_SD_25_HEAAC", "video/mp4", "", "", "", ""));
    videoProfiles.push_back(ConfigurationUtil::createVideoProfile("MP4_AVC_HD_25_HEAAC", "video/mp4", "", "", "", ""));
    videoProfiles.push_back(ConfigurationUtil::createVideoProfile("MP4_AVC_SD_25_HEAAC_EBUTTD", "video/mp4", "", "", "", ""));
    videoProfiles.push_back(ConfigurationUtil::createVideoProfile("MP4_AVC_HD_25_HEAAC_EBUTTD", "video/mp4", "", "", "", ""));
    videoProfiles.push_back(ConfigurationUtil::createVideoProfile("TS_AVC_SD_25_HEAAC", "video/mpeg", "", "temi", "", ""));
    videoProfiles.push_back(ConfigurationUtil::createVideoProfile("TS_AVC_HD_25_HEAAC", "video/mpeg", "", "temi", "", ""));
    videoProfiles.push_back(ConfigurationUtil::createVideoProfile("MP4_AVC_SD_25_HEAAC", "video/mp4", "", "", "", ""));
    videoProfiles.push_back(ConfigurationUtil::createVideoProfile("MP4_AVC_HD_25_HEAAC", "video/mp4", "", "", "", ""));
    videoProfiles.push_back(ConfigurationUtil::createVideoProfile("TS_AVC_SD_25_E-AC3", "video/mpeg", "", "temi", "", ""));
    videoProfiles.push_back(ConfigurationUtil::createVideoProfile("TS_AVC_HD_25_E-AC3", "video/mpeg", "", "temi", "", ""));
    videoProfiles.push_back(ConfigurationUtil::createVideoProfile("MP4_AVC_SD_25_E-AC3", "video/mp4", "", "", "", ""));
    videoProfiles.push_back(ConfigurationUtil::createVideoProfile("MP4_AVC_HD_25_E-AC3", "video/mp4", "", "", "", ""));
    videoProfiles.push_back(ConfigurationUtil::createVideoProfile("MP4_AVC_SD_25_E-AC3_EBUTTD", "video/mp4", "dash", "dash_pr", "", ""));
    videoProfiles.push_back(ConfigurationUtil::createVideoProfile("MP4_AVC_HD_25_E-AC3_EBUTTD", "video/mp4", "dash", "dash_pr", "", ""));
    return videoProfiles;
}


void AndroidPlatform::Broadcast_SetVideoRectangle(int x, int y, int width, int height)
{
    LOGD("AndroidPlatform x=" << x << " y=" << y << " w=" << width << " h=" << height);
}

std::shared_ptr<Channel> AndroidPlatform::Broadcast_GetCurrentChannel()
{
    LOGD("AndroidPlatform Broadcast_GetCurrentChannel");
    return mCurrentChannel;
}

std::vector<Channel> AndroidPlatform::Broadcast_GetChannelList()
{
    // create one mock channel with ccid 1
    LOGD("AndroidPlatform Broadcast_GetChannelList");
    return mChannelList;
}

int AndroidPlatform::Broadcast_SetChannelToCcid(
    std::string ccid,
    bool trickplay,
    std::string contentAccessDescriptorURL,
    int quiet
    )
{
    LOGD("Broadcast_SetChannelToCcid ccid=" << ccid << " trickplay=" << trickplay << " contentAccessDescriptorURL=" << contentAccessDescriptorURL << " quiet=" << quiet);
    // find the channel with the given ccid
    for (auto& channel : mChannelList) {
        if (channel.GetCcid() == ccid) {
            mCurrentChannel = std::make_shared<Channel>(channel);
            break;
        }
    }
    return STATUS_OK;
}

int AndroidPlatform::Broadcast_SetChannelToNull()
{
    LOGD("AndroidPlatform Broadcast_SetChannelToNull");
    return STATUS_OK;
}

int AndroidPlatform::Broadcast_SetChannelToTriplet(
    int idType,
    int onid,
    int tsid,
    int sid,
    int sourceID,
    std::string ipBroadcastID,
    bool trickplay,
    std::string contentAccessDescriptorURL,
    int quiet
    )
{
    LOGD("Broadcast_SetChannelToTriplet idType=" << idType << " onid=" << onid << " tsid=" << tsid << " sid=" << sid << " sourceID=" << sourceID << " ipBroadcastID=" << ipBroadcastID << " trickplay=" << trickplay << " contentAccessDescriptorURL=" << contentAccessDescriptorURL << " quiet=" << quiet);
    for (auto& channel : mChannelList) {
        if (channel.GetOnid() == onid && channel.GetTsid() == tsid && channel.GetSid() == sid) {
            mCurrentChannel = std::make_shared<Channel>(channel);
            break;
        }
    }
    return STATUS_OK;
}

int AndroidPlatform::Broadcast_SetChannelToDsd(
    std::string dsd,
    int sid,
    bool trickplay,
    std::string contentAccessDescriptorURL,
    int quiet
    )
{
    LOGI("Broadcast_SetChannelToDsd dsd=" << dsd << " sid=" << sid << " trickplay=" << trickplay << " contentAccessDescriptorURL=" << contentAccessDescriptorURL << " quiet=" << quiet);
    return STATUS_OK;
}

std::vector<Programme> AndroidPlatform::Broadcast_GetProgrammes(std::string ccid)
{
    LOGD("ccid=" << ccid);
    return std::vector<Programme>();
}

std::vector<Component> AndroidPlatform::Broadcast_GetComponents(std::string ccid, int
    componentType)
{
    LOGD("ccid=" << ccid << " componentType=" << componentType);
    std::vector<Component> components;
    return components;
}

std::shared_ptr<Component> AndroidPlatform::Broadcast_GetPrivateAudioComponent(std::string
    componentTag)
{
    LOGD("componentTag=" << componentTag);
    return nullptr;
}

std::shared_ptr<Component> AndroidPlatform::Broadcast_GetPrivateVideoComponent(std::string
    componentTag)
{
    LOGD("componentTag=" << componentTag);
    return nullptr;
}

void AndroidPlatform::Broadcast_OverrideComponentSelection(
    int componentType,
    std::string id
    )
{
    LOGD("componentType=" << componentType << " id=" << id);
}

void AndroidPlatform::Broadcast_RestoreComponentSelection(int componentType)
{
    LOGD("componentType=" << componentType);
}

void AndroidPlatform::Broadcast_SetPresentationSuspended(bool presentationSuspended)
{
    LOGD("presentationSuspended=" << (presentationSuspended ? "yes" : "no"));
}

void AndroidPlatform::Broadcast_Stop()
{
    LOGD("Broadcast_Stop");
}

void AndroidPlatform::Broadcast_Reset()
{
    LOGD("Broadcast_Reset");
    Broadcast_SetPresentationSuspended(false);
}


} // namespace orb