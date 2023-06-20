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

#include "jni_utils.h"
#include "application_manager_native.h"
#include "network_services_native.h"
#include "json_rpc_native.h"

extern "C"
JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *)
{
    #ifdef BUILD_INFO
    __android_log_print(ANDROID_LOG_INFO, "Orb/Native", BUILD_INFO);
    #endif
    
    JniUtils::Init(vm, JNI_VERSION_1_6);
    InitialiseApplicationManagerNative();
    InitialiseNetworkServicesNative();
    InitialiseJsonRpcNative();
    return JNI_VERSION_1_6;
}

