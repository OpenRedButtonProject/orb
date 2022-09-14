/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 *
 * General logging for each supported platform
 *
 * Note: This file is part of the platform-agnostic application manager library.
 */

#ifndef __MANAGER_LOG_H
#define __MANAGER_LOG_H

#ifdef ANDROID

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
   "ERROR",     // 0
   "INFO",     // 1
   "DEBUG",     // 2
};

enum
{
   LOG_LVL_ERROR,     // 0
   LOG_LVL_INFO,     // 1
   LOG_LVL_DEBUG     // 2
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
         fprintf(LOG_FP, "[%s] %s:%s:%d: " fmt "\n", app_mgr_log_level_strings[level], __FILENAME__, __FUNCTION__, __LINE__, ##args); \
         fflush( LOG_FP ); \
      } \
} while (0)

#ifdef __cplusplus
}
#endif // __cplusplus

#else

#error UNKNOWN BUILD OPTION

#endif // ANDROID RDK

#endif // __MANAGER_LOG_H
