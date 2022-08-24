/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

#pragma once

#include "RequestHandler.h"

namespace orb {

/**
 * @brief orb::ConfigurationRequestHandler
 *
 * RequestHandler implementation for handling Configuration-related requests issued by the WPE bridge.
 */
class ConfigurationRequestHandler : public RequestHandler {

public:

  ConfigurationRequestHandler();
  ~ConfigurationRequestHandler();

  virtual bool Handle(JsonObject token, std::string method, JsonObject params, JsonObject& response) override;

private:


}; // class ConfigurationRequestHandler

} // namespace orb
