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

#include "DvbBrokerSession.h"
#include "DvbClientCallback.h"
#include "OrbInterface.h"

#define TAG                "DvbBrokerSession"
#define LOGI(x, ...)    __android_log_print(ANDROID_LOG_INFO, TAG, "%s:%u " x "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__);
#define LOGE(x, ...)    __android_log_print(ANDROID_LOG_ERROR, TAG, "%s:%u " x "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__);

using org::orbtv::orbservice::DvbBrokerSession;
using org::orbtv::orbservice::DvbClientCallback;
using org::orbtv::orbservice::IDvbClientSession;

using namespace std;

static ::android::sp<IDvbClientSession> g_dvb_client;
static ::orb::DvbBroker* g_dvb_broker;

::android::sp<DvbBrokerSession> DvbBrokerSession::s_instance = nullptr;
mutex DvbBrokerSession::s_mtx;

::android::sp<DvbBrokerSession> DvbBrokerSession::getInstance()
{
    if (s_instance == nullptr) {
        lock_guard<mutex> lock(s_mtx);
        if (s_instance == nullptr) {
            s_instance = new DvbBrokerSession();
        }
    }
    return s_instance;
}

::android::binder::Status DvbBrokerSession::initialise(const ::android::sp<IDvbClientSession>& dvb_client)
{
   ::android::binder::Status status;

   if (dvb_client == nullptr)
   {
      LOGE("dvb is null")
   }
   else
   {
      g_dvb_client = dvb_client;
      g_dvb_broker = orb::OrbInterface::instance().connectDvb(new DvbClientCallback(dvb_client));
      LOGI("")
   }

   return status;
}

::android::binder::Status DvbBrokerSession::processAitSection(int32_t aitPid, int32_t serviceId, const vector<uint8_t>& data)
{
   ::android::binder::Status status;

   g_dvb_broker->processAitSection(aitPid, serviceId, data);

   return status;
}
