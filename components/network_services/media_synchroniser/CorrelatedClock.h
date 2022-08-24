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

#ifndef __CORRELATEDCLOCK_H
#define __CORRELATEDCLOCK_H

#include "ClockBase.h"
#include "ClockUtilities.h"
#include "Correlation.h"

class CorrelatedClock : public ClockBase {
private:
   double mFreq;
   Correlation mCorrelation;

public:
   explicit CorrelatedClock(ClockBase *parentClock,
                            Correlation cor = Correlation(0, 0),
                            double tickRate = 0,
                            double speed = 1.0) :
      ClockBase{tickRate, speed, parentClock},
      mCorrelation{cor}
   {
      if (mParent)
      {
         mParent->bind(this);
      }
      mFreq = tickRate;
   };

   ~CorrelatedClock() override
   {
      if (mParent != nullptr)
      {
         mParent->unbind(this);
      }
   };

   friend std::ostream &operator<<(std::ostream &, const CorrelatedClock &);


   Correlation &getCorrelation();

   void setCorrelation(const Correlation &cor);

   void setCorrelationAndSpeed(const Correlation &, double);


   ClockBase* getParent() const override;

   void setParent(ClockBase *) override;


   u_int64_t getTicks() const override;

   double getTickRate() const override;

   void setTickRate(double) override;


   u_int64_t fromParentTicks(u_int64_t) override;

   u_int64_t toParentTicks(u_int64_t) override;


   double getSpeed() const override;

   void setSpeed(double) override;

   double errorAtTime(double) override;

   double calcWhen(double) override;

   double isChangeSignificant(Correlation &, double, double);

   double quantifyChange(Correlation &, double);

   void rebaseCorrelationAtTicks(unsigned long tickValue);
};


#endif // __CORRELATEDCLOCK_H

