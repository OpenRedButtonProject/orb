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
#define CB_REQUEST_NEGOTIATE_METHODS 1
#define CB_REQUEST_SUBSCRIBE_UNSUBSCRIBE 2
#define CB_RECEIVE_ERROR 3
#define CB_REQUEST_FEATURE_SUPPORT_INFO 4
#define CB_REQUEST_FEATURE_SETTINGS_QUERY 5
#define CB_REQUEST_FEATURE_SUPPRESS 6
#define CB_NOTIFY_VOICE_READY 7
#define CB_NOTIFY_STATE_MEDIA 8
#define CB_NOTIFY_STATE_MEDIA_ALL_VALUES 9

#define CB_NUMBER_OF_ITEMS 10

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

    void RequestNegotiateMethods(
        int connection,
        int id,
        std::string terminalToApp,
        std::string appToTerminal) override
    {
        __android_log_print(ANDROID_LOG_INFO, "JsonRpcCallback",
                            "JSON-RPC-EXAMPLE #3: RequestNegotiateMethods...");
        JNIEnv *env = JniUtils::GetEnv();
        jstring j_terminalToApp = env->NewStringUTF(terminalToApp.c_str());
        jstring j_appToTerminal = env->NewStringUTF(appToTerminal.c_str());
        env->CallVoidMethod(
                mCallbackObject,
                g_cb[CB_REQUEST_NEGOTIATE_METHODS],
                connection, id, j_terminalToApp, j_appToTerminal);
        env->DeleteLocalRef(j_terminalToApp);
        env->DeleteLocalRef(j_appToTerminal);
    }

    void RequestSubscribe(
        int connection,
        int id,
        bool subtitles, bool dialogueEnhancement,
        bool uiMagnifier, bool highContrastUI,
        bool screenReader, bool responseToUserAction,
        bool audioDescription, bool inVisionSigning) override
    {
        __android_log_print(ANDROID_LOG_INFO, "JsonRpcCallback",
                            "JSON-RPC-EXAMPLE #3: Android native called with request. Call Java...");
        JNIEnv *env = JniUtils::GetEnv();
        env->CallVoidMethod(
                mCallbackObject,
                g_cb[CB_REQUEST_SUBSCRIBE_UNSUBSCRIBE],
                true, connection, id,
                subtitles, dialogueEnhancement, uiMagnifier, highContrastUI,
                screenReader, responseToUserAction, audioDescription, inVisionSigning);
    }

    void RequestUnsubscribe(
        int connection,
        int id,
        bool subtitles, bool dialogueEnhancement,
        bool uiMagnifier, bool highContrastUI,
        bool screenReader, bool responseToUserAction,
        bool audioDescription, bool inVisionSigning) override
    {
        __android_log_print(ANDROID_LOG_INFO, "JsonRpcCallback",
                            "JSON-RPC-EXAMPLE #3: Android native called with request. Call Java...");
        JNIEnv *env = JniUtils::GetEnv();
        env->CallVoidMethod(
                mCallbackObject,
                g_cb[CB_REQUEST_SUBSCRIBE_UNSUBSCRIBE],
                false, connection, id,
                subtitles, dialogueEnhancement, uiMagnifier, highContrastUI,
                screenReader, responseToUserAction, audioDescription, inVisionSigning);
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

    void RequestFeatureSupportInfo(
        int connection,
        int id,
        int feature) override
    {
        __android_log_print(ANDROID_LOG_INFO, "JsonRpcCallback",
                            "JSON-RPC-EXAMPLE #3a: Android native called with request. Call Java...");
        JNIEnv *env = JniUtils::GetEnv();
        env->CallVoidMethod(
                mCallbackObject,
                g_cb[CB_REQUEST_FEATURE_SUPPORT_INFO],
                connection, id, feature);
    }

    void RequestFeatureSettingsQuery(
        int connection,
        int id,
        int feature) override
    {
        __android_log_print(ANDROID_LOG_INFO, "JsonRpcCallback",
                            "JSON-RPC-EXAMPLE #3a: Android native called with request. Call Java...");
        JNIEnv *env = JniUtils::GetEnv();
        env->CallVoidMethod(
                mCallbackObject,
                g_cb[CB_REQUEST_FEATURE_SETTINGS_QUERY],
                connection, id, feature);
    }

    void RequestFeatureSuppress(
        int connection,
        int id,
        int feature) override
    {
        __android_log_print(ANDROID_LOG_INFO, "JsonRpcCallback",
                            "JSON-RPC-EXAMPLE #3a: Android native called with request. Call Java...");
        JNIEnv *env = JniUtils::GetEnv();
        env->CallVoidMethod(
                mCallbackObject,
                g_cb[CB_REQUEST_FEATURE_SUPPRESS],
                connection, id, feature);
    }

    void NotifyVoiceReady(
        int connection,
        bool isReady) override
    {
        __android_log_print(ANDROID_LOG_INFO, "JsonRpcCallback",
                            "JSON-RPC-EXAMPLE #3a: Android native called with request. Call Java...");
        JNIEnv *env = JniUtils::GetEnv();
        env->CallVoidMethod(
                mCallbackObject,
                g_cb[CB_NOTIFY_VOICE_READY],
                connection, isReady);
    }

    void NotifyStateMedia(
        int connection,
        std::string state,
        bool actPause, bool actPlay, bool actFastForward, bool actFastReverse, bool actStop,
        bool actSeekContent, bool actSeekRelative, bool actSeekLive, bool actWallclock)
    {
        __android_log_print(ANDROID_LOG_INFO, "JsonRpcCallback",
                            "JSON-RPC-EXAMPLE #3a: Android native called with request. Call Java...");
        JNIEnv *env = JniUtils::GetEnv();
        jstring j_state = env->NewStringUTF(state.c_str());
        env->CallVoidMethod(
                mCallbackObject,
                g_cb[CB_NOTIFY_STATE_MEDIA],
                connection,
                j_state,
                actPause, actPlay, actFastForward, actFastReverse, actStop,
                actSeekContent, actSeekRelative, actSeekLive, actWallclock);
        env->DeleteLocalRef(j_state);
    }

    void NotifyStateMedia(
        int connection,
        std::string state, std::string kind, std::string type, std::string currentTime,
        std::string rangeStart, std::string rangeEnd,
        bool actPause, bool actPlay, bool actFastForward, bool actFastReverse, bool actStop,
        bool actSeekContent, bool actSeekRelative, bool actSeekLive, bool actWallclock,
        std::string mediaId, std::string title, std::string secTitle, std::string synopsis,
        bool subtitlesEnabled, bool subtitlesAvailable,
        bool audioDescripEnabled, bool audioDescripAvailable,
        bool signLangEnabled, bool signLangAvailable)
    {
        __android_log_print(ANDROID_LOG_INFO, "JsonRpcCallback",
                            "JSON-RPC-EXAMPLE #3a: Android native called with request. Call Java...");
        JNIEnv *env = JniUtils::GetEnv();
        jstring j_state = env->NewStringUTF(state.c_str());
        jstring j_kind = env->NewStringUTF(kind.c_str());
        jstring j_type = env->NewStringUTF( type.c_str());
        jstring j_currentTime = env->NewStringUTF(currentTime.c_str());
        jstring j_rangeStart = env->NewStringUTF(rangeStart.c_str());
        jstring j_rangeEnd = env->NewStringUTF(rangeEnd.c_str());
        jstring j_mediaId = env->NewStringUTF(state.c_str());   // can be null
        jstring j_title = env->NewStringUTF(kind.c_str());
        jstring j_secTitle = env->NewStringUTF( type.c_str());   // can be null
        jstring j_synopsis = env->NewStringUTF(synopsis.c_str());   // can be null
        env->CallVoidMethod(
                mCallbackObject,
                g_cb[CB_NOTIFY_STATE_MEDIA_ALL_VALUES],
                connection,
                j_state, j_kind, j_type, j_currentTime,
                j_rangeStart, j_rangeEnd,
                actPause, actPlay, actFastForward, actFastReverse, actStop,
                actSeekContent, actSeekRelative, actSeekLive, actWallclock,
                j_mediaId, j_title, j_secTitle, j_synopsis,
                subtitlesEnabled, subtitlesAvailable,
                audioDescripEnabled, audioDescripAvailable,
                signLangEnabled, signLangAvailable);
        env->DeleteLocalRef(j_state);
        env->DeleteLocalRef(j_kind);
        env->DeleteLocalRef(j_type);
        env->DeleteLocalRef(j_currentTime);
        env->DeleteLocalRef(j_rangeStart);
        env->DeleteLocalRef(j_rangeEnd);
        env->DeleteLocalRef(j_mediaId);
        env->DeleteLocalRef(j_title);
        env->DeleteLocalRef(j_secTitle);
        env->DeleteLocalRef(j_synopsis);
    }

// Convert std::string to jstring using:
// 1. jstring jstr = env->NewStringUTF(str.c_str());
// 2. use jstr
// 3. env->DeleteLocalRef(jstr); // always do this, otherwise we will leak memory!
// and, convert jstring to std::string using:
// 1. std::string str = JniUtils::MakeStdString(env, str)
    void ReceiveError(
        int connection,
        int id,
        int code,
        std::string message) override
    {
        __android_log_print(ANDROID_LOG_INFO, "JsonRpcCallback",
                            "JSON-RPC-EXAMPLE #3a: Android native called with request. Call Java...");
        JNIEnv *env = JniUtils::GetEnv();
        jstring j_message = env->NewStringUTF(message.c_str());
        env->CallVoidMethod(
                mCallbackObject,
                g_cb[CB_RECEIVE_ERROR],
                connection, id, code, j_message);
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
    g_cb[CB_REQUEST_DIALOGUE_ENHANCEMENT_OVERRIDE] = env->GetMethodID(managerClass,
        "onRequestDialogueEnhancementOverride", "(III)V");
    g_cb[CB_REQUEST_NEGOTIATE_METHODS] = env->GetMethodID(managerClass,
        "onRequestNegotiateMethods", "(IILjava/lang/String;Ljava/lang/String;)V");
    g_cb[CB_REQUEST_SUBSCRIBE_UNSUBSCRIBE] = env->GetMethodID(managerClass,
        "onRequestSubscribe", "(ZIIZZZZZZZZ)V");
    g_cb[CB_REQUEST_FEATURE_SUPPORT_INFO] = env->GetMethodID(managerClass,
         "onRequestFeatureSupportInfo", "(III)V");
    g_cb[CB_REQUEST_FEATURE_SETTINGS_QUERY] = env->GetMethodID(managerClass,
         "onRequestFeatureSettingsQuery", "(III)V");
    g_cb[CB_REQUEST_FEATURE_SUPPRESS] = env->GetMethodID(managerClass,
         "onRequestFeatureSuppress", "(III)V");
    g_cb[CB_NOTIFY_VOICE_READY] = env->GetMethodID(managerClass,
         "onNotifyVoiceReady", "(IZ)V");
    g_cb[CB_NOTIFY_STATE_MEDIA] = env->GetMethodID(managerClass,
         "onNotifyStateMedia", "(ILjava/lang/String;ZZZZZZZZZ)V");
    g_cb[CB_NOTIFY_STATE_MEDIA_ALL_VALUES] = env->GetMethodID(managerClass,
         "onNotifyStateMediaAllValues","(ILjava/lang/String;Ljava/lang/String;"
               "Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;"
               "ZZZZZZZZZ"
               "Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;"
               "ZZZZZZ)V");
    g_cb[CB_RECEIVE_ERROR] = env->GetMethodID(managerClass,
         "onReceiveError", "(IIILjava/lang/String;)V");
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

// java -> cpp
extern "C"
JNIEXPORT void JNICALL
Java_org_orbtv_orblibrary_JsonRpc_nativeOnRespondNegotiateMethods(
    JNIEnv *env,
    jobject object,
    jint connection,
    jint id,
    jstring terminalToApp,
    jstring appToTerminal)
{
    __android_log_print(ANDROID_LOG_INFO, "JsonRpcCallback",
                        "JSON-RPC-EXAMPLE #8: Android native called with response. Call service...");
    std::string terminalToAppStr = JniUtils::MakeStdString(env, terminalToApp);
    std::string appToTerminalStr = JniUtils::MakeStdString(env, appToTerminal);
    GetService(env, object)->RespondNegotiateMethods(
            connection, id, terminalToAppStr, appToTerminalStr);
}

extern "C"
JNIEXPORT void JNICALL
Java_org_orbtv_orblibrary_JsonRpc_nativeOnRespondSubscribe(
    JNIEnv *env,
    jobject object,
    jboolean isSubscribe,
    jint connection,
    jint id,
    jboolean subtitles,
    jboolean dialogueEnhancement,
    jboolean uiMagnifier,
    jboolean highContrastUI,
    jboolean screenReader,
    jboolean responseToUserAction,
    jboolean audioDescription,
    jboolean inVisionSigning)
{
    __android_log_print(ANDROID_LOG_INFO, "JsonRpcCallback",
                        "JSON-RPC-EXAMPLE #8: Android native called with response. Call service...");

    if (isSubscribe) {
        GetService(env, object)->RespondSubscribe(
                connection, id,
                subtitles, dialogueEnhancement, uiMagnifier, highContrastUI,
                screenReader, responseToUserAction, audioDescription, inVisionSigning);
    } else {
        GetService(env, object)->RespondUnsubscribe(
                connection, id,
                subtitles, dialogueEnhancement, uiMagnifier, highContrastUI,
                screenReader, responseToUserAction, audioDescription, inVisionSigning);
    }
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
// and, convert jstring to std::string using:
// 1. std::string str = JniUtils::MakeStdString(env, str)
extern "C"
JNIEXPORT void JNICALL
Java_org_orbtv_orblibrary_JsonRpc_nativeOnRespondFeatureSupportInfo(
    JNIEnv *env,
    jobject object,
    jint connection,
    jint id,
    jint feature,
    jstring value)
{
    __android_log_print(ANDROID_LOG_INFO, "JsonRpcCallback",
                        "JSON-RPC-EXAMPLE #8a: Android native called with response. Call service...");
    std::string valueStr = JniUtils::MakeStdString(env, value);
    GetService(env, object)->RespondFeatureSupportInfo(
            connection, id, feature, valueStr);
}


extern "C"
JNIEXPORT void JNICALL
Java_org_orbtv_orblibrary_JsonRpc_nativeOnRespondFeatureSettingsQuery(
    JNIEnv *env,
    jobject object,
    jint connection,
    jint id,
    jint feature)
{
    __android_log_print(ANDROID_LOG_INFO, "JsonRpcCallback",
                        "JSON-RPC-EXAMPLE #8a: Android native called with response. Call service...");
    GetService(env, object)->RespondFeatureSettingsQuery(
            connection, id, feature);
}

extern "C"
JNIEXPORT void JNICALL
Java_org_orbtv_orblibrary_JsonRpc_nativeOnRespondFeatureSuppress(
    JNIEnv *env,
    jobject object,
    jint connection,
    jint id,
    jint feature,
    jstring value)
{
    __android_log_print(ANDROID_LOG_INFO, "JsonRpcCallback",
                        "JSON-RPC-EXAMPLE #8a: Android native called with response. Call service...");
    std::string valueStr = JniUtils::MakeStdString(env, value);
    GetService(env, object)->RespondFeatureSuppress(
            connection, id, feature, valueStr);
}

extern "C"
JNIEXPORT void JNICALL
Java_org_orbtv_orblibrary_JsonRpc_nativeOnRespondError(
    JNIEnv *env,
    jobject object,
    jint connection,
    jint id,
    jint code,
    jstring message)
{
    __android_log_print(ANDROID_LOG_INFO, "JsonRpcCallback",
                        "JSON-RPC-EXAMPLE #8a: Android native called with response. Call service...");
    std::string messageStr = JniUtils::MakeStdString(env, message);
    GetService(env, object)->RespondError(
            connection, id, code, messageStr);
}

extern "C"
JNIEXPORT void JNICALL
Java_org_orbtv_orblibrary_JsonRpc_nativeOnRespondErrorWithMethod(
    JNIEnv *env,
    jobject object,
    jint connection,
    jint id,
    jint code,
    jstring message,
    jstring method)
{
    __android_log_print(ANDROID_LOG_INFO, "JsonRpcCallback",
                        "JSON-RPC-EXAMPLE #8a: Android native called with response. Call service...");
    std::string messageStr = JniUtils::MakeStdString(env, message);
    std::string methodStr = JniUtils::MakeStdString(env, method);
    GetService(env, object)->RespondError(
            connection, id, code, messageStr, methodStr);
}

static NetworkServices::JsonRpcService* GetService(JNIEnv *env, jobject object)
{
    return reinterpret_cast<NetworkServices::JsonRpcService *>(
        env->GetLongField(object, g_service));
}
