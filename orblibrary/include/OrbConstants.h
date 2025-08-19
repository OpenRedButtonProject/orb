#ifndef ORB_CONSTANTS_H
#define ORB_CONSTANTS_H

#include <string>

namespace orb
{
// TODO is this alredy defined in AIDL?
enum ApplicationType {
  APP_TYPE_HBBTV,
  APP_TYPE_OPAPP,
  APP_TYPE_VIDEO
};

enum ChannelStatus {
  // Codes used to indicate the status of a channel.
  CHANNEL_STATUS_UNREALIZED = -4,
  CHANNEL_STATUS_PRESENTING = -3,
  CHANNEL_STATUS_CONNECTING = -2,
  CHANNEL_STATUS_CONNECTING_RECOVERY = -1,
  // Channel change errors - See OIPF DAE spec section 7.13.1.2 onChannelChangeError table
  CHANNEL_STATUS_WRONG_TUNER = 0,
  CHANNEL_STATUS_NO_SIGNAL = 1,
  CHANNEL_STATUS_TUNER_IN_USE = 2,
  CHANNEL_STATUS_PARENTAL_LOCKED = 3,
  CHANNEL_STATUS_ENCRYPTED = 4,
  CHANNEL_STATUS_UNKNOWN_CHANNEL = 5,
  CHANNEL_STATUS_INTERRUPTED = 6,
  CHANNEL_STATUS_RECORDING_IN_PROGRESS = 7,
  CHANNEL_STATUS_CANNOT_RESOLVE_URI = 8,
  CHANNEL_STATUS_INSUFFICIENT_BANDWIDTH = 9,
  CHANNEL_STATUS_CANNOT_BE_CHANGED = 10,
  CHANNEL_STATUS_INSUFFICIENT_RESOURCES = 11,
  CHANNEL_STATUS_CHANNEL_NOT_IN_TS = 12,
  CHANNEL_STATUS_UNKNOWN_ERROR = 100
};

const uint16_t KEY_SET_RED = 0x1;
const uint16_t KEY_SET_GREEN = 0x2;
const uint16_t KEY_SET_YELLOW = 0x4;
const uint16_t KEY_SET_BLUE = 0x8;
const uint16_t KEY_SET_NAVIGATION = 0x10;
const uint16_t KEY_SET_VCR = 0x20;
const uint16_t KEY_SET_SCROLL = 0x40;
const uint16_t KEY_SET_INFO = 0x80;
const uint16_t KEY_SET_NUMERIC = 0x100;
const uint16_t KEY_SET_ALPHA = 0x200;
const uint16_t KEY_SET_OTHER = 0x400;

// Event types
const std::string CHANNEL_STATUS_CHANGE = "ChannelStatusChanged";
const std::string NETWORK_STATUS = "NetworkStatus";

static constexpr std::string APPLICATION_TYPE_HBBTV = "HBBTV";
static constexpr std::string APPLICATION_TYPE_OPAPP = "OPAPP";
static constexpr std::string APPLICATION_TYPE_VIDEO = "VIDEO";

// static std::string ApplicationTypeToString(orb::ApplicationType type) {
//   switch (type) {
//     case orb::ApplicationType::APP_TYPE_HBBTV:
//       return APPLICATION_TYPE_HBBTV;
//     case orb::ApplicationType::APP_TYPE_OPAPP:
//       return APPLICATION_TYPE_OPAPP;
//     case orb::ApplicationType::APP_TYPE_VIDEO:
//       return APPLICATION_TYPE_VIDEO;
//     default:
//       return "UNKNOWN";
//   }
// }

}

#endif  /* ORB_CONSTANTS_H */
