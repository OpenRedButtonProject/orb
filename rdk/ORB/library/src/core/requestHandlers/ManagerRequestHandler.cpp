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

#include "ManagerRequestHandler.h"
#include "ORBPlatform.h"
#include "ORBEngine.h"
#include "ORBLogging.h"
#include <fstream>
#include <sys/sysinfo.h>

#define MANAGER_CREATE_APPLICATION "createApplication"
#define MANAGER_DESTROY_APPLICATION "destroyApplication"
#define MANAGER_SHOW_APPLICATION "showApplication"
#define MANAGER_HIDE_APPLICATION "hideApplication"
#define MANAGER_SEARCH_OWNER "searchOwner"
#define MANAGER_GET_FREE_MEM "getFreeMem"
#define MANAGER_GET_KEY_ICON "getKeyIcon"
#define MANAGER_SET_KEY_VALUE "setKeyValue"
#define MANAGER_GET_KEY_MAXIMUM_VALUE "getKeyMaximumValue"
#define MANAGER_GET_KEY_VALUES "getKeyValues"
#define MANAGER_GET_APPLICATION_SCHEME "getApplicationScheme"

#define KEY_SET_RED 0x1
#define KEY_SET_GREEN 0x2
#define KEY_SET_YELLOW 0x4
#define KET_SET_BLUE 0x8
#define KEY_SET_NAVIGATION 0x10
#define KEY_SET_VCR 0x20
#define KEY_SET_NUMERIC 0x100

namespace orb {
/**
 * Constructor.
 */
ManagerRequestHandler::ManagerRequestHandler()
{
}

/**
 * Destructor.
 */
ManagerRequestHandler::~ManagerRequestHandler()
{
}

/**
 * @brief ManagerRequestHandler::Handle
 *
 * Handles the given Manager request.
 *
 * @param token    (in)  The request token
 * @param method   (in)  The requested method
 * @param params   (in)  A JSON object containing the input parameters (if any)
 * @param response (out) A JSON object containing the response
 *
 * @return true in success, otherwise false
 */
bool ManagerRequestHandler::Handle(
    json token,
    std::string method,
    json params,
    json& response)
{
    bool ret = true;
    response = "{}"_json;

    // Manager.createApplication
    if (method == MANAGER_CREATE_APPLICATION)
    {
        std::string url = params["url"];
        int appId = GetAppIdFromToken(token);
        CheckInternetConnectivity();
        bool canCreate = ORBEngine::GetSharedInstance().GetApplicationManager()->CreateApplication(
            appId, url);
        response["result"] = canCreate;
    }
    // Manager.destroyApplication
    else if (method == MANAGER_DESTROY_APPLICATION)
    {
        int callingAppId = GetAppIdFromToken(token);
        ORBEngine::GetSharedInstance().GetApplicationManager()->DestroyApplication(callingAppId);
    }
    // Manager.showApplication
    else if (method == MANAGER_SHOW_APPLICATION)
    {
        int callingAppId = GetAppIdFromToken(token);
        ORBEngine::GetSharedInstance().GetApplicationManager()->ShowApplication(callingAppId);
    }
    // Manager.hideApplication
    else if (method == MANAGER_HIDE_APPLICATION)
    {
        int callingAppId = GetAppIdFromToken(token);
        ORBEngine::GetSharedInstance().GetApplicationManager()->HideApplication(callingAppId);
    }
    // Manager.searchOwner
    else if (method == MANAGER_SEARCH_OWNER)
    {
        std::string owner = params["owner"];
        response["result"] = owner;
    }
    // Manager.getFreeMem
    else if (method == MANAGER_GET_FREE_MEM)
    {
        struct sysinfo info;
        sysinfo(&info);
        response["result"] = info.freeram;
    }
    // Manager.getKeyIcon
    else if (method == MANAGER_GET_KEY_ICON)
    {
        int code = params["code"];
        std::string keyUri = ORBEngine::GetSharedInstance().GetORBPlatform()->Manager_GetKeyIcon(
            code);
        response["result"] = keyUri;
    }
    // Manager.setKeyValue
    else if (method == MANAGER_SET_KEY_VALUE)
    {
        int value = params["value"];
        int callingAppId = GetAppIdFromToken(token);
        ORBEngine::GetSharedInstance().GetORBPlatform()->Platform_SetCurrentKeySetMask(value);
        response["result"] = ORBEngine::GetSharedInstance().GetApplicationManager()->SetKeySetMask(
            callingAppId, value);
    }
    // Manager.getKeyMaximumValue
    else if (method == MANAGER_GET_KEY_MAXIMUM_VALUE)
    {
        int maximumValue =
            KEY_SET_RED |
            KEY_SET_GREEN |
            KEY_SET_YELLOW |
            KET_SET_BLUE |
            KEY_SET_NAVIGATION |
            KEY_SET_VCR |
            KEY_SET_NUMERIC;
        response["result"] = maximumValue;
    }
    // Manager.getKeyValues
    else if (method == MANAGER_GET_KEY_VALUES)
    {
        int callingAppId = GetAppIdFromToken(token);
        int value = ORBEngine::GetSharedInstance().GetApplicationManager()->GetKeySetMask(
            callingAppId);
        response["result"] = value;
    }
    else if (method == MANAGER_GET_APPLICATION_SCHEME)
    {
        int callingAppId = GetAppIdFromToken(token);
        response["result"] = ORBEngine::GetSharedInstance().GetApplicationManager()->GetApplicationScheme(callingAppId);
    }
    // UnknownMethod
    else
    {
        response = ORBBridgeRequestHandler::MakeErrorResponse("UnknownMethod");
        ret = false;
    }

    return ret;
}

/**
 * @brief ManagerRequestHandler::CheckInternetConnectivity
 *
 * Checks if the device is currently connected to the Internet, and if yes,
 * notifies the application manager accordingly.
 */
void ManagerRequestHandler::CheckInternetConnectivity()
{
    ORB_LOG_NO_ARGS();
    bool isConnectedToInternet =
        ORBEngine::GetSharedInstance().GetORBPlatform()->Network_IsConnectedToInternet();
    ORBEngine::GetSharedInstance().GetApplicationManager()->OnNetworkAvailabilityChanged(
        isConnectedToInternet);
}

/**
 * Get the application id from the specified JSON token.
 *
 * @param token The token
 *
 * @return The application id or INVALID_APP_ID
 */
int ManagerRequestHandler::GetAppIdFromToken(json token)
{
    json payload = token["payload"];
    int appId = INVALID_APP_ID;
    if (!payload["appId"].empty())
    {
        appId = payload["appId"];
    }
    return appId;
}
} // namespace orb
