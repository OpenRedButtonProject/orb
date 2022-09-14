/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

#pragma once

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace orb {
/**
 * Request handler interface.
 */
class RequestHandler {
public:

   /**
    * @brief RequestHandler::MakeErrorResponse
    *
    * Prepare an error response with the specified message.
    *
    * @param message The error message.
    *
    * @return A JSON object representing the error message
    */
   static json MakeErrorResponse(std::string message)
   {
      json errorResponse;
      errorResponse["error"] = message;
      return errorResponse;
   }

   /**
    * @brief RequestHandler::Handle
    *
    * Handle the specified request issued by the WPE bridge.
    *
    * @param token     (in)  The JSON token included in the request
    * @param method    (in)  The requested method
    * @param params    (in)  The requested method's input parameters
    * @param response  (out) The resulting response
    *
    * @return true in success, otherwise false
    */
   virtual bool Handle(json token, std::string method, json params, json& response) = 0;
}; // class RequestHandler
} // namespace orb
