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
#include "org/orbtv/orbservice/IOrbcSession.h"
#include "org/orbtv/orbservice/IDvbiSession.h"
#ifndef NDK_AIDL
#include <android/binder_libbinder.h>
#endif

#include "DvbiSession.h"

#define TAG          "DvbiSession"
#define LOGI(x, ...) __android_log_print(ANDROID_LOG_INFO, TAG, "%s:%u " x "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__);
#define LOGE(x, ...) __android_log_print(ANDROID_LOG_ERROR, TAG, "%s:%u " x "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__);

#ifdef NDK_AIDL
namespace aidl {
#endif

using org::orbtv::orbservice::IOrbcSession;
using org::orbtv::orbservice::IDvbiSession;
using org::orbtv::orbservice::DvbiSession;

static SH_PTR<IOrbcSession> g_orb_session;
static SH_PTR<DvbiSession> g_dvb_session;
static const char g_lang[4] = {"eng"};
static const char g_cnty[4] = {"gbr"};
static const char g_ccid[8] = {"ccid:01"};


extern "C" JNIEXPORT void JNICALL
Java_org_orbtv_mock203app_MainActivity_nativeServiceConnected(
        JNIEnv* env,
        jobject /* this */,
        jobject binder)
{
   AIBinder* pBinder = AIBinder_fromJavaBinder(env, binder);

#ifdef NDK_AIDL
   const ::ndk::SpAIBinder spBinder(pBinder);
   g_orb_session = IOrbcSession::fromBinder(spBinder);
   g_dvb_session = SharedRefBase::make<DvbiSession>();
#else
   g_orb_session = IOrbcSession::asInterface(AIBinder_toPlatformBinder(pBinder));
   g_dvb_session = new DvbiSession();
#endif

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
   STATUS status;
   vector<uint8_t> buffer(4);

   status = g_orb_session->onServiceListChanged();
   if (status.isOk())
   {
      LOGI("onServiceListChanged success")
   }
   else
   {
      LOGI("onServiceListChanged failed")
   }
   status = g_orb_session->initialise(g_dvb_session);
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

// ----------------------------------------------------------------------------------
//              Start of Interface functions required by ORB
// ----------------------------------------------------------------------------------

STATUS DvbiSession::getPreferredUILanguage(vector<uint8_t>* lang)
{
    STATUS status;
    vector<uint8_t> lng(g_lang, g_lang + 3);

    LOGI("")
    *lang =  lng;

    return status;
}

STATUS DvbiSession::getCountryId(vector<uint8_t>* country_id)
{
    STATUS status;
    vector<uint8_t> cid(g_cnty, g_cnty + 3);

    LOGI("")
    *country_id = cid;

    return status;
}

STATUS DvbiSession::getSubtitlesEnabled(bool* enabled)
{
    STATUS status;

    LOGI("")
    *enabled = false;

    return status;
}


STATUS DvbiSession::getAudioDescriptionEnabled(bool* enabled)
{
   STATUS status;

   LOGI("")
   *enabled = false;

   return status;
}

STATUS DvbiSession::getCurrentCcid(vector<uint8_t> *pccid)
{
    STATUS status;
    vector<uint8_t> ccid(g_ccid, g_ccid + 7);

    LOGI("")
    *pccid = ccid;

    return status;
}
