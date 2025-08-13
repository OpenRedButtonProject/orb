#include "BroadcastUtil.h"

namespace orb
{
    Json::Value BroadcastUtil::convertChannelListToJson(std::vector<Channel> channels)
    {
        Json::Value json;
        for (auto& channel : channels) {
            json.append(convertChannelToJson(channel));
        }
        return json;
    }

    Json::Value BroadcastUtil::convertChannelToJson(Channel channel)
    {
        Json::Value json;
        json["name"] = channel.GetName();
        json["ccid"] = channel.GetCcid();
        json["channelType"] = channel.GetChannelType();
        json["idType"] = channel.GetIdType();
        json["majorChannel"] = channel.GetMajorChannel();
        json["terminalChannel"] = channel.GetTerminalChannel();
        json["nid"] = channel.GetNid();
        json["onid"] = channel.GetOnid();
        json["tsid"] = channel.GetTsid();
        json["sid"] = channel.GetSid();
        json["hidden"] = channel.IsHidden();
        json["sourceId"] = channel.GetSourceId();
        json["dsd"] = channel.GetDsd();
        json["ipBroadcastId"] = channel.GetIpBroadcastId();

        return json;
    }

    bool BroadcastUtil::isIpChannel(std::shared_ptr<Channel> channel)
    {
        return channel->GetIdType() == Channel::CHANNEL_ID_IPTV_SDS
            || channel->GetIdType() == Channel::CHANNEL_ID_IPTV_URI;
    }

}