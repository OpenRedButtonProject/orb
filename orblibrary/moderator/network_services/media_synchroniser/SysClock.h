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

#ifndef __SYSCLOCK_H
#define __SYSCLOCK_H

#include "ClockBase.h"
#include "ClockUtilities.h"

class SysClock : public ClockBase {
private:
    double mPrecision;
    double mMaxFreqErrorPpm;
    double mFreq;

public:
    SysClock(double tickRate = 1000000000, double maxFreqErrorPpm = 500);

    ~SysClock() = default;

    friend std::ostream &operator<<(std::ostream &, const SysClock &);

    void setParent(ClockBase *base) override;

    u_int64_t fromParentTicks(u_int64_t d) override;

    u_int64_t toParentTicks(u_int64_t d) override;


    u_int64_t getTicks() const override;

    double getTickRate() const override;

    double calcWhen(double d) override;


    void setSpeed(double d) override;

    double errorAtTime(double d) override;

    void SetAvailability(bool b) override;

    void setMaxFreqError(double freqError);

    double getRootMaxFreqError();
};


#endif //__SYSCLOCK_H

