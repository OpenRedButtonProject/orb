/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

#pragma once

#include "RequestHandler.h"
#include "application_manager.h"

namespace orb {

/**
 * @brief orb::BroadcastRequestHandler
 *
 * RequestHandler implementation for handling Broadcast-related requests issued by the WPE bridge.
 */
class BroadcastRequestHandler : public RequestHandler {

public:

  BroadcastRequestHandler();
  ~BroadcastRequestHandler();

  virtual bool Handle(JsonObject token, std::string method, JsonObject params, JsonObject& response) override;

private:

  int AddStreamEventListener(std::string targetUrl, std::string eventName, int componentTag, int streamEventId);
  void RemoveStreamEventListener(int id);
  bool IsRequestAllowed(JsonObject token, ApplicationManager::MethodRequirement methodType);

}; // class BroadcastRequestHandler

} // namespace orb
