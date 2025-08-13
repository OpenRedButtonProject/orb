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

#ifndef CHANNEL_H
#define CHANNEL_H

#include <string>

namespace orb
{
/**
 * @brief orb::Channel
 *
 * HbbTV channel representation.
 */
class Channel
{
public:

    /**
     * Enumerates the Channel::idType property values.
     * (See OIPF DAE spec section 7.13.11.1 Constants)
     */
    enum IdType
    {
        CHANNEL_ID_ANALOG        = 0x00,// (decimal 00)
        CHANNEL_ID_DVB_C         = 0x0a,// (decimal 10)
        CHANNEL_ID_DVB_S         = 0x0b,// (decimal 11)
        CHANNEL_ID_DVB_T         = 0x0c,// (decimal 12)
        CHANNEL_ID_DVB_SI_DIRECT = 0x0d, // (decimal 13)
        CHANNEL_ID_DVB_C2        = 0x0e,// (decimal 14)
        CHANNEL_ID_DVB_S2        = 0x0f,// (decimal 15)
        CHANNEL_ID_DVB_T2        = 0x10,// (decimal 16)
        CHANNEL_ID_ISDB_C        = 0x14,// (decimal 20)
        CHANNEL_ID_ISDB_S        = 0x15,// (decimal 21)
        CHANNEL_ID_ISDB_T        = 0x16,// (decimal 22)
        CHANNEL_ID_ATSC_T        = 0x1e,// (decimal 30)
        CHANNEL_ID_IPTV_SDS      = 0x28,// (decimal 40)
        CHANNEL_ID_IPTV_URI      = 0x29,// (decimal 41)
        CHANNEL_ID_UNSPECIFIED   = 0xff
    };

    /**
     * Enumerates the Channel::channelType property values.
     * (See OIPF DAE spec section 7.13.11.1 Constants)
     */
    enum Type
    {
        CHANNEL_TYPE_TV          = 0x000,// (decimal 000)
        CHANNEL_TYPE_RADIO       = 0x001,// (decimal 001)
        CHANNEL_TYPE_OTHER       = 0x002,// (decimal 002)
        CHANNEL_TYPE_ALL         = 0x080,// (decimal 128)
        CHANNEL_TYPE_HBB_DATA    = 0x100,// (decimal 256)
        CHANNEL_TYPE_UNSPECIFIED = 0xfff
    };

public:

    /**
     * Default constructor.
     */
    Channel()
        : m_ccid("")
        , m_name("")
        , m_dsd("")
        , m_ipBroadcastId("")
        , m_channelType(CHANNEL_TYPE_UNSPECIFIED)
        , m_idType(CHANNEL_ID_UNSPECIFIED)
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

    /**
     * Constructor.
     * (For more details on the input parameters see OIPF DAE spec sections: 7.13.11.2 & 7.13.11.3.1)
     *
     * @param ccid            Unique identifier of the channel
     * @param name            The name of the channel
     * @param dsd             (See OIPF DAE spec section 7.13.11.2)
     * @param ipBroadcastId   (See OIPF DAE spec section 7.13.11.2)
     * @param channelType     The type of channel set to one of the Channel::Type enumerated values
     * @param idType          The type of identification for the channel as indicated by one of the
     *                        Channel::IdType enumerated values
     * @param majorChannel    The major channel number, if assigned
     * @param terminalChannel An integer property which shall be set to the value of the terminal's
     *                        Logical Channel Number as used by the terminal's native UI
     *                        (See HbbTV 2.0.3 spec section 8.2.5)
     * @param nid             The DVB or ISDB network ID
     * @param onid            The DVB or ISDB original network ID
     * @param tsid            The DVB or ISDB transport stream ID
     * @param sid             The DVB or ISDB service ID
     * @param hidden          Flag indicating whether the channel shall be included in the default
     *                        channel list
     * @param sourceId        ATSC source_ID value
     */
    Channel(
        std::string ccid,
        std::string name,
        std::string dsd,
        std::string ipBroadcastId,
        Type channelType,
        IdType idType,
        int majorChannel,
        int terminalChannel,
        int nid,
        int onid,
        int tsid,
        int sid,
        bool hidden,
        int sourceId
        )
        : m_ccid(ccid)
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

    ~Channel()
    {
    }

    void SetCcid(std::string ccid)
    {
        m_ccid = ccid;
    }

    void SetName(std::string name)
    {
        m_name = name;
    }

    void SetDsd(std::string dsd)
    {
        m_dsd = dsd;
    }

    void SetIpBroadcastId(std::string ipBroadcastId)
    {
        m_ipBroadcastId = ipBroadcastId;
    }

    void SetChannelType(Channel::Type channelType)
    {
        m_channelType = channelType;
    }

    void SetIdType(Channel::IdType idType)
    {
        m_idType = idType;
    }

    void SetMajorChannel(int majorChannel)
    {
        m_majorChannel = majorChannel;
    }

    void SetTerminalChannel(int terminalChannel)
    {
        m_terminalChannel = terminalChannel;
    }

    void SetNid(int nid)
    {
        m_nid = nid;
    }

    void SetOnid(int onid)
    {
        m_onid = onid;
    }

    void SetTsId(int tsid)
    {
        m_tsid = tsid;
    }

    void SetSid(int sid)
    {
        m_sid = sid;
    }

    void SetHidden(bool hidden)
    {
        m_hidden = hidden;
    }

    void SetSourceId(int sourceId)
    {
        m_sourceId = sourceId;
    }

    std::string GetCcid() const
    {
        return m_ccid;
    }

    std::string GetName() const
    {
        return m_name;
    }

    std::string GetDsd() const
    {
        return m_dsd;
    }

    std::string GetIpBroadcastId() const
    {
        return m_ipBroadcastId;
    }

    Type GetChannelType() const
    {
        return m_channelType;
    }

    Channel::IdType GetIdType() const
    {
        return m_idType;
    }

    int GetMajorChannel() const
    {
        return m_majorChannel;
    }

    int GetTerminalChannel() const
    {
        return m_terminalChannel;
    }

    int GetNid() const
    {
        return m_nid;
    }

    int GetOnid() const
    {
        return m_onid;
    }

    int GetTsid() const
    {
        return m_tsid;
    }

    int GetSid() const
    {
        return m_sid;
    }

    bool IsHidden() const
    {
        return m_hidden;
    }

    int GetSourceId() const
    {
        return m_sourceId;
    }

private:

    // member variables

    std::string m_ccid;
    std::string m_name;
    std::string m_dsd;
    std::string m_ipBroadcastId;
    Type m_channelType;
    IdType m_idType;
    int m_majorChannel;
    int m_terminalChannel;
    int m_nid;
    int m_onid;
    int m_tsid;
    int m_sid;
    bool m_hidden;
    int m_sourceId;
}; // class Channel
} // namespace orb
#endif // CHANNEL_H