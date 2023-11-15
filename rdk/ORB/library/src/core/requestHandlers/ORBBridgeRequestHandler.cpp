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
#include "ORBBridgeRequestHandler.h"
#include "BroadcastRequestHandler.h"
#include "ConfigurationRequestHandler.h"
#include "ManagerRequestHandler.h"
#include "ParentalControlRequestHandler.h"
#include "ProgrammeRequestHandler.h"
#include "DrmRequestHandler.h"
#include "NetworkRequestHandler.h"

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
    else if (object == "Drm")
    {
        return std::make_shared<DrmRequestHandler>();
    }
    else if (object == "Network")
    {
        return std::make_shared<NetworkRequestHandler>();
    }
    return nullptr;
}
} // namespace orb