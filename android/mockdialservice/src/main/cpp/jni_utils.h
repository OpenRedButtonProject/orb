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

#ifndef HBBTVBROWSER_JNI_UTILS_H
#define HBBTVBROWSER_JNI_UTILS_H

#include <jni.h>
#include <pthread.h>
#include <string>

namespace JniUtils {
/**
 * You must call this method before calling JniUtils::GetEnv().
 * @param jvm The Java virtual machine.
 * @param version The JNI version.
 */
void Init(JavaVM *jvm, jint version);

/**
 * Attach the current thread if needed and get the env. If this method attaches the thread, it
 * is automatically detached when the thread is destroyed.
 * @return A JNI env.
 */
JNIEnv* GetEnv();

/**
 * Make a STD string copy of a JNI UTF string.
 * @param env The JNI env.
 * @param jni_utf_str JNI UTF string.
 * @return A STD string.
 */
std::string MakeStdString(JNIEnv *env, jstring jni_utf_str);
}

#endif //HBBTVBROWSER_JNI_UTILS_H

