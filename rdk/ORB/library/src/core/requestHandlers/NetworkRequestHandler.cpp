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
#include "NetworkRequestHandler.h"
#include "ORBEngine.h"

#include <netdb.h>
#include <arpa/inet.h>

#define NETWORK_RESOLVE_HOST_ADDRESS "resolveHostAddress"
#define NETWORK_RESOLVE_NETWORK_ERROR "resolveNetworkError"

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
        std::string hostName = params["hostname"];
        std::string hostAddress = ResolveHostAddress(hostName);
        response["result"] = hostAddress;
    }
    // Network.resolveNetworkError
    else if (method == NETWORK_RESOLVE_NETWORK_ERROR)
    {
        std::string responseText = params.value("responseText", "");
        std::string dashErrorCode = ORBEngine::GetSharedInstance().GetORBPlatform()->Network_ResolveNetworkError(responseText);
        response["result"] = dashErrorCode;
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
    std::string hostAddress = "";
    struct hostent *hp = gethostbyname(hostName.c_str());
    if (hp != NULL && hp->h_addr_list[0] != NULL)
    {
        hostAddress = inet_ntoa(*(struct in_addr *)(hp->h_addr_list[0]));
    }
    return hostAddress;
}
} // namespace orb