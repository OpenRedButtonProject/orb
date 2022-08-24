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

#include "WallClockService.h"
#include "media_synchroniser.h"
#include "log.h"
#include "SysClock.h"

#include <math.h>
#include <iostream>

#define NANOS_IN_SEC (u_int64_t)1000000000

namespace NetworkServices {
WallClockService::WallClockService(int port, ::SysClock *sysClock, bool followUp) :
    UdpSocketService("lws-wc", port, false), m_clock(sysClock)
{
    m_followup = followUp;
}

bool WallClockService::OnConnection()
{
    LOG(LOG_DEBUG, "Connected to WC service: \n");
    return true;
}

void WallClockService::OnMessageReceived(struct lws *wsi, const std::string &text)
{
    u_int64_t recv_ticks = m_clock->getTicks();
    u_int64_t recv_nanos = ClockUtilities::timeNanos();
    WCMessage msg = WCMessage::Unpack(text.c_str(), text.length());
    WCMessage reply = msg;     // copy original

    if (msg.msgtype == WCMessage::TYPE_REQUEST)
    {
        reply.receiveNanos = recv_nanos;

        if (m_followup)
        {
            reply.msgtype = WCMessage::TYPE_RESPONSE_WITH_FOLLOWUP;
        }
        else
        {
            reply.msgtype = WCMessage::TYPE_RESPONSE;
        }
        reply.setPrecision(m_clock->dispersionAtTime(recv_ticks));
        reply.setMaxFreqError(m_clock->getRootMaxFreqError());
        reply.transmitNanos = ClockUtilities::timeNanos();
        WCMessage::WCMsgData msgData(reply.pack());
        SendMessage(wsi, &msgData, sizeof(WCMessage::WCMsgData));

        if (m_followup)
        {
            WCMessage followupReply = reply;     // copy reply
            followupReply.transmitNanos = ClockUtilities::timeNanos();
            followupReply.msgtype = WCMessage::TYPE_FOLLOWUP;
            msgData = followupReply.pack();
            SendMessage(wsi, &msgData, sizeof(WCMessage::WCMsgData));
        }
    }
    else
    {
        LOG(LOG_ERROR, "Wall clock server received non request message.\n");
    }
}

void WallClockService::OnDisconnected()
{
    LOG(LOG_INFO, "disconnected from WC service\n");
}

WallClockService::WCMessage::WCMsgData WallClockService::WCMessage::pack()
{
    WCMsgData msgData;
    msgData.version = 0;
    msgData.msgtype = 2;
    msgData.precision = precision;
    msgData._ = 0;
    msgData.maxFreqError = htonl(maxFreqError);
    if (!originalOriginate.isNull())
    {
        msgData.os = htonl((u_int32_t) (originalOriginate.value().os));
        msgData.on = htonl((u_int32_t) (originalOriginate.value().on));
    }
    else
    {
        msgData.os = htonl((u_int32_t) (originateNanos / NANOS_IN_SEC));
        msgData.on = htonl((u_int32_t) (originateNanos % NANOS_IN_SEC));
    }

    msgData.rs = htonl((u_int32_t) (receiveNanos / NANOS_IN_SEC));
    msgData.rn = htonl((u_int32_t) (receiveNanos % NANOS_IN_SEC));
    msgData.ts = htonl((u_int32_t) (transmitNanos / NANOS_IN_SEC));
    msgData.tn = htonl((u_int32_t) (transmitNanos % NANOS_IN_SEC));
    LOG(LOG_DEBUG, "pack %llu:%llu:%llu\n", originateNanos, receiveNanos, transmitNanos);
    return msgData;
}

void WallClockService::WCMessage::setPrecision(double precisionSecs)
{
    if (precisionSecs != 0)
    {
        precision = (int8_t) ceil(log(precisionSecs) / log(2));
    }
    else
    {
        precision = 0;
    }
    LOG(LOG_DEBUG, "precision: %d %f\n", precision, precisionSecs);
}

void WallClockService::WCMessage::setMaxFreqError(double maxFreqErrorPpm)
{
    maxFreqError = maxFreqErrorPpm * 256;
    LOG(LOG_DEBUG, "maxFreqError: %f", maxFreqErrorPpm);
}

WallClockService::WCMessage WallClockService::WCMessage::Unpack(const void *msg, size_t length)
{
    WCMessage wc_msg;
    Nullable<OriginalOriginate> wc_origin;
    if (sizeof(WCMsgData) == length)
    {
        WCMsgData msgData;
        memcpy(&msgData, msg, length);
        if (msgData.version == 0)
        {
            if (msgData.msgtype <= TYPE_FOLLOWUP)
            {
                msgData.os = ntohl(msgData.os);
                msgData.on = ntohl(msgData.on);
                msgData.maxFreqError = ntohl(msgData.maxFreqError);
                msgData.rs = ntohl(msgData.rs);
                msgData.rn = ntohl(msgData.rn);
                msgData.ts = ntohl(msgData.ts);
                msgData.tn = ntohl(msgData.tn);
                u_int64_t o = (u_int64_t) msgData.os * NANOS_IN_SEC + msgData.on;
                u_int64_t r = (u_int64_t) msgData.rs * NANOS_IN_SEC + msgData.rn;
                u_int64_t t = (u_int64_t) msgData.ts * NANOS_IN_SEC + msgData.tn;
                if (msgData.on >= NANOS_IN_SEC)
                {
                    wc_origin = { msgData.os, msgData.on };
                }
                wc_msg = {(MessageType) msgData.msgtype, msgData.precision, msgData.maxFreqError, o,
                          r, t, wc_origin};
                LOG(LOG_DEBUG, "unpack %d:%d:%d\n", msgData.os, msgData.rs, msgData.ts);
            }
            else
            {
                LOG(LOG_ERROR, "Faulty message type [%d].\n", msgData.msgtype);
            }
        }
        else
        {
            LOG(LOG_ERROR, "Faulty payload version [%d].\n", msgData.version);
        }
    }
    else
    {
        LOG(LOG_ERROR, "Faulty payload size [%zu]. Expected size is %u.\n", length,
            sizeof(WCMsgData));
    }

    return wc_msg;
}
} // namespace NetworkServices