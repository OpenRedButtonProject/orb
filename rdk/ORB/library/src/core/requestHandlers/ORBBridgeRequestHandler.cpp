/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */
#include "ORBBridgeRequestHandler.h"
#include "BroadcastRequestHandler.h"
#include "ConfigurationRequestHandler.h"
#include "ManagerRequestHandler.h"
#include "ParentalControlRequestHandler.h"
#include "ProgrammeRequestHandler.h"

namespace orb
{
/**
 * @brief ORBBridgeRequestHandler::MakeErrorResponse
 *
 * Prepare an error response with the specified message.
 *
 * @param message The error message.
 *
 * @return A JSON object representing the error message
 */
json ORBBridgeRequestHandler::MakeErrorResponse(std::string message)
{
   json errorResponse;
   errorResponse["error"] = message;
   return errorResponse;
}

/**
 * @brief ORBBridgeRequestHandler::Get
 *
 * Get the request handler implementation that corresponds to the given object.
 *
 * @param object The object name
 *
 * @return A shared pointer to the corresponding request handler implementation
 */
std::shared_ptr<ORBBridgeRequestHandler> ORBBridgeRequestHandler::Get(std::string object)
{
   if (object == "Broadcast")
   {
      return std::make_shared<BroadcastRequestHandler>();
   }
   else if (object == "Configuration")
   {
      return std::make_shared<ConfigurationRequestHandler>();
   }
   else if (object == "Manager")
   {
      return std::make_shared<ManagerRequestHandler>();
   }
   else if (object == "Programme")
   {
      return std::make_shared<ProgrammeRequestHandler>();
   }
   else if (object == "ParentalControl")
   {
      return std::make_shared<ParentalControlRequestHandler>();
   }
   return nullptr;
}
} // namespace orb