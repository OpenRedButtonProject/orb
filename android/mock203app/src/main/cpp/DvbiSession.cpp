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
#include <android/log.h>

#include <android/binder_ibinder_jni.h>
#include "generated/aidl/org/orbtv/orbservice/IOrbcSession.h"
#include "generated/aidl/org/orbtv/orbservice/IDvbiSession.h"

#include "DvbiSession.h"

#define TAG          "DvbiSession"
#define LOGI(x, ...) __android_log_print(ANDROID_LOG_INFO, TAG, "%s:%u " x "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__);
#define LOGE(x, ...) __android_log_print(ANDROID_LOG_ERROR, TAG, "%s:%u " x "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__);

using aidl::org::orbtv::orbservice::IOrbcSession;
using aidl::org::orbtv::orbservice::IDvbiSession;
using aidl::org::orbtv::orbservice::DvbiSession;
using aidl::org::orbtv::orbservice::DataBuffer;
using ndk::ScopedAStatus;
using namespace std;

std::shared_ptr<IOrbcSession> g_orb_session;
static DvbiSession g_dvb_session;
static const char g_lang[4] = {"eng"};
static const char g_cnty[3] = {"uk"};
static const char g_ccid[8] = {"ccid:01"};


extern "C" JNIEXPORT void JNICALL
Java_org_orbtv_mock203app_MainActivity_nativeServiceConnected(
        JNIEnv* env,
        jobject /* this */,
        jobject binder)
{
   AIBinder* pBinder = AIBinder_fromJavaBinder(env, binder);

   const ::ndk::SpAIBinder spBinder(pBinder);
   g_orb_session = IOrbcSession::fromBinder(spBinder);

   ::ndk::SpAIBinder dvbbinder = g_dvb_session.asBinder();
   std::shared_ptr<IDvbiSession> idvb = aidl::org::orbtv::orbservice::DvbiSession::fromBinder(dvbbinder);
   if (aidl::org::orbtv::orbservice::DvbiSession::setDefaultImpl(idvb))
   {
      LOGI("set default ok")
   }
   else
   {
      LOGE("set default failed")
   }

   LOGI("[cpp] onServiceConnected");
}

extern "C" JNIEXPORT void JNICALL
Java_org_orbtv_mock203app_MainActivity_nativeServiceDisconnected(
        JNIEnv* env,
        jobject /* this */)
{
   g_orb_session = nullptr;

   LOGI("[cpp] onServiceDisconnected");
}


extern "C" JNIEXPORT void JNICALL
Java_org_orbtv_mock203app_MainActivity_nativeTest(
        JNIEnv* env,
        jobject /* this */)
{
   ScopedAStatus status;
   int8_t data[4] = { 40, 0, 60, 70 };
   DataBuffer buffer(3, data);

   status = g_orb_session->onServiceListChanged();
   if (status.isOk())
   {
      LOGI("onServiceListChanged success")
   }
   else
   {
      LOGI("onServiceListChanged failed")
   }
   std::shared_ptr<IDvbiSession> idvb = DvbiSession::getDefaultImpl();
   if (idvb == nullptr)
   {
       LOGE("idvb is null")
   }
   status = g_orb_session->initialise(idvb);
   if (status.isOk())
   {
      LOGI("intialise() success")
   }
   else
   {
      LOGE("intialise() failed")
   }
    status = g_orb_session->processAIT(123, 9897, buffer);
    if (status.isOk())
    {
        LOGI("processAIT() success")
    }
    else
    {
        LOGI("processAIT() failed")
    }

    LOGI("");
}

DvbiSession::DvbiSession()
{
}

ScopedAStatus DvbiSession::getPreferredUILanguage(std::string* lang)
{
   ScopedAStatus status;

   LOGI("")
   *lang =  std::string(g_lang);

   status.set(AStatus_fromStatus(STATUS_OK));
   return status;
}

ScopedAStatus DvbiSession::getCountryId(std::string* country_id)
{
   ScopedAStatus status;

   LOGI("")
   *country_id = std::string(g_cnty);

   status.set(AStatus_fromStatus(STATUS_OK));
   return status;
}

ScopedAStatus DvbiSession::getSubtitlesEnabled(bool* enabled)
{
   ScopedAStatus status;

   LOGI("")
   *enabled = false;

   status.set(AStatus_fromStatus(STATUS_OK));
   return status;
}


ScopedAStatus DvbiSession::getAudioDescriptionEnabled(bool* enabled)
{
   ScopedAStatus status;

   LOGI("")
   *enabled = false;

   status.set(AStatus_fromStatus(STATUS_OK));
   return status;
}

ScopedAStatus DvbiSession::getCurrentCcid(std::string *ccid)
{
   ScopedAStatus status;

   LOGI("")
   *ccid = std::string(g_ccid);

   status.set(AStatus_fromStatus(STATUS_OK));
   return status;
}
