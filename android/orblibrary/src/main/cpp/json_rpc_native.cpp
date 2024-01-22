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


#define CB_REQUEST_NEGOTIATE_METHODS 0
#define CB_REQUEST_SUBSCRIBE_UNSUBSCRIBE 1
#define CB_RECEIVE_ERROR 2
#define CB_RECEIVE_ERROR_ALL_PARAMS 3
#define CB_REQUEST_FEATURE_SUPPORT_INFO 4
#define CB_REQUEST_FEATURE_SETTINGS_QUERY 5
#define CB_REQUEST_FEATURE_SUPPRESS 6
#define CB_REQUEST_DIALOGUE_ENHANCEMENT_OVERRIDE 7
#define CB_REQUEST_TRIGGER_RESPONSE_TO_USER_ACTION 8
#define CB_RECEIVE_INTENT_CONFIRM 9
#define CB_NOTIFY_VOICE_READY 10
#define CB_NOTIFY_STATE_MEDIA 11
#define CB_RESPOND_MESSAGE 12
#define CB_NUMBER_OF_ITEMS 13

#define LENGTH_OF_EMPTY_ID 0
#define CMD_INTENT_PAUSE 0
#define CMD_INTENT_PLAY 1
#define CMD_INTENT_FAST_FORWARD 2
#define CMD_INTENT_FAST_REVERSE 3
#define CMD_INTENT_STOP 4

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

    void RequestNegotiateMethods() override
    {
        JNIEnv *env = JniUtils::GetEnv();
        env->CallVoidMethod(
                mCallbackObject,
                g_cb[CB_REQUEST_NEGOTIATE_METHODS]);
    }

    void RequestSubscribe(
        bool subtitles, bool dialogueEnhancement,
        bool uiMagnifier, bool highContrastUI,
        bool screenReader, bool responseToUserAction,
        bool audioDescription, bool inVisionSigning) override
    {
        JNIEnv *env = JniUtils::GetEnv();
        env->CallVoidMethod(
                mCallbackObject,
                g_cb[CB_REQUEST_SUBSCRIBE_UNSUBSCRIBE],
                true,
                subtitles, dialogueEnhancement, uiMagnifier, highContrastUI,
                screenReader, responseToUserAction, audioDescription, inVisionSigning);
    }

    void RequestUnsubscribe(
        bool subtitles, bool dialogueEnhancement,
        bool uiMagnifier, bool highContrastUI,
        bool screenReader, bool responseToUserAction,
        bool audioDescription, bool inVisionSigning) override
    {
        JNIEnv *env = JniUtils::GetEnv();
        env->CallVoidMethod(
                mCallbackObject,
                g_cb[CB_REQUEST_SUBSCRIBE_UNSUBSCRIBE],
                false,
                subtitles, dialogueEnhancement, uiMagnifier, highContrastUI,
                screenReader, responseToUserAction, audioDescription, inVisionSigning);
    }

    void RequestDialogueEnhancementOverride(
        int connection,
        std::string id,
        int dialogueEnhancementGain) override
    {
        JNIEnv *env = JniUtils::GetEnv();
        jstring j_id = env->NewStringUTF(id.c_str());
        env->CallVoidMethod(
            mCallbackObject,
            g_cb[CB_REQUEST_DIALOGUE_ENHANCEMENT_OVERRIDE],
            connection, j_id, dialogueEnhancementGain);
        env->DeleteLocalRef(j_id);
    }

    void RequestTriggerResponseToUserAction(
            int connection,
            std::string id,
            std::string magnitude) override
    {
        JNIEnv *env = JniUtils::GetEnv();
        jstring j_id = env->NewStringUTF(id.c_str());
        jstring j_magnitude = env->NewStringUTF(magnitude.c_str());
        env->CallVoidMethod(
                mCallbackObject,
                g_cb[CB_REQUEST_TRIGGER_RESPONSE_TO_USER_ACTION],
                connection, j_id, j_magnitude);
        env->DeleteLocalRef(j_id);
        env->DeleteLocalRef(j_magnitude);
    }

    void RequestFeatureSupportInfo(
        int connection,
        std::string id,
        int feature) override
    {
        JNIEnv *env = JniUtils::GetEnv();
        jstring j_id = env->NewStringUTF(id.c_str());
        env->CallVoidMethod(
                mCallbackObject,
                g_cb[CB_REQUEST_FEATURE_SUPPORT_INFO],
                connection, j_id, feature);
        env->DeleteLocalRef(j_id);
    }

    void RequestFeatureSettingsQuery(
        int connection,
        std::string id,
        int feature) override
    {
        JNIEnv *env = JniUtils::GetEnv();
        jstring j_id = env->NewStringUTF(id.c_str());
        env->CallVoidMethod(
                mCallbackObject,
                g_cb[CB_REQUEST_FEATURE_SETTINGS_QUERY],
                connection, j_id, feature);
        env->DeleteLocalRef(j_id);
    }

    void RequestFeatureSuppress(
        int connection,
        std::string id,
        int feature) override
    {
        JNIEnv *env = JniUtils::GetEnv();
        jstring j_id = env->NewStringUTF(id.c_str());
        env->CallVoidMethod(
                mCallbackObject,
                g_cb[CB_REQUEST_FEATURE_SUPPRESS],
                connection, j_id, feature);
        env->DeleteLocalRef(j_id);
    }

    void ReceiveIntentConfirm(
            int connection,
            std::string id,
            std::string method) override
    {
        JNIEnv *env = JniUtils::GetEnv();
        jstring j_id = env->NewStringUTF(id.c_str());
        jstring j_method = env->NewStringUTF(method.c_str());
        env->CallVoidMethod(
                mCallbackObject,
                g_cb[CB_RECEIVE_INTENT_CONFIRM],
                connection, j_id, j_method);
        env->DeleteLocalRef(j_id);
    }

    void NotifyVoiceReady(
        bool isReady) override
    {
        JNIEnv *env = JniUtils::GetEnv();
        env->CallVoidMethod(
                mCallbackObject,
                g_cb[CB_NOTIFY_VOICE_READY],
                isReady);
    }

    void NotifyStateMedia(
        std::string state) override
    {
        JNIEnv *env = JniUtils::GetEnv();
        jstring j_state = env->NewStringUTF(state.c_str());
        env->CallVoidMethod(
                mCallbackObject,
                g_cb[CB_NOTIFY_STATE_MEDIA],
                j_state);
        env->DeleteLocalRef(j_state);
    }

    void RespondMessage(
            std::string info) override
    {
        JNIEnv *env = JniUtils::GetEnv();
        jstring j_info = env->NewStringUTF(info.c_str());
        env->CallVoidMethod(
                mCallbackObject,
                g_cb[CB_RESPOND_MESSAGE],
                j_info);
        env->DeleteLocalRef(j_info);
    }

    void ReceiveError(
        int code,
        std::string message) override
    {
        JNIEnv *env = JniUtils::GetEnv();
        jstring j_message = env->NewStringUTF(message.c_str());
        env->CallVoidMethod(
                mCallbackObject,
                g_cb[CB_RECEIVE_ERROR],
                code, j_message);
        env->DeleteLocalRef(j_message);
    }

    void ReceiveError(
            int code,
            std::string message,
            std::string method,
            std::string data) override
    {
        JNIEnv *env = JniUtils::GetEnv();
        jstring j_message = env->NewStringUTF(message.c_str());
        jstring j_method = env->NewStringUTF(method.c_str());
        jstring j_data = env->NewStringUTF(data.c_str());
        env->CallVoidMethod(
                mCallbackObject,
                g_cb[CB_RECEIVE_ERROR_ALL_PARAMS],
                code, j_message, j_method, j_data);
        env->DeleteLocalRef(j_message);
        env->DeleteLocalRef(j_method);
        env->DeleteLocalRef(j_data);
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

    g_cb[CB_REQUEST_NEGOTIATE_METHODS] = env->GetMethodID(managerClass,
        "onRequestNegotiateMethods", "()V");
    g_cb[CB_REQUEST_SUBSCRIBE_UNSUBSCRIBE] = env->GetMethodID(managerClass,
        "onRequestSubscribe", "(ZZZZZZZZZ)V");
    g_cb[CB_REQUEST_FEATURE_SUPPORT_INFO] = env->GetMethodID(managerClass,
         "onRequestFeatureSupportInfo", "(ILjava/lang/String;I)V");
    g_cb[CB_REQUEST_FEATURE_SETTINGS_QUERY] = env->GetMethodID(managerClass,
         "onRequestFeatureSettingsQuery", "(ILjava/lang/String;I)V");
    g_cb[CB_REQUEST_FEATURE_SUPPRESS] = env->GetMethodID(managerClass,
         "onRequestFeatureSuppress", "(ILjava/lang/String;I)V");
    g_cb[CB_REQUEST_DIALOGUE_ENHANCEMENT_OVERRIDE] = env->GetMethodID(managerClass,
         "onRequestDialogueEnhancementOverride", "(ILjava/lang/String;I)V");
    g_cb[CB_REQUEST_TRIGGER_RESPONSE_TO_USER_ACTION] = env->GetMethodID(managerClass,
         "onRequestTriggerResponseToUserAction", "(ILjava/lang/String;Ljava/lang/String;)V");
    g_cb[CB_RECEIVE_INTENT_CONFIRM] = env->GetMethodID(managerClass,
         "onReceiveIntentConfirm", "(ILjava/lang/String;Ljava/lang/String;)V");
    g_cb[CB_NOTIFY_VOICE_READY] = env->GetMethodID(managerClass,
         "onNotifyVoiceReady", "(Z)V");
    g_cb[CB_NOTIFY_STATE_MEDIA] = env->GetMethodID(managerClass,
         "onNotifyStateMedia", "(Ljava/lang/String;)V");
    g_cb[CB_RESPOND_MESSAGE] = env->GetMethodID(managerClass,
         "onRespondMessage", "(Ljava/lang/String;)V");
    g_cb[CB_RECEIVE_ERROR] = env->GetMethodID(managerClass,
         "onReceiveError", "(ILjava/lang/String;)V");
    g_cb[CB_RECEIVE_ERROR_ALL_PARAMS] = env->GetMethodID(managerClass,
               "onReceiveError", "(ILjava/lang/String;Ljava/lang/String;Ljava/lang/String;)V");
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
    service->Start();
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
JNIEXPORT void JNICALL Java_org_orbtv_orblibrary_JsonRpc_nativeOnRespondDialogueEnhancementOverride(
    JNIEnv *env,
    jobject object,
    jint connection,
    jstring id,
    jint dialogueEnhancementGain)
{
    std::string idStr = JniUtils::MakeStdString(env, id);
    GetService(env, object)->RespondDialogueEnhancementOverride(
        connection, idStr, dialogueEnhancementGain);
}

extern "C"
JNIEXPORT void JNICALL Java_org_orbtv_orblibrary_JsonRpc_nativeOnRespondTriggerResponseToUserAction(
        JNIEnv *env,
        jobject object,
        jint connection,
        jstring id,
        jboolean actioned)
{
    std::string idStr = JniUtils::MakeStdString(env, id);
    GetService(env, object)->RespondTriggerResponseToUserAction(
            connection, idStr, actioned);
}

extern "C"
JNIEXPORT void JNICALL
Java_org_orbtv_orblibrary_JsonRpc_nativeOnRespondFeatureSupportInfo(
    JNIEnv *env,
    jobject object,
    jint connection,
    jstring id,
    jint feature,
    jstring value)
{
    std::string idStr = JniUtils::MakeStdString(env, id);
    std::string valueStr = JniUtils::MakeStdString(env, value);
    GetService(env, object)->RespondFeatureSupportInfo(
            connection, idStr, feature, valueStr);
}

extern "C"
JNIEXPORT void JNICALL
Java_org_orbtv_orblibrary_JsonRpc_nativeOnRespondFeatureSuppress(
    JNIEnv *env,
    jobject object,
    jint connection,
    jstring id,
    jint feature,
    jstring value)
{
    std::string idStr = JniUtils::MakeStdString(env, id);
    std::string valueStr = JniUtils::MakeStdString(env, value);
    GetService(env, object)->RespondFeatureSuppress(
            connection, idStr, feature, valueStr);
}

extern "C"
JNIEXPORT void JNICALL
Java_org_orbtv_orblibrary_JsonRpc_nativeOnRespondError(
    JNIEnv *env,
    jobject object,
    jint connection,
    jstring id,
    jint code,
    jstring message)
{
    std::string idStr = JniUtils::MakeStdString(env, id);
    std::string messageStr = JniUtils::MakeStdString(env, message);
    GetService(env, object)->RespondError(
            connection, idStr, code, messageStr);
}

extern "C"
JNIEXPORT void JNICALL
Java_org_orbtv_orblibrary_JsonRpc_nativeOnRespondErrorWithData(
    JNIEnv *env,
    jobject object,
    jint connection,
    jstring id,
    jint code,
    jstring message,
    jstring data)
{
    std::string idStr = JniUtils::MakeStdString(env, id);
    std::string messageStr = JniUtils::MakeStdString(env, message);
    std::string dataStr = JniUtils::MakeStdString(env, data);
    GetService(env, object)->RespondError(
            connection, idStr, code, messageStr, dataStr);
}

extern "C"
JNIEXPORT void JNICALL
Java_org_orbtv_orblibrary_JsonRpc_nativeOnQuerySubtitles(
    JNIEnv *env,
    jobject object,
    jint connection,
    jstring id,
    jboolean enabled,
    jint size,
    jstring fontFamily,
    jstring textColour,
    jint textOpacity,
    jstring edgeType,
    jstring edgeColour,
    jstring backgroundColour,
    jint backgroundOpacity,
    jstring windowColour,
    jint windowOpacity,
    jstring language)
{
    std::string idStr = JniUtils::MakeStdString(env, id);
    std::string fontFamilyStr = JniUtils::MakeStdString(env, fontFamily);
    std::string textColourStr = JniUtils::MakeStdString(env, textColour);
    std::string edgeTypeStr = JniUtils::MakeStdString(env, edgeType);
    std::string edgeColourStr = JniUtils::MakeStdString(env, edgeColour);
    std::string backgroundColourStr = JniUtils::MakeStdString(env, backgroundColour);
    std::string windowColourStr = JniUtils::MakeStdString(env, windowColour);
    std::string languageStr = JniUtils::MakeStdString(env, language);
    if (idStr.length() == LENGTH_OF_EMPTY_ID) {

        GetService(env, object)->NotifySubtitles(
                enabled, size, fontFamilyStr, textColourStr, textOpacity,
                edgeTypeStr, edgeColourStr, backgroundColourStr, backgroundOpacity,
                windowColourStr, windowOpacity, languageStr);
    }
    else {
        GetService(env, object)->RespondFeatureSettingsSubtitles(
                connection, idStr,
                enabled, size, fontFamilyStr, textColourStr, textOpacity,
                edgeTypeStr, edgeColourStr, backgroundColourStr, backgroundOpacity,
                windowColourStr, windowOpacity, languageStr);
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_org_orbtv_orblibrary_JsonRpc_nativeOnQueryDialogueEnhancement(
    JNIEnv *env,
    jobject object,
    jint connection,
    jstring id,
    jint gainPreference,
    jint gain,
    jint limitMin,
    jint limitMax)
{
    std::string idStr = JniUtils::MakeStdString(env, id);
    if (idStr.length() == LENGTH_OF_EMPTY_ID) {
        GetService(env, object)->NotifyDialogueEnhancement(
                gainPreference, gain, limitMin, limitMax);
    }
    else {
        GetService(env, object)->RespondFeatureSettingsDialogueEnhancement(
                connection, idStr,
                gainPreference, gain, limitMin, limitMax);
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_org_orbtv_orblibrary_JsonRpc_nativeOnQueryUIMagnifier(
    JNIEnv *env,
    jobject object,
    jint connection,
    jstring id,
    jboolean enabled,
    jstring magType)
{
    std::string idStr = JniUtils::MakeStdString(env, id);
    std::string magTypeStr = JniUtils::MakeStdString(env, magType);
    if (idStr.length() == LENGTH_OF_EMPTY_ID) {
        GetService(env, object)->NotifyUIMagnifier(enabled, magTypeStr);
    }
    else {
        GetService(env, object)->RespondFeatureSettingsUIMagnifier(
                connection, idStr, enabled, magTypeStr);
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_org_orbtv_orblibrary_JsonRpc_nativeOnQueryHighContrastUI(
    JNIEnv *env,
    jobject object,
    jint connection,
    jstring id,
    jboolean enabled,
    jstring hcType)
{
    std::string idStr = JniUtils::MakeStdString(env, id);
    std::string hcTypeStr = JniUtils::MakeStdString(env, hcType);
    if (idStr.length() == LENGTH_OF_EMPTY_ID) {
        GetService(env, object)->NotifyHighContrastUI(enabled, hcTypeStr);
    }
    else {
        GetService(env, object)->RespondFeatureSettingsHighContrastUI(
                connection, idStr, enabled, hcTypeStr);
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_org_orbtv_orblibrary_JsonRpc_nativeOnQueryScreenReader(
    JNIEnv *env,
    jobject object,
    jint connection,
    jstring id,
    jboolean enabled,
    jint speed,
    jstring voice,
    jstring language)
{
    std::string idStr = JniUtils::MakeStdString(env, id);
    std::string voiceStr = JniUtils::MakeStdString(env, voice);
    std::string languageStr = JniUtils::MakeStdString(env, language);
    if (idStr.length() == LENGTH_OF_EMPTY_ID) {
        GetService(env, object)->NotifyScreenReader(
                enabled, speed, voiceStr, languageStr);
    }
    else {
        GetService(env, object)->RespondFeatureSettingsScreenReader(
                connection, idStr,
                enabled, speed, voiceStr, languageStr);
    }

}

extern "C"
JNIEXPORT void JNICALL
Java_org_orbtv_orblibrary_JsonRpc_nativeOnQueryResponseToUserAction(
    JNIEnv *env,
    jobject object,
    jint connection,
    jstring id,
    jboolean enabled,
    jstring type)
{
    std::string idStr = JniUtils::MakeStdString(env, id);
    std::string typeStr = JniUtils::MakeStdString(env, type);
    if (idStr.length() == LENGTH_OF_EMPTY_ID) {
        GetService(env, object)->NotifyResponseToUserAction(enabled, typeStr);
    }
    else {
        GetService(env, object)->RespondFeatureSettingsResponseToUserAction(
                connection, idStr, enabled, typeStr);
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_org_orbtv_orblibrary_JsonRpc_nativeOnQueryAudioDescription(
    JNIEnv *env,
    jobject object,
    jint connection,
    jstring id,
    jboolean enabled,
    jint gainPreference,
    jint panAzimuthPreference)
{
    std::string idStr = JniUtils::MakeStdString(env, id);
    if (idStr.length() == LENGTH_OF_EMPTY_ID) {
        GetService(env, object)->NotifyAudioDescription(
                enabled, gainPreference, panAzimuthPreference);
    }
    else {
        GetService(env, object)->RespondFeatureSettingsAudioDescription(
                connection, idStr,
                enabled, gainPreference, panAzimuthPreference);
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_org_orbtv_orblibrary_JsonRpc_nativeOnQueryInVisionSigning(
    JNIEnv *env,
    jobject object,
    jint connection,
    jstring id,
    jboolean enabled)
{
    std::string idStr = JniUtils::MakeStdString(env, id);
    if (idStr.length() == LENGTH_OF_EMPTY_ID) {
        GetService(env, object)->NotifyInVisionSigning(
                enabled);
    }
    else {
        GetService(env, object)->RespondFeatureSettingsInVisionSigning(
                connection, idStr, enabled);
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_org_orbtv_orblibrary_JsonRpc_nativeOnSendIntentMediaBasics(
    JNIEnv *env,
    jobject object,
    jint cmd)
{
    switch(cmd) {
        case CMD_INTENT_PAUSE:
            GetService(env, object)->SendIntentMediaPause();
            break;
        case CMD_INTENT_PLAY:
            GetService(env, object)->SendIntentMediaPlay();
            break;
        case CMD_INTENT_FAST_FORWARD:
            GetService(env, object)->SendIntentMediaFastForward();
            break;
        case CMD_INTENT_FAST_REVERSE:
            GetService(env, object)->SendIntentMediaFastReverse();
            break;
        case CMD_INTENT_STOP:
            GetService(env, object)->SendIntentMediaStop();
            break;
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_org_orbtv_orblibrary_JsonRpc_nativeOnSendIntentMediaSeekContent(
    JNIEnv *env,
    jobject object,
    jstring anchor,
    int offset)
{
    std::string anchorStr = JniUtils::MakeStdString(env, anchor);
    GetService(env, object)->SendIntentMediaSeekContent(anchorStr, offset);
}

extern "C"
JNIEXPORT void JNICALL
Java_org_orbtv_orblibrary_JsonRpc_nativeOnSendIntentMediaSeekRelative(
    JNIEnv *env,
    jobject object,
    int offset)
{
    GetService(env, object)->SendIntentMediaSeekRelative(offset);
}

extern "C"
JNIEXPORT void JNICALL
Java_org_orbtv_orblibrary_JsonRpc_nativeOnSendIntentMediaSeekLive(
    JNIEnv *env,
    jobject object,
    int offset)
{
    GetService(env, object)->SendIntentMediaSeekLive(offset);
}

extern "C"
JNIEXPORT void JNICALL
Java_org_orbtv_orblibrary_JsonRpc_nativeOnSendIntentMediaSeekWallclock(
    JNIEnv *env,
    jobject object,
    jstring dateTime)
{
    std::string dateTimeStr = JniUtils::MakeStdString(env, dateTime);
    GetService(env, object)->SendIntentMediaSeekWallclock(dateTimeStr);
}

extern "C"
JNIEXPORT void JNICALL
Java_org_orbtv_orblibrary_JsonRpc_nativeOnSendIntentSearch(
    JNIEnv *env,
    jobject object,
    jstring query)
{
    std::string queryStr = JniUtils::MakeStdString(env, query);
    GetService(env, object)->SendIntentSearch(queryStr);
}

extern "C"
JNIEXPORT void JNICALL
Java_org_orbtv_orblibrary_JsonRpc_nativeOnSendIntentDisplay(
    JNIEnv *env,
    jobject object,
    jstring mediaId)
{
    std::string mediaIdStr = JniUtils::MakeStdString(env, mediaId);
    GetService(env, object)->SendIntentDisplay(mediaIdStr);
}

extern "C"
JNIEXPORT void JNICALL
Java_org_orbtv_orblibrary_JsonRpc_nativeOnSendIntentPlayback(
    JNIEnv *env,
    jobject object,
    jstring mediaId,
    jstring anchor,
    jint offset)
{
    std::string mediaIdStr = JniUtils::MakeStdString(env, mediaId);
    std::string anchorStr = JniUtils::MakeStdString(env, anchor);
    GetService(env, object)->SendIntentPlayback(mediaIdStr, anchorStr, offset);
}

extern "C"
JNIEXPORT void JNICALL
Java_org_orbtv_orblibrary_JsonRpc_nativeOnVoiceRequestDescription(
        JNIEnv *env,
        jobject object)
{
    GetService(env, object)->VoiceRequestDescription();
}

static NetworkServices::JsonRpcService* GetService(JNIEnv *env, jobject object)
{
    return reinterpret_cast<NetworkServices::JsonRpcService *>(
        env->GetLongField(object, g_service));
}
