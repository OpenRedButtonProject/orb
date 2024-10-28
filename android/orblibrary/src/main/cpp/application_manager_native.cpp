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

#include "application_manager_native.h"

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
#define CB_ON_APPLICATION_TYPE_UPDATED 11
#define CB_NUMBER_OF_ITEMS 12

static jfieldID gJavaManagerPointerField;
static jmethodID gCb[CB_NUMBER_OF_ITEMS];

static ApplicationManager* GetManager(JNIEnv *env, jobject object);

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
        env->CallVoidMethod(mJavaCbObject, gCb[CB_LOAD_APPLICATION], app_id, j_entry_url, nullptr);
        env->DeleteLocalRef(j_entry_url);
    }

    void LoadApplication(uint16_t app_id, const char *entry_url, int array_size, const std::vector<uint16_t> graphics) override
    {
        if (array_size == 0) {
            LoadApplication(app_id, entry_url);
        }
        JNIEnv *env = JniUtils::GetEnv();
        jstring j_entry_url = env->NewStringUTF(entry_url);
        jintArray j_array = env->NewIntArray(array_size);
        jint *int_ptr = static_cast<jint *>(calloc(array_size, sizeof(jint)));
        if (int_ptr == nullptr)
        {
            LoadApplication(app_id, entry_url);
        }
        else
        {
            for (int i = 0; i < array_size; i++)
            {
                int_ptr[i] = graphics[i];
            }
            env->SetIntArrayRegion(j_array, 0, array_size, int_ptr);
            env->CallVoidMethod(mJavaCbObject, gCb[CB_LOAD_APPLICATION], app_id, j_entry_url, j_array);
        }
        env->DeleteLocalRef(j_entry_url);
        env->DeleteLocalRef(j_array);
        free(int_ptr);
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

    void DispatchApplicationSchemeUpdatedEvent(const std::string &scheme) {
        JNIEnv *env = JniUtils::GetEnv();
        jstring j_appType = env->NewStringUTF(scheme.c_str());
        env->CallVoidMethod(mJavaCbObject, gCb[CB_ON_APPLICATION_TYPE_UPDATED], j_appType);
        env->DeleteLocalRef(j_appType);
    }

private:
    jobject mJavaCbObject;
};

void InitialiseApplicationManagerNative()
{
    JNIEnv *env = JniUtils::GetEnv();
    jclass managerClass = env->FindClass("org/orbtv/orblibrary/ApplicationManager");
    gJavaManagerPointerField = env->GetFieldID(managerClass, "mJniManagerPointerField", "J");
    // Add new callback methods here
    gCb[CB_STOP_BROADCAST] = env->GetMethodID(managerClass,
        "jniCbStopBroadcast", "()V");
    gCb[CB_RESET_BROADCAST_PRESENTATION] = env->GetMethodID(managerClass,
        "jniCbResetBroadcastPresentation", "()V");
    gCb[CB_LOAD_APPLICATION] = env->GetMethodID(managerClass,
        "jniCbLoadApplication", "(ILjava/lang/String;[I)V");
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
    gCb[CB_ON_APPLICATION_TYPE_UPDATED] = env->GetMethodID(managerClass,
                                                            "jniCbonApplicationSchemeUpdated", "(Ljava/lang/String;)V");
}

extern "C"
JNIEXPORT void JNICALL Java_org_orbtv_orblibrary_ApplicationManager_jniInitialize(JNIEnv *env,
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
JNIEXPORT jint JNICALL Java_org_orbtv_orblibrary_ApplicationManager_jniGetOrbHbbTVVersion(JNIEnv *env,
    jobject object)
{
    return ORB_HBBTV_VERSION;
}

extern "C"
JNIEXPORT void JNICALL Java_org_orbtv_orblibrary_ApplicationManager_jniFinalize(JNIEnv *env, jobject
    object)
{
    delete GetManager(env, object);
}

extern "C"
JNIEXPORT jboolean JNICALL Java_org_orbtv_orblibrary_ApplicationManager_jniCreateApplication(
    JNIEnv *env, jobject object,
    jint calling_app_id, jstring j_url)
{
    std::string url = JniUtils::MakeStdString(env, j_url);
    return GetManager(env, object)->CreateApplication(calling_app_id, url);
}

extern "C"
JNIEXPORT void JNICALL Java_org_orbtv_orblibrary_ApplicationManager_jniDestroyApplication(
    JNIEnv *env, jobject object,
    jint calling_app_id)
{
    GetManager(env, object)->DestroyApplication(calling_app_id);
}

extern "C"
JNIEXPORT void JNICALL Java_org_orbtv_orblibrary_ApplicationManager_jniShowApplication(JNIEnv *env,
    jobject object,
    jint calling_app_id)
{
    GetManager(env, object)->ShowApplication(calling_app_id);
}

extern "C"
JNIEXPORT void JNICALL Java_org_orbtv_orblibrary_ApplicationManager_jniHideApplication(JNIEnv *env,
    jobject object,
    jint calling_app_id)
{
    GetManager(env, object)->HideApplication(calling_app_id);
}

extern "C"
JNIEXPORT jint JNICALL Java_org_orbtv_orblibrary_ApplicationManager_jniSetKeySetMask(JNIEnv *env,
    jobject object,
    jint calling_app_id, jint key_set_mask, jintArray other_keys)
{
    int result = 0;
    jsize length = 0;
    std::vector<uint16_t> otherKeys;
    jint* otherKeysNative = nullptr;

    if (other_keys != nullptr) {
        otherKeysNative = env->GetIntArrayElements(other_keys, nullptr);
        length = env->GetArrayLength(other_keys);
        if (otherKeysNative != nullptr && length > 0) {
            otherKeys.assign(otherKeysNative, otherKeysNative + length);
        }
    }

    result = GetManager(env, object)->SetKeySetMask(calling_app_id, key_set_mask, otherKeys);
    if (other_keys != nullptr && otherKeysNative != nullptr) {
        env->ReleaseIntArrayElements(other_keys, otherKeysNative, 0);
    }

    return result;
}

extern "C"
JNIEXPORT jint JNICALL Java_org_orbtv_orblibrary_ApplicationManager_jniGetKeySetMask(JNIEnv *env,
    jobject object,
    jint calling_app_id)
{
    return GetManager(env, object)->GetKeySetMask(calling_app_id);
}

extern "C"
JNIEXPORT jintArray JNICALL Java_org_orbtv_orblibrary_ApplicationManager_jniGetOtherKeyValues(JNIEnv *env,
    jobject object,
    jint calling_app_id)
{
    std::vector<uint16_t> values = GetManager(env, object)->GetOtherKeyValues(calling_app_id);
    jintArray resultArray = env->NewIntArray(values.size());
    if (resultArray != nullptr) {
        env->SetIntArrayRegion(resultArray, 0, values.size(), reinterpret_cast<const jint*>(values.data()));
    }

    return resultArray;
}

extern "C"
JNIEXPORT jstring JNICALL Java_org_orbtv_orblibrary_ApplicationManager_jniGetApplicationScheme(JNIEnv *env,
                                                                                     jobject object,
                                                                                     jint calling_app_id)
{
    return env->NewStringUTF(GetManager(env, object)->GetApplicationScheme(calling_app_id).c_str());
}

extern "C"
JNIEXPORT jboolean JNICALL Java_org_orbtv_orblibrary_ApplicationManager_jniInKeySet(JNIEnv *env,
    jobject object,
    jint calling_app_id, jint key_set)
{
    return GetManager(env, object)->InKeySet(calling_app_id, key_set);
}

extern "C"
JNIEXPORT void JNICALL Java_org_orbtv_orblibrary_ApplicationManager_jniProcessAitSection(
    JNIEnv *env, jobject object,
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
JNIEXPORT jboolean JNICALL Java_org_orbtv_orblibrary_ApplicationManager_jniProcessXmlAit(
    JNIEnv *env, jobject object,
    jstring j_data, jboolean is_dvbi, jstring j_scheme)
{
    std::string data = JniUtils::MakeStdString(env, j_data);
    std::string scheme = JniUtils::MakeStdString(env, j_scheme);
    return GetManager(env, object)->ProcessXmlAit(data, is_dvbi, scheme);
}

extern "C"
JNIEXPORT jboolean
JNICALL Java_org_orbtv_orblibrary_ApplicationManager_jniIsTeletextApplicationSignalled(JNIEnv *env,
    jobject object)
{
    return GetManager(env, object)->IsTeletextApplicationSignalled();
}

extern "C"
JNIEXPORT jboolean JNICALL Java_org_orbtv_orblibrary_ApplicationManager_jniRunTeletextApplication(
    JNIEnv *env, jobject object)
{
    return GetManager(env, object)->RunTeletextApplication();
}

extern "C"
JNIEXPORT void JNICALL Java_org_orbtv_orblibrary_ApplicationManager_jniOnNetworkAvailabilityChanged(
    JNIEnv *env, jobject object,
    jboolean available)
{
    GetManager(env, object)->OnNetworkAvailabilityChanged(available);
}

extern "C"
JNIEXPORT void JNICALL Java_org_orbtv_orblibrary_ApplicationManager_jniOnLoadApplicationFailed(
    JNIEnv *env, jobject object,
    jint app_id)
{
    GetManager(env, object)->OnLoadApplicationFailed(app_id);
}

extern "C"
JNIEXPORT void JNICALL Java_org_orbtv_orblibrary_ApplicationManager_jniOnApplicationPageChanged(
    JNIEnv *env, jobject object,
    jint app_id, jstring j_url)
{
    std::string url = JniUtils::MakeStdString(env, j_url);
    GetManager(env, object)->OnApplicationPageChanged(app_id, url);
}

extern "C"
JNIEXPORT void JNICALL Java_org_orbtv_orblibrary_ApplicationManager_jniOnChannelChange(JNIEnv *env,
    jobject object,
    jint onet_id, jint trans_id, jint serv_id)
{
    GetManager(env, object)->OnChannelChanged(onet_id, trans_id, serv_id);
}

extern "C"
JNIEXPORT void JNICALL Java_org_orbtv_orblibrary_ApplicationManager_jniOnBroadcastStopped(
    JNIEnv *env, jobject object)
{
    GetManager(env, object)->OnBroadcastStopped();
}

extern "C"
JNIEXPORT jboolean JNICALL Java_org_orbtv_orblibrary_ApplicationManager_jniIsRequestAllowed(
    JNIEnv *env, jobject object,
    jint calling_app_id, jstring j_calling_page_url, jint method_requirement)
{
    std::string calling_page_url = JniUtils::MakeStdString(env, j_calling_page_url);
    return GetManager(env, object)->IsRequestAllowed(calling_app_id, calling_page_url,
        static_cast<ApplicationManager::MethodRequirement>(method_requirement));
}

static ApplicationManager* GetManager(JNIEnv *env, jobject object)
{
    return reinterpret_cast<ApplicationManager *>(env->GetLongField(object,
        gJavaManagerPointerField));
}
