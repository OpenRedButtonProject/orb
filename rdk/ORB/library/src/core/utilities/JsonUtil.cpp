/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */
#include "JsonUtil.h"

namespace orb
{
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
   channel->SetChannelType(static_cast<Channel::Type>(jsonChannel.value("channelType", Channel::Type::CHANNEL_TYPE_UNSPECIFIED)));
   channel->SetIdType(static_cast<Channel::IdType>(jsonChannel.value("idType", Channel::IdType::CHANNEL_ID_UNSPECIFIED)));
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
   json json_parentalRatings;
   for (unsigned int i = 0; i < programme.GetParentalRatings().size(); i++)
   {
      json_parentalRatings.push_back(ParentalRatingToJsonObject(programme.GetParentalRatings()[i]));
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
} // namespace orb
