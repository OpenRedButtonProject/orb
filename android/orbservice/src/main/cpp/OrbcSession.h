#pragma once

#include "org/orbtv/orbservice/BnOrbcSession.h"
#include <android/binder_auto_utils.h>
#include "org/orbtv/orbservice/IDvbiSession.h"

#ifdef NDK_AIDL
#define STATUS ndk::ScopedAStatus
#define SH_PTR std::shared_ptr
#else
#define STATUS ::android::binder::Status
#define SH_PTR ::android::sp
#endif

#ifdef NDK_AIDL
namespace aidl {
#endif

namespace org::orbtv::orbservice {

class OrbcSession : public BnOrbcSession {
public:
  STATUS initialise(const SH_PTR<IDvbiSession>& in_dvb) override;
  STATUS processAIT(int32_t in_aitPid, int32_t in_serviceId, const std::vector<uint8_t>& in_data) override;
  STATUS onServiceListChanged() override;
  STATUS onParentalRatingChanged(bool in_blocked) override;

};

} // namespace org::orbtv::orbservice

#ifdef NDK_AIDL
} // namespace aidl
#endif
