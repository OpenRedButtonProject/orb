/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

#ifndef __MANAGER_LOG_H
#define __MANAGER_LOG_H
#if 1
#include <android/log.h>
#include <assert.h>
#define LOG_ERROR ANDROID_LOG_ERROR
#define LOG_INFO ANDROID_LOG_INFO
#define LOG_DEBUG ANDROID_LOG_DEBUG
#define LOG(level, args ...) __android_log_print(level, "Orb/ApplicationManager", args)
#define ASSERT(condition) assert(condition)
#else
#define LOG_ERROR 0
#define LOG_INFO 1
#define LOG_DEBUG 2
#define LOG(level, args ...)
#define ASSERT(condition)
#endif
#endif // __MANAGER_LOG_H

