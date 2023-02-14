/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

#pragma once

#include "ORBBridgeRequestHandler.h"
#include "application_manager.h"

namespace orb {
/**
 * @brief orb::BroadcastRequestHandler
 *
 * RequestHandler implementation for handling Broadcast-related requests issued by the WPE bridge.
 */
class BroadcastRequestHandler : public ORBBridgeRequestHandler {
public:

   BroadcastRequestHandler();
   ~BroadcastRequestHandler();

   virtual bool Handle(json token, std::string method, json params, json& response) override;

private:

   int AddStreamEventListener(std::string targetUrl, std::string eventName, int componentTag, int streamEventId);
   void RemoveStreamEventListener(int id);
   bool IsRequestAllowed(json token, ApplicationManager::MethodRequirement methodType);
   void CancelSearch(int queryId);
}; // class BroadcastRequestHandler
} // namespace orb
