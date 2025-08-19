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

#include <iostream>
#include "SysClock.h"
#include "ClockUtilities.h"
#include "ClockBase.h"

SysClock::SysClock(double tickRate, double maxFreqErrorPpm) : ClockBase(tickRate, 1, nullptr)
{
    mMaxFreqErrorPpm = maxFreqErrorPpm;
    mFreq = tickRate;
    double sampleSize = std::min(1000.0, std::max(10.0, tickRate / 10));
    mPrecision = ClockUtilities::measurePrecision(*this, sampleSize);
}

void SysClock::setParent(ClockBase *base)
{
    mParent = nullptr;
    std::cout << "system clock cannot have a parent\n";
}

u_int64_t SysClock::fromParentTicks(u_int64_t d)
{
    return 0;
}

u_int64_t SysClock::toParentTicks(u_int64_t d)
{
    return 0;
}

void SysClock::SetAvailability(bool b)
{
    mAvailability = true;
    std::cout << "Cannot change system clock availability\n";
}

u_int64_t SysClock::getTicks() const
{
    return (u_int64_t)(ClockUtilities::time() * mFreq);
}

double SysClock::getTickRate() const
{
    return mFreq;
}

double SysClock::calcWhen(double d)
{
    return d / mFreq;
}

void SysClock::setSpeed(double d)
{
    mSpeed = 1.0;
    std::cout << "System clock speed is always 1.0\n";
}

double SysClock::errorAtTime(double d)
{
    return mPrecision;
}

double SysClock::getRootMaxFreqError()
{
    return mMaxFreqErrorPpm;
}

std::ostream &operator<<(std::ostream &os, const SysClock &clock)
{
    os << "SysClock(t=" << "###" << ", freq=" << clock.mFreq << ")\n";
    return os;
}

void SysClock::setMaxFreqError(double freqError)
{
    mMaxFreqErrorPpm = freqError;
}
