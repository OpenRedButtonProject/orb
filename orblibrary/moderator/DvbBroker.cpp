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
 *
 * ORB DvbBroker
 *
 */

#include "DvbBroker.h"
#include "log.h"

using namespace std;

namespace orb
{

DvbBroker::DvbBroker()
{
    mDvbClient = nullptr;
}

DvbBroker::~DvbBroker()
{
}

// Set DVB Client callback object
void DvbBroker::setDvbClient(IDvbClient* dvb_client)
{
    mDvbClient = dvb_client;
}

void DvbBroker::processAitSection(int32_t aitPid, int32_t serviceId, const vector<uint8_t>& data)
{
    LOGI("pid: " << aitPid << "serviceId: " << serviceId);
}

void DvbBroker::processXmlAit(const vector<uint8_t>& data)
{
    LOGI("");
}

void DvbBroker::receiveDsmccContent(int requestId, const vector<uint8_t>& content)
{
    LOGI("requestId: " << requestId);
}

} // namespace orb
