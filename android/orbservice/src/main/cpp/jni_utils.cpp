/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
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

#include "jni_utils.h"

#include <jni.h>
#include <pthread.h>

static JavaVM *g_jvm;
static jint g_version;
static pthread_key_t g_thread_key;

static void on_thread_destroyed(void *unused);

/**
 * You must call this method before calling JniUtils::GetEnv().
 * @param jvm The Java virtual machine.
 * @param version The JNI version.
 */
void JniUtils::Init(JavaVM *jvm, jint version)
{
    g_jvm = jvm;
    g_version = version;

    // JNIEnv cannot be shared amongst threads. Attach each thread and detach when destroyed
    pthread_key_create(&g_thread_key, on_thread_destroyed);
}

/**
 * Attach the current thread if needed and get the env. If this method attaches the thread, it
 * is automatically detached when the thread is destroyed.
 * @return A JNI env.
 */
JNIEnv * JniUtils::GetEnv()
{
    JNIEnv *env = nullptr;
    if (g_jvm->GetEnv(reinterpret_cast<void **>(&env), g_version) == JNI_EDETACHED)
    {
        g_jvm->AttachCurrentThread(&env, nullptr);
        pthread_setspecific(g_thread_key, env);
    }
    return env;
}

/**
 * Make a STD string copy of a JNI UTF string.
 * @param env The JNI env.
 * @param jni_utf_str JNI UTF string.
 * @return A STD string.
 */
std::string JniUtils::MakeStdString(JNIEnv *env, jstring jni_utf_str)
{
    std::string str;
    if (jni_utf_str != nullptr)
    {
        const char *utf_str = env->GetStringUTFChars(jni_utf_str, nullptr);
        if (utf_str != nullptr)
        {
            str = utf_str;
            env->ReleaseStringUTFChars(jni_utf_str, utf_str);
        }
    }
    return str;
}

static void on_thread_destroyed(void *unused)
{
    JNIEnv *env = nullptr;
    if (g_jvm->GetEnv(reinterpret_cast<void **>(&env), g_version) != JNI_EDETACHED)
    {
        g_jvm->DetachCurrentThread();
    }
}
