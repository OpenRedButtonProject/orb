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

#ifndef __CLOCKBASE_H
#define __CLOCKBASE_H

#include <algorithm>
#include <limits>
#include <unordered_map>
#include <stdexcept>

struct Notifiable
{
    virtual void notify() = 0;
};

class ClockBase : public Notifiable {
private:
    std::unordered_map<Notifiable *, bool> mDependants;
protected:
    bool mAvailability;
    ClockBase *mParent;

    double mTickRate;
    double mSpeed;

public:
    ClockBase() : mAvailability{true},
        mTickRate{0},
        mSpeed{1.0},
        mParent{nullptr}
    {
    };

    explicit ClockBase(double tickRate = 0, double speed = 1.0, ClockBase *parent = nullptr) :
        mAvailability{true}
    {
        if (tickRate < 0)
        {
            //throw std::out_of_range("tickRate");
            tickRate = 0;
        }
        mSpeed = speed;
        mParent = parent;
        mTickRate = tickRate;
    };

    virtual ~ClockBase() = default;

    virtual void setParent(ClockBase *) = 0;

    virtual ClockBase* getParent() const;

    bool isAvailable();

    virtual void SetAvailability(bool);

    const ClockBase* getRoot() const;

    std::vector<ClockBase *> getAncestry();


    void bind(Notifiable *);

    void unbind(Notifiable *);

    virtual void notify();


    virtual u_int64_t fromParentTicks(u_int64_t) = 0;

    virtual u_int64_t toParentTicks(u_int64_t) = 0;

    u_int64_t fromRootTicks(u_int64_t ticks);

    u_int64_t toRootTicks(u_int64_t);

    u_int64_t toOtherClockTicks(ClockBase &otherClock, u_int64_t ticks);

    virtual u_int64_t getTicks() const;

    virtual double getTickRate() const;

    virtual void setTickRate(double);

    u_int64_t getNanosToTicks(double) const;

    double getNanos() const;

    virtual double calcWhen(double) = 0;


    virtual void setSpeed(double) = 0;

    virtual double getSpeed() const;

    double getEffectiveSpeed() const;


    u_int64_t clockDiff(ClockBase &otherClock);

    double dispersionAtTime(double);

    virtual double errorAtTime(double) = 0;

    double getRootMaxFreqError() const;
};


#endif //__CLOCKBASE_H
