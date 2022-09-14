/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */
#include "ORBJsonRpcClient.h"
#include <plugins/plugins.h>

// Declare module name for tracer.
MODULE_NAME_DECLARATION(BUILD_REFERENCE)

#define EVENT_JAVASCRIPT_EVENT_DISPATCH_REQUESTED "javaScriptEventDispatchRequested"
#define EVENT_DVB_URL_LOADED "dvbUrlLoaded"
#define EVENT_INPUT_KEY_GENERATED "inputKeyGenerated"

#define TIMEOUT_FOR_TWOWAY_METHODS 2000 // milliseconds
#define TIMEOUT_FOR_ONEWAY_METHODS 500  // milliseconds

namespace orb
{
/**
 * Constructor.
 */
ORBJsonRpcClient::ORBJsonRpcClient(
    OnJavaScriptEventDispatchRequested_cb onJavaScriptEventDispatchRequested_cb,
    OnDvbUrlLoaded_cb onDvbUrlLoaded_cb,
    OnInputKeyGenerated_cb onInputKeyGenerated_cb
    )
    : ORBGenericClient(onJavaScriptEventDispatchRequested_cb, onDvbUrlLoaded_cb,
                       onInputKeyGenerated_cb)
    , m_remoteObject("ORB.1", "client.events.88")
{
    fprintf(stderr, "[ORBJsonRpcClient::ORBJsonRpcClient]\n");
    WPEFramework::Core::SystemInfo::SetEnvironment(_T("THUNDER_ACCESS"), (_T("127.0.0.1:9998")));
    m_subscribedEvents[EVENT_JAVASCRIPT_EVENT_DISPATCH_REQUESTED] = false;
    m_subscribedEvents[EVENT_DVB_URL_LOADED] = false;
    m_subscribedEvents[EVENT_INPUT_KEY_GENERATED] = false;
}

ORBJsonRpcClient::~ORBJsonRpcClient()
{
    fprintf(stderr, "[ORBJsonRpcClient::~ORBJsonRpcClient]\n");
    UnsubscribeFromJavaScriptEventDispatchRequestedEvent();
    UnsubscribeFromDvbUrlLoadedEvent();
    UnsubscribeFromInputKeyGeneratedEvent();
}

std::string ORBJsonRpcClient::ExecuteBridgeRequest(std::string jsonRequest)
{
    fprintf(stderr, "[ORBJsonRpcClient::ExecuteBridgeRequest] request=%s\n", jsonRequest.c_str());

    JsonObject params;
    JsonObject result;

    params.FromString(jsonRequest);

    uint32_t error_code = m_remoteObject.Invoke<JsonObject, JsonObject>(
        TIMEOUT_FOR_TWOWAY_METHODS, _T("ExecuteWpeBridgeRequest"), params, result);

    std::string resultAsString;
    result.ToString(resultAsString);

    fprintf(stderr, "[ORBJsonRpcClient::ExecuteBridgeRequest] error_code=%u result=%s\n",
        error_code, resultAsString.c_str());

    if (error_code == 0)
    {
        return resultAsString;
    }

    return _T("{}");
}

std::string ORBJsonRpcClient::CreateToken(std::string uri)
{
    fprintf(stderr, "[ORBJsonRpcClient::CreateToken] uri=%s\n", uri.c_str());

    Core::JSON::String params;
    JsonObject result;

    params.FromString(uri);

    uint32_t error_code = m_remoteObject.Invoke<Core::JSON::String, JsonObject>(
        TIMEOUT_FOR_TWOWAY_METHODS, _T("CreateToken"), params, result);

    std::string resultAsString;
    result.ToString(resultAsString);

    fprintf(stderr, "[ORBJsonRpcClient::CreateToken] error_code=%u result=%s\n",
        error_code, resultAsString.c_str());

    if (error_code == 0)
    {
        return resultAsString;
    }

    return "{}";
}

void ORBJsonRpcClient::LoadDvbUrl(std::string url, int requestId)
{
    fprintf(stderr, "[ORBJsonRpcClient::LoadDvbUrl] url=%s requestId=%d\n", url.c_str(), requestId);

    LoadDvbUrlParamsData params;
    params.Url = url;
    params.RequestId = requestId;

    uint32_t error_code = m_remoteObject.Invoke<LoadDvbUrlParamsData, void>(
        TIMEOUT_FOR_ONEWAY_METHODS, _T("LoadDvbUrl"), params);

    fprintf(stderr, "[ORBJsonRpcClient::LoadDvbUrl] error_code=%u\n", error_code);

    return;
}

void ORBJsonRpcClient::NotifyApplicationLoadFailed(std::string url, std::string errorDescription)
{
    fprintf(stderr, "[ORBJsonRpcClient::NotifyApplicationLoadFailed] url=%s errorDescription=%s\n",
        url.c_str(), errorDescription.c_str());

    JsonObject params;
    params["url"] = url;
    params["errorDescription"] = errorDescription;

    uint32_t error_code = m_remoteObject.Invoke<JsonObject, void>(
        TIMEOUT_FOR_ONEWAY_METHODS, _T("ApplicationLoadFailed"), params);

    fprintf(stderr, "[ORBJsonRpcClient::NotifyApplicationLoadFailed] error_code=%u\n", error_code);

    return;
}

void ORBJsonRpcClient::NotifyApplicationPageChanged(std::string url)
{
    fprintf(stderr, "[ORBJsonRpcClient::NotifyApplicationPageChanged] url=%s \n", url.c_str());

    Core::JSON::String params;
    params.FromString(url);

    uint32_t error_code = m_remoteObject.Invoke<Core::JSON::String, void>(
        TIMEOUT_FOR_ONEWAY_METHODS, _T("ApplicationPageChanged"), params);

    fprintf(stderr, "[ORBJsonRpcClient::NotifyApplicationPageChanged] error_code=%u\n", error_code);

    return;
}

/**
 * Subscribe to the JavaScriptEventDispatchRequested event.
 */
void ORBJsonRpcClient::SubscribeToJavaScriptEventDispatchRequestedEvent()
{
    uint32_t error_code = WPEFramework::Core::ERROR_NONE;

    if (m_subscribedEvents[EVENT_JAVASCRIPT_EVENT_DISPATCH_REQUESTED])
    {
        return;
    }

    error_code = m_remoteObject.Subscribe<JsonObject>(
        TIMEOUT_FOR_ONEWAY_METHODS,
        _T(EVENT_JAVASCRIPT_EVENT_DISPATCH_REQUESTED),
        &ORBJsonRpcClient::JavaScriptEventDispatchRequested,
        this
        );

    fprintf(stderr, "[ORBJsonRpcClient::SubscribeToJavaScriptEventDispatchRequestedEvent] %s\n",
        error_code == WPEFramework::Core::ERROR_NONE ? "success" : "failure");

    if (error_code == WPEFramework::Core::ERROR_NONE)
    {
        m_subscribedEvents[EVENT_JAVASCRIPT_EVENT_DISPATCH_REQUESTED] = true;
    }
}

/**
 * Subscribe to the DvbUrlLoaded event.
 */
void ORBJsonRpcClient::SubscribeToDvbUrlLoadedEvent()
{
    uint32_t error_code = WPEFramework::Core::ERROR_NONE;

    if (m_subscribedEvents[EVENT_DVB_URL_LOADED])
    {
        return;
    }

    error_code = m_remoteObject.Subscribe<JsonObject>(
        TIMEOUT_FOR_ONEWAY_METHODS,
        _T(EVENT_DVB_URL_LOADED),
        &ORBJsonRpcClient::DvbUrlLoaded,
        this
        );

    fprintf(stderr, "[ORBJsonRpcClient::SubscribeToDvbUrlLoadedEvent] %s\n",
        error_code == WPEFramework::Core::ERROR_NONE ? "success" : "failure");

    if (error_code == WPEFramework::Core::ERROR_NONE)
    {
        m_subscribedEvents[EVENT_DVB_URL_LOADED] = true;
    }
}

/**
 * Subscribe to the InputKeyGenerated event.
 */
void ORBJsonRpcClient::SubscribeToInputKeyGeneratedEvent()
{
    uint32_t error_code = WPEFramework::Core::ERROR_NONE;

    if (m_subscribedEvents[EVENT_INPUT_KEY_GENERATED])
    {
        return;
    }

    error_code = m_remoteObject.Subscribe<Core::JSON::DecSInt32>(
        TIMEOUT_FOR_ONEWAY_METHODS,
        _T(EVENT_INPUT_KEY_GENERATED),
        &ORBJsonRpcClient::InputKeyGenerated,
        this
        );

    fprintf(stderr, "[ORBJsonRpcClient::SubscribeToInputKeyGeneratedEvent] %s\n",
        error_code == WPEFramework::Core::ERROR_NONE ? "success" : "failure");

    if (error_code == WPEFramework::Core::ERROR_NONE)
    {
        m_subscribedEvents[EVENT_INPUT_KEY_GENERATED] = true;
    }
}

/**
 * Unsubscribe from the JavaScriptEventDispatchRequested event.
 */
void ORBJsonRpcClient::UnsubscribeFromJavaScriptEventDispatchRequestedEvent()
{
    if (m_subscribedEvents[EVENT_JAVASCRIPT_EVENT_DISPATCH_REQUESTED])
    {
        m_remoteObject.Unsubscribe(TIMEOUT_FOR_ONEWAY_METHODS, _T(
            EVENT_JAVASCRIPT_EVENT_DISPATCH_REQUESTED));
        m_subscribedEvents[EVENT_JAVASCRIPT_EVENT_DISPATCH_REQUESTED] = false;
    }
}

/**
 * Unsubscribe from the DvbUrlLoaded event.
 */
void ORBJsonRpcClient::UnsubscribeFromDvbUrlLoadedEvent()
{
    if (m_subscribedEvents[EVENT_DVB_URL_LOADED])
    {
        m_remoteObject.Unsubscribe(TIMEOUT_FOR_ONEWAY_METHODS, _T(EVENT_DVB_URL_LOADED));
        m_subscribedEvents[EVENT_DVB_URL_LOADED] = false;
    }
}

/**
 * Unsubscribe from the InputKeyGenerated event.
 */
void ORBJsonRpcClient::UnsubscribeFromInputKeyGeneratedEvent()
{
    if (m_subscribedEvents[EVENT_INPUT_KEY_GENERATED])
    {
        m_remoteObject.Unsubscribe(TIMEOUT_FOR_ONEWAY_METHODS, _T(EVENT_INPUT_KEY_GENERATED));
        m_subscribedEvents[EVENT_INPUT_KEY_GENERATED] = false;
    }
}

/******************************************************************************
** Event handlers
*****************************************************************************/



void ORBJsonRpcClient::JavaScriptEventDispatchRequested(const JsonObject& params)
{
    std::string eventName = params["eventName"].String();
    std::string eventProperties = params["eventProperties"].String();
    m_onJavaScriptEventDispatchRequested(eventName, eventProperties);
}

void ORBJsonRpcClient::DvbUrlLoaded(const JsonObject& params)
{
    int requestId = params["requestId"].Number();
    unsigned int fileContentLength = params["fileContentLength"].Number();
    m_onDvbUrlLoaded(requestId, fileContentLength);
}

void ORBJsonRpcClient::InputKeyGenerated(const Core::JSON::DecSInt32 keyCode)
{
    m_onInputKeyGenerated(keyCode.Value());
}

/******************************************************************************
** ORB client creator method
*****************************************************************************/



/**
 * Create a new ORB client instance.
 *
 * @param onJavaScriptEventDispatchRequested_cb The OnJavaScriptEventDispatchRequested callback
 * @param onDvbUrlLoaded_cb                     The OnDvbUrlLoaded callback
 * @param onInputKeyGenerated_cb                The OnInputKeyGenerated callback
 *
 * @return Pointer to the new ORB client instance
 */
std::shared_ptr<ORBGenericClient> CreateORBClient(
    OnJavaScriptEventDispatchRequested_cb onJavaScriptEventDispatchRequested_cb,
    OnDvbUrlLoaded_cb onDvbUrlLoaded_cb,
    OnInputKeyGenerated_cb onInputKeyGenerated_cb
    )
{
    return std::make_shared<ORBJsonRpcClient>(
        onJavaScriptEventDispatchRequested_cb,
        onDvbUrlLoaded_cb,
        onInputKeyGenerated_cb
        );
}
} // namespace orb
