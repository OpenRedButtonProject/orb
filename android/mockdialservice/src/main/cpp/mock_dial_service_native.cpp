/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

#include <cstdio>
#include <cstdlib>
#include <pthread.h>
#include <string>
#include <mutex>
#include <jni.h>

#include "jni_utils.h"
#include "log.h"

extern "C" {
   #include "dial_server.h"
   #include "dial_data.h"
   #include "quick_ssdp.h"
   #include <jni.h>
}

#define CB_START_APP 0
#define CB_HIDE_APP 1
#define CB_STOP_APP 2
#define CB_GET_APP_STATUS 3
#define CB_NUMBER_OF_ITEMS 4

static void * SsdpLooper(void *instance);
static DIALStatus OnStartApp(DIALServer *ds, const char *app_name, const char *payload,
   const char *query_string, const char *additional_data_url, DIAL_run_t *run_id, void *callback_data);
static DIALStatus OnHideApp(DIALServer *ds, const char *app_name, DIAL_run_t *run_id, void *callback_data);
static void OnStopApp(DIALServer *ds, const char *app_name, DIAL_run_t run_id, void *callback_data);
static DIALStatus OnGetAppStatus(DIALServer *ds, const char *app_name, DIAL_run_t run_id, int *can_stop,
   void *callback_data);
static DIALData * CreateDialData(const std::string &key_value);

static jclass g_cb_class;
static jmethodID g_cb_method[CB_NUMBER_OF_ITEMS];
static DIALServer *g_dial_server = nullptr;
static pthread_t g_ssdp_looper;

static struct {
   std::string uuid;
   std::string friendly_name;
   std::string model_name;
   std::string ip_addr;
   std::string mac_addr;
   int port{};
} g_config;

struct DIALAppCallbacks g_app_callbacks = {
   OnStartApp,
   OnHideApp,
   OnStopApp,
   OnGetAppStatus
};

extern "C"
JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *)
{
   JniUtils::Init(vm, JNI_VERSION_1_6);
   JNIEnv *env = JniUtils::GetEnv();
   jclass cb_class = env->FindClass("org/orbtv/mockdialservice/MockDialService");
   g_cb_class = reinterpret_cast<jclass>(env->NewGlobalRef(cb_class));
   g_cb_method[CB_START_APP] = env->GetStaticMethodID(
      g_cb_class, "jniStartApp", "(Ljava/lang/String;Ljava/lang/String;)I");
   g_cb_method[CB_HIDE_APP] = env->GetStaticMethodID(
      g_cb_class, "jniHideApp", "(Ljava/lang/String;)I");
   g_cb_method[CB_STOP_APP] = env->GetStaticMethodID(
      g_cb_class, "jniStopApp", "(Ljava/lang/String;)V");
   g_cb_method[CB_GET_APP_STATUS] = env->GetStaticMethodID(
      g_cb_class, "jniGetAppStatus", "(Ljava/lang/String;)I");
   return JNI_VERSION_1_6;
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_org_orbtv_mockdialservice_MockDialService_jniStartServer(JNIEnv *env, jclass clazz,
   jstring uuid, jstring friendly_name, jstring model_name, jstring ip_addr, jstring mac_addr)
{
   if (g_dial_server == nullptr)
   {
      g_config.uuid = JniUtils::MakeStdString(env, uuid);
      g_config.friendly_name = JniUtils::MakeStdString(env, friendly_name);
      g_config.model_name = JniUtils::MakeStdString(env, model_name);
      g_config.ip_addr = JniUtils::MakeStdString(env, ip_addr);
      g_config.mac_addr = JniUtils::MakeStdString(env, mac_addr);
      g_dial_server = DIAL_create();
      if (g_dial_server != nullptr)
      {
         if (DIAL_start(g_dial_server))
         {
            LOG(LOG_DEBUG, "Started DIAL server on port %d", g_config.port);
            g_config.port = DIAL_get_port(g_dial_server);
            pthread_create(&g_ssdp_looper, nullptr, SsdpLooper, nullptr);
         }
         else
         {
            free(g_dial_server);
            g_dial_server = nullptr;
         }
      }
   }
   return g_dial_server != nullptr;
}

extern "C"
JNIEXPORT void JNICALL
Java_org_orbtv_mockdialservice_MockDialService_jniStopServer(JNIEnv *env, jclass clazz)
{
   if (g_dial_server != nullptr)
   {
      stop_ssdp();
      DIAL_stop(g_dial_server);
      free(g_dial_server);
      g_dial_server = nullptr;
   }
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_org_orbtv_mockdialservice_MockDialService_jniRegisterApp(JNIEnv *env, jclass clazz,
   jstring j_name, jstring j_data_1, jstring j_data_2)
{
   jboolean success = false;
   if (g_dial_server != nullptr)
   {
      std::string name = JniUtils::MakeStdString(env, j_name);
      DIALData *data = CreateDialData(JniUtils::MakeStdString(env, j_data_1));
      if (data != nullptr)
      {
         data->next = CreateDialData(JniUtils::MakeStdString(env, j_data_2));
      }
      success = DIAL_register_app(g_dial_server, name.c_str(), &g_app_callbacks, nullptr,
         1, "* https://*", data) != -1;
   }
   return success;
}

extern "C"
JNIEXPORT void JNICALL
Java_org_orbtv_mockdialservice_MockDialService_jniUnregisterApp(JNIEnv *env, jclass clazz,
   jstring j_name)
{
   if (g_dial_server != nullptr)
   {
      std::string name = JniUtils::MakeStdString(env, j_name);
      DIAL_unregister_app(g_dial_server, name.c_str());
   }
}

static void * SsdpLooper(void *)
{
   LOG(LOG_DEBUG, "Starting SSDP server");
   run_ssdp(g_config.port, g_config.friendly_name.c_str(), g_config.model_name.c_str(),
      g_config.uuid.c_str(), g_config.ip_addr.c_str(), g_config.mac_addr.c_str());
   if (g_dial_server != nullptr)
   {
      LOG(LOG_ERROR, "SSDP server stopped unexpectedly");
      DIAL_stop(g_dial_server);
      free(g_dial_server);
      g_dial_server = nullptr;
   }
   return nullptr;
}

static DIALStatus OnStartApp(DIALServer *ds, const char *app_name, const char *payload,
   const char *query_string, const char *additional_data_url, DIAL_run_t *run_id, void *callback_data)
{
   JNIEnv *env = JniUtils::GetEnv();
   jstring j_name = env->NewStringUTF(app_name);
   jstring j_payload = env->NewStringUTF(payload);
   int status = env->CallStaticIntMethod(g_cb_class, g_cb_method[CB_START_APP], j_name, j_payload);
   env->DeleteLocalRef(j_payload);
   env->DeleteLocalRef(j_name);
   return static_cast<DIALStatus>(status);
}

static DIALStatus OnHideApp(DIALServer *ds, const char *app_name, DIAL_run_t *run_id, void *callback_data)
{
   JNIEnv *env = JniUtils::GetEnv();
   jstring j_name = env->NewStringUTF(app_name);
   int status = env->CallStaticIntMethod(g_cb_class, g_cb_method[CB_HIDE_APP], j_name);
   env->DeleteLocalRef(j_name);
   return static_cast<DIALStatus>(status);
}


static void OnStopApp(DIALServer *ds, const char *app_name, DIAL_run_t run_id, void *callback_data)
{
   JNIEnv *env = JniUtils::GetEnv();
   jstring j_name = env->NewStringUTF(app_name);
   env->CallStaticVoidMethod(g_cb_class, g_cb_method[CB_STOP_APP], j_name);
   env->DeleteLocalRef(j_name);
}

static DIALStatus OnGetAppStatus(DIALServer *ds, const char *app_name, DIAL_run_t run_id,
   int *can_stop, void *callback_data)
{
   *can_stop = 0; // TODO We don't expose this
   JNIEnv *env = JniUtils::GetEnv();
   jstring j_name = env->NewStringUTF(app_name);
   int status = env->CallStaticIntMethod(g_cb_class, g_cb_method[CB_GET_APP_STATUS], j_name);
   env->DeleteLocalRef(j_name);
   return static_cast<DIALStatus>(status);
}

static DIALData * CreateDialData(const std::string &key_value)
{
   size_t delimiter = key_value.find('=');
   if (delimiter == std::string::npos)
   {
      return nullptr;
   }
   std::string key = key_value.substr(0, delimiter);
   std::string value = key_value.substr(delimiter + 1);
   if (key.empty() || value.empty())
   {
      return nullptr;
   }
   // DIAL uses free() to release DIALData and its members
   auto *data = static_cast<DIALData *>(calloc(1, sizeof(DIALData)));
   data->key = strdup(key.c_str());
   data->value = strdup(value.c_str());
   return data;
}
