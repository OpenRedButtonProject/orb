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

#include <jni.h>
#include <pthread.h>
#include <android/log.h>

#include <android/binder_ibinder_jni.h>
#ifndef NDK_AIDL
#include <android/binder_libbinder.h>
#endif
#include "OrbcSession.h"
#include "org/orbtv/orbservice/IDvbiSession.h"

#define TAG                "OrbcSession"
#define LOGI(x, ...)    __android_log_print(ANDROID_LOG_INFO, TAG, "%s:%u " x "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__);
#define LOGE(x, ...)    __android_log_print(ANDROID_LOG_ERROR, TAG, "%s:%u " x "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__);

#ifdef NDK_AIDL
namespace aidl {
#endif

using org::orbtv::orbservice::OrbcSession;
using org::orbtv::orbservice::IDvbiSession;
using namespace std;

static SH_PTR<IDvbiSession> g_dvb_session;

extern "C" JNIEXPORT jobject JNICALL
Java_org_orbtv_orbservice_OrbService_createBinder(
        JNIEnv* env,
        jobject /* this */)
{
   static OrbcSession orb_session;
   LOGI("")
#ifdef NDK_AIDL
   AIBinder* binder = orb_session.asBinder().get();
#else
   AIBinder* binder = AIBinder_fromPlatformBinder(android::IInterface::asBinder(&orb_session));
#endif
   return env->NewGlobalRef(AIBinder_toJavaBinder(env, binder));
}

STATUS
OrbcSession::initialise(const SH_PTR<IDvbiSession>& dvb)
{
   STATUS status;
   bool enabled;

   if (dvb == nullptr)
   {
      LOGE("dvb is null")
   }
   else
   {
      g_dvb_session = dvb;
      LOGI("")
      //status = dvb->getSubtitlesEnabled(&enabled);
      //LOGI("en=%d ok%d", enabled, status.isOk())
   }

   return STATUS::ok();
}

STATUS
OrbcSession::processAIT(int32_t aitPid, int32_t serviceId, const std::vector<uint8_t>& in_data)
{
   STATUS status;
   string ccid;
   vector<uint8_t> vccid;

   LOGI("(%d, %d)", aitPid, serviceId)

   if (g_dvb_session == nullptr)
   {
      LOGE("g_dvb_session is null")
   }
   else
   {
      status = g_dvb_session->getCurrentCcid(&vccid);
      if (status.isOk())
      {
          std::string ccid( vccid.begin(), vccid.end() );
          LOGI("ccid ok %s", ccid.c_str())
      }
   }

   return STATUS::ok();
}

STATUS
OrbcSession::onServiceListChanged()
{
   STATUS status;

   LOGI("")

   //status.set(AStatus_fromStatus(STATUS_OK));
   return status;
}

STATUS
OrbcSession::onParentalRatingChanged(bool in_blocked)
{
   LOGI("blk?=%d", in_blocked)

   return STATUS::ok();
}

#ifdef NDK_AIDL
} // namespace aidl
#endif
