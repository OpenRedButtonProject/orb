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
 */

#include "URI.h"
#include <algorithm>

namespace orb {
URI::URI()
{
}

URI::~URI()
{
}

std::shared_ptr<URI> URI::Parse(std::string uri)
{
    typedef std::string::iterator iterator_t;
    std::shared_ptr<URI> result = std::make_shared<URI>();
    if (uri.empty())
    {
        return result;
    }
    iterator_t uriEnd = uri.end();
    iterator_t queryStart = std::find(uri.begin(), uriEnd, L'?');
    iterator_t protocolStart = uri.begin();
    iterator_t protocolEnd = std::find(protocolStart, uriEnd, L':');
    if (protocolEnd != uriEnd)
    {
        std::string prot = &*(protocolEnd);
        std::string separator("://");
        if ((prot.length() > 3) && (prot.substr(0, 3) == separator))
        {
            result->SetProtocol(std::string(protocolStart, protocolEnd));
            protocolEnd += 3;
        }
        else
        {
            protocolEnd = uri.begin();
        }
    }
    else
    {
        protocolEnd = uri.begin();
    }
    iterator_t hostStart = protocolEnd;
    iterator_t pathStart = std::find(hostStart, queryStart, L'/');
    iterator_t hostEnd = std::find(protocolEnd, (pathStart != uriEnd) ? pathStart : queryStart,
        L':');
    result->SetHost(std::string(hostStart, hostEnd));
    if ((hostEnd != uriEnd) && ((&*(hostEnd))[0] == L':'))
    {
        hostEnd++;
        iterator_t portEnd = (pathStart != uriEnd) ? pathStart : queryStart;
        result->SetPort(std::string(hostEnd, portEnd));
    }
    if (pathStart != queryStart)
    {
        result->SetPath(std::string(pathStart, queryStart));
    }
    if (queryStart != uriEnd)
    {
        result->SetQueryString(std::string(queryStart, uri.end()));
    }
    return result;
}

std::string URI::GetProtocol() const
{
    return m_protocol;
}

std::string URI::GetHost() const
{
    return m_host;
}

std::string URI::GetPort() const
{
    return m_port;
}

std::string URI::GetPath() const
{
    return m_path;
}

std::string URI::GetQueryString() const
{
    return m_queryString;
}

void URI::SetProtocol(std::string protocol)
{
    m_protocol = protocol;
}

void URI::SetHost(std::string host)
{
    m_host = host;
}

void URI::SetPort(std::string port)
{
    m_port = port;
}

void URI::SetPath(std::string path)
{
    m_path = path;
}

void URI::SetQueryString(std::string queryString)
{
    m_queryString = queryString;
}
} // namespace orb
