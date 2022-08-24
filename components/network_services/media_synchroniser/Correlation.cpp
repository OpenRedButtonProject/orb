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
#include "Correlation.h"

Correlation Correlation::butWith(std::unordered_map<std::string, u_int64_t> correlationConfig)
{
   u_int64_t parentTicks = mParentTicks;
   u_int64_t childTicks = mChildTicks;
   unsigned long int initialError = mInitialError;
   unsigned long int errorGrowthRate = mErrorGrowthRate;

   auto it = correlationConfig.find("parentTicks");
   if (it != correlationConfig.end())
   {
      parentTicks = it->second;
   }

   it = correlationConfig.find("childTicks");
   if (it != correlationConfig.end())
   {
      childTicks = it->second;
   }

   it = correlationConfig.find("initialError");
   if (it != correlationConfig.end())
   {
      initialError = it->second;
   }

   it = correlationConfig.find("errorGrowthRate");
   if (it != correlationConfig.end())
   {
      errorGrowthRate = it->second;
   }

   return Correlation(parentTicks, childTicks, initialError, errorGrowthRate);
}

u_int64_t Correlation::getParentTicks() const
{
   return mParentTicks;
}

u_int64_t Correlation::getChildTicks() const
{
   return mChildTicks;
}

unsigned long Correlation::getInitialError() const
{
   return mInitialError;
}

unsigned long Correlation::getErrorGrowthRate() const
{
   return mErrorGrowthRate;
}

std::ostream &operator<<(std::ostream &os, const Correlation &cor)
{
   os << "Correlation(parentTicks=" << cor.mParentTicks <<
      ", childTicks=" << cor.mChildTicks <<
      ", initialError=" << cor.mInitialError <<
      ", errorGrowthRate=" << cor.mErrorGrowthRate << ")\n";
   return os;
}

bool operator==(const Correlation &c1, const Correlation &c2)
{
   return(c1.mParentTicks == c2.mParentTicks &&
          c1.mChildTicks == c2.mChildTicks &&
          c1.mErrorGrowthRate == c2.mErrorGrowthRate &&
          c1.mInitialError == c2.mInitialError);
}

bool operator!=(const Correlation &c1, const Correlation &c2)
{
   return !(c1 == c2);
}
