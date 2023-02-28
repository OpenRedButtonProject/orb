/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

#pragma once

#include "ORBBridgeRequestHandler.h"

namespace orb {
/**
 * @brief orb::NetworkRequestHandler
 *
 * RequestHandler implementation for handling Network-related requests issued by the WPE bridge.
 */
class NetworkRequestHandler : public ORBBridgeRequestHandler {
public:

   NetworkRequestHandler();
   ~NetworkRequestHandler();

   virtual bool Handle(json token, std::string method, json params, json& response) override;

private:

   std::string ResolveHostAddress(std::string hostName);
}; // class ManagerRequestHandler
} // namespace orb
