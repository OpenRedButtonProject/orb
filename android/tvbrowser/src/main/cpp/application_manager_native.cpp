/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

#include <jni.h>
#include <pthread.h>
#include <android/log.h>

#include <memory>
#include "application_manager.h"

#include "jni_utils.h"

#define CB_STOP_BROADCAST 0
#define CB_RESET_BROADCAST_PRESENTATION 1
#define CB_LOAD_APPLICATION 2
#define CB_SHOW_APPLICATION 3
#define CB_HIDE_APPLICATION 4
#define CB_GET_XML_AIT_CONTENTS 5
#define CB_ON_APPLICATION_LOAD_ERROR 6
#define CB_ON_TRANSITIONED_TO_BROADCAST_RELATED 7
#define CB_GET_PARENTAL_CONTROL_AGE 8
#define CB_GET_PARENTAL_CONTROL_REGION 9
#define CB_GET_PARENTAL_CONTROL_REGION3 10
#define CB_NUMBER_OF_ITEMS 11

static jfieldID gJavaManagerPointerField;
static jmethodID gCb[CB_NUMBER_OF_ITEMS];

class AndroidSessionCallback : public ApplicationManager::SessionCallback {
public:
    explicit AndroidSessionCallback(jobject callbackObject)
    {
        JNIEnv *env = JniUtils::GetEnv();
        mJavaCbObject = reinterpret_cast<jclass>(env->NewGlobalRef(callbackObject));
    }

    ~AndroidSessionCallback() override
    {
        JNIEnv *env = JniUtils::GetEnv();
        env->DeleteGlobalRef(mJavaCbObject);
    }

    void LoadApplication(uint16_t app_id, const char *entry_url) override
    {
        JNIEnv *env = JniUtils::GetEnv();
        jstring j_entry_url = env->NewStringUTF(entry_url);
        env->CallVoidMethod(mJavaCbObject, gCb[CB_LOAD_APPLICATION], app_id, j_entry_url);
        env->DeleteLocalRef(j_entry_url);
    }

    void ShowApplication() override
    {
        JNIEnv *env = JniUtils::GetEnv();
        env->CallVoidMethod(mJavaCbObject, gCb[CB_SHOW_APPLICATION]);
    }

    void HideApplication() override
    {
        JNIEnv *env = JniUtils::GetEnv();
        env->CallVoidMethod(mJavaCbObject, gCb[CB_HIDE_APPLICATION]);
    }

    void StopBroadcast() override
    {
        JNIEnv *env = JniUtils::GetEnv();
        env->CallVoidMethod(mJavaCbObject, gCb[CB_STOP_BROADCAST]);
    }

    void ResetBroadcastPresentation() override
    {
        JNIEnv *env = JniUtils::GetEnv();
        env->CallVoidMethod(mJavaCbObject, gCb[CB_RESET_BROADCAST_PRESENTATION]);
    }

    void DispatchApplicationLoadErrorEvent() override
    {
        JNIEnv *env = JniUtils::GetEnv();
        env->CallVoidMethod(mJavaCbObject, gCb[CB_ON_APPLICATION_LOAD_ERROR]);
    }

    void DispatchTransitionedToBroadcastRelatedEvent() override
    {
        JNIEnv *env = JniUtils::GetEnv();
        env->CallVoidMethod(mJavaCbObject, gCb[CB_ON_TRANSITIONED_TO_BROADCAST_RELATED]);
    }

    std::string GetXmlAitContents(const std::string &url) override
    {
        JNIEnv *env = JniUtils::GetEnv();
        jstring j_url = env->NewStringUTF(url.c_str());
        auto j_resource = reinterpret_cast<jstring>(env->CallObjectMethod(mJavaCbObject,
            gCb[CB_GET_XML_AIT_CONTENTS], j_url));
        std::string resource = JniUtils::MakeStdString(env, j_resource);
        env->DeleteLocalRef(j_resource);
        env->DeleteLocalRef(j_url);
        return resource;
    }

    int GetParentalControlAge() override
    {
        JNIEnv *env = JniUtils::GetEnv();
        return env->CallIntMethod(mJavaCbObject, gCb[CB_GET_PARENTAL_CONTROL_AGE]);
    }

    std::string GetParentalControlRegion() override
    {
        JNIEnv *env = JniUtils::GetEnv();
        auto j_region = static_cast<jstring>(env->CallObjectMethod(mJavaCbObject,
            gCb[CB_GET_PARENTAL_CONTROL_REGION]));
        std::string region = JniUtils::MakeStdString(env, j_region);
        env->DeleteLocalRef(j_region);
        return region;
    }

    std::string GetParentalControlRegion3() override
    {
        JNIEnv *env = JniUtils::GetEnv();
        auto j_region = static_cast<jstring>(env->CallObjectMethod(mJavaCbObject,
            gCb[CB_GET_PARENTAL_CONTROL_REGION3]));
        std::string region = JniUtils::MakeStdString(env, j_region);
        env->DeleteLocalRef(j_region);
        return region;
    }

private:
    jobject mJavaCbObject;
};

static ApplicationManager* GetManager(JNIEnv *env, jobject object)
{
    return reinterpret_cast<ApplicationManager *>(env->GetLongField(object,
        gJavaManagerPointerField));
}

extern "C"
JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *)
{
    JniUtils::Init(vm, JNI_VERSION_1_6);
    JNIEnv *env = JniUtils::GetEnv();

    jclass managerClass = env->FindClass("org/orbtv/tvbrowser/ApplicationManager");

    gJavaManagerPointerField = env->GetFieldID(managerClass, "mJniManagerPointerField", "J");

    // Add new callback methods here
    gCb[CB_STOP_BROADCAST] = env->GetMethodID(managerClass,
        "jniCbStopBroadcast", "()V");

    gCb[CB_RESET_BROADCAST_PRESENTATION] = env->GetMethodID(managerClass,
        "jniCbResetBroadcastPresentation", "()V");

    gCb[CB_LOAD_APPLICATION] = env->GetMethodID(managerClass,
        "jniCbLoadApplication", "(ILjava/lang/String;)V");

    gCb[CB_SHOW_APPLICATION] = env->GetMethodID(managerClass,
        "jniCbShowApplication", "()V");

    gCb[CB_HIDE_APPLICATION] = env->GetMethodID(managerClass,
        "jniCbHideApplication", "()V");

    gCb[CB_GET_XML_AIT_CONTENTS] = env->GetMethodID(managerClass,
        "jniCbGetXmlAitContents", "(Ljava/lang/String;)Ljava/lang/String;");

    gCb[CB_ON_APPLICATION_LOAD_ERROR] = env->GetMethodID(managerClass,
        "jniCbOnApplicationLoadError", "()V");

    gCb[CB_ON_TRANSITIONED_TO_BROADCAST_RELATED] = env->GetMethodID(managerClass,
        "jniCbOnTransitionedToBroadcastRelated", "()V");

    gCb[CB_GET_PARENTAL_CONTROL_AGE] = env->GetMethodID(managerClass,
        "jniCbonNativeGetParentalControlAge", "()I");

    gCb[CB_GET_PARENTAL_CONTROL_REGION] = env->GetMethodID(managerClass,
        "jniCbonNativeGetParentalControlRegion", "()Ljava/lang/String;");

    gCb[CB_GET_PARENTAL_CONTROL_REGION3] = env->GetMethodID(managerClass,
        "jniCbonNativeGetParentalControlRegion3", "()Ljava/lang/String;");

    return JNI_VERSION_1_6;
}

extern "C"
JNIEXPORT void JNICALL Java_org_orbtv_tvbrowser_ApplicationManager_jniInitialize(JNIEnv *env,
    jobject object,
    jobject javaCallbackObject)
{
    // We use a raw pointer here, as the common way to associate native objects with a Java object
    // is to store the pointer in a long. Java is responsible for calling ApplicationManager.nativeUninitialize
    auto sessionCallback = std::make_unique<AndroidSessionCallback>(javaCallbackObject);
    auto *manager = new ApplicationManager(std::move(sessionCallback));
    env->SetLongField(object, gJavaManagerPointerField, jlong(manager));
}

extern "C"
JNIEXPORT void JNICALL Java_org_orbtv_tvbrowser_ApplicationManager_jniFinalize(JNIEnv *env, jobject
    object)
{
    delete GetManager(env, object);
}

extern "C"
JNIEXPORT jboolean JNICALL Java_org_orbtv_tvbrowser_ApplicationManager_jniCreateApplication(
    JNIEnv *env, jobject object,
    jint calling_app_id, jstring j_url)
{
    std::string url = JniUtils::MakeStdString(env, j_url);
    return GetManager(env, object)->CreateApplication(calling_app_id, url);
}

extern "C"
JNIEXPORT void JNICALL Java_org_orbtv_tvbrowser_ApplicationManager_jniDestroyApplication(
    JNIEnv *env, jobject object,
    jint calling_app_id)
{
    GetManager(env, object)->DestroyApplication(calling_app_id);
}

extern "C"
JNIEXPORT void JNICALL Java_org_orbtv_tvbrowser_ApplicationManager_jniShowApplication(JNIEnv *env,
    jobject object,
    jint calling_app_id)
{
    GetManager(env, object)->ShowApplication(calling_app_id);
}

extern "C"
JNIEXPORT void JNICALL Java_org_orbtv_tvbrowser_ApplicationManager_jniHideApplication(JNIEnv *env,
    jobject object,
    jint calling_app_id)
{
    GetManager(env, object)->HideApplication(calling_app_id);
}

extern "C"
JNIEXPORT jint JNICALL Java_org_orbtv_tvbrowser_ApplicationManager_jniSetKeySetMask(JNIEnv *env,
    jobject object,
    jint calling_app_id, jint key_set_mask)
{
    return GetManager(env, object)->SetKeySetMask(calling_app_id, key_set_mask);
}

extern "C"
JNIEXPORT jint JNICALL Java_org_orbtv_tvbrowser_ApplicationManager_jniGetKeySetMask(JNIEnv *env,
    jobject object,
    jint calling_app_id)
{
    return GetManager(env, object)->GetKeySetMask(calling_app_id);
}

extern "C"
JNIEXPORT jboolean JNICALL Java_org_orbtv_tvbrowser_ApplicationManager_jniInKeySet(JNIEnv *env,
    jobject object,
    jint calling_app_id, jint key_set)
{
    return GetManager(env, object)->InKeySet(calling_app_id, key_set);
}

extern "C"
JNIEXPORT void JNICALL Java_org_orbtv_tvbrowser_ApplicationManager_jniProcessAitSection(JNIEnv *env,
    jobject object,
    jint ait_pid, jint service_id, jbyteArray data)
{
    jsize length = env->GetArrayLength(data);
    if (length > 0)
    {
        auto *buffer = new unsigned char[length];
        env->GetByteArrayRegion(data, 0, length, reinterpret_cast<jbyte *>(buffer));
        GetManager(env, object)->ProcessAitSection(ait_pid, service_id, buffer, length);
        delete[] buffer;
    }
}

extern "C"
JNIEXPORT jboolean JNICALL Java_org_orbtv_tvbrowser_ApplicationManager_jniProcessXmlAit(JNIEnv *env,
    jobject object,
    jstring j_data)
{
    std::string data = JniUtils::MakeStdString(env, j_data);
    return GetManager(env, object)->ProcessXmlAit(data);
}

extern "C"
JNIEXPORT jboolean
JNICALL Java_org_orbtv_tvbrowser_ApplicationManager_jniIsTeletextApplicationSignalled(JNIEnv *env,
    jobject object)
{
    return GetManager(env, object)->IsTeletextApplicationSignalled();
}

extern "C"
JNIEXPORT jboolean JNICALL Java_org_orbtv_tvbrowser_ApplicationManager_jniRunTeletextApplication(
    JNIEnv *env, jobject object)
{
    return GetManager(env, object)->RunTeletextApplication();
}

extern "C"
JNIEXPORT void JNICALL Java_org_orbtv_tvbrowser_ApplicationManager_jniOnNetworkAvailabilityChanged(
    JNIEnv *env, jobject object,
    jboolean available)
{
    GetManager(env, object)->OnNetworkAvailabilityChanged(available);
}

extern "C"
JNIEXPORT void JNICALL Java_org_orbtv_tvbrowser_ApplicationManager_jniOnLoadApplicationFailed(
    JNIEnv *env, jobject object,
    jint app_id)
{
    GetManager(env, object)->OnLoadApplicationFailed(app_id);
}

extern "C"
JNIEXPORT void JNICALL Java_org_orbtv_tvbrowser_ApplicationManager_jniOnApplicationPageChanged(
    JNIEnv *env, jobject object,
    jint app_id, jstring j_url)
{
    std::string url = JniUtils::MakeStdString(env, j_url);
    GetManager(env, object)->OnApplicationPageChanged(app_id, url);
}

extern "C"
JNIEXPORT void JNICALL Java_org_orbtv_tvbrowser_ApplicationManager_jniOnChannelChange(JNIEnv *env,
    jobject object,
    jint onet_id, jint trans_id, jint serv_id)
{
    GetManager(env, object)->OnChannelChanged(onet_id, trans_id, serv_id);
}

extern "C"
JNIEXPORT void JNICALL Java_org_orbtv_tvbrowser_ApplicationManager_jniOnBroadcastStopped(
    JNIEnv *env, jobject object)
{
    GetManager(env, object)->OnBroadcastStopped();
}

extern "C"
JNIEXPORT jboolean JNICALL Java_org_orbtv_tvbrowser_ApplicationManager_jniIsRequestAllowed(
    JNIEnv *env, jobject object,
    jint calling_app_id, jstring j_calling_page_url, jint method_requirement)
{
    std::string calling_page_url = JniUtils::MakeStdString(env, j_calling_page_url);
    return GetManager(env, object)->IsRequestAllowed(calling_app_id, calling_page_url,
        static_cast<ApplicationManager::MethodRequirement>(method_requirement));
}
