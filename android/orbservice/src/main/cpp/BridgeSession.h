#pragma once

#include <iostream>
#include <mutex>

#include <android/binder_auto_utils.h>
#include "org/orbtv/orbservice/BnBridgeSession.h"
#include "org/orbtv/orbservice/IBrowserSession.h"

using namespace std;

namespace org::orbtv::orbservice {

class BridgeSession : public BnBridgeSession {
private:
    static ::android::sp<BridgeSession> s_instance;
    static mutex s_mtx;

public:
    BridgeSession(const BridgeSession& obj) = delete; // prevent copies
    void operator=(const BridgeSession &) = delete; // prevent assignments
    static ::android::sp<BridgeSession> getInstance();
    BridgeSession() {}

public:
  ::android::binder::Status initialise(const ::android::sp<IBrowserSession>& browser) override;
  ::android::binder::Status executeRequest(const vector<uint8_t>& jsonstr, vector<uint8_t>* result) override;
  ::android::binder::Status getTvKeyCodeForApp(int32_t a_code, int32_t appId, int32_t* tv_code) override;
  ::android::binder::Status notifyLoadApplicationFailed(int32_t appId) override;
  ::android::binder::Status notifyApplicationPageChanged(int32_t appId, const vector<uint8_t>& url) override;
  ::android::binder::Status LoadDsmccDvbUrl(const vector<uint8_t>& dvb_url, int32_t requestId) override;
};

} // namespace org::orbtv::orbservice
