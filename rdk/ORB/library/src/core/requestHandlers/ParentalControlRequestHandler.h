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
#include "ParentalRating.h"

#include <map>
#include <vector>

namespace orb {
/**
 * @brief orb::ParentalControlRequestHandler
 *
 * RequestHandler implementation for handling ParentalControl-related requests issued by the WPE bridge.
 */
class ParentalControlRequestHandler : public ORBBridgeRequestHandler {
public:

    ParentalControlRequestHandler();
    ~ParentalControlRequestHandler();

    virtual bool Handle(json token, std::string method, json params, json& response) override;

private:

    std::map<std::string, std::vector<ParentalRating> > GetRatingSchemes();
    std::shared_ptr<ParentalRating> GetThreshold(json params);
    bool IsRatingBlocked(json params);
}; // class ParentalControlRequestHandler
} // namespace orb
