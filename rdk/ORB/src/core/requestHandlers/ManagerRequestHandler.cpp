/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

#include "ManagerRequestHandler.h"
#include "ORBPlatform.h"
#include "ORB.h"
#include <fstream>

using namespace WPEFramework::Plugin;

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
{}

/**
 * Destructor.
 */
ManagerRequestHandler::~ManagerRequestHandler()
{}

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
  JsonObject token,
  std::string method,
  JsonObject params,
  JsonObject& response)
{
  bool ret = true;

  // Manager.createApplication
  if (method == MANAGER_CREATE_APPLICATION) {
    std::string url = params["url"].String();
    int appId = GetAppIdFromToken(token);
    CheckInternetConnectivity();
    bool canCreate = ORB::instance(nullptr)->GetApplicationManager()->CreateApplication(appId, url);
    response["canCreate"] = canCreate;
  }

  // Manager.destroyApplication
  else if (method == MANAGER_DESTROY_APPLICATION) {
    int callingAppId = GetAppIdFromToken(token);
    ORB::instance(nullptr)->GetApplicationManager()->DestroyApplication(callingAppId);
    response.FromString("{}");
  }

  // Manager.showApplication
  else if (method == MANAGER_SHOW_APPLICATION) {
    int callingAppId = GetAppIdFromToken(token);
    ORB::instance(nullptr)->GetApplicationManager()->ShowApplication(callingAppId);
    response.FromString("{}");
  }

  // Manager.hideApplication
  else if (method == MANAGER_HIDE_APPLICATION) {
    int callingAppId = GetAppIdFromToken(token);
    ORB::instance(nullptr)->GetApplicationManager()->HideApplication(callingAppId);
    response.FromString("{}");
  }

  // Manager.searchOwner
  else if (method == MANAGER_SEARCH_OWNER) {
    std::string owner = params[0].String();
    response["uri"] = owner;
  }

  // Manager.getFreeMem
  else if (method == MANAGER_GET_FREE_MEM) {
    response["freeMem"] = -1;
  }

  // Manager.getKeyIcon
  else if (method == MANAGER_GET_KEY_ICON) {
    int code = params["code"].Number();
    std::string keyUri = ORB::instance(nullptr)->GetORBPlatform()->Manager_GetKeyIcon(code);
    response["keyUri"] = keyUri;
  }

  // Manager.setKeyValue
  else if (method == MANAGER_SET_KEY_VALUE) {
    int value = params["value"].Number();
    int callingAppId = GetAppIdFromToken(token);
    ORB::instance(nullptr)->GetApplicationManager()->SetKeySetMask(callingAppId, value);
    response["keyMask"] = value;
  }

  // Manager.getKeyMaximumValue
  else if (method == MANAGER_GET_KEY_MAXIMUM_VALUE) {
    int maximumValue = 
      KEY_SET_RED |
      KEY_SET_GREEN |
      KEY_SET_YELLOW |
      KET_SET_BLUE |
      KEY_SET_NAVIGATION |
      KEY_SET_VCR |
      KEY_SET_NUMERIC;
    response["maximumValue"] = maximumValue;
  }

  // Manager.getKeyValues
  else if (method == MANAGER_GET_KEY_VALUES) {
    int callingAppId = GetAppIdFromToken(token);
    int value = ORB::instance(nullptr)->GetApplicationManager()->GetKeySetMask(callingAppId); 
    response["value"] = value;
  }

  // UnknownMethod
  else {
    response = RequestHandler::MakeErrorResponse("UnknownMethod");
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
  fprintf(stderr, "[ManagerRequestHandler::CheckInternetConnectivity]\n");
  bool isConnectedToInternet = ORB::instance(nullptr)->GetORBPlatform()->Network_IsConnectedToInternet();
  ORB::instance(nullptr)->GetApplicationManager()->OnNetworkAvailabilityChanged(isConnectedToInternet);
}

/**
 * Get the application id from the specified JSON token.
 * 
 * @param token The token
 * 
 * @return The application id or INVALID_APP_ID
 */
int ManagerRequestHandler::GetAppIdFromToken(JsonObject token)
{
  JsonObject payload = token["payload"].Object();
  int appId = INVALID_APP_ID;
  if (payload.HasLabel("appId")) {
    appId = payload["appId"].Number();
  }
  return appId;
}

} // namespace orb
