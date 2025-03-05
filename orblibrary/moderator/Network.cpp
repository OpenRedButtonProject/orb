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
 * ORB Network
 *
 */

#include <sys/sysinfo.h>

#include "Network.hpp"
#include "log.h"

using namespace std;

namespace orb
{

Network::Network()
{
}


Network::~Network()
{
}

string Network::request(string method, Json::Value token, Json::Value params)
{
    string response = "{\"error\": \"Request not implemented\"}";

    LOGI("method: " << method)

    if (method == "resolveHostAddress")
    {
        LOGI("")
    }
    else // Unknown Method
    {
        LOGE("")
    }

    return response;
}


} // namespace orb
