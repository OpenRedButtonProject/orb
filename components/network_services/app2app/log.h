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

#ifndef __MANAGER_LOG_H
#define __MANAGER_LOG_H
#if 1
#include <android/log.h>
#include <assert.h>
#define LOG_ERROR ANDROID_LOG_ERROR
#define LOG_INFO ANDROID_LOG_INFO
#define LOG_DEBUG ANDROID_LOG_DEBUG
#define LOG(level, args ...) __android_log_print(level, "Orb/app2app", args)
#define ASSERT(condition) assert(condition)
#else
#define LOG_ERROR 0
#define LOG_INFO 1
#define LOG_DEBUG 2
#define LOG(level, args ...)
#define ASSERT(condition)
#endif
#endif // __MANAGER_LOG_H

