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
 * @brief orb::DrmRequestHandler
 *
 * RequestHandler implementation for handling Drm-related requests issued by the WPE bridge.
 */
class DrmRequestHandler : public ORBBridgeRequestHandler {
public:

    DrmRequestHandler();
    ~DrmRequestHandler();

    virtual bool Handle(json token, std::string method, json params, json& response) override;
}; // class DrmRequestHandler
} // namespace orb
