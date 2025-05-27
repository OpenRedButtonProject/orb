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
 * ORB Application Manager
 *
 */

#include <sys/sysinfo.h>

#include "AppManager.hpp"
#include "log.h"

#define LINKED_APP_SCHEME_1_1 "urn:dvb:metadata:cs:LinkedApplicationCS:2019:1.1"

using namespace std;

namespace orb
{

AppManager& AppManager::instance()
{
    static AppManager s_interface;
    return s_interface;
}

string AppManager::executeRequest(string method, Json::Value token, Json::Value params, ApplicationType apptype)
{
    // TODO Set up proper responses
    string response = R"({"Response": "AppManager request [)" + method + R"(] not implemented"})";

    LOGI("Request with method [" + method + "] received");
    if (method == "createApplication")
    {
        LOGI("app type: ") << apptype;
    }
    else if (method == "destroyApplication")
    {
        // no response
        LOGI("");
    }
    else if (method == "showApplication")
    {
        // no response
        LOGI("");
    }
    else if (method == "hideApplication")
    {
        // no response
        LOGI("");
    }
    else if (method == "searchOwner")
    {
        LOGI("");
    }
    else if (method == "getFreeMem")
    {
        // TODO: ask DVB client for this
        LOGI("");
    }
    else if (method == "getKeyIcon")
    {
        LOGI("");
    }
    else if (method == "setKeyValue")
    {
        LOGI("");
    }
    else if (method == "getKeyMaximumValue")
    {
        LOGI("");
    }
    else if (method == "getKeyValues")
    {
        LOGI("");
    }
    else if (method == "getApplicationScheme")
    {
        LOGI("");
    }
    else if (method == "getApplicationUrl")
    {
        LOGI("");
    }
    else if (method == "getRunningAppIds")
    {
        // TODO: string array?
        LOGI("");
    }
    else // Unknown Method
    {
        response = R"({"error": "AppManager request [)" + method + R"(] invalid method"})";
        LOGE("Invalid Method [" + method +"]");
    }

    LOGI("Response: " << response);

    return response;
}

bool AppManager::IsRequestAllowed(string token)
{
    return false;
}

} // namespace orb
