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

#include <iostream>
#include "CorrelatedClock.h"

u_int64_t CorrelatedClock::getTicks() const
{
    return mCorrelation.getChildTicks() +
           (mParent->getTicks() - mCorrelation.getParentTicks()) / mParent->getTickRate() * mFreq *
           getSpeed();;
}

double CorrelatedClock::getTickRate() const
{
    return mFreq;
}

void CorrelatedClock::setTickRate(double tickRate)
{
    mFreq = tickRate;
    notify();
}

void CorrelatedClock::setSpeed(double speed)
{
    mSpeed = speed;
    notify();
}

double CorrelatedClock::getSpeed() const
{
    return mSpeed;
}

ClockBase * CorrelatedClock::getParent() const
{
    return mParent;
}

void CorrelatedClock::setParent(ClockBase *clock)
{
    if (mParent != clock)
    {
        if (mParent)
        {
            mParent->unbind(this);
        }

        mParent = clock;

        if (mParent)
        {
            mParent->bind(this);
        }

        notify();
    }
}

u_int64_t CorrelatedClock::fromParentTicks(u_int64_t ticks)
{
    return mCorrelation.getChildTicks() +
           (ticks - mCorrelation.getParentTicks()) * mFreq * mSpeed / mParent->getTickRate();
}

u_int64_t CorrelatedClock::toParentTicks(u_int64_t ticks)
{
    if (mSpeed == 0.0)
    {
        if (ticks == mCorrelation.getChildTicks())
        {
            return mCorrelation.getParentTicks();
        }
        else
        {
            return std::numeric_limits<double>::quiet_NaN();
        }
    }
    else
    {
        return mCorrelation.getParentTicks() +
               (ticks - mCorrelation.getChildTicks()) * mParent->getTickRate() / mFreq / mSpeed;
    }
}

double CorrelatedClock::quantifyChange(Correlation &newCorrelation, double newSpeed)
{
    double nx = newCorrelation.getParentTicks();
    double nt = newCorrelation.getChildTicks();
    double ox = toParentTicks(nt);
    double ot = fromParentTicks(nx);

    if (newSpeed != mSpeed)
    {
        return std::numeric_limits<double>::max();
    }
    else
    {
        if (newSpeed != 0.0)
        {
            return std::abs(nx - ox) / mParent->getTickRate();
        }
        else
        {
            return std::abs(nt - ot) / mTickRate;
        }
    }
}

double CorrelatedClock::isChangeSignificant(Correlation &newCorrelation, double newSpeed, double
    thresholdSecs)
{
    double delta = quantifyChange(newCorrelation, newSpeed);
    return delta > thresholdSecs;
}

double CorrelatedClock::calcWhen(double ticks)
{
    double parentTicks = toParentTicks(ticks);
    return mParent->calcWhen(parentTicks);
}

double CorrelatedClock::errorAtTime(double ticks)
{
    double parentTicks = toParentTicks(ticks);
    double deltaSecs = std::abs(parentTicks - mCorrelation.getParentTicks()) /
        mParent->getTickRate();
    return mCorrelation.getInitialError() + deltaSecs * mCorrelation.getErrorGrowthRate();
}

std::ostream &operator<<(std::ostream &os, const CorrelatedClock &clock)
{
    os << "SysClock(t=" << "###" << ", freq=" << clock.mFreq <<
        ", " << clock.mCorrelation << ", at speed=" << clock.mSpeed << ")\n";
    return os;
}

Correlation &CorrelatedClock::getCorrelation()
{
    return mCorrelation;
}

void CorrelatedClock::setCorrelation(const Correlation &cor)
{
    mCorrelation = cor;
    notify();
}

void CorrelatedClock::setCorrelationAndSpeed(const Correlation &newCorrelation, double newSpeed)
{
    mCorrelation = newCorrelation;
    mSpeed = newSpeed;
    notify();
}

void CorrelatedClock::rebaseCorrelationAtTicks(unsigned long tickValue)
{
    if (tickValue > 0)
    {
        double pt = toParentTicks(tickValue);
        double deltaSecs = (pt - mCorrelation.getParentTicks()) / mParent->getTickRate();
        double initError = mCorrelation.getInitialError() + deltaSecs *
            mCorrelation.getErrorGrowthRate();


        std::unordered_map<std::string, u_int64_t> correlationConfig;
        correlationConfig["childTicks"] = tickValue;
        correlationConfig["parentTicks"] = pt;
        correlationConfig["initialError"] = initError;

        mCorrelation = mCorrelation.butWith(correlationConfig);
    }
}
