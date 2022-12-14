/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

#include "ORBComRpcClient.h"
#include "ORBLogging.h"

MODULE_NAME_DECLARATION(BUILD_REFERENCE)

#define EVENT_JAVASCRIPT_EVENT_DISPATCH_REQUESTED "javaScriptEventDispatchRequested"
#define EVENT_DVB_URL_LOADED "dvbUrlLoaded"
#define EVENT_DVB_URL_LOADED_NO_DATA "dvbUrlLoadedNoData"
#define EVENT_INPUT_KEY_GENERATED "inputKeyGenerated"

namespace orb
{
/******************************************************************************
** Event handlers
*****************************************************************************/

/**
 * @brief ORBComRpcClient::NotificationHandler::JavaScriptEventDispatchRequest
 *
 * React to the 'JavaScriptEventDispatchRequest' event accordingly
 *
 * @param name
 * @param properties
 * @param broadcastRelated
 * @param targetOrigin
 */
void ORBComRpcClient::NotificationHandler::JavaScriptEventDispatchRequest(
    std::string name,
    std::string properties,
    bool broadcastRelated,
    std::string targetOrigin
    )
{
    ORB_LOG("%s, %s, %d, %s", name.c_str(), properties.c_str(), broadcastRelated,
        targetOrigin.c_str());
    if (_parent.m_subscribedEvents[EVENT_JAVASCRIPT_EVENT_DISPATCH_REQUESTED] == true)
    {
        std::string eventName = name;
        std::string eventProperties = properties;
        _parent.m_onJavaScriptEventDispatchRequested(eventName, eventProperties);
    }
}

/**
 * @brief ORBComRpcClient::NotificationHandler::DvbUrlLoaded
 *
 * React to the 'DvbUrlLoaded' event accordingly
 *
 * @param requestId
 * @param fileContent
 * @param fileContentLength
 */
void ORBComRpcClient::NotificationHandler::DvbUrlLoaded(
    int requestId,
    const uint8_t *fileContent,
    unsigned int fileContentLength
    )
{
    ORB_LOG_NO_ARGS();

    if (_parent.m_subscribedEvents[EVENT_DVB_URL_LOADED] == true)
    {
        ORB_LOG("Dispatching event dvbUrlLoaded");
        unsigned char *_fileContent = reinterpret_cast<unsigned
                                                       char *>(const_cast<uint8_t *>(fileContent));
        _parent.m_onDvbUrlLoaded(requestId, _fileContent, fileContentLength);
    }
}

/**
 * @brief ORBComRpcClient::NotificationHandler::DvbUrlLoadedNoData
 *
 * React to the 'DvbUrlLoadedNoData' event accordingly
 *
 * @param requestId
 * @param fileContentLength
 */
void ORBComRpcClient::NotificationHandler::DvbUrlLoadedNoData(int requestId, unsigned int
    fileContentLength)
{
    ORB_LOG_NO_ARGS();

    if (_parent.m_subscribedEvents[EVENT_DVB_URL_LOADED_NO_DATA] == true)
    {
        ORB_LOG("Dispatching event dvbUrlLoadedNoData");
        _parent.m_onDvbUrlLoadedNoData(requestId, fileContentLength);
    }
}

/**
 * @brief ORBComRpcClient::NotificationHandler::EventInputKeyGenerated
 *
 * React to the 'EventInputKeyGenerated'  accordingly
 *
 * @param keyCode   The JavaScript key code
 * @param keyAction The JavaScript key action (0 = keyup , 1 = keydown)
 */
void ORBComRpcClient::NotificationHandler::EventInputKeyGenerated(int keyCode, uint8_t keyAction)
{
    ORB_LOG("%d", keyCode);

    if (_parent.m_subscribedEvents[EVENT_INPUT_KEY_GENERATED] == true)
    {
        ORB_LOG("Dispatching event inputKeyGenerated");
        _parent.m_onInputKeyGenerated(keyCode, keyAction);
    }
}

/******************************************************************************
** Initialise/Deinitialise and helper methods
*****************************************************************************/

/**
 * @brief ORBComRpcClient::ORBComRpcClient()
 *
 * Initialize ORBComRpcClient
 */
ORBComRpcClient::ORBComRpcClient(
    OnJavaScriptEventDispatchRequested_cb onJavaScriptEventDispatchRequested_cb,
    OnDvbUrlLoaded_cb onDvbUrlLoaded_cb,
    OnDvbUrlLoadedNoData_cb onDvbUrlLoadedNoData_cb,
    OnInputKeyGenerated_cb onInputKeyGenerated_cb
    )
    :  ORBGenericClient(onJavaScriptEventDispatchRequested_cb, onDvbUrlLoaded_cb,
                        onDvbUrlLoadedNoData_cb, onInputKeyGenerated_cb),
    m_remoteConnection(GetConnectionEndpoint()),
    m_engine(Core::ProxyType<RPC::InvokeServerType<1, 0, 4> >::Create()),
    m_client(Core::ProxyType<RPC::CommunicatorClient>::Create(m_remoteConnection,
                                                              Core::ProxyType<Core::IIPCServer>(
                                                                  m_engine))),
    m_notification(this),
    m_valid(false),
    m_subscribedEvents({})
{
    // Announce our arrival over COM-RPC
    m_engine->Announcements(m_client->Announcement());

    // Check we opened the link correctly (if Thunder isn't running, this will be false)
    if (!m_client.IsValid())
    {
        ORB_LOG("Failed to open link");
        m_valid = false;
        return;
    }

    ORB_LOG("Connecting to Thunder @ '%s'", m_client->Source().RemoteId().c_str());
    m_controller = m_client->Open<PluginHost::IShell>(_T("ORB"), ~0, 3000);
    if (!m_controller)
    {
        ORB_LOG("Failed to open IShell interface of ORB - is Thunder running?");
        m_valid = false;
        return;
    }

    //////////////////////////////////////////////////
    // check if plugin is activated functionality
    //////////////////////////////////////////////////

    // query the interface of orb
    _orb = m_controller->QueryInterface<Exchange::IORB>();
    if (!_orb)
    {
        ORB_LOG("Failed to open IORB interface of ORB - is Thunder running?");
        m_valid = false;
        return;
    }

    // register for notifications
    _orb->AddRef();
    _orb->Register(&m_notification);

    m_valid = true;

    // event subscription init
    m_subscribedEvents[EVENT_JAVASCRIPT_EVENT_DISPATCH_REQUESTED] = false;
    m_subscribedEvents[EVENT_DVB_URL_LOADED] = false;
    m_subscribedEvents[EVENT_INPUT_KEY_GENERATED] = false;
}

/**
 * @brief ORBComRpcClient::~ORBComRpcClient
 *
 * Disposal and clean up
 */
ORBComRpcClient::~ORBComRpcClient()
{
    if (m_controller)
    {
        m_controller->Release();
    }

    if (_orb)
    {
        // Remove our notification callback
        _orb->Unregister(&m_notification);

        // clean up
        _orb->Release();
    }

    // disconnect from comrpc socket
    m_client->Close(RPC::CommunicationTimeOut);
    if (m_client.IsValid())
    {
        m_client.Release();
    }

    // Dispose of any singletons we created (Thunder uses a lot of singletons internally)
    Core::Singleton::Dispose();
}

/**
 * @brief ORBComRpcClient::IsValid
 *
 * Return true if we connected to Thunder successfully and managed to
 * find the COM-RPC interface(s) we care about
 *
 * @return true
 * @return false
 */
bool ORBComRpcClient::IsValid()
{
    return m_valid;
}

/**
 * @brief ORBComRpcClient::GetConnectionEndpoint
 *
 * Returns path for communication (COMRPC)
 *
 * @return Core::NodeId
 */
Core::NodeId ORBComRpcClient::GetConnectionEndpoint()
{
    std::string communicatorPath = "";

    // On Linux, Thunder defaults to /tmp/communicator for the generic COM-RPC
    // interface
   #if PLUGIN_ORB_PRIVATE_COMRPC == false
    communicatorPath = "/tmp/communicator";
   #else
    communicatorPath = "/tmp/ORB";
   #endif

    ORB_LOG("Communicator Path: %s", communicatorPath.c_str());

    return Core::NodeId(communicatorPath.c_str());
}

/******************************************************************************
** Browser API Methods
*****************************************************************************/

/**
 * @brief ORBComRpcClient::ExecuteBridgeRequest
 *
 * Calls the ORBImplementation::ExecuteBridgeRequest COMRPC endpoint
 *
 * @param request
 * @return std::string
 */
std::string ORBComRpcClient::ExecuteBridgeRequest(std::string request)
{
    std::string result = "";
    if (_orb)
    {
        ORB_LOG("request=%s", request.c_str());
        result = _orb->ExecuteBridgeRequest(request);
    }
    return result;
}

/**
 * @brief ORBComRpcClient::CreateToken
 *
 * Calls the ORBImplementation::CreateToken COMRPC endpoint
 *
 * @param uri
 * @return std::string
 */
std::string ORBComRpcClient::CreateToken(std::string uri)
{
    std::string result = "";
    if (_orb)
    {
        ORB_LOG("uri=%s", uri.c_str());
        result = _orb->CreateToken(uri);
    }
    return result;
}

/**
 * @brief ORBComRpcClient::LoadDvbUrl
 *
 * Calls the ORBImplementation::LoadDvbUrl COMRPC endpoint
 *
 * @param url
 * @param requestId
 */
void ORBComRpcClient::LoadDvbUrl(std::string url, int requestId)
{
    if (_orb)
    {
        ORB_LOG("url=%s requestId=%d", url.c_str(), requestId);
        _orb->LoadDvbUrl(url, requestId);
    }
}

/**
 * @brief ORBComRpcClient::NotifyApplicationLoadFailed
 *
 * Calls the ORBImplementation::NotifyApplicationLoadFailed COMRPC endpoint
 *
 * @param url
 * @param errorDescription
 */
void ORBComRpcClient::NotifyApplicationLoadFailed(std::string url, std::string errorDescription)
{
    if (_orb)
    {
        ORB_LOG("url=%s errorDescription=%s", url.c_str(), errorDescription.c_str());
        _orb->NotifyApplicationLoadFailed(url, errorDescription);
    }
}

/**
 * @brief ORBComRpcClient::NotifyApplicationPageChanged
 *
 * Calls the ORBImplementation::NotifyApplicationPageChanged COMRPC endpoint
 *
 * @param url
 */
void ORBComRpcClient::NotifyApplicationPageChanged(std::string url)
{
    if (_orb)
    {
        ORB_LOG("url=%s", url.c_str());
        _orb->NotifyApplicationPageChanged(url);
    }
}

/**
 * Get the User-Agent string.
 *
 * @return The User-Agent string
 */
std::string ORBComRpcClient::GetUserAgentString()
{
    std::string userAgentString;
    if (_orb)
    {
        ORB_LOG_NO_ARGS();
        userAgentString = _orb->GetUserAgentString();
    }
    return userAgentString;
}

/******************************************************************************
** Events subscribe and unsubscribe
*****************************************************************************/

/**
 * Subscribe to 'JavaScriptEventDispatchRequestedEvent'
 */
void ORBComRpcClient::SubscribeToJavaScriptEventDispatchRequestedEvent()
{
    ORB_LOG_NO_ARGS();
    m_subscribedEvents[EVENT_JAVASCRIPT_EVENT_DISPATCH_REQUESTED] = true;
}

/**
 * Subscribe to 'DvbUrlLoadedEvent'
 */
void ORBComRpcClient::SubscribeToDvbUrlLoadedEvent()
{
    ORB_LOG_NO_ARGS();
    m_subscribedEvents[EVENT_DVB_URL_LOADED] = true;
}

/**
 * Subscribe to 'DvbUrlLoadedNoDataEvent'
 */
void ORBComRpcClient::SubscribeToDvbUrlLoadedNoDataEvent()
{
    ORB_LOG_NO_ARGS();
    m_subscribedEvents[EVENT_DVB_URL_LOADED_NO_DATA] = true;
}

/**
 * Subscribe to 'InputKeyGeneratedEvent'
 */
void ORBComRpcClient::SubscribeToInputKeyGeneratedEvent()
{
    ORB_LOG_NO_ARGS();
    m_subscribedEvents[EVENT_INPUT_KEY_GENERATED] = true;
}

/**
 * Unsubscribe from 'JavaScriptEventDispatchRequestedEvent'
 */
void ORBComRpcClient::UnsubscribeFromJavaScriptEventDispatchRequestedEvent()
{
    ORB_LOG_NO_ARGS();
    m_subscribedEvents[EVENT_JAVASCRIPT_EVENT_DISPATCH_REQUESTED] = false;
}

/**
 * Unsubscribe from 'DvbUrlLoadedEvent'
 */
void ORBComRpcClient::UnsubscribeFromDvbUrlLoadedEvent()
{
    ORB_LOG_NO_ARGS();
    m_subscribedEvents[EVENT_DVB_URL_LOADED] = false;
}

/**
 * Unsubscribe from 'DvbUrlLoadedNoDataEvent'
 */
void ORBComRpcClient::UnsubscribeFromDvbUrlLoadedNoDataEvent()
{
    ORB_LOG_NO_ARGS();
    m_subscribedEvents[EVENT_DVB_URL_LOADED_NO_DATA] = false;
}

/**
 * Unsubscribe from 'InputKeyGeneratedEvent'
 */
void ORBComRpcClient::UnsubscribeFromInputKeyGeneratedEvent()
{
    ORB_LOG_NO_ARGS();
    m_subscribedEvents[EVENT_INPUT_KEY_GENERATED] = false;
}

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
    OnDvbUrlLoadedNoData_cb onDvbUrlLoadedNoData_cb,
    OnInputKeyGenerated_cb onInputKeyGenerated_cb
    )
{
    return std::make_shared<ORBComRpcClient>(
        onJavaScriptEventDispatchRequested_cb,
        onDvbUrlLoaded_cb,
        onDvbUrlLoadedNoData_cb,
        onInputKeyGenerated_cb
        );
}
}   // namespace orb