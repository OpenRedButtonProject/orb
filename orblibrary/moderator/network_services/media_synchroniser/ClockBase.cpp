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

#include <set>
#include <vector>
#include "ClockBase.h"

ClockBase * ClockBase::getParent() const
{
    return nullptr;
}

std::vector<ClockBase *> ClockBase::getAncestry(void)
{
    std::vector<ClockBase *> ancestry;
    ClockBase *parent = nullptr;
    ancestry.push_back(this);

    for (int index = 0; index < ancestry.size(); index++)
    {
        parent = ancestry[index]->mParent;
        if (parent != nullptr)
        {
            ancestry.push_back(parent);
        }
    }

    return ancestry;
}

const ClockBase * ClockBase::getRoot() const
{
    const ClockBase *rootClock = this;
    const ClockBase *parentClock = mParent;

    while (parentClock != nullptr)
    {
        rootClock = parentClock;
        parentClock = parentClock->mParent;
    }

    return rootClock;
}

bool ClockBase::isAvailable()
{
    return mAvailability && (mParent == nullptr || mParent->isAvailable());
}

void ClockBase::SetAvailability(bool availability)
{
    bool isChange = false;

    if (mAvailability != availability)
    {
        isChange = true;
    }

    if (mParent != nullptr)
    {
        isChange = isChange && mParent->isAvailable();
    }

    mAvailability = availability;

    if (isChange)
    {
        notify();
    }
}

void ClockBase::notify(void)
{
    for (auto &it: mDependants)
    {
        it.first->notify();
    }
}

void ClockBase::bind(Notifiable *clock)
{
    if (clock != nullptr && clock != this)
    {
        mDependants[clock] = true;
    }
}

void ClockBase::unbind(Notifiable *clock)
{
    mDependants.erase(clock);
}

double ClockBase::getSpeed() const
{
    return 1.0;
}

double ClockBase::getEffectiveSpeed() const
{
    double effectiveSpeed = mSpeed;
    ClockBase const *parent = mParent;

    while (parent != nullptr)
    {
        effectiveSpeed = effectiveSpeed * parent->mSpeed;
        parent = parent->mParent;
    }

    return effectiveSpeed;
}

u_int64_t ClockBase::toRootTicks(u_int64_t ticks)
{
    u_int64_t parentTicks = 0.0;
    if (mParent == nullptr)
    {
        return ticks;
    }
    else
    {
        parentTicks = toParentTicks(ticks);
        return mParent->toRootTicks(parentTicks);
    }
}

u_int64_t ClockBase::fromRootTicks(u_int64_t ticks)
{
    u_int64_t rootTicks = 0.0;
    u_int64_t parentTicks = 0.0;
    if (mParent == nullptr)
    {
        return ticks;
    }
    else
    {
        parentTicks = mParent->fromRootTicks(ticks);
        return fromParentTicks(parentTicks);
    }
}

u_int64_t ClockBase::getTicks() const
{
    return 0;
}

double ClockBase::getTickRate() const
{
    return 0;
}

void ClockBase::setTickRate(double)
{
}

u_int64_t ClockBase::getNanosToTicks(double nanos) const
{
    double nanosToTick = 0;

    if (mTickRate > 0 && nanos >= 0)
    {
        nanosToTick = nanos * mTickRate / 1000000000;
    }

    return nanosToTick;
}

double ClockBase::getNanos(void) const
{
    double nanos = 0;

    if (mTickRate > 0)
    {
        nanos = getTicks() * 1000000000 / mTickRate;
    }

    return nanos;
}

u_int64_t ClockBase::clockDiff(ClockBase &otherClock)
{
    u_int64_t diff = std::numeric_limits<u_int64_t>::max();
    u_int64_t rootTicks = getRoot()->getTicks();
    u_int64_t thisRootTicks = fromRootTicks(rootTicks);
    u_int64_t otherRootTicks = otherClock.fromRootTicks(rootTicks);

    if ((getEffectiveSpeed() == otherClock.getEffectiveSpeed() ||
         mTickRate == otherClock.mTickRate) && mTickRate > 0.0)
    {
        diff = (thisRootTicks > otherRootTicks ?
                thisRootTicks - otherRootTicks :
                otherRootTicks - thisRootTicks) / mTickRate;
    }

    return diff;
}

u_int64_t ClockBase::toOtherClockTicks(ClockBase &otherClock, u_int64_t ticks)
{
    u_int64_t otherTicks = ticks;
    std::set<ClockBase *> intersect;
    std::vector<ClockBase *> ancestors = getAncestry();
    std::vector<ClockBase *> otherAncestors = otherClock.getAncestry();

    auto compareClocks = [](ClockBase *a, ClockBase *b) {
            return a->getTicks() < b->getTicks();
        };
    std::sort(ancestors.begin(), ancestors.end(), compareClocks);
    std::sort(otherAncestors.begin(), otherAncestors.end(), compareClocks);
    set_intersection(ancestors.begin(), ancestors.end(), otherAncestors.begin(),
        otherAncestors.end(),
        std::inserter(intersect, intersect.begin()));

    if (intersect.empty())
    {
        otherTicks = 0.0;
    }
    else
    {
        for (auto elem : intersect)
        {
            ancestors.erase(std::remove(ancestors.begin(), ancestors.end(), elem), ancestors.end());
            otherAncestors.erase(std::remove(otherAncestors.begin(), otherAncestors.end(), elem),
                otherAncestors.end());
        }

        for (auto elem : ancestors)
        {
            otherTicks = elem->toParentTicks(otherTicks);
        }

        std::reverse(otherAncestors.begin(), otherAncestors.end());
        for (auto elem : otherAncestors)
        {
            otherTicks = elem->fromParentTicks(otherTicks);
        }
    }

    return otherTicks;
}

double ClockBase::dispersionAtTime(double ticks)
{
    double dispersion = errorAtTime(ticks);
    double parentTicks = 0.0;

    if (mParent != nullptr)
    {
        parentTicks = toParentTicks(ticks);
        dispersion += mParent->dispersionAtTime(parentTicks);
    }

    return dispersion;
}

double ClockBase::getRootMaxFreqError() const
{
    const ClockBase *rootClock = getRoot();
    double maxFreqError = 0.0;

    if (rootClock == this)
    {
        return rootClock->getRootMaxFreqError();
    }

    return maxFreqError;
}
