/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

#include "Module.h"
#include "ORB.h"
#include "ORBEngine.h"
#include <vector>

using namespace orb;

namespace WPEFramework {
namespace Plugin {
/**
 * @brief ORB::RegisterAll
 *
 * Register all JSON-RPC methods exposed by the ORB plugin.
 */
void ORB::RegisterAll()
{
   fprintf(stderr, "RegisterAll call %d\n", getpid());
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

   std::string responseAsString = ORBEngine::GetSharedInstance().ExecuteBridgeRequest(requestAsString);
   response.FromString(responseAsString);

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
   SYSLOG(Logging::Notification, (_T("[ORB::CreateToken] uri=%s"), uri.Value().c_str()));

   std::string tokenAsString = ORBEngine::GetSharedInstance().CreateToken(uri.Value());
   token.FromString(tokenAsString);

   SYSLOG(Logging::Notification, (_T("[ORB::CreateToken] token=%s"), tokenAsString.c_str()));

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
   SYSLOG(Logging::Notification, (_T("[ORB::ApplicationLoadFailed] url=%s error=%s"),
                                  params.Url.Value().c_str(),
                                  params.ErrorDescription.Value().c_str()
                                  ));

   ORBEngine::GetSharedInstance().NotifyApplicationLoadFailed(params.Url.Value(), params.ErrorDescription.Value());

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
   SYSLOG(Logging::Notification, (_T("[ORB::ApplicationPageChanged] url=%s"), url.Value().c_str()));

   ORBEngine::GetSharedInstance().NotifyApplicationPageChanged(url.Value());

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

   ORBEngine::GetSharedInstance().LoadDvbUrl(params.Url.Value(), params.RequestId.Value());

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

   response = ORBEngine::GetSharedInstance().SendKeyEvent(keyCode.Value());

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

void ORB::EventInputKeyGenerated(Core::JSON::DecSInt32 keyCode)
{
   fprintf(stderr, "[ORB::EventInputKeyGenerated] keyCode=%d\n", keyCode.Value());
   Notify(_T("inputKeyGenerated"), keyCode);
}
} // namespace Plugin
} // namespace WPEFramework
