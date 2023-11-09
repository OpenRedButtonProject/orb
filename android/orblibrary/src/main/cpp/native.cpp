/**
 * ORB Software. Copyright (c) 2023 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

#include <jni.h>
#include <pthread.h>
#include <android/log.h>

#include "jni_utils.h"
#include "application_manager_native.h"
#include "network_services_native.h"

extern "C"
JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *)
{
    #ifdef BUILD_INFO
    __android_log_print(ANDROID_LOG_INFO, "Orb/Native", BUILD_INFO);
    #endif
    
    JniUtils::Init(vm, JNI_VERSION_1_6);
    InitialiseApplicationManagerNative();
    InitialiseNetworkServicesNative();
    return JNI_VERSION_1_6;
}

