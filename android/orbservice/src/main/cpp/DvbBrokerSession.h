#pragma once

#include <iostream>
#include <mutex>

#include <android/binder_auto_utils.h>
#include "org/orbtv/orbservice/BnDvbBrokerSession.h"
#include "org/orbtv/orbservice/IDvbClientSession.h"

namespace org::orbtv::orbservice {

class DvbBrokerSession : public BnDvbBrokerSession {
private:
    static ::android::sp<DvbBrokerSession> s_instance;
    static std::mutex s_mtx;

public:
    DvbBrokerSession(const DvbBrokerSession& obj) = delete; // prevent copies
    void operator=(const DvbBrokerSession &) = delete; // prevent assignments
    static ::android::sp<DvbBrokerSession> getInstance();
    DvbBrokerSession() {};

public:
    ::android::binder::Status initialise(const ::android::sp<IDvbClientSession>& dvb_client) override;
    ::android::binder::Status processAitSection(int32_t in_aitPid, int32_t in_serviceId, const std::vector<uint8_t>& in_data) override;
};

} // namespace org::orbtv::orbservice
