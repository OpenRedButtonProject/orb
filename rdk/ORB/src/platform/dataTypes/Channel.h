/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

#pragma once

#include <core/core.h>

using namespace WPEFramework::Core::JSON;

// Codes used to indicate the status of a channel
#define CHANNEL_STATUS_UNREALIZED -4
#define CHANNEL_STATUS_PRESENTING -3
#define CHANNEL_STATUS_CONNECTING -2
#define CHANNEL_STATUS_RECOVERING -1

// Channel change errors (see OIPF DAE spec section 7.13.1.2 onChannelChangeError table)
#define CHANNEL_STATUS_NOT_SUPPORTED 0
#define CHANNEL_STATUS_NO_SIGNAL 1
#define CHANNEL_STATUS_TUNER_IN_USE 2
#define CHANNEL_STATUS_PARENTAL_LOCKED 3
#define CHANNEL_STATUS_ENCRYPTED 4
#define CHANNEL_STATUS_UNKNOWN_CHANNEL 5
#define CHANNEL_STATUS_INTERRUPTED 6
#define CHANNEL_STATUS_RECORDING_IN_PROGRESS 7
#define CHANNEL_STATUS_CANNOT_RESOLVE_URI 8
#define CHANNEL_STATUS_INSUFFICIENT_BANDWIDTH 9
#define CHANNEL_STATUS_CANNOT_BE_CHANGED 10
#define CHANNEL_STATUS_INSUFFICIENT_RESOURCES 11
#define CHANNEL_STATUS_CHANNEL_NOT_IN_TS 12
#define CHANNEL_STATUS_UNKNOWN_ERROR 100

namespace orb {
/**
 * @brief orb::Channel
 *
 * HbbTV channel representation.
 */
class Channel {
public:

   static std::shared_ptr<Channel> FromJsonObject(JsonObject jsonChannel);
   static std::shared_ptr<Channel> FromJsonString(std::string jsonChannelAsString);

   Channel(
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
      );

   Channel(JsonObject jsonChannel);

   Channel();
   ~Channel();

   void SetFromJsonObject(JsonObject jsonChannel);

   void SetValid(bool valid);
   void SetCcid(std::string ccid);
   void SetName(std::string name);
   void SetDsd(std::string dsd);
   void SetIpBroadcastId(std::string ipBroadcastId);
   void SetChannelType(int channelType);
   void SetIdType(int idType);
   void SetMajorChannel(int majorChannel);
   void SetTerminalChannel(int terminalChannel);
   void SetNid(int nid);
   void SetOnid(int onid);
   void SetTsId(int tsid);
   void SetSid(int sid);
   void SetHidden(bool hidden);
   void SetSourceId(int sourceId);

   bool IsValid() const;
   std::string GetCcid() const;
   std::string GetName() const;
   std::string GetDsd() const;
   std::string GetIpBroadcastId() const;
   int GetChannelType() const;
   int GetIdType() const;
   int GetMajorChannel() const;
   int GetTerminalChannel() const;
   int GetNid() const;
   int GetOnid() const;
   int GetTsid() const;
   int GetSid() const;
   bool IsHidden() const;
   int GetSourceId() const;

   JsonObject ToJsonObject() const;

private:

   bool m_valid;
   std::string m_ccid;
   std::string m_name;
   std::string m_dsd;
   std::string m_ipBroadcastId;
   int m_channelType;
   int m_idType;
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
