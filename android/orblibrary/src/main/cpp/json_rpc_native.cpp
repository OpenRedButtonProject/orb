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
#define CB_NOTIFY_STATE_MEDIA_ALL_VALUES 12
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

    void RequestNegotiateMethods(
        int connection,
        std::string id,
        std::string terminalToApp,
        std::string appToTerminal) override
    {
        JNIEnv *env = JniUtils::GetEnv();
        jstring j_id = env->NewStringUTF(id.c_str());
        jstring j_terminalToApp = env->NewStringUTF(terminalToApp.c_str());
        jstring j_appToTerminal = env->NewStringUTF(appToTerminal.c_str());
        env->CallVoidMethod(
                mCallbackObject,
                g_cb[CB_REQUEST_NEGOTIATE_METHODS],
                connection, j_id, j_terminalToApp, j_appToTerminal);
        env->DeleteLocalRef(j_id);
        env->DeleteLocalRef(j_terminalToApp);
        env->DeleteLocalRef(j_appToTerminal);
    }

    void RequestSubscribe(
        int connection,
        std::string id,
        bool subtitles, bool dialogueEnhancement,
        bool uiMagnifier, bool highContrastUI,
        bool screenReader, bool responseToUserAction,
        bool audioDescription, bool inVisionSigning) override
    {
        JNIEnv *env = JniUtils::GetEnv();
        jstring j_id = env->NewStringUTF(id.c_str());
        env->CallVoidMethod(
                mCallbackObject,
                g_cb[CB_REQUEST_SUBSCRIBE_UNSUBSCRIBE],
                true, connection, j_id,
                subtitles, dialogueEnhancement, uiMagnifier, highContrastUI,
                screenReader, responseToUserAction, audioDescription, inVisionSigning);
        env->DeleteLocalRef(j_id);
    }

    void RequestUnsubscribe(
        int connection,
        std::string id,
        bool subtitles, bool dialogueEnhancement,
        bool uiMagnifier, bool highContrastUI,
        bool screenReader, bool responseToUserAction,
        bool audioDescription, bool inVisionSigning) override
    {
        JNIEnv *env = JniUtils::GetEnv();
        jstring j_id = env->NewStringUTF(id.c_str());
        env->CallVoidMethod(
                mCallbackObject,
                g_cb[CB_REQUEST_SUBSCRIBE_UNSUBSCRIBE],
                false, connection, j_id,
                subtitles, dialogueEnhancement, uiMagnifier, highContrastUI,
                screenReader, responseToUserAction, audioDescription, inVisionSigning);
        env->DeleteLocalRef(j_id);
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
        int connection,
        bool isReady) override
    {
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
        JNIEnv *env = JniUtils::GetEnv();
        jstring j_state = env->NewStringUTF(state.c_str());
        jstring j_kind = env->NewStringUTF(kind.c_str());
        jstring j_type = env->NewStringUTF( type.c_str());
        jstring j_currentTime = env->NewStringUTF(currentTime.c_str());
        jstring j_rangeStart = env->NewStringUTF(rangeStart.c_str());
        jstring j_rangeEnd = env->NewStringUTF(rangeEnd.c_str());
        jstring j_mediaId = env->NewStringUTF(mediaId.c_str());
        jstring j_title = env->NewStringUTF(title.c_str());
        jstring j_secTitle = env->NewStringUTF( secTitle.c_str());
        jstring j_synopsis = env->NewStringUTF(synopsis.c_str());
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

    void ReceiveError(
        int connection,
        std::string id,
        int code,
        std::string message) override
    {
        JNIEnv *env = JniUtils::GetEnv();
        jstring j_id = env->NewStringUTF(id.c_str());
        jstring j_message = env->NewStringUTF(message.c_str());
        env->CallVoidMethod(
                mCallbackObject,
                g_cb[CB_RECEIVE_ERROR],
                connection, j_id, code, j_message);
        env->DeleteLocalRef(j_id);
        env->DeleteLocalRef(j_message);
    }

    void ReceiveError(
            int connection,
            std::string id,
            int code,
            std::string message,
            std::string method,
            std::string data) override
    {
        JNIEnv *env = JniUtils::GetEnv();
        jstring j_id = env->NewStringUTF(id.c_str());
        jstring j_message = env->NewStringUTF(message.c_str());
        jstring j_method = env->NewStringUTF(method.c_str());
        jstring j_data = env->NewStringUTF(data.c_str());
        env->CallVoidMethod(
                mCallbackObject,
                g_cb[CB_RECEIVE_ERROR_ALL_PARAMS],
                connection, j_id, code, j_message, j_method, j_data);
        env->DeleteLocalRef(j_id);
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
        "onRequestNegotiateMethods", "(ILjava/lang/String;Ljava/lang/String;Ljava/lang/String;)V");
    g_cb[CB_REQUEST_SUBSCRIBE_UNSUBSCRIBE] = env->GetMethodID(managerClass,
        "onRequestSubscribe", "(ZILjava/lang/String;ZZZZZZZZ)V");
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
         "onReceiveError", "(ILjava/lang/String;ILjava/lang/String;)V");
    g_cb[CB_RECEIVE_ERROR_ALL_PARAMS] = env->GetMethodID(managerClass,
               "onReceiveError", "(ILjava/lang/String;ILjava/lang/String;Ljava/lang/String;Ljava/lang/String;)V");
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
JNIEXPORT void JNICALL
Java_org_orbtv_orblibrary_JsonRpc_nativeOnRespondNegotiateMethods(
    JNIEnv *env,
    jobject object,
    jint connection,
    jstring id,
    jstring terminalToApp,
    jstring appToTerminal)
{
    std::string idStr = JniUtils::MakeStdString(env, id);
    std::string terminalToAppStr = JniUtils::MakeStdString(env, terminalToApp);
    std::string appToTerminalStr = JniUtils::MakeStdString(env, appToTerminal);
    GetService(env, object)->RespondNegotiateMethods(
            connection, idStr, terminalToAppStr, appToTerminalStr);
}

extern "C"
JNIEXPORT void JNICALL
Java_org_orbtv_orblibrary_JsonRpc_nativeOnRespondSubscribe(
    JNIEnv *env,
    jobject object,
    jboolean isSubscribe,
    jint connection,
    jstring id,
    jboolean subtitles,
    jboolean dialogueEnhancement,
    jboolean uiMagnifier,
    jboolean highContrastUI,
    jboolean screenReader,
    jboolean responseToUserAction,
    jboolean audioDescription,
    jboolean inVisionSigning)
{
    std::string idStr = JniUtils::MakeStdString(env, id);

    if (isSubscribe) {
        GetService(env, object)->RespondSubscribe(
                connection, idStr,
                subtitles, dialogueEnhancement, uiMagnifier, highContrastUI,
                screenReader, responseToUserAction, audioDescription, inVisionSigning);
    } else {
        GetService(env, object)->RespondUnsubscribe(
                connection, idStr,
                subtitles, dialogueEnhancement, uiMagnifier, highContrastUI,
                screenReader, responseToUserAction, audioDescription, inVisionSigning);
    }
}

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
    jint cmd,
    jint connection,
    jstring id,
    jstring origin)
{
    std::string idStr = JniUtils::MakeStdString(env, id);
    std::string originStr = JniUtils::MakeStdString(env, origin);
    switch(cmd) {
        case CMD_INTENT_PAUSE:
            GetService(env, object)->SendIntentMediaPause(connection, idStr, originStr);
            break;
        case CMD_INTENT_PLAY:
            GetService(env, object)->SendIntentMediaPlay(connection, idStr, originStr);
            break;
        case CMD_INTENT_FAST_FORWARD:
            GetService(env, object)->SendIntentMediaFastForward(connection, idStr, originStr);
            break;
        case CMD_INTENT_FAST_REVERSE:
            GetService(env, object)->SendIntentMediaFastReverse(connection, idStr, originStr);
            break;
        case CMD_INTENT_STOP:
            GetService(env, object)->SendIntentMediaStop(connection, idStr, originStr);
            break;
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_org_orbtv_orblibrary_JsonRpc_nativeOnSendIntentMediaSeekContent(
    JNIEnv *env,
    jobject object,
    jint connection,
    jstring id,
    jstring origin,
    jstring anchor,
    int offset)
{
    std::string idStr = JniUtils::MakeStdString(env, id);
    std::string originStr = JniUtils::MakeStdString(env, origin);
    std::string anchorStr = JniUtils::MakeStdString(env, anchor);
    GetService(env, object)->SendIntentMediaSeekContent(connection, idStr, originStr, anchorStr, offset);
}

extern "C"
JNIEXPORT void JNICALL
Java_org_orbtv_orblibrary_JsonRpc_nativeOnSendIntentMediaSeekRelative(
    JNIEnv *env,
    jobject object,
    jint connection,
    jstring id,
    jstring origin,
    int offset)
{
    std::string idStr = JniUtils::MakeStdString(env, id);
    std::string originStr = JniUtils::MakeStdString(env, origin);
    GetService(env, object)->SendIntentMediaSeekRelative(connection, idStr, originStr, offset);
}

extern "C"
JNIEXPORT void JNICALL
Java_org_orbtv_orblibrary_JsonRpc_nativeOnSendIntentMediaSeekLive(
    JNIEnv *env,
    jobject object,
    jint connection,
    jstring id,
    jstring origin,
    int offset)
{
    std::string idStr = JniUtils::MakeStdString(env, id);
    std::string originStr = JniUtils::MakeStdString(env, origin);
    GetService(env, object)->SendIntentMediaSeekLive(connection, idStr, originStr, offset);
}

extern "C"
JNIEXPORT void JNICALL
Java_org_orbtv_orblibrary_JsonRpc_nativeOnSendIntentMediaSeekWallclock(
    JNIEnv *env,
    jobject object,
    jint connection,
    jstring id,
    jstring origin,
    jstring dateTime)
{
    std::string idStr = JniUtils::MakeStdString(env, id);
    std::string originStr = JniUtils::MakeStdString(env, origin);
    std::string dateTimeStr = JniUtils::MakeStdString(env, dateTime);
    GetService(env, object)->SendIntentMediaSeekWallclock(connection, idStr, originStr, dateTimeStr);
}

extern "C"
JNIEXPORT void JNICALL
Java_org_orbtv_orblibrary_JsonRpc_nativeOnSendIntentSearch(
    JNIEnv *env,
    jobject object,
    jint connection,
    jstring id,
    jstring origin,
    jstring query)
{
    std::string idStr = JniUtils::MakeStdString(env, id);
    std::string originStr = JniUtils::MakeStdString(env, origin);
    std::string queryStr = JniUtils::MakeStdString(env, query);
    GetService(env, object)->SendIntentSearch(connection, idStr, originStr, queryStr);
}

extern "C"
JNIEXPORT void JNICALL
Java_org_orbtv_orblibrary_JsonRpc_nativeOnSendIntentDisplay(
    JNIEnv *env,
    jobject object,
    jint connection,
    jstring id,
    jstring origin,
    jstring mediaId)
{
    std::string idStr = JniUtils::MakeStdString(env, id);
    std::string originStr = JniUtils::MakeStdString(env, origin);
    std::string mediaIdStr = JniUtils::MakeStdString(env, mediaId);
    GetService(env, object)->SendIntentDisplay(connection, idStr, originStr, mediaIdStr);
}

extern "C"
JNIEXPORT void JNICALL
Java_org_orbtv_orblibrary_JsonRpc_nativeOnSendIntentPlayback(
    JNIEnv *env,
    jobject object,
    jint connection,
    jstring id,
    jstring origin,
    jstring mediaId,
    jstring anchor,
    jint offset)
{
    std::string idStr = JniUtils::MakeStdString(env, id);
    std::string originStr = JniUtils::MakeStdString(env, origin);
    std::string mediaIdStr = JniUtils::MakeStdString(env, mediaId);
    std::string anchorStr = JniUtils::MakeStdString(env, anchor);
    GetService(env, object)->SendIntentPlayback(connection, idStr, originStr, mediaIdStr, anchorStr, offset);
}

static NetworkServices::JsonRpcService* GetService(JNIEnv *env, jobject object)
{
    return reinterpret_cast<NetworkServices::JsonRpcService *>(
        env->GetLongField(object, g_service));
}
