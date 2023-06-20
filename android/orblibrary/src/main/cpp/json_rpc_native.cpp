/**
 * ORB Software. Copyright (c) 2023 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

#include "json_rpc_native.h"

#include <jni.h>
#include <pthread.h>
#include <android/log.h>

#include <memory>
#include "JsonRpcService.h"

#include "jni_utils.h"

#define CB_REQUEST_DIALOGUE_ENHANCEMENT_OVERRIDE 0
#define CB_NUMBER_OF_ITEMS 1

static jfieldID g_service;
static jmethodID g_cb[CB_NUMBER_OF_ITEMS];

static NetworkServices::JsonRpcService* GetService(JNIEnv *env, jobject object);

// Convert std::string to jstring using:
// 1. jstring jstr = env->NewStringUTF(str.c_str());
// 2. use jstr
// 3. env->DeleteLocalRef(jstr); // always do this, otherwise we will leak memory!
// and, convert jstring to std::string using:
// 1. std::string str = JniUtils::MakeStdString(env, str)

class JsonRpcCallback : public NetworkServices::JsonRpcService::SessionCallback {
public:

    explicit JsonRpcCallback(jobject callbackObject)
    {
        JNIEnv *env = JniUtils::GetEnv();
        mCallbackObject = reinterpret_cast<jclass>(env->NewGlobalRef(callbackObject));
    }

    ~JsonRpcCallback() override
    {
        JNIEnv *env = JniUtils::GetEnv();
        env->DeleteGlobalRef(mCallbackObject);
    }

    void RequestDialogueEnhancementOverride(
        int connection,
        int id,
        int dialogueEnhancementGain) override
    {
        __android_log_print(ANDROID_LOG_INFO, "JsonRpcCallback",
            "JSON-RPC-EXAMPLE #3: Android native called with request. Call Java...");
        JNIEnv *env = JniUtils::GetEnv();
        env->CallVoidMethod(
            mCallbackObject,
            g_cb[CB_REQUEST_DIALOGUE_ENHANCEMENT_OVERRIDE],
            connection, id, dialogueEnhancementGain);
    }
private:
    jobject mCallbackObject;
};

void InitialiseJsonRpcNative()
{
    JNIEnv *env = JniUtils::GetEnv();
    jclass managerClass = env->FindClass("org/orbtv/orblibrary/JsonRpc");
    g_service = env->GetFieldID(managerClass, "mServicePointerField", "J");

    // Callback methods
    g_cb[CB_REQUEST_DIALOGUE_ENHANCEMENT_OVERRIDE] = env->GetMethodID(managerClass,
        "onRequestDialogueEnhancementOverride", "(III)V");
}

extern "C"
JNIEXPORT void JNICALL Java_org_orbtv_orblibrary_JsonRpc_nativeOpen(
    JNIEnv *env,
    jobject object,
    jint port,
    jstring endpoint)
{
    // We use a raw pointer here, as the common way to associate native objects with a Java object
    // is to store the pointer in a long. Java is responsible for calling nativeClose
    auto sessionCallback = std::make_unique<JsonRpcCallback>(object);
    auto *service = new NetworkServices::JsonRpcService(
        port, JniUtils::MakeStdString(env, endpoint),
        std::move(sessionCallback));
    env->SetLongField(object, g_service, jlong(service));
}

extern "C"
JNIEXPORT void JNICALL Java_org_orbtv_orblibrary_JsonRpc_nativeClose(
    JNIEnv *env,
    jobject object)
{
    delete GetService(env, object);
}

extern "C"
JNIEXPORT void JNICALL Java_org_orbtv_orblibrary_JsonRpc_nativeOnRespondDialogueEnhancementOverride(
    JNIEnv *env,
    jobject object,
    jint connection,
    jint id,
    jint dialogueEnhancementGain)
{
    __android_log_print(ANDROID_LOG_INFO, "JsonRpcCallback",
        "JSON-RPC-EXAMPLE #8: Android native called with response. Call service...");
    GetService(env, object)->RespondDialogueEnhancementOverride(
        connection, id, dialogueEnhancementGain);
}

static NetworkServices::JsonRpcService* GetService(JNIEnv *env, jobject object)
{
    return reinterpret_cast<NetworkServices::JsonRpcService *>(
        env->GetLongField(object, g_service));
}
