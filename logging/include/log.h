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
 *
 * General logging for each supported platform
 *
 * Note: This file is part of the platform-agnostic application manager library.
 */

#ifndef COMMON_LOG_H
#define COMMON_LOG_H

#ifdef IS_CHROMIUM
#include <base/logging.h>
#elif defined(ANDROID)

#include <android/log.h>
#include <assert.h>
#define LOG_ERROR ANDROID_LOG_ERROR
#define LOG_INFO ANDROID_LOG_INFO
#define LOG_DEBUG ANDROID_LOG_DEBUG
#define LOG(level, args ...) __android_log_print(level, "Orb/ApplicationManager", args)
#define ASSERT(condition) assert(condition)

#elif RDK

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *app_mgr_log_level_strings [] = {
    "ERROR",    // 0
    "INFO",    // 1
    "DEBUG",    // 2
};

enum
{
    LOG_LVL_ERROR,    // 0
    LOG_LVL_INFO,    // 1
    LOG_LVL_DEBUG    // 2
};

static unsigned char app_mgr_log_run_level = LOG_LVL_DEBUG;

#define LOG_ERROR LOG_LVL_ERROR
#define LOG_INFO LOG_LVL_INFO
#define LOG_DEBUG LOG_LVL_DEBUG

#define LOG_FP stdout
#define LOG_SHOULD_I( level ) (level <= app_mgr_log_run_level)

#ifdef __FILENAME__
#undef __FILENAME__
#endif
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#define LOG(level, fmt, args ...) do {  \
        if (LOG_SHOULD_I(level)) { \
            fprintf(LOG_FP, "[%s] %s:%s:%d: " fmt "\n", app_mgr_log_level_strings[level], \
    __FILENAME__, __FUNCTION__, __LINE__, ##args); \
            fflush( LOG_FP ); \
        } \
} while (0)

#ifdef __cplusplus
}
#endif // __cplusplus


 #else
 #error Not Chromium or Android build
 #endif

 #define LOGI(str)   LOG(INFO) << __FUNCTION__ << "," << __LINE__ << ": " << str
 #define LOGE(str)   LOG(ERROR) << __FUNCTION__ << "," << __LINE__ << ": " << str
 #define LOGD(str)   DLOG(INFO) << __FUNCTION__ << "," << __LINE__ << ": " << str

 #endif // COMMON_LOG_H
