#ifndef ORB_CONSTANTS_H
#define ORB_CONSTANTS_H

#include <string>

namespace orb
{
// TODO is this alredy defined in AIDL?
enum ApplicationType {
  APP_TYPE_HBBTV = 0,
  APP_TYPE_OPAPP,
  APP_TYPE_MAX
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

const uint16_t KEY_SET_RED        =   0x1;
const uint16_t KEY_SET_GREEN      =   0x2;
const uint16_t KEY_SET_YELLOW     =   0x4;
const uint16_t KEY_SET_BLUE       =   0x8;
const uint16_t KEY_SET_NAVIGATION =  0x10;
const uint16_t KEY_SET_VCR        =  0x20;
const uint16_t KEY_SET_SCROLL     =  0x40;
const uint16_t KEY_SET_INFO       =  0x80;
const uint16_t KEY_SET_NUMERIC    = 0x100;
const uint16_t KEY_SET_ALPHA      = 0x200;
const uint16_t KEY_SET_OTHER      = 0x400;

// Event types
const std::string CHANNEL_STATUS_CHANGE = "ChannelStatusChanged";
const std::string NETWORK_STATUS = "NetworkStatus";

static constexpr std::string APPLICATION_TYPE_HBBTV = "HBBTV";
static constexpr std::string APPLICATION_TYPE_OPAPP = "OPAPP";

namespace Manager {
// Javascript ApplicationManager API methods
const std::string MANAGER_CREATE_APP = "createApplication";
const std::string MANAGER_DESTROY_APP = "destroyApplication";
const std::string MANAGER_SHOW_APP = "showApplication";
const std::string MANAGER_HIDE_APP = "hideApplication";
const std::string MANAGER_GET_APP_IDS = "getRunningAppIds";
const std::string MANAGER_GET_APP_URL = "getApplicationUrl";
const std::string MANAGER_GET_APP_SCHEME = "getApplicationScheme";
const std::string MANAGER_GET_FREE_MEM = "getFreeMem";
const std::string MANAGER_GET_KEY_VALUES = "getKeyValues";
const std::string MANAGER_GET_OKEY_VALUES = "getOtherKeyValues";
const std::string MANAGER_GET_KEY_MAX_VAL = "getKeyMaximumValue";
const std::string MANAGER_GET_MAX_OKEYS = "getKeyMaximumOtherKeys";
const std::string MANAGER_SET_KEY_VALUE = "setKeyValue";
const std::string MANAGER_GET_KEY_ICON = "getKeyIcon";

// OpApp API methods
const std::string MANAGER_GET_OP_APP_STATE = "getOpAppState";
const std::string MANAGER_OP_APP_REQUEST_BACKGROUND = "opAppRequestBackground";
const std::string MANAGER_OP_APP_REQUEST_FOREGROUND = "opAppRequestForeground";
const std::string MANAGER_OP_APP_REQUEST_TRANSIENT = "opAppRequestTransient";
const std::string MANAGER_OP_APP_REQUEST_OTRANSIENT = "opAppRequestOverlaidTransient";
const std::string MANAGER_OP_APP_REQUEST_OFOREGROUND = "opAppRequestOverlaidForeground";
} // namespace Manager

} // namespace orb

#endif  /* ORB_CONSTANTS_H */
