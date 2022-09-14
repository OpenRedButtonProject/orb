/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
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
