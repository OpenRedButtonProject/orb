#pragma once

#include <android/binder_auto_utils.h>
#include "org/orbtv/orbservice/IDvbClientSession.h"
#include "IDvbClient.h"

namespace org::orbtv::orbservice {

class DvbClientCallback : public orb::IDvbClient {

public:
    DvbClientCallback(const ::android::sp<IDvbClientSession>& dvbclient)
      : mDvbClientSession(dvbclient)
    {
    }

public:

    virtual std::string request(std::string jsonRequest) override;

    virtual void getDvbContent(std::string url, int requestId) override;

private:
    ::android::sp<IDvbClientSession> mDvbClientSession;
};

} // namespace org::orbtv::orbservice
