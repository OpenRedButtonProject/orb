/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

#include "Channel.h"

namespace orb {
Channel::Channel()
    : m_valid(false)
    , m_ccid("")
    , m_name("")
    , m_dsd("")
    , m_ipBroadcastId("")
    , m_channelType(0)
    , m_idType(0)
    , m_majorChannel(0)
    , m_terminalChannel(0)
    , m_nid(0)
    , m_onid(0)
    , m_tsid(0)
    , m_sid(0)
    , m_hidden(false)
    , m_sourceId(0)
{
}

Channel::Channel(JsonObject jsonChannel)
{
    m_valid = jsonChannel.Get("valid").Boolean();
    m_ccid = jsonChannel.Get("ccid").String();
    m_name = jsonChannel.Get("name").String();
    m_dsd = jsonChannel.Get("dsd").String();
    m_ipBroadcastId = jsonChannel.Get("ipBroadcastId").String();
    m_channelType = jsonChannel.Get("channelType").Number();
    m_idType = jsonChannel.Get("idType").Number();
    m_majorChannel = jsonChannel.Get("majorChannel").Number();
    m_terminalChannel = jsonChannel.Get("terminalChannel").Number();
    m_nid = jsonChannel.Get("nid").Number();
    m_onid = jsonChannel.Get("onid").Number();
    m_tsid = jsonChannel.Get("tsid").Number();
    m_sid = jsonChannel.Get("sid").Number();
    m_hidden = jsonChannel.Get("hidden").Number();
    m_sourceId = jsonChannel.Get("sourceId").Number();
}

Channel::Channel(
    bool valid,
    std::string ccid,
    std::string name,
    std::string dsd,
    std::string ipBroadcastId,
    int channelType,
    int idType,
    int majorChannel,
    int terminalChannel,
    int nid,
    int onid,
    int tsid,
    int sid,
    bool hidden,
    int sourceId
    )
    : m_valid(valid)
    , m_ccid(ccid)
    , m_name(name)
    , m_dsd(dsd)
    , m_ipBroadcastId(ipBroadcastId)
    , m_channelType(channelType)
    , m_idType(idType)
    , m_majorChannel(majorChannel)
    , m_terminalChannel(terminalChannel)
    , m_nid(nid)
    , m_onid(onid)
    , m_tsid(tsid)
    , m_sid(sid)
    , m_hidden(hidden)
    , m_sourceId(sourceId)
{
}

Channel::~Channel()
{
}

void Channel::SetFromJsonObject(JsonObject jsonChannel)
{
    m_valid = jsonChannel.Get("valid").Boolean();
    m_ccid = jsonChannel.Get("ccid").String();
    m_name = jsonChannel.Get("name").String();
    m_dsd = jsonChannel.Get("dsd").String();
    m_ipBroadcastId = jsonChannel.Get("ipBroadcastId").String();
    m_channelType = jsonChannel.Get("channelType").Number();
    m_idType = jsonChannel.Get("idType").Number();
    m_majorChannel = jsonChannel.Get("majorChannel").Number();
    m_terminalChannel = jsonChannel.Get("terminalChannel").Number();
    m_nid = jsonChannel.Get("nid").Number();
    m_onid = jsonChannel.Get("onid").Number();
    m_tsid = jsonChannel.Get("tsid").Number();
    m_sid = jsonChannel.Get("sid").Number();
    m_hidden = jsonChannel.Get("hidden").Number();
    m_sourceId = jsonChannel.Get("sourceId").Number();
}

void Channel::SetValid(bool valid)
{
    m_valid = valid;
}

void Channel::SetCcid(std::string ccid)
{
    m_ccid = ccid;
}

void Channel::SetName(std::string name)
{
    m_name = name;
}

void Channel::SetDsd(std::string dsd)
{
    m_dsd = dsd;
}

void Channel::SetIpBroadcastId(std::string ipBroadcastId)
{
    m_ipBroadcastId = ipBroadcastId;
}

void Channel::SetChannelType(int channelType)
{
    m_channelType = channelType;
}

void Channel::SetIdType(int idType)
{
    m_idType = idType;
}

void Channel::SetMajorChannel(int majorChannel)
{
    m_majorChannel = majorChannel;
}

void Channel::SetTerminalChannel(int terminalChannel)
{
    m_terminalChannel = terminalChannel;
}

void Channel::SetNid(int nid)
{
    m_nid = nid;
}

void Channel::SetOnid(int onid)
{
    m_onid = onid;
}

void Channel::SetTsId(int tsid)
{
    m_tsid = tsid;
}

void Channel::SetSid(int sid)
{
    m_sid = sid;
}

void Channel::SetHidden(bool hidden)
{
    m_hidden = hidden;
}

void Channel::SetSourceId(int sourceId)
{
    m_sourceId = sourceId;
}

bool Channel::IsValid() const
{
    return m_valid;
}

std::string Channel::GetCcid() const
{
    return m_ccid;
}

std::string Channel::GetName() const
{
    return m_name;
}

std::string Channel::GetDsd() const
{
    return m_dsd;
}

std::string Channel::GetIpBroadcastId() const
{
    return m_ipBroadcastId;
}

int Channel::GetChannelType() const
{
    return m_channelType;
}

int Channel::GetIdType() const
{
    return m_idType;
}

int Channel::GetMajorChannel() const
{
    return m_majorChannel;
}

int Channel::GetTerminalChannel() const
{
    return m_terminalChannel;
}

int Channel::GetNid() const
{
    return m_nid;
}

int Channel::GetOnid() const
{
    return m_onid;
}

int Channel::GetTsid() const
{
    return m_tsid;
}

int Channel::GetSid() const
{
    return m_sid;
}

bool Channel::IsHidden() const
{
    return m_hidden;
}

int Channel::GetSourceId() const
{
    return m_sourceId;
}

JsonObject Channel::ToJsonObject() const
{
    JsonObject json_channel;
    json_channel.Set("valid", m_valid);
    json_channel.Set("ccid", m_ccid);
    json_channel.Set("name", m_name);
    json_channel.Set("dsd", m_dsd);
    json_channel.Set("ipBroadcastId", m_ipBroadcastId);
    json_channel.Set("channelType", m_channelType);
    json_channel.Set("idType", m_idType);
    json_channel.Set("majorChannel", m_majorChannel);
    json_channel.Set("terminalChannel", m_terminalChannel);
    json_channel.Set("nid", m_nid);
    json_channel.Set("onid", m_onid);
    json_channel.Set("tsid", m_tsid);
    json_channel.Set("sid", m_sid);
    json_channel.Set("hidden", m_hidden);
    json_channel.Set("sourceId", m_sourceId);
    return json_channel;
}

std::shared_ptr<Channel> Channel::FromJsonObject(JsonObject jsonChannel)
{
    return std::make_shared<Channel>(
        jsonChannel["valid"].Boolean(),
        jsonChannel["ccid"].String(),
        jsonChannel["name"].String(),
        jsonChannel["dsd"].String(),
        jsonChannel["ipBroadcastId"].String(),
        jsonChannel["channelType"].Number(),
        jsonChannel["idType"].Number(),
        jsonChannel["majorChannel"].Number(),
        jsonChannel["terminalChannel"].Number(),
        jsonChannel["nid"].Number(),
        jsonChannel["onid"].Number(),
        jsonChannel["tsid"].Number(),
        jsonChannel["sid"].Number(),
        jsonChannel["hidden"].Boolean(),
        jsonChannel["sourceId"].Number()
        );
}

std::shared_ptr<Channel> Channel::FromJsonString(std::string jsonChannelAsString)
{
    JsonObject jsonChannel;
    jsonChannel.FromString(jsonChannelAsString);
    return FromJsonObject(jsonChannel);
}
} // namespace orb
