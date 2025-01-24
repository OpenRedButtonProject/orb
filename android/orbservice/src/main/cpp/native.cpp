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
#include <android/binder_libbinder.h>

#include "jni_utils.h"
#include "BridgeSession.h"

#define TAG                "orbservice/native"
#define LOGI(x, ...)    __android_log_print(ANDROID_LOG_INFO, TAG, "%s:%u " x "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__);
#define LOGE(x, ...)    __android_log_print(ANDROID_LOG_ERROR, TAG, "%s:%u " x "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__);


using namespace org::orbtv::orbservice;

extern "C"
JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *)
{
    LOGI("")

    JniUtils::Init(vm, JNI_VERSION_1_6);

    return JNI_VERSION_1_6;
}

extern "C" JNIEXPORT jobject JNICALL
Java_org_orbtv_orbservice_BridgeService_createBinder(
        JNIEnv* env,
        jobject /* this */)
{
   LOGI("")
   AIBinder* binder = AIBinder_fromPlatformBinder(BridgeSession::getInstance());
   return env->NewGlobalRef(AIBinder_toJavaBinder(env, binder));
}

