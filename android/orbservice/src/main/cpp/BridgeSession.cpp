/**
 * ORB Software. Copyright (c) 2023 Ocean Blue Software Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <android/log.h>

#include "BridgeSession.h"
#include "BrowserCallback.h"
#include "OrbInterface.h"

#define TAG                "BridgeSession"
#define LOGI(x, ...)    __android_log_print(ANDROID_LOG_INFO, TAG, "%s:%u " x "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__);
#define LOGE(x, ...)    __android_log_print(ANDROID_LOG_ERROR, TAG, "%s:%u " x "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__);

using org::orbtv::orbservice::BridgeSession;
using org::orbtv::orbservice::IBrowserSession;
using org::orbtv::orbservice::BrowserCallback;

static ::android::sp<IBrowserSession> g_browser_session;

::android::sp<BridgeSession> BridgeSession::s_instance = nullptr;
mutex BridgeSession::s_mtx;

::android::sp<BridgeSession> BridgeSession::getInstance()
{
    if (s_instance == nullptr) {
        lock_guard<mutex> lock(s_mtx);
        if (s_instance == nullptr) {
            s_instance = new BridgeSession();
        }
    }
    return s_instance;
}

::android::binder::Status BridgeSession::initialise(const ::android::sp<IBrowserSession>& browser)
{
   ::android::binder::Status status;
   bool enabled;

   if (browser == nullptr)
   {
      LOGE("dvb is null")
   }
   else
   {
       orb::Moderator* moderator = orb::OrbInterface::instance().getHbbTvModerator();
       moderator->setBrowserCallback(new BrowserCallback(browser));
       LOGI("")
   }

   return ::android::binder::Status::ok();
}

::android::binder::Status BridgeSession::executeRequest(const vector<uint8_t>& jsonstr, vector<uint8_t>* rtn)
{
    string json_request(jsonstr.begin(), jsonstr.end());
    orb::Moderator* moderator = orb::OrbInterface::instance().getHbbTvModerator();

    LOGI("json_request=%s", json_request.c_str());

    string result = moderator->executeRequest(json_request);

    rtn->assign(result.begin(), result.end());

    return ::android::binder::Status::ok();
}

::android::binder::Status BridgeSession::getTvKeyCodeForApp(int32_t a_code, int32_t appId, int32_t* tv_code)
{
    LOGI("")
    *tv_code = 0;
    return ::android::binder::Status::ok();
}

::android::binder::Status BridgeSession::notifyLoadApplicationFailed(int32_t appId)
{
    LOGI("")
    return ::android::binder::Status::ok();
}

::android::binder::Status BridgeSession::notifyApplicationPageChanged(int32_t appId, const vector<uint8_t>& url)
{
    LOGI("")
    return ::android::binder::Status::ok();
}

::android::binder::Status BridgeSession::LoadDsmccDvbUrl(const vector<uint8_t>& dvb_url, int32_t requestId)
{
    LOGI("")
    return ::android::binder::Status::ok();
}
