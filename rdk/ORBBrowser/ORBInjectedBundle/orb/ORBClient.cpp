/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */
#include "ORBClient.h"
#include "Tags.h"
#include <interfaces/json/JsonData_ORB.h>

using namespace WPEFramework::JsonData::ORB;
using namespace WPEFramework;

#define TIMEOUT_FOR_TWOWAY_METHODS 2000 // milliseconds
#define TIMEOUT_FOR_ONEWAY_METHODS 500  // milliseconds

extern WKBundleRef g_Bundle;

namespace orb {
// event handlers
void JavaScriptEventDispatchRequested(const JavaScriptEventDispatchRequestedParamsData& params);
void DvbUrlLoaded(const DvbUrlLoadedParamsData& params);

/**
 * Private constructor.
 */
ORBClient::ORBClient()
   : m_remoteObject("ORB.1", "client.events.88")
{
   fprintf(stderr, "[ORBClient::ORBClient]\n");
   WPEFramework::Core::SystemInfo::SetEnvironment(_T("THUNDER_ACCESS"), (_T("127.0.0.1:9998")));

   m_subscribedEvents["JavaScriptEventDispatchRequestedEvent"] = false;
   m_subscribedEvents["DvbUrlLoaded"] = false;
}

/**
 * Destructor.
 */
ORBClient::~ORBClient()
{
   m_dsmccCallers.clear();
   m_dsmccCallbacks.clear();
   m_subscribedEvents.clear();

   // Unsubscribe from events
   m_remoteObject.Unsubscribe(TIMEOUT_FOR_ONEWAY_METHODS, _T("javaScriptEventDispatchRequested"));
   m_remoteObject.Unsubscribe(TIMEOUT_FOR_ONEWAY_METHODS, _T("dvbUrlLoaded"));
}

/**
 * Subscribe with the 'javaScriptEventDispatchRequested' event of the ORB Thunder plugin.
 */
void ORBClient::SubscribeWithJavaScriptEventDispatchRequestedEvent()
{
   uint32_t error_code = WPEFramework::Core::ERROR_NONE;

   // subscribe to javaScriptEventDispatchRequested event
   if (m_subscribedEvents["JavaScriptEventDispatchRequestedEvent"] == false)
   {
      error_code = m_remoteObject.Subscribe<JavaScriptEventDispatchRequestedParamsData>(
         TIMEOUT_FOR_ONEWAY_METHODS, _T("javaScriptEventDispatchRequested"), &JavaScriptEventDispatchRequested);

      fprintf(stderr, "[ORBClient::ORBClient] Subscribe to event 'javaScriptEventDispatchRequested': %s\n",
         error_code == WPEFramework::Core::ERROR_NONE ? "success" : "failure");
   }

   if (error_code == WPEFramework::Core::ERROR_NONE)
   {
      m_subscribedEvents["JavaScriptEventDispatchRequestedEvent"] = true;
   }
}

/**
 * Subscribe with the 'dvbUrlLoaded' event of the ORB Thunder plugin.
 */
void ORBClient::SubscribeWithDvbUrlLoadedEvent()
{
   uint32_t error_code = WPEFramework::Core::ERROR_NONE;

   // subscribe to dvbUrlLoaded event
   if (m_subscribedEvents["DvbUrlLoaded"] == false)
   {
      error_code = m_remoteObject.Subscribe<DvbUrlLoadedParamsData>(
         TIMEOUT_FOR_ONEWAY_METHODS, _T("dvbUrlLoaded"), &DvbUrlLoaded);

      fprintf(stderr, "[ORBClient::ORBClient] Subscribe to event 'dvbUrlLoaded': %s\n",
         error_code == WPEFramework::Core::ERROR_NONE ? "success" : "failure");
   }

   if (error_code == WPEFramework::Core::ERROR_NONE)
   {
      m_subscribedEvents["DvbUrlLoaded"] = true;
   }
}

/**
 * @brief ORBClient::CreateToken
 *
 * Create a new JSON token for the current application and the given uri.
 *
 * @param uri The given URI
 *
 * @return The resulting JSON token
 */
JsonObject ORBClient::CreateToken(std::string uri)
{
   fprintf(stderr, "[ORBClient::CreateToken] uri=%s\n", uri.c_str());

   Core::JSON::String params;
   JsonObject result;

   params.FromString(uri);

   uint32_t error_code = m_remoteObject.Invoke<Core::JSON::String, JsonObject>(
      TIMEOUT_FOR_TWOWAY_METHODS, _T("CreateToken"), params, result);

   std::string resultAsString;
   result.ToString(resultAsString);
   fprintf(stderr, "[ORBClient::CreateToken] error_code=%u result=%s\n", error_code, resultAsString.c_str());

   if (error_code == 0)
   {
      return result;
   }

   result.FromString("{}");
   return result;
}

/**
 * @brief ORBClient::ExecuteWpeBridgeRequest
 *
 * Execute the given WPE bridge request.
 * The reuqest is a stringified JSON object of the following form:
 *
 * {
 *    "token": <token>
 *    "method": <method>
 *    "params": <params>
 * }
 *
 * The response is also a stringified JSON object containing the results, if any.
 *
 * @param request A stringified JSON object representing the WPE bridge request
 *
 * @return A stringified JSON object representing the response
 */
std::string ORBClient::ExecuteWpeBridgeRequest(std::string request)
{
   fprintf(stderr, "[ORBClient::ExecuteWpeBridgeRequest] request=%s\n", request.c_str());

   JsonObject params;
   JsonObject result;

   params.FromString(request);

   uint32_t error_code = m_remoteObject.Invoke<JsonObject, JsonObject>(
      TIMEOUT_FOR_TWOWAY_METHODS, _T("ExecuteWpeBridgeRequest"), params, result);

   std::string resultAsString;
   result.ToString(resultAsString);
   fprintf(stderr, "[ORBClient::ExecuteWpeBridgeRequest] error_code=%u result=%s\n", error_code, resultAsString.c_str());

   if (error_code == 0)
   {
      return resultAsString;
   }

   return _T("{}");
}

/**
 * @brief ORBClient::LoadDvbUrl
 *
 * Load the specified DVB URL through the DSM-CC implementation.
 *
 * @param url       The DVB URL
 * @param requestId The request identifier
 */
void ORBClient::LoadDvbUrl(std::string url, int requestId)
{
   fprintf(stderr, "[ORBClient::LoadDvbUrl] url=%s requestId=%d\n", url.c_str(), requestId);

   LoadDvbUrlParamsData params;
   params.Url = url;
   params.RequestId = requestId;

   uint32_t error_code = m_remoteObject.Invoke<LoadDvbUrlParamsData, void>(
      TIMEOUT_FOR_ONEWAY_METHODS, _T("LoadDvbUrl"), params);

   fprintf(stderr, "[ORBClient::LoadDvbUrl] error_code=%u\n", error_code);

   return;
}

/**
 * @brief ORB::ApplicationLoadFailed
 *
 * Notify the application manager and the current JavaScript context that the specified HbbTV application
 * has failed to load.
 *
 * @param url              The application url
 * @param errorDescription The error description
 */
void ORBClient::ApplicationLoadFailed(std::string url, std::string errorDescription)
{
   fprintf(stderr, "[ORBClient::ApplicationLoadFailed] url=%s errorDescription=%s\n", url.c_str(), errorDescription.c_str());

   JsonObject params;
   params["url"] = url;
   params["errorDescription"] = errorDescription;

   uint32_t error_code = m_remoteObject.Invoke<JsonObject, void>(
      TIMEOUT_FOR_ONEWAY_METHODS, _T("ApplicationLoadFailed"), params);

   fprintf(stderr, "[ORBClient::ApplicationLoadFailed] error_code=%u\n", error_code);

   return;
}

/**
 * @brief ORBClient::ApplicationPageChanged
 *
 * Notify the application manager that the page of the current HbbTV application has changed
 * and is about to load.
 *
 * @param url The application page URL
 */
void ORBClient::ApplicationPageChanged(std::string url)
{
   fprintf(stderr, "[ORBClient::ApplicationPageChanged] url=%s \n", url.c_str());

   Core::JSON::String params;
   params.FromString(url);

   uint32_t error_code = m_remoteObject.Invoke<Core::JSON::String, void>(
      TIMEOUT_FOR_ONEWAY_METHODS, _T("ApplicationPageChanged"), params);

   fprintf(stderr, "[ORBClient::ApplicationPageChanged] error_code=%u\n", error_code);

   return;
}

/**
 * Dispatch the specified event to the current JavaScript context.
 *
 * @param type             The event type
 * @param properties       The event properties
 * @param broadcastRelated Indicates whether the event is broadcast-related or not
 * @param targetOrigin     The target origin
 */
void ORBClient::DispatchEvent(std::string type, JsonObject properties, bool broadcastRelated, std::string targetOrigin)
{
   std::string propertiesAsString;
   properties.ToString(propertiesAsString);
   std::string kScript = "document.dispatchBridgeEvent('" + type + "', " + propertiesAsString + ")";
   JSStringRef scriptStr = JSStringCreateWithUTF8CString(kScript.c_str());
   JSValueRef exception = nullptr;
   JSEvaluateScript(m_javaScriptContext, scriptStr, nullptr, nullptr, 0, &exception);
   JSStringRelease(scriptStr);
}

/**
 * Set the current JavaScript context.
 *
 * @param jsContextRef The current JavaScript context
 */
void ORBClient::SetJavaScriptContext(JSContextRef jsContextRef)
{
   m_javaScriptContext = jsContextRef;
}

/**
 * Adds the given DSM-CC caller for the given request id.
 *
 * @param requestId The request identifier
 * @param caller    Pointer to the caller object
 */
void ORBClient::AddDsmccCaller(int requestId, void *caller)
{
   m_dsmccCallers[requestId] = caller;
}

/**
 * Add the given DSM-CC callback for the given request id.
 *
 * @param requestId The request identifier
 * @param callback  The callback
 */
void ORBClient::AddDsmccCallback(int requestId, OnDvbUrlLoaded callback)
{
   m_dsmccCallbacks[requestId] = callback;
}

/**
 * Get the DSM-CC caller that corresponds to the given request id.
 *
 * @param requestId The request identifier
 *
 * @return Pointer to the DSM-CC caller object
 */
void * ORBClient::GetDsmccCaller(int requestId)
{
   return m_dsmccCallers[requestId];
}

/**
 * Get the DSM-CC callback that corresponds to the given request id.
 *
 * @param requestId The request identifier
 *
 * @return The DSM-CC callback
 */
OnDvbUrlLoaded ORBClient::GetDsmccCallback(int requestId)
{
   return m_dsmccCallbacks[requestId];
}

/**
 * Remove the DSM-CC caller that corresponds to the given request id.
 *
 * @param requestId The request identifier
 */
void ORBClient::RemoveDsmccCaller(int requestId)
{
   m_dsmccCallers.erase(requestId);
}

/**
 * Remove the DSM-CC callback that corresponds to the given request id.
 *
 * @param requestId The request id
 */
void ORBClient::RemoveDsmccCallback(int requestId)
{
   m_dsmccCallbacks.erase(requestId);
}

/*****************************************************************************
 * Event handlers
 *****************************************************************************/


void JavaScriptEventDispatchRequested(const JavaScriptEventDispatchRequestedParamsData& params)
{
   fprintf(stderr, "[ORBClient::JavaScriptEventDispatchRequested] type=%s properties=%s\n",
      params.EventName.Value().c_str(),
      params.EventProperties.Value().c_str()
      );

   // Prepare input
   JsonObject input;
   input["type"] = params.EventName.Value();
   input["properties"] = params.EventProperties.Value();

   std::string inputAsString;
   input.ToString(inputAsString);

   // Send synchronous message to the injected bundle's main thread
   WKStringRef messageName = WKStringCreateWithUTF8CString(Tags::DispatchEvent);
   WKStringRef messageBody = WKStringCreateWithUTF8CString(inputAsString.c_str());
   WKBundlePostSynchronousMessage(g_Bundle, messageName, messageBody, nullptr);

   // Release resources
   WKRelease(messageBody);
   WKRelease(messageName);
}

/**
 * Callback responding to the 'dvburlloaded' event of the ORB Thunder plugin.
 *
 * @param params The event data
 */
void DvbUrlLoaded(const DvbUrlLoadedParamsData& params)
{
   int requestId = params.RequestId.Value();
   unsigned int fileContentLength = params.FileContentLength;

   fprintf(stderr, "[ORBBClient::DvbUrlLoaded] requestId=%d\n", requestId);

   OnDvbUrlLoaded callback = ORBClient::GetSharedInstance().GetDsmccCallback(requestId);
   void *caller = ORBClient::GetSharedInstance().GetDsmccCaller(requestId);

   callback(requestId, fileContentLength, caller);

   ORBClient::GetSharedInstance().RemoveDsmccCallback(requestId);
   ORBClient::GetSharedInstance().RemoveDsmccCaller(requestId);
}
} // namespace orb
