/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
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


#endif // __ORB_LOGGING__
