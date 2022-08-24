/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

#pragma once

#include "RequestHandler.h"
#include "ParentalRating.h"

namespace orb {

/**
 * @brief orb::ProgrammeRequestHandler
 *
 * RequestHandler implementation for handling Programme-related requests issued by the WPE bridge.
 */
class ProgrammeRequestHandler : public RequestHandler {

public:

  ProgrammeRequestHandler();
  ~ProgrammeRequestHandler();

  virtual bool Handle(JsonObject token, std::string method, JsonObject params, JsonObject& response) override;

private:

  std::shared_ptr<ParentalRating> GetParentalRating();

}; // class ProgrammeRequestHandler

} // namespace orb
