/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */
#include "WpeBridge.h"
#include "ORBClient.h"
#include <vector>
#include "Tags.h"

using namespace orb;

namespace WPEFramework {
namespace JavaScript {
namespace WPEBridge {
/**
 * Converts the given JavaScript value to an std::string.
 *
 * @param context The JavaScript context
 * @param value The JavaScript value
 *
 * @return The std::string
 */
static std::string JSValueRefToStdString(JSContextRef context, JSValueRef value)
{
    JSStringRef jsStringRef = JSValueToStringCopy(context, value, nullptr);
    size_t bufferSize = JSStringGetLength(jsStringRef);
    char *buffer = (char *) malloc(bufferSize + 1);
    JSStringGetUTF8CString(jsStringRef, buffer, bufferSize + 1);
    std::string s(buffer);
    JSStringRelease(jsStringRef);
    free(buffer);
    return s;
}

/**
 * Converts the specified WKStringRef to an STL string.
 *
 * @param wkStringRef The WKStringRef
 *
 * @return The STL string
 */
static std::string ToStdString(WKStringRef wkStringRef)
{
    std::string s;
    char *buffer;
    int bufferLength;
    if (WKStringIsEmpty(wkStringRef))
    {
        return s;
    }
    bufferLength = WKStringGetLength(wkStringRef);
    buffer = (char *) malloc(bufferLength + 1);
    WKStringGetUTF8CString(wkStringRef, buffer, bufferLength + 1);
    std::string stringBuffer(buffer);
    s = stringBuffer;
    free(buffer);
    return s;
}

// JavaScript code to create the wpeBridge object
const char kWPEBridgeSrc[] = R"jssrc(
window.wpeBridge = {};
)jssrc";


/**
 * Implements the window.wpeBridge.request() function.
 *
 * @param context
 * @param
 * @param
 * @param argumentCount
 * @param arguments
 * @param
 *
 * @return
 */
static JSValueRef request(
    JSContextRef context,
    JSObjectRef,
    JSObjectRef,
    size_t argumentCount,
    const JSValueRef arguments[],
    JSValueRef *)
{
    std::string request = JSValueRefToStdString(context, arguments[0]);
    std::string response = ORBClient::GetSharedInstance().ExecuteWpeBridgeRequest(request);
    return JSValueMakeString(context, JSStringCreateWithUTF8CString(response.c_str()));
}

void Initialise()
{
    fprintf(stderr, "[WpeBridge::Initialise]\n");
}

/**
 * Injects the following objects to the main frame's JavaScript context:
 *
 * window.wpeBridge
 * document.token
 *
 * @param frame
 */
void InjectJS(WKBundleFrameRef frame)
{
    if (!WKBundleFrameIsMainFrame(frame))
    {
        return;
    }

    fprintf(stderr, "[WpeBridge::InjectJS] In main frame\n");

    WKURLRef frameUrl = WKBundleFrameCopyURL(frame);
    WKStringRef frameUrlAsString = WKURLCopyString(frameUrl);
    std::string uri = ToStdString(frameUrlAsString);
    fprintf(stderr, "[WpeBridge::InjectJS] uri=%s\n", uri.c_str());
    WKRelease(frameUrl);

    JsonObject jsonToken = ORBClient::GetSharedInstance().CreateToken(uri);
    std::string token;
    jsonToken.ToString(token);
    fprintf(stderr, "[WpeBridge::InjectJS] token = %s\n", token.c_str());

    JSValueRef exception = nullptr;

    // Run JavaScript code to create the document.token
    std::string tokenScript = "\ndocument.token=" + token + ";\n";
    JSGlobalContextRef context = WKBundleFrameGetJavaScriptContext(frame);
    JSStringRef tokenScriptStr = JSStringCreateWithUTF8CString(tokenScript.c_str());
    JSEvaluateScript(context, tokenScriptStr, nullptr, nullptr, 0, &exception);
    JSStringRelease(tokenScriptStr);

    // Run JavaScript code to create the wpeBridge object
    JSStringRef wpeBridgeScriptStr = JSStringCreateWithUTF8CString(kWPEBridgeSrc);
    JSEvaluateScript(context, wpeBridgeScriptStr, nullptr, nullptr, 0, &exception);
    JSStringRelease(wpeBridgeScriptStr);

    // Retrieve ref to the wpeBridge object
    JSStringRef wpeBridgeStr = JSStringCreateWithUTF8CString("wpeBridge");
    JSObjectRef windowObject = JSContextGetGlobalObject(context);
    JSObjectRef wpeBridgeObject = const_cast<JSObjectRef>(JSObjectGetProperty(context, windowObject,
        wpeBridgeStr, &exception));
    JSStringRelease(wpeBridgeStr);

    // Add 'request' function to wpeBridge object
    JSStringRef requestStr = JSStringCreateWithUTF8CString("request");
    JSValueRef requestFun = JSObjectMakeFunctionWithCallback(context, requestStr, request);
    JSObjectSetProperty(context, wpeBridgeObject, requestStr, requestFun,
        kJSPropertyAttributeReadOnly | kJSPropertyAttributeDontDelete |
        kJSPropertyAttributeDontEnum, &exception);
    JSStringRelease(requestStr);

    // Pass javascript context to native wpeBridge implementation
    // This is the first time the ORBClient signleton is called from the WPE web process.
    ORBClient::GetSharedInstance().SetJavaScriptContext(context);
    ORBClient::GetSharedInstance().SubscribeWithJavaScriptEventDispatchRequestedEvent();

    // Trigger the Manager::OnApplicationPageChanged event
    ORBClient::GetSharedInstance().ApplicationPageChanged(ToStdString(frameUrlAsString));
    WKRelease(frameUrlAsString);
}

/**
 * @brief Handles Message received from WebKitImplementation
 *
 * Used to call ORBClient::DispatchEvent method when async events are received.
 * DispatchEvent needs to be called from the main thread and this can be achieved by
 * using g_main_context_invoke_full of WebKitImplementation.cpp
 *
 * @param page
 * @param messageName - The type of message received. At the moment, Tag::DispatchEvent is supported
 * @param messageBody - The messageBody follows the convention: {"type": "<string>", "properties": <json_object>}
 * @return true
 * @return false
 */
bool HandleMessageToPage(WKBundlePageRef page, WKStringRef messageName, WKTypeRef messageBody)
{
    // convert WKTypeRef to std::string
    std::string info;
    WKStringRef wkStringRef = static_cast<WKStringRef>(messageBody);
    size_t bufferSize = WKStringGetMaximumUTF8CStringSize(wkStringRef);
    std::unique_ptr<char[]> buffer(new char[bufferSize]);
    size_t stringLength = WKStringGetUTF8CString(wkStringRef, buffer.get(), bufferSize);
    info = Core::ToString(buffer.get(), stringLength - 1);

    fprintf(stderr, "[WpeBridge::HandleMessageToPage] %s\n", info.c_str());

    if (WKStringIsEqualToUTF8CString(messageName, Tags::DispatchEvent))
    {
        // parse the json object
        JsonObject jo; jo.FromString(info);
        std::string type = jo["type"].String();
        JsonObject properties = jo["properties"].Object();

        ORBClient::GetSharedInstance().DispatchEvent(type, properties, true, "");

        return true;
    }
    return false;
}
} // WPEBridge
} // JavaScript
} // WPEFramework
