/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
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

