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

#pragma once

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace orb {
/**
 * Request handler interface.
 */
class ORBBridgeRequestHandler {
public:

    /**
     * @brief ORBBridgeRequestHandler::MakeErrorResponse
     *
     * Prepare an error response with the specified message.
     *
     * @param message The error message.
     *
     * @return A JSON object representing the error message
     */
    static json MakeErrorResponse(std::string message);

    /**
     * @brief ORBBridgeRequestHandler::Get
     *
     * Get the request handler implementation that corresponds to the given object.
     *
     * @param object The object name
     *
     * @return A shared pointer to the corresponding request handler implementation
     */
    static std::shared_ptr<ORBBridgeRequestHandler> Get(std::string object);

    /**
     * @brief ORBBridgeRequestHandler::Handle
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
}; // class ORBBridgeRequestHandler
} // namespace orb
