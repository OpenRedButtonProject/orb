/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

#pragma once

#include "ORBBridgeRequestHandler.h"
#include "ParentalRating.h"

namespace orb {
/**
 * @brief orb::ProgrammeRequestHandler
 *
 * RequestHandler implementation for handling Programme-related requests issued by the WPE bridge.
 */
class ProgrammeRequestHandler : public ORBBridgeRequestHandler {
public:

   ProgrammeRequestHandler();
   ~ProgrammeRequestHandler();

   virtual bool Handle(json token, std::string method, json params, json& response) override;

private:

   std::shared_ptr<ParentalRating> GetParentalRating();
}; // class ProgrammeRequestHandler
} // namespace orb
