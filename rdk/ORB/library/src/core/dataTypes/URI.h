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

#pragma once

#include <memory>
#include <string>

namespace orb {
/**
 * @brief orb::URI
 *
 * Implementation of the URI data type.
 */
class URI {
public:

    URI();
    ~URI();

    static std::shared_ptr<URI> Parse(std::string uri);

    std::string GetProtocol() const;
    std::string GetHost() const;
    std::string GetPort() const;
    std::string GetPath() const;
    std::string GetQueryString() const;

    void SetProtocol(std::string protocol);
    void SetHost(std::string host);
    void SetPort(std::string port);
    void SetPath(std::string path);
    void SetQueryString(std::string queryString);

private:

    std::string m_protocol;
    std::string m_host;
    std::string m_port;
    std::string m_path;
    std::string m_queryString;
}; // class URI
} // namespace orb
