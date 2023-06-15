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

#define CB_HELLO_TERMINAL 0
#define CB_NUMBER_OF_ITEMS 1

static jfieldID g_service;
static jmethodID g_cb[CB_NUMBER_OF_ITEMS];

static NetworkServices::JsonRpcService* GetService(JNIEnv *env, jobject object);

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

    void HelloTerminal(int id, const std::string &message) override
    {
        // id is the id of the WebSocket connection -- we should probably reply to the connection
        // that sends the message? need to check hbbtv spec. also note, that the JSON RPC
        // protocol has a call ID... this needs to be included in the response, but is not currently
        // provided here... might need connectionId, callId, or a change ID to be a string:
        // connection-id:call-id

        JNIEnv *env = JniUtils::GetEnv();
        jstring j_message = env->NewStringUTF(message.c_str());
        env->CallVoidMethod(mCallbackObject, g_cb[CB_HELLO_TERMINAL]);
        env->DeleteLocalRef(j_message);
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
    g_cb[CB_HELLO_TERMINAL] = env->GetMethodID(managerClass,
        "onHelloTerminalCallback", "(Ljava/lang/String;)V");
}

extern "C"
JNIEXPORT void JNICALL Java_org_orbtv_orblibrary_JsonRpc_nativeInit(
    JNIEnv *env, jobject object)
{
    // We use a raw pointer here, as the common way to associate native objects with a Java object
    // is to store the pointer in a long. Java is responsible for calling nativeUninit
    auto sessionCallback = std::make_unique<JsonRpcCallback>(object);
    auto *service = new NetworkServices::JsonRpcService(
        8190, "/hbbtv/random", std::move(sessionCallback));
    env->SetLongField(object, g_service, jlong(service));
}

extern "C"
JNIEXPORT void JNICALL Java_org_orbtv_orblibrary_JsonRpc_nativeUninit(
        JNIEnv *env, jobject object)
{
    delete GetService(env, object);
}

extern "C"
JNIEXPORT void JNICALL Java_org_orbtv_orblibrary_JsonRpc_nativeHelloApp(
    JNIEnv *env, jobject object)
{
    GetService(env, object)->HelloApp(1);
}

static NetworkServices::JsonRpcService* GetService(JNIEnv *env, jobject object)
{
    return reinterpret_cast<NetworkServices::JsonRpcService *>(
        env->GetLongField(object, g_service));
}
