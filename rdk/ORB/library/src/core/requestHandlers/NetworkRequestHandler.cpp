/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */
#include "NetworkRequestHandler.h"

#include <netdb.h>
#include <arpa/inet.h>

#define NETWORK_RESOLVE_HOST_ADDRESS "resolveHostAddress"

namespace orb {
/**
 * Constructor.
 */
NetworkRequestHandler::NetworkRequestHandler()
{
}

/**
 * Destructor.
 */
NetworkRequestHandler::~NetworkRequestHandler()
{
}

/**
 * @brief NetworkRequestHandler::Handle
 *
 * Handles the given Manager request.
 *
 * @param token    (in)  The request token
 * @param method   (in)  The requested method
 * @param params   (in)  A JSON object containing the input parameters (if any)
 * @param response (out) A JSON object containing the response
 *
 * @return true in success, otherwise false
 */
bool NetworkRequestHandler::Handle(
   json token,
   std::string method,
   json params,
   json& response)
{
   bool ret = true;
   response = "{}"_json;

   // Network.resolveHostAddress
   if (method == NETWORK_RESOLVE_HOST_ADDRESS)
   {
      std::string hostName = params["hostName"];
      std::string hostAddress = ResolveHostAddress(hostName);
      response["result"] = hostAddress;
   }
   // UnknownMethod
   else
   {
      response = ORBBridgeRequestHandler::MakeErrorResponse("UnknownMethod");
      ret = false;
   }

   return ret;
}

/**
 * Resolve the IP address of the specified host.
 *
 * @param hostName The given host name
 *
 * @return The host IP address or an empty string in case of failure
 */
std::string NetworkRequestHandler::ResolveHostAddress(std::string hostName)
{
   std::string hostAddress;
   struct hostent *hp = gethostbyname(hostName.c_str());
   if (hp->h_addr_list[0] != NULL)
   {
      hostAddress = inet_ntoa(*(struct in_addr *)(hp->h_addr_list[0]));
   }
   return hostAddress;
}
} // namespace orb