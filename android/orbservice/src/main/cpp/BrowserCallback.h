#pragma once

#include <android/binder_auto_utils.h>
#include "org/orbtv/orbservice/IBrowserSession.h"
#include "IBrowser.h"

namespace org::orbtv::orbservice {

class BrowserCallback : public orb::IBrowser {

public:
    BrowserCallback(const ::android::sp<IBrowserSession>& browser)
      : mBrowserSession(browser)
    {
    }

public:

    // Load new application at URL with new app_id for a reference to this application
    void loadApplication(std::string app_id, std::string url) override;

    // Show application
    void showApplication() override;

    // Hide application
    void hideApplication() override;

    // Dispatch event
    void dispatchEvent(std::string type, std::string properties) override;

    // Dispatch key event
    bool dispatchKeyEvent(int32_t action, int32_t key_code) override;

    // Provide DSM-CC content
    void provideDsmccContent(std::string url, const std::vector<uint8_t>& content) override;

private:
    ::android::sp<IBrowserSession> mBrowserSession;
};

} // namespace org::orbtv::orbservice
