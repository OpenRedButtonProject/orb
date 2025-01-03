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

#include <algorithm>
#include <numeric>
#include <regex>
#include <string>
#include "ContentIdentificationService.h"
#include "CSSUtilities.h"
#include "log.h"

namespace NetworkServices {
ContentIdentificationProperties::ContentIdentificationProperties()
{
    m_currentMessage["protocolVersion"] = CSSUtilities::CIIMessageProperties::protocolVersion;
    std::string pStatus;
    for (auto const &str: CSSUtilities::CIIMessageProperties::presentationStatus)
    {
        pStatus += str + '|';
    }
    m_pattern << "^(" << pStatus << "[^ ]+)( [^ ]+)*$";
}

bool ContentIdentificationProperties::setProperty(const std::string &key,
    const Json::Value &value)
{
    bool result = false;

    if (!key.empty())
    {
        //check keys only in CII protocol property names
        if (std::find(std::begin(CSSUtilities::CIIMessageProperties::keys), std::end(
            CSSUtilities::CIIMessageProperties::keys), key) !=
            std::end(CSSUtilities::CIIMessageProperties::keys))
        {
            if (value.isString())
            {
                std::string val = value.asString();
                if (key.find("Url") != std::string::npos && val.back() == '/')
                {
                    LOG(LOG_ERROR, "%s is not a valid url \n", val.c_str());
                    return result;
                }

                if (key == "contentIdStatus" &&
                    std::find(std::begin(CSSUtilities::CIIMessageProperties::contentIdStatus),
                        std::end(CSSUtilities::CIIMessageProperties::contentIdStatus),
                        key) !=
                    std::end(CSSUtilities::CIIMessageProperties::contentIdStatus))
                {
                    LOG(LOG_ERROR, "contentIdStatus cannot be equal to %s\n", val.c_str());
                    return result;
                }
                else if (key == "presentationStatus")
                {
                    if (regex_match(val, std::regex(m_pattern.str())))
                    {
                        m_currentMessage[key] = std::move(value);
                        result = true;
                    }
                    else
                    {
                        LOG(LOG_ERROR, "PresentationStatus cannot be equal to %s\n",
                            val.c_str());
                    }
                }
                else if (key == "protocolVersion" &&
                         val != CSSUtilities::CIIMessageProperties::protocolVersion)
                {
                    LOG(LOG_ERROR, "CII Server supports protocol version %s\n",
                        CSSUtilities::CIIMessageProperties::protocolVersion.c_str());
                }
                else
                {
                    //Regular expression from RFC3986 appendix B
                    if (regex_match(val,
                        std::regex(
                            "^(([^:/?#]+):)?(//([^/?#]*))?([^?#]*)(\\?([^#]*))?(#(.*))?")))
                    {
                        m_currentMessage[key] = std::move(value);
                        result = true;
                    }
                    else
                    {
                        LOG(LOG_ERROR, "Incorrect format for value %s\n", val.c_str());
                    }
                }
            }
            else
            {
                if (key == "private" || (key == "timelines" && value.isArray()) || ((key ==
                                                                                     "mrsUrl" ||
                                                                                     key ==
                                                                                     "teUrl") &&
                                                                                    value.isNull()))
                {
                    m_currentMessage[key] = std::move(value);
                    result = true;
                }
                else
                {
                    LOG(LOG_ERROR, "Incorrect format for %s\n", key.c_str());
                }
            }
        }
        else
        {
            LOG(LOG_ERROR,
                "Key property name does not match with CII message protocol property names\n");
        }
    }
    else
    {
        LOG(LOG_ERROR, "Key Properties and/or values cannot be empty\n");
    }

    return result;
}

ContentIdentificationService::ContentIdentificationService(int port,
                                                           ContentIdentificationProperties *props) :
    WebSocketService("lws-cii", port, false, ""), m_properties(props)
{
}

bool ContentIdentificationService::OnConnection(WebSocketService::WebSocketConnection *connection)
{
    LOG(LOG_INFO, "%s connected to CII service\n", connection->Uri().c_str());

    Json::Value currentMessage(std::move(m_properties->toJson()));
    connection->SendMessage(pack(currentMessage, false));
    m_previousMessage = std::move(currentMessage);
    return true;
}

void ContentIdentificationService::OnMessageReceived(
    WebSocketService::WebSocketConnection *connection,
    const std::string &text)
{
    LOG(LOG_INFO, "Received unexpected message on connection %s: %s\n", connection->Uri().c_str(),
        text.c_str());
}

void ContentIdentificationService::OnDisconnected(WebSocketService::WebSocketConnection *connection)
{
    LOG(LOG_INFO, "%s disconnected from CII service\n", connection->Uri().c_str());
}

void ContentIdentificationService::updateClients(bool onlydiff)
{
    Json::Value currentMessage(std::move(m_properties->toJson()));
    for (auto const &connection : connections_)
    {
        connection.second->SendMessage(pack(currentMessage, onlydiff));
    }
    m_previousMessage = std::move(currentMessage);
}

bool ContentIdentificationService::setCIIMessageProperty(const std::string &key,
    const Json::Value &value)
{
    return m_properties->setProperty(key, value);
}

std::string ContentIdentificationService::pack(const Json::Value &currentMessage, bool onlydiff,
    bool alwaysSendTimelines)
{
    Json::Value diffMessage;
    for (auto const &CIIKey : CSSUtilities::CIIMessageProperties::keys)
    {
        if (currentMessage[CIIKey] != m_previousMessage[CIIKey])
        {
            diffMessage[CIIKey] = currentMessage[CIIKey];
        }
    }

    if (alwaysSendTimelines)
    {
        diffMessage["timelines"] = currentMessage["timelines"];
    }

    // if (onlydiff) {
    //     LOG(LOG_DEBUG, "ContentIdentificationService::pack:: \n%s\n", diffMessage.toStyledString().c_str());
    //     return Json::writeString(m_wbuilder, diffMessage);
    // }

    LOG(LOG_DEBUG, "ContentIdentificationService::pack:: \n%s\n",
        currentMessage.toStyledString().c_str());
#if JSONCPP_VERSION_HEXA > 0x01080200
    return Json::writeString(m_wbuilder, currentMessage);
#else
    return m_writer.write(currentMessage);
#endif
}
}
