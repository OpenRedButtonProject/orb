#include "ORBComRpcClient.h"
MODULE_NAME_DECLARATION(BUILD_REFERENCE)
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
   ORB_LOG("%s, %s, %d, %s", name.c_str(), properties.c_str(), broadcastRelated, targetOrigin.c_str());
   std::string eventName = name;
   std::string eventProperties = properties;
   _parent.m_onJavaScriptEventDispatchRequested(eventName, eventProperties);
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
   const uint8_t* fileContent, 
   const uint16_t fileContentLength
)
{
   ORB_LOG_NO_ARGS();
}

/**
 * @brief ORBComRpcClient::NotificationHandler::EventInputKeyGenerated
 * 
 * React to the 'EventInputKeyGenerated'  accordingly
 * 
 * @param keyCode 
 */
void ORBComRpcClient::NotificationHandler::EventInputKeyGenerated(int keyCode)
{
   ORB_LOG("%d", keyCode);
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
   OnInputKeyGenerated_cb onInputKeyGenerated_cb
   )
   :  ORBGenericClient(onJavaScriptEventDispatchRequested_cb, onDvbUrlLoaded_cb, onInputKeyGenerated_cb),
      m_remoteConnection(GetConnectionEndpoint()),
      m_engine(Core::ProxyType<RPC::InvokeServerType<1, 0, 4>>::Create()),
      m_client(Core::ProxyType<RPC::CommunicatorClient>::Create(m_remoteConnection, Core::ProxyType<Core::IIPCServer>(m_engine))),
      m_notification(this),
      m_valid(false)
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
   std::string communicatorPath;
   Core::SystemInfo::GetEnvironment(_T("COMMUNICATOR_PATH"), communicatorPath);

   // On linux, Thunder defaults to /tmp/communicator for the generic COM-RPC
   // interface
   if (communicatorPath.empty())
   {
      communicatorPath = "/tmp/communicator";
   }

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
      ORB_LOG("Calling ExecuteBridgeRequest: %s", request.c_str());
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
      ORB_LOG("Calling CreateToken: %s", uri.c_str());
      result = _orb->CreateToken(uri);
   }
   return result;
}

/**
 * @brief 
 * 
 * @param url 
 * @param requestId 
 */
void ORBComRpcClient::LoadDvbUrl(std::string url, int requestId)
{
   if (_orb)
   {
      ORB_LOG("Calling LoadDvbUrl: %s, %d", url.c_str(), requestId);
      _orb->LoadDvbUrl(url, requestId);
   }
}

/**
 * @brief 
 * 
 * @param url 
 * @param errorDescription 
 */
void ORBComRpcClient::NotifyApplicationLoadFailed(std::string url, std::string errorDescription)
{
   if (_orb)
   {
      ORB_LOG("Calling NotifyApplicationLoadFailed: %s, %s", url.c_str(), errorDescription.c_str());
      _orb->NotifyApplicationLoadFailed(url, errorDescription);
   }
}

/**
 * @brief 
 * 
 * @param url 
 */
void ORBComRpcClient::NotifyApplicationPageChanged(std::string url)
{
   if (_orb)
   {
      ORB_LOG("Calling NotifyApplicationPageChanged: %s", url.c_str());
      _orb->NotifyApplicationPageChanged(url);
   }
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
   OnInputKeyGenerated_cb onInputKeyGenerated_cb
   )
{
   return std::make_shared<ORBComRpcClient>(
      onJavaScriptEventDispatchRequested_cb,
      onDvbUrlLoaded_cb,
      onInputKeyGenerated_cb
      );
}

}   // namespace orb