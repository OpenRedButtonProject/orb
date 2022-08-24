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

#ifndef __CORRELATION_H
#define __CORRELATION_H

#include <stdexcept>
#include <unordered_map>

class Correlation {
private:
    u_int64_t mParentTicks;
    u_int64_t mChildTicks;
    unsigned long int mInitialError;
    unsigned long int mErrorGrowthRate;

public:
    Correlation(u_int64_t parentTicks,
                u_int64_t childTicks,
                unsigned long int initialError = 0,
                unsigned long int errorGrowthRate = 0) : mInitialError{initialError},
        mErrorGrowthRate{errorGrowthRate}
    {
        mParentTicks = parentTicks;
        mChildTicks = childTicks;
    };

    ~Correlation() = default;

    friend std::ostream &operator<<(std::ostream &, const Correlation &);
    friend bool operator==(const Correlation &c1, const Correlation &c2);
    friend bool operator!=(const Correlation &c1, const Correlation &c2);

    Correlation butWith(std::unordered_map<std::string, u_int64_t>);

    u_int64_t getParentTicks() const;

    u_int64_t getChildTicks() const;

    unsigned long getInitialError() const;

    unsigned long getErrorGrowthRate() const;
};

#endif //__CORRELATION_H

