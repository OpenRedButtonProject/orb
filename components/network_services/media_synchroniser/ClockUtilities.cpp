/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 *
 * NOTICE: This file has been created by Ocean Blue Software and is based on
 * the original work (https://github.com/bbc/pydvbcss) of the British
 * Broadcasting Corporation, as part of a translation of that work from a
 * Python library/tool to a native service. The following is the copyright
 * notice of the original work:
 *
 * Copyright 2015 British Broadcasting Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "ClockUtilities.h"
#include <ctime>
#include <vector>

namespace ClockUtilities {
double measurePrecision(ClockBase &clock, unsigned long sampleSize)
{
    std::vector<unsigned long int> diffs;
    double precision = 0.0;
    u_int64_t tickNow;
    u_int64_t tickLater;

    if (clock.getTickRate() > 0)
    {
        while (diffs.size() < sampleSize)
        {
            tickNow = clock.getTicks();
            tickLater = clock.getTicks();
            if (tickNow < tickLater)
            {
                diffs.push_back(tickLater - tickNow);
            }
        }
        unsigned long int result = *std::min_element(std::begin(diffs), std::end(diffs));
        precision = result / clock.getTickRate();
    }
    return precision;
}

double time()
{
    struct timespec mTimeStamp;
    clock_gettime(CLOCK_MONOTONIC, &mTimeStamp);
    return double(mTimeStamp.tv_sec + mTimeStamp.tv_nsec * 1.0e-9);
}

u_int64_t timeNanos()
{
    struct timespec mTimeStamp;
    clock_gettime(CLOCK_MONOTONIC, &mTimeStamp);
    return(mTimeStamp.tv_sec * (u_int64_t)1000000000 + mTimeStamp.tv_nsec);
}

unsigned long int timeMicros()
{
    struct timespec mTimeStamp;
    clock_gettime(CLOCK_MONOTONIC, &mTimeStamp);
    return(mTimeStamp.tv_sec * 1000000 + mTimeStamp.tv_nsec / 1000);
}

void sleep(unsigned long const timeToSleep)
{
    if (timeToSleep > 0)
    {
        struct timespec requestTimeStamp;
        requestTimeStamp.tv_sec = timeToSleep;
        requestTimeStamp.tv_nsec = (timeToSleep % 1) * 1000000000;

        clock_nanosleep(CLOCK_MONOTONIC, 0, &requestTimeStamp, NULL);
    }
}
}
