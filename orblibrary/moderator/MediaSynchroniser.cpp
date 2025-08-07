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
 * ORB MediaSynchroniser
 *
 */

#include <sys/sysinfo.h>

#include "MediaSynchroniser.hpp"
#include "log.hpp"

using namespace std;

namespace orb
{
string MediaSynchroniser::executeRequest(string method, Json::Value token, Json::Value params)
{
    // TODO Set up proper responses
    string response = R"({"Response": "MediaSynchroniser request [)" + method + R"(] not implemented"})";

    LOGI("Request with method [" + method + "] received");
    if (method == "instantiate")
    {
        // integer response
        LOGI("");
    }
    else if (method == "initialise")
    {
        // boolean response
        LOGI("");
    }
    else if (method == "destroy")
    {
        // no response
        LOGI("");
    }
    else if (method == "enableInterDeviceSync")
    {
        // boolean response
        LOGI("");
    }
    else if (method == "disableInterDeviceSync")
    {
        // no response
        LOGI("");
    }
    else if (method == "nrOfSlaves")
    {
        // integer response
        LOGI("");
    }
    else if (method == "interDeviceSyncEnabled")
    {
        // boolean response
        LOGI("");
    }
    else if (method == "getContentIdOverride")
    {
        // string response
        LOGI("");
    }
    else if (method == "getBroadcastCurrentTime")
    {
        // long integer response
        LOGI("");
    }
    else if (method == "startTimelineMonitoring")
    {
        // boolean response
        LOGI("");
    }
    else if (method == "stopTimelineMonitoring")
    {
        // no response
        LOGI("");
    }
    else if (method == "setContentIdOverride")
    {
        // no response
        LOGI("");
    }
    else if (method == "setContentTimeAndSpeed")
    {
        // no response
        LOGI("");
    }
    else if (method == "updateCssCiiProperties")
    {
        // no response
        LOGI("");
    }
    else if (method == "setTimelineAvailability")
    {
        // boolean response
        LOGI("");
    }
    else // Unknown Method
    {
        response = R"({"error": "MediaSynchroniser request [)" + method + R"(] invalid method"})";
        LOGE("Invalid Method [" + method +"]");
    }

    return response;
}

} // namespace orb
