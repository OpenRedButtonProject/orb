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
