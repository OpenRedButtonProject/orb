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

#include "ORBBridgeRequestHandler.h"
#include "application_manager.h"

namespace orb {
/**
 * @brief orb::BroadcastRequestHandler
 *
 * RequestHandler implementation for handling Broadcast-related requests issued by the WPE bridge.
 */
class BroadcastRequestHandler : public ORBBridgeRequestHandler {
public:

    BroadcastRequestHandler();
    ~BroadcastRequestHandler();

    virtual bool Handle(json token, std::string method, json params, json& response) override;

private:

    int AddStreamEventListener(std::string targetUrl, std::string eventName, int componentTag, int
        streamEventId);
    void RemoveStreamEventListener(int id);
    bool IsRequestAllowed(json token, ApplicationManager::MethodRequirement methodType);
    void CancelSearch(int queryId);
}; // class BroadcastRequestHandler
} // namespace orb
