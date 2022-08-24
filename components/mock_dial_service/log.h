/**
 * General logging for each supported platform.
 * Note: This file is part of the platform-agnostic application manager library.
 * @preserve Copyright (c) Ocean Blue Software Ltd.
 * @license MIT (see LICENSE for full license)
 */

#ifndef __MANAGER_LOG_H
#define __MANAGER_LOG_H
#if ANDROID_DEBUG
#include <android/log.h>
#include <assert.h>
#define LOG_ERROR ANDROID_LOG_ERROR
#define LOG_INFO ANDROID_LOG_INFO
#define LOG_DEBUG ANDROID_LOG_DEBUG
#define LOG(level, args ...) __android_log_print(level, "MockDialService", args)
#define ASSERT(condition) assert(condition)
#else
#define LOG_ERROR 0
#define LOG_INFO 1
#define LOG_DEBUG 2
#define LOG(level, args ...)
#define ASSERT(condition)
#endif
#endif // __MANAGER_LOG_H
