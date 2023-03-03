/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */
#include "JsonUtil.h"

namespace orb
{
json JsonUtil::CapabilitiesToJsonObject(std::shared_ptr<Capabilities> capabilities)
{
    json jsonCapabilities;

    json jsonOptionStrings;
    for (std::string optionString : capabilities->m_optionStrings)
    {
        jsonOptionStrings.push_back(optionString);
    }
    jsonCapabilities.emplace("optionStrings", jsonOptionStrings);

    json jsonProfileNameFragments;
    for (std::string profileNameFragment : capabilities->m_profileNameFragments)
    {
        jsonProfileNameFragments.push_back(profileNameFragment);
    }
    jsonCapabilities.emplace("profileNameFragments", jsonProfileNameFragments);

    json jsonParentalSchemes;
    for (std::string parentalScheme : capabilities->m_parentalSchemes)
    {
        jsonParentalSchemes.push_back(parentalScheme);
    }
    jsonCapabilities.emplace("parentalSchemes", jsonParentalSchemes);

    if (!capabilities->m_graphicsLevels.empty())
    {
        json jsonGraphicsLevels;
        for (std::string graphicsLevel : capabilities->m_graphicsLevels)
        {
            jsonGraphicsLevels.push_back(graphicsLevel);
        }
        jsonCapabilities.emplace("graphicsLevels", jsonGraphicsLevels);
    }

    if (!capabilities->m_broadcastUrns.empty())
    {
        json jsonBroadcastUrns;
        for (std::string broadcastUrn : capabilities->m_broadcastUrns)
        {
            jsonBroadcastUrns.push_back(broadcastUrn);
        }
        jsonCapabilities.emplace("broadcastUrns", jsonBroadcastUrns);
    }

    jsonCapabilities.emplace("displaySizeWidth", capabilities->m_displaySizeWidth);
    jsonCapabilities.emplace("displaySizeHeight", capabilities->m_displaySizeHeight);
    jsonCapabilities.emplace("displaySizeMeasurementType",
        capabilities->m_displaySizeMeasurementType);

    if (!capabilities->m_audioOutputFormat.empty())
    {
        jsonCapabilities.emplace("audioOutputFormat", capabilities->m_audioOutputFormat);
    }

    if (!capabilities->m_html5MediaVariableRateMin.empty())
    {
        jsonCapabilities.emplace("html5MediaVariableRateMin",
            capabilities->m_html5MediaVariableRateMin);
    }

    if (!capabilities->m_html5MediaVariableRateMax.empty())
    {
        jsonCapabilities.emplace("html5MediaVariableRateMax",
            capabilities->m_html5MediaVariableRateMax);
    }

    return jsonCapabilities;
}

json JsonUtil::AudioProfileToJsonObject(AudioProfile audioProfile)
{
    json jsonAudioProfile;
    jsonAudioProfile.emplace("name", audioProfile.m_name);
    jsonAudioProfile.emplace("type", audioProfile.m_type);
    if (!audioProfile.m_transport.empty())
    {
        jsonAudioProfile.emplace("transport", audioProfile.m_transport);
    }
    if (!audioProfile.m_syncTl.empty())
    {
        jsonAudioProfile.emplace("syncTl", audioProfile.m_syncTl);
    }
    if (!audioProfile.m_drmSystemId.empty())
    {
        jsonAudioProfile.emplace("drmSystemId", audioProfile.m_drmSystemId);
    }
    return jsonAudioProfile;
}

json JsonUtil::VideoProfileToJsonObject(VideoProfile videoProfile)
{
    json jsonVideoProfile;
    jsonVideoProfile.emplace("name", videoProfile.m_name);
    jsonVideoProfile.emplace("type", videoProfile.m_type);
    if (!videoProfile.m_transport.empty())
    {
        jsonVideoProfile.emplace("transport", videoProfile.m_transport);
    }
    if (!videoProfile.m_syncTl.empty())
    {
        jsonVideoProfile.emplace("syncTl", videoProfile.m_syncTl);
    }
    if (!videoProfile.m_drmSystemId.empty())
    {
        jsonVideoProfile.emplace("drmSystemId", videoProfile.m_drmSystemId);
    }
    if (!videoProfile.m_hdr.empty())
    {
        jsonVideoProfile.emplace("hdr", videoProfile.m_hdr);
    }
    return jsonVideoProfile;
}

json JsonUtil::VideoDisplayFormatToJsonObject(VideoDisplayFormat videoDisplayFormat)
{
    json jsonVideoDisplayFormat;
    jsonVideoDisplayFormat.emplace("width", videoDisplayFormat.m_width);
    jsonVideoDisplayFormat.emplace("height", videoDisplayFormat.m_height);
    jsonVideoDisplayFormat.emplace("frameRate", videoDisplayFormat.m_frameRate);
    jsonVideoDisplayFormat.emplace("bitDepth", videoDisplayFormat.m_bitDepth);
    jsonVideoDisplayFormat.emplace("colorimetry", videoDisplayFormat.m_colorimetry);
    return jsonVideoDisplayFormat;
}

std::shared_ptr<Channel> JsonUtil::ChannelFromJsonString(std::string jsonChannelAsString)
{
    json jsonChannel = json::parse(jsonChannelAsString);
    return ChannelFromJsonObject(jsonChannel);
}

std::shared_ptr<Channel> JsonUtil::ChannelFromJsonObject(json jsonChannel)
{
    std::shared_ptr<Channel> channel = std::make_shared<Channel>();
    channel->SetCcid(jsonChannel.value("ccid", ""));
    channel->SetName(jsonChannel.value("name", ""));
    channel->SetDsd(jsonChannel.value("dsd", ""));
    channel->SetIpBroadcastId(jsonChannel.value("ipBroadcastId", ""));
    channel->SetChannelType(static_cast<Channel::Type>(jsonChannel.value("channelType",
        Channel::Type::CHANNEL_TYPE_UNSPECIFIED)));
    channel->SetIdType(static_cast<Channel::IdType>(jsonChannel.value("idType",
        Channel::IdType::CHANNEL_ID_UNSPECIFIED)));
    channel->SetMajorChannel(jsonChannel.value("majorChannel", -1));
    channel->SetTerminalChannel(jsonChannel.value("terminalChannel", -1));
    channel->SetNid(jsonChannel.value("nid", -1));
    channel->SetOnid(jsonChannel.value("onid", -1));
    channel->SetTsId(jsonChannel.value("tsid", -1));
    channel->SetSid(jsonChannel.value("sid", -1));
    channel->SetHidden(jsonChannel.value("hidden", false));
    channel->SetSourceId(jsonChannel.value("sourceId", -1));
    return channel;
}

json JsonUtil::ChannelToJsonObject(Channel channel)
{
    json jsonChannel;
    jsonChannel["ccid"] = channel.GetCcid();
    jsonChannel["name"] = channel.GetName();
    jsonChannel["dsd"] = channel.GetDsd();
    jsonChannel["ipBroadcastId"] = channel.GetIpBroadcastId();
    jsonChannel["channelType"] = channel.GetChannelType();
    jsonChannel["idType"] = channel.GetIdType();
    jsonChannel["majorChannel"] = channel.GetMajorChannel();
    jsonChannel["terminalChannel"] = channel.GetTerminalChannel();
    jsonChannel["nid"] = channel.GetNid();
    jsonChannel["onid"] = channel.GetOnid();
    jsonChannel["tsid"] = channel.GetTsid();
    jsonChannel["sid"] = channel.GetSid();
    jsonChannel["hidden"] = channel.IsHidden();
    jsonChannel["sourceId"] = channel.GetSourceId();
    return jsonChannel;
}

std::shared_ptr<Programme> JsonUtil::ProgrammeFromJsonString(std::string jsonProgrammeAsString)
{
    json jsonProgramme = json::parse(jsonProgrammeAsString);
    return ProgrammeFromJsonObject(jsonProgramme);
}

std::shared_ptr<Programme> JsonUtil::ProgrammeFromJsonObject(json jsonProgramme)
{
    std::vector<ParentalRating> parentalRatings;

    json array = jsonProgramme["parentalRatings"];
    for (int i = 0; i < array.size(); i++)
    {
        json jsonParentalRating = array[i];
        parentalRatings.push_back(
            ParentalRating(
                jsonParentalRating.value("name", ""),
                jsonParentalRating.value("scheme", ""),
                jsonParentalRating.value("region", ""),
                jsonParentalRating.value("value", -1),
                jsonParentalRating.value("labels", -1)
                )
            );
    }

    return std::make_shared<Programme>(
        jsonProgramme.value("programmeID", ""),
        jsonProgramme.value("name", ""),
        jsonProgramme.value("description", ""),
        jsonProgramme.value("longDescription", ""),
        jsonProgramme.value("channelID", ""),
        jsonProgramme.value("startTime", -1L),
        jsonProgramme.value("duration", -1L),
        static_cast<Programme::ProgrammeIdType>(jsonProgramme.value("programmeIDType", -1)),
        parentalRatings
        );
}

json JsonUtil::ProgrammeToJsonObject(Programme programme)
{
    json json_programme;
    json_programme.emplace("programmeID", programme.GetProgrammeId());
    json_programme.emplace("programmeIDType", programme.GetProgrammeIdType());
    json_programme.emplace("name", programme.GetName());
    json_programme.emplace("description", programme.GetDescription());
    json_programme.emplace("longDescription", programme.GetLongDescription());
    json_programme.emplace("startTime", (int64_t) programme.GetStartTime());
    json_programme.emplace("duration", (int64_t) programme.GetDuration());
    json_programme.emplace("channelID", programme.GetChannelId());
    json json_parentalRatings = json::array();
    for (unsigned int i = 0; i < programme.GetParentalRatings().size(); i++)
    {
        json_parentalRatings.push_back(ParentalRatingToJsonObject(
            programme.GetParentalRatings()[i]));
    }
    json_programme.emplace("parentalRatings", json_parentalRatings);
    return json_programme;
}

json JsonUtil::ParentalRatingToJsonObject(ParentalRating parentalRating)
{
    json json_parentalRating;
    json_parentalRating.emplace("name", parentalRating.GetName());
    json_parentalRating.emplace("scheme", parentalRating.GetScheme());
    json_parentalRating.emplace("region", parentalRating.GetRegion());
    json_parentalRating.emplace("value", parentalRating.GetValue());
    json_parentalRating.emplace("labels", parentalRating.GetLabels());
    return json_parentalRating;
}

json JsonUtil::LocalSystemToJsonObject(LocalSystem localSystem)
{
    json json_LocalSystem;
    json_LocalSystem.emplace("vendorName", localSystem.GetVendorName());
    json_LocalSystem.emplace("modelName", localSystem.GetModelName());
    json_LocalSystem.emplace("softwareVersion", localSystem.GetSoftwareVersion());
    json_LocalSystem.emplace("hardwareVersion", localSystem.GetHardwareVersion());
    return json_LocalSystem;
}

json JsonUtil::ComponentToJsonObject(Component component)
{
    // Video component
    if (component.GetComponentType() == COMPONENT_TYPE_VIDEO)
    {
        json json_videoComponent;
        json_videoComponent.emplace("id", component.GetId());
        json_videoComponent.emplace("componentTag", component.GetComponentTag());
        json_videoComponent.emplace("pid", component.GetPid());
        json_videoComponent.emplace("type", COMPONENT_TYPE_VIDEO);
        json_videoComponent.emplace("encoding", component.GetEncoding());
        json_videoComponent.emplace("encrypted", component.IsEncrypted());
        json_videoComponent.emplace("aspectRatio", component.GetAspectRatio());
        json_videoComponent.emplace("active", component.IsActive());
        if (component.IsHidden())
        {
            json_videoComponent.emplace("hidden", true);
        }
        return json_videoComponent;
    }
    // Audio component
    else if (component.GetComponentType() == COMPONENT_TYPE_AUDIO)
    {
        json json_audioComponent;
        json_audioComponent.emplace("id", component.GetId());
        json_audioComponent.emplace("componentTag", component.GetComponentTag());
        json_audioComponent.emplace("pid", component.GetPid());
        json_audioComponent.emplace("type", COMPONENT_TYPE_AUDIO);
        json_audioComponent.emplace("encoding", component.GetEncoding());
        json_audioComponent.emplace("encrypted", component.IsEncrypted());
        json_audioComponent.emplace("language", component.GetLanguage());
        json_audioComponent.emplace("audioDescription", component.HasAudioDescription());
        json_audioComponent.emplace("audioChannels", component.GetAudioChannels());
        json_audioComponent.emplace("active", component.IsActive());
        if (component.IsHidden())
        {
            json_audioComponent.emplace("hidden", true);
        }
        return json_audioComponent;
    }
    // Subtitle component
    else if (component.GetComponentType() == COMPONENT_TYPE_SUBTITLE)
    {
        json json_subtitleComponent;
        json_subtitleComponent.emplace("id", component.GetId());
        json_subtitleComponent.emplace("componentTag", component.GetComponentTag());
        json_subtitleComponent.emplace("pid", component.GetPid());
        json_subtitleComponent.emplace("type", COMPONENT_TYPE_SUBTITLE);
        json_subtitleComponent.emplace("encoding", component.GetEncoding());
        json_subtitleComponent.emplace("encrypted", component.IsEncrypted());
        json_subtitleComponent.emplace("language", component.GetLanguage());
        json_subtitleComponent.emplace("hearingImpaired", component.IsHearingImpaired());
        json_subtitleComponent.emplace("label", component.GetLabel());
        json_subtitleComponent.emplace("active", component.IsActive());
        if (component.IsHidden())
        {
            json_subtitleComponent.emplace("hidden", true);
        }
        return json_subtitleComponent;
    }

    json json_invalidComponent;
    return json_invalidComponent;
}

json JsonUtil::DrmSystemStatusToJsonObject(DrmSystemStatus drmSystemStatus)
{
    json json_drmSystemStatus;
    json json_drmSystemIds;

    json_drmSystemStatus.emplace("DRMSystem", drmSystemStatus.GetDrmSystem());
    json_drmSystemStatus.emplace("status", drmSystemStatus.GetStatus());

    for (std::string id : drmSystemStatus.GetDrmSystemIds())
    {
        json_drmSystemIds.push_back(id);
    }

    json_drmSystemStatus.emplace("DRMSystemIDs", json_drmSystemIds);
    json_drmSystemStatus.emplace("protectionGateways", drmSystemStatus.GetProtectionGateways());
    json_drmSystemStatus.emplace("supportedFormats", drmSystemStatus.GetSupportedFormats());

    return json_drmSystemStatus;
}
} // namespace orb
