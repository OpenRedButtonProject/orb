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
#else // RDK
#include "Module.h"
#undef LOG_ERROR
#undef LOG_INFO
#undef LOG_DEBUG
#undef LOG
#undef ASSERT
#define ANDROID_DEBUG 1
#define LOG_ERROR WPEFramework::Trace::Error
#define LOG_INFO WPEFramework::Trace::Information
#define LOG_DEBUG WPEFramework::Trace::Information
using namespace WPEFramework;
#define LOG(level, args ...) SYSLOG(level, (args))

#endif
#endif // __MANAGER_LOG_H
