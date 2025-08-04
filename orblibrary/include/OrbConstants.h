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
