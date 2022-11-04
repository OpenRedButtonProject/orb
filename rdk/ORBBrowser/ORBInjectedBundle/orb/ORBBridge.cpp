/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */
#include "ORBBridge.h"
#include "Tags.h"

#define TIMEOUT_FOR_TWOWAY_METHODS 2000 // milliseconds
#define TIMEOUT_FOR_ONEWAY_METHODS 500  // milliseconds

extern WKBundleRef g_Bundle;

using namespace WPEFramework;

namespace orb {
void JavaScriptEventDispatchRequested(std::string name, std::string properties);
void DvbUrlLoaded(int requestId, unsigned char *content, unsigned int contentLength);
void InputKeyGenerated(int keyCode);

/**
 * Private constructor.
 */
ORBBridge::ORBBridge()
{
   fprintf(stderr, "[ORBBridge::ORBBridge]\n");
   m_orbClient = CreateORBClient(
      JavaScriptEventDispatchRequested,
      DvbUrlLoaded,
      InputKeyGenerated
      );
}

/**
 * Destructor.
 */
ORBBridge::~ORBBridge()
{
   m_dsmccCallers.clear();
   m_dsmccCallbacks.clear();
}

/**
 * Dispatch the specified event to the current JavaScript context.
 *
 * @param type             The event type
 * @param properties       The event properties
 * @param broadcastRelated Indicates whether the event is broadcast-related or not
 * @param targetOrigin     The target origin
 */
void ORBBridge::DispatchEvent(std::string type, JsonObject properties, bool broadcastRelated, std::string targetOrigin)
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
void ORBBridge::SetJavaScriptContext(JSContextRef jsContextRef)
{
   m_javaScriptContext = jsContextRef;
}

/**
 * Generate the specified key event.
 *
 * @param keyCode The input key code
 */
void ORBBridge::GenerateKey(int keyCode)
{
   std::string kScript = "document.dispatchEvent(new KeyboardEvent('keydown',{keyCode:" + std::to_string(keyCode) + "}));";
   JSStringRef scriptStr = JSStringCreateWithUTF8CString(kScript.c_str());
   JSValueRef exception = nullptr;
   JSEvaluateScript(m_javaScriptContext, scriptStr, nullptr, nullptr, 0, &exception);
   JSStringRelease(scriptStr);
}

/**
 * Adds the given DSM-CC caller for the given request id.
 *
 * @param requestId The request identifier
 * @param caller    Pointer to the caller object
 */
void ORBBridge::AddDsmccCaller(int requestId, void *caller)
{
   m_dsmccCallers[requestId] = caller;
}

/**
 * Add the given DSM-CC callback for the given request id.
 *
 * @param requestId The request identifier
 * @param callback  The callback
 */
void ORBBridge::AddDsmccCallback(int requestId, OnDvbUrlLoaded callback)
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
void * ORBBridge::GetDsmccCaller(int requestId)
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
OnDvbUrlLoaded ORBBridge::GetDsmccCallback(int requestId)
{
   return m_dsmccCallbacks[requestId];
}

/**
 * Remove the DSM-CC caller that corresponds to the given request id.
 *
 * @param requestId The request identifier
 */
void ORBBridge::RemoveDsmccCaller(int requestId)
{
   m_dsmccCallers.erase(requestId);
}

/**
 * Remove the DSM-CC callback that corresponds to the given request id.
 *
 * @param requestId The request id
 */
void ORBBridge::RemoveDsmccCallback(int requestId)
{
   m_dsmccCallbacks.erase(requestId);
}

/**
 * Callback responding to the 'javascripteventdispatchrequested' event of the ORB service.
 *
 * @param name
 * @param properties
 */
void JavaScriptEventDispatchRequested(std::string name, std::string properties)
{
   fprintf(stderr, "[ORBBridge::OnJavaScriptEventDispatchRequested] type=%s properties=%s\n",
      name.c_str(),
      properties.c_str()
      );

   // Prepare input
   JsonObject input;
   input["type"] = name;
   input["properties"] = properties;

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
 * Callback responding to the 'dvburlloaded' event of the ORB service.
 *
 * @param requestId
 * @param content
 * @param contentLength
 */
void DvbUrlLoaded(int requestId, unsigned char *content, unsigned int contentLength)
{
   fprintf(stderr, "[ORBBridge::DvbUrlLoaded] requestId=%d\n", requestId);

   OnDvbUrlLoaded callback = ORBBridge::GetSharedInstance().GetDsmccCallback(requestId);
   void *caller = ORBBridge::GetSharedInstance().GetDsmccCaller(requestId);

   callback(requestId, content, contentLength, caller);

   ORBBridge::GetSharedInstance().RemoveDsmccCallback(requestId);
   ORBBridge::GetSharedInstance().RemoveDsmccCaller(requestId);
}

/**
 * Callback responding to the 'inputkeygenerated' event of the ORB service.
 *
 * @parm keyCode
 */
void InputKeyGenerated(int keyCode)
{
   fprintf(stderr, "[ORBBridge::InputKeyGenerated] keyCode=%d\n", keyCode);

   // Prepare input
   JsonObject input;
   input["actionName"] = "GenerateKey";
   input["keyCode"] = keyCode;
   std::string inputAsString;
   input.ToString(inputAsString);

   // Send synchronous message to the injected bundle's main thread
   WKStringRef messageName = WKStringCreateWithUTF8CString(Tags::Action);
   WKStringRef messageBody = WKStringCreateWithUTF8CString(inputAsString.c_str());
   WKBundlePostSynchronousMessage(g_Bundle, messageName, messageBody, nullptr);

   // Release resources
   WKRelease(messageBody);
   WKRelease(messageName);
}
} // namespace orb
