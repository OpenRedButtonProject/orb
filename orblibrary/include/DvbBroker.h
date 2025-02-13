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

#include "IDvbClient.h"

namespace orb
{

/**
 * The DVB ORB Broker is the entry point from DVB to ORB broker/moderator functionality
 *
 */
class DvbBroker
{
public:
    DvbBroker();
    ~DvbBroker();

    // Set DVB integration callback object
    void setDvbClient(IDvbClient *dvb_client);

    // -----------------------------------------------
    // Interface functions provided to DVB integration
    // -----------------------------------------------
    void processAitSection(int32_t aitPid, int32_t serviceId, const std::vector<uint8_t>& data);

    void processXmlAit(const std::vector<uint8_t>& data);

    void receiveDsmccContent(int requestId, const std::vector<uint8_t>& content);

private:
    IDvbClient* mDvbClient;

}; // class OrbDvbBroker

} // namespace orb
