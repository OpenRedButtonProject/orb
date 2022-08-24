/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

#pragma once

#include <plugins/plugins.h>

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
    static JsonObject MakeErrorResponse(std::string message)
    {
        JsonObject errorResponse;
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
    virtual bool Handle(JsonObject token, std::string method, JsonObject params,
        JsonObject& response) = 0;
}; // class RequestHandler
} // namespace orb
