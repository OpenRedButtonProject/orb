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
#ifndef __ORB_LOGGING_H__
#define __ORB_LOGGING_H__

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string>
#include <cstring>

/* Simple file name, i.e. without the full path */
#define SIMPLE_FILE_NAME strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__

/**
 * Produce a log entry with the specified message and arguments.
 */
#define ORB_LOG(msg, ...) do \
    { \
        fprintf(stderr, "ORB [%s]::[%s]::[%d] ", SIMPLE_FILE_NAME, __FUNCTION__, __LINE__); \
        fprintf(stderr, msg, ##__VA_ARGS__); \
        fprintf(stderr, "\n"); \
    } \
    while (0)

/**
 * Produce a log entry without any message or arguments.
 */
#define ORB_LOG_NO_ARGS() do \
    { \
        fprintf(stderr, "ORB [%s]::[%s]::[%d]\n", SIMPLE_FILE_NAME, __FUNCTION__, __LINE__); \
    } \
    while (0)

/**
 * Produce a log entry for errors. The entry will contain a message and id.
 */
#define ORB_ERROR(msg, id) do \
    { \
        fprintf(stderr, "ORB-ERROR [%s]::[%s]::[%d] ", SIMPLE_FILE_NAME, __FUNCTION__, __LINE__); \
        fprintf(stderr, msg, id); \
        fprintf(stderr, "\n"); \
    } \
    while (0)

#endif // __ORB_LOGGING__
