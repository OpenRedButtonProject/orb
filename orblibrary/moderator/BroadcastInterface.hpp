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

#include <string>
#include <json/json.h>

#include "ComponentBase.hpp"

namespace orb
{
class IOrbBrowser;

class BroadcastInterface : ComponentBase
{
public:
    struct ParentalRating {
        std::string name;
        std::string scheme;
        int value;
        int labels;
        std::string region;
    };

    struct DASHEvent {
        std::string id;
        double startTime;
        double duration;
        std::string contentEncoding;
    };

    explicit BroadcastInterface(IOrbBrowser* browser);

    /**
     * BroadcastInterface request
     *
     * @param method BroadcastInterface method
     * @param token TODO to be replaced by application ID
     * @param params JSON params. TODO add details
     *
     * @return JSON encoded response string
     */
    std::string executeRequest(std::string method, Json::Value token, Json::Value params) override;

    // BroadcastSessionCallback interface implementation
    
    void DispatchChannelStatusChangedEvent(const int onetId, const int transId, const int servId,
        const int statusCode, const bool permanentError);
    
    void DispatchServiceInstanceChangedEvent(const int index);

    void DispatchParentalRatingChangeEvent(const bool blocked);

    void DispatchParentalRatingErrorEvent(const std::string contentID,
        const std::vector<ParentalRating> ratings, const std::string DRMSystemID);

    void DispatchSelectedComponentChangedEvent(const int componentType);

    void DispatchStreamEvent(const int id, const std::string name, const std::string data,
        const std::string text, const std::string status, const DASHEvent dashEvent);

    void DispatchProgrammesChangedEvent();

private:
    IOrbBrowser *mOrbBrowser;

}; // class BroadcastInterface

} // namespace orb
