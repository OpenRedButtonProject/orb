/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

#pragma once

#include "ORBBridgeRequestHandler.h"

namespace orb {
/**
 * @brief orb::ManagerRequestHandler
 *
 * RequestHandler implementation for handling Manager-related requests issued by the WPE bridge.
 */
class ManagerRequestHandler : public ORBBridgeRequestHandler {
public:

    ManagerRequestHandler();
    ~ManagerRequestHandler();

    virtual bool Handle(json token, std::string method, json params, json& response) override;

private:

    void CheckInternetConnectivity();

    int GetAppIdFromToken(json token);
}; // class ManagerRequestHandler
} // namespace orb
