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

#ifndef WIP_DVBCSS_HBBTV_CONTENTIDENTIFICATIONSERVICE_H
#define WIP_DVBCSS_HBBTV_CONTENTIDENTIFICATIONSERVICE_H

#include "websocket_service.h"
#include <json/json.h>
#include <string>
#include <list>
#include <iostream>
#include <sstream>

namespace NetworkServices {
class ContentIdentificationProperties {
public:
    ContentIdentificationProperties();
    bool setProperty(const std::string &key,
        const Json::Value &value);
    void removeProperty(const std::string &key)
    {
        m_currentMessage.removeMember(key);
    }

    Json::Value getProperty(const std::string &key) const
    {
        return m_currentMessage[key];
    }

    Json::Value toJson() const
    {
        return m_currentMessage;
    }

private:
    std::stringstream m_pattern;
    Json::Value m_currentMessage;
};

class ContentIdentificationService : public WebSocketService {
public:

    ContentIdentificationService(int port, ContentIdentificationProperties *props);

    bool OnConnection(WebSocketConnection *connection) override;

    void OnMessageReceived(WebSocketConnection *connection, const std::string &text) override;

    void OnDisconnected(WebSocketConnection *connection) override;

    void updateClients(bool onlydiff);

    bool setCIIMessageProperty(const std::string &key, const Json::Value &value);

    int nrOfClients() const
    {
        return connections_.size();
    }

private:
    ContentIdentificationProperties *m_properties;
    Json::Value m_previousMessage;
#if JSONCPP_VERSION_HEXA > 0x01080200
    Json::StreamWriterBuilder m_wbuilder;
#else
    Json::FastWriter m_writer;
#endif
    std::stringstream m_pattern;

    std::string pack(const Json::Value &currentMessage, bool onlydiff, bool alwaysSendTimelines =
            true);
};
}
#endif //WIP_DVBCSS_HBBTV_CONTENTIDENTIFICATIONSERVICE_H
