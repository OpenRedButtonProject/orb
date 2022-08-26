/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

#include "Module.h"
#include "ORB.h"
#include "ORBEvents.h"
#include "ORBPlatform.h"
#include "Keys.h"
#include "Base64.h"
#include <vector>

namespace WPEFramework {
namespace Plugin {
/**
 * @brief ResolveObjectAndMethod
 *
 * Resolves the object and method from the specified input, which has the following form:
 *
 * <object>.<method>
 *
 * @param input The input string
 * @param object (out) Holds the resolved object in success
 * @param method (out) Holds the resolved method in success
 *
 * @return true in success, otherwise false
 */
static
bool ResolveObjectAndMethod(std::string input, std::string& object, std::string& method)
{
   std::vector<std::string> tokens;
   for (auto i = strtok(&input[0], "."); i != NULL; i = strtok(NULL, "."))
   {
      tokens.push_back(i);
   }
   if (tokens.size() != 2)
   {
      return false;
   }

   object = tokens[0];
   method = tokens[1];

   return true;
}

/**
 * @brief ORB::RegisterAll
 *
 * Register all JSON-RPC methods exposed by the ORB plugin.
 */
void ORB::RegisterAll()
{
   JSONRPC::Register<JsonObject, JsonObject>(_T("ExecuteWpeBridgeRequest"), &ORB::ExecuteWpeBridgeRequest, this);
   JSONRPC::Register<Core::JSON::String, JsonObject>(_T("CreateToken"), &ORB::CreateToken, this);
   JSONRPC::Register<ApplicationLoadFailedParamsData, void>(_T("ApplicationLoadFailed"), &ORB::ApplicationLoadFailed, this);
   JSONRPC::Register<Core::JSON::String, void>(_T("ApplicationPageChanged"), &ORB::ApplicationPageChanged, this);
   JSONRPC::Register<LoadDvbUrlParamsData, void>(_T("LoadDvbUrl"), &ORB::LoadDvbUrl, this);
   JSONRPC::Register<Core::JSON::DecUInt16, Core::JSON::Boolean>(_T("SendKeyEvent"), &ORB::SendKeyEvent, this);
}

/**
 * @brief ORB::UnregisterAll
 *
 * Unregister all JSON-RPC methods exposed by the ORB plugin.
 */
void ORB::UnregisterAll()
{
   JSONRPC::Unregister(_T("ExecuteWpeBridgeRequest"));
   JSONRPC::Unregister(_T("CreateToken"));
   JSONRPC::Unregister(_T("ApplicationLoadFailed"));
   JSONRPC::Unregister(_T("ApplicationPageChanged"));
   JSONRPC::Unregister(_T("LoadDvbUrl"));
   JSONRPC::Unregister(_T("SendKeyEvent"));
}

/**
 * @brief ORB::ExecuteWpeBridgeRequest
 *
 * Execute the given WPE bridge request.
 * The request is a JSON object of the following form:
 *
 * {
 *    "token": <token>
 *    "method": <method>
 *    "params": <params>
 * }
 *
 * The response is also a JSON object containing the results, if any.
 *
 * @param request  A JSON object representing the WPE bridge request
 * @param response A JSON object representing the response
 *
 * @return Core::ERROR_NONE
 */
uint32_t ORB::ExecuteWpeBridgeRequest(JsonObject request, JsonObject& response)
{
   std::string requestAsString;
   request.ToString(requestAsString);

   SYSLOG(Logging::Notification, (_T("[ORB::ExecuteWpeBridgeRequest] request=%s"), requestAsString.c_str()));

   // Extract token payload and perform security check
   JsonObject jsonToken = request["token"].Object();
   JsonObject jsonTokenPayload = GetTokenManager()->GetTokenPayload(jsonToken);
   if (jsonTokenPayload.IsNull() || !jsonTokenPayload.IsSet())
   {
      response = RequestHandler::MakeErrorResponse("Forbidden");
      return(Core::ERROR_NONE);
   }

   // Save token payload to string for logging purposes
   std::string token;
   jsonTokenPayload.ToString(token);
   SYSLOG(Logging::Notification, (_T("[ORB::ExecuteWpeBridgeRequest] token=%s"), token.c_str()));

   // Resolve object and method
   std::string object;
   std::string method;
   if (!ResolveObjectAndMethod(request["method"].String(), object, method))
   {
      response = RequestHandler::MakeErrorResponse("UnknownMethod");
      return(Core::ERROR_NONE);
   }

   SYSLOG(Logging::Notification, (_T("[ORB::ExecuteWpeBridgeRequest] object=%s method=%s"), object.c_str(), method.c_str()));

   // Execute requested method
   JsonObject params = request["params"].Object();
   if (object == "Broadcast")
   {
      GetBroadcastRequestHandler()->Handle(jsonToken, method, params, response);
   }
   else if (object == "Configuration")
   {
      GetConfigurationRequestHandler()->Handle(jsonToken, method, params, response);
   }
   else if (object == "Manager")
   {
      GetManagerRequestHandler()->Handle(jsonToken, method, params, response);
   }
   else if (object == "Programme")
   {
      GetProgrammeRequestHandler()->Handle(jsonToken, method, params, response);
   }
   else if (object == "ParentalControl")
   {
      GetParentalControlRequestHandler()->Handle(jsonToken, method, params, response);
   }
   else
   {
      response = RequestHandler::MakeErrorResponse("UnknownMethod");
   }

   std::string responseAsString;
   response.ToString(responseAsString);
   SYSLOG(Logging::Notification, (_T("[ORB::ExecuteWpeBridgeRequest] response=%s"), responseAsString.c_str()));

   return(Core::ERROR_NONE);
}

/**
 * @brief ORB::CreateToken
 *
 * Create a new JSON token for the current application and the given uri.
 *
 * @param uri   The given URI
 * @param token The resulting JSON token
 *
 * @return Core::ERROR_NONE
 */
uint32_t ORB::CreateToken(Core::JSON::String uri, JsonObject& token)
{
   SYSLOG(Logging::Notification, (_T("[ORB::CreateToken] uri=%s appId=%hu"), uri.Value().c_str(), _currentAppId));

   if (_currentAppId == UINT16_MAX)
   {
      SYSLOG(Logging::Notification, (_T("[ORB::CreateToken] No app is currently running")));
      token.FromString("{}");
      return(Core::ERROR_NONE);
   }

   token = _tokenManager->CreateToken(_currentAppId, uri.Value());

   return(Core::ERROR_NONE);
}

/**
 * @brief ORB::ApplicationLoadFailed
 *
 * Notify the application manager and the current JavaScript context that the specified HbbTV application
 * has failed to load.
 *
 * @param params Container with the appId, url, and error description
 *
 * @return Core::ERROR_NONE
 */
uint32_t ORB::ApplicationLoadFailed(const ApplicationLoadFailedParamsData& params)
{
   SYSLOG(Logging::Notification, (_T("[ORB::ApplicationLoadFailed] appId=%d url=%s error=%s"),
                                  _currentAppId,
                                  params.Url.Value().c_str(),
                                  params.ErrorDescription.Value().c_str()
                                  ));

   // notify the application manager that the loading of the given application has failed
   bool isConnectedToInternet = GetORBPlatform()->Network_IsConnectedToInternet();
   GetApplicationManager()->OnNetworkAvailabilityChanged(isConnectedToInternet);
   GetApplicationManager()->OnLoadApplicationFailed(_currentAppId);

   // notify the current JavaScript context that the given application has failed to load
   JsonObject properties;
   properties["url"] = params.Url.Value();
   NotifyJavaScriptEventDispatchRequested("ApplicationLoadError", properties, false, "");

   return(Core::ERROR_NONE);
}

/**
 * @brief ORB::ApplicationPageChanged
 *
 * Notify the application manager that the page of the current HbbTV application has changed
 * and is about to load.
 *
 * @param url The application page URL
 *
 * @return Core::ERROR_NONE
 */
uint32_t ORB::ApplicationPageChanged(Core::JSON::String url)
{
   SYSLOG(Logging::Notification, (_T("[ORB::ApplicationPageChanged] appId=%d url=%s"), _currentAppId, url.Value().c_str()));

   GetApplicationManager()->OnApplicationPageChanged(_currentAppId, url.Value());

   return(Core::ERROR_NONE);
}

/**
 * @brief ORB::LoadDvbUrl
 *
 * Load the specified DVB URL through the DSM-CC implementation.
 *
 * @param params Container with the DVB URL and the request identifier
 *
 * @return Core::ERROR_NONE
 */
uint32_t ORB::LoadDvbUrl(const LoadDvbUrlParamsData& params)
{
   SYSLOG(Logging::Notification, (_T("[ORB::LoadDvbUrl] url=%s requestId=%d"),
                                  params.Url.Value().c_str(), params.RequestId.Value()));

   GetORBPlatform()->Dsmcc_RequestFile(params.Url.Value(), params.RequestId.Value());

   return(Core::ERROR_NONE);
}

/**
 * @brief ORB::SendKeyToCurrentApplication
 *
 * Send the specified key event to the current HbbTV application (if any).
 *
 * @param keyCode  The event's JavaScript key code
 * @param response True if the key event was consumed by the current HbbTV application, otherwise false
 *
 * @return Core::ERROR_NONE
 */
uint32_t ORB::SendKeyEvent(Core::JSON::DecUInt16 keyCode, Core::JSON::Boolean& response)
{
   SYSLOG(Logging::Notification, (_T("[ORB::SendKeyEvent] keyCode=%hu"), keyCode.Value()));

   // check if there is any application currently running
   if (_currentAppId == UINT16_MAX)
   {
      SYSLOG(Logging::Notification, (_T("[ORB::SendKeyEvent] No app is currently running")));
      response = false;
      return(Core::ERROR_NONE);
   }

   uint16_t mask = _applicationManager->GetKeySetMask(_currentAppId);
   uint16_t keyEventCode = 0;

   keyEventCode = Keys::ResolveKeyEvent(keyCode.Value());

   if (mask & keyEventCode)
   {
      response = true;
      _orbPlatform->Application_SendKeyEvent(static_cast<int>(keyCode.Value()));
   }

   return(Core::ERROR_NONE);
}

/**
 * @brief ORB::EventJavaScriptEventDispatchRequested
 *
 * Trigger the JavaScriptEventDispatchRequested event.
 *
 * @param params Parameters container
 */
void ORB::EventJavaScriptEventDispatchRequested(JavaScriptEventDispatchRequestedParamsData& params)
{
   fprintf(stderr, "[ORB::EventJavaScriptEventDispatchRequested]\n");
   Notify(_T("javaScriptEventDispatchRequested"), params);
}

/**
 * @brief ORB::EventDvbUrlLoaded
 *
 * Trigger the DvbUrlLoaded event.
 *
 * @param params Parameters container (requestId, fileContent, fileContentLength)
 */
void ORB::EventDvbUrlLoaded(DvbUrlLoadedParamsData& params)
{
   fprintf(stderr, "[ORB::EventDvbUrlLoaded] requestId=%d fileContentLength=%u\n",
      params.RequestId.Value(),
      params.FileContentLength.Value()
      );
   Notify(_T("dvbUrlLoaded"), params);
}
} // namespace Plugin
} // namespace WPEFramework
