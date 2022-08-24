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
 * @brief orb::ManagerRequestHandler
 *
 * RequestHandler implementation for handling Manager-related requests issued by the WPE bridge.
 */
class ManagerRequestHandler : public RequestHandler {
public:

    ManagerRequestHandler();
    ~ManagerRequestHandler();

    virtual bool Handle(JsonObject token, std::string method, JsonObject params,
        JsonObject& response) override;

private:

    void CheckInternetConnectivity();

    int GetAppIdFromToken(JsonObject token);
}; // class ManagerRequestHandler
} // namespace orb
