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

#ifndef WIP_DVBCSS_HBBTV_WALLCLOCKSERVICE_H
#define WIP_DVBCSS_HBBTV_WALLCLOCKSERVICE_H

#include "UdpSocketService.h"
#include "SysClock.h"
#include "Nullable.h"

#include <string>

namespace NetworkServices {
class WallClockService : public UdpSocketService {
public:
   WallClockService(int port, ::SysClock *sysClock, bool followUp = true);
   virtual ~WallClockService() = default;
   bool OnConnection();
   void OnMessageReceived(struct lws *wsi, const std::string &text);
   void OnDisconnected();

private:
   struct WCMessage
   {
      struct WCMsgData
      {
         u_int8_t version;
         u_int8_t msgtype;
         int8_t precision;
         u_int8_t _;
         u_int32_t maxFreqError;
         u_int32_t os;
         u_int32_t on;
         u_int32_t rs;
         u_int32_t rn;
         u_int32_t ts;
         u_int32_t tn;
      };

      struct OriginalOriginate
      {
         u_int64_t os;
         u_int64_t on;
      };

      enum MessageType
      {
         TYPE_REQUEST = 0,        // request
         TYPE_RESPONSE = 1,        // response with no follow-up
         TYPE_RESPONSE_WITH_FOLLOWUP = 2,        // response to be followed by a follow-up response
         TYPE_FOLLOWUP = 3        // follow-up response
      };

      MessageType msgtype;
      int8_t precision;
      u_int32_t maxFreqError;
      u_int64_t originateNanos;
      u_int64_t receiveNanos;
      u_int64_t transmitNanos;
      Nullable<OriginalOriginate> originalOriginate;

      WCMsgData pack();
      static WCMessage Unpack(const void *msg, size_t length);

      void setPrecision(double precisionSecs);
      void setMaxFreqError(double maxFreqErrorPpm);
   };

   bool m_followup;
   ::SysClock *m_clock;
};
} // namespace NetworkServices

#endif //WIP_DVBCSS_HBBTV_WALLCLOCKSERVICE_H

