#include "ORBComRpcClient.h"
#include "ORBLogging.h"

namespace orb
{
   /**
    * @brief ORBComRpcClient::ORBComRpcClient()
    * 
    * Initialize ORBComRpcClient
    */
   ORBComRpcClient::ORBComRpcClient()
      :  m_remoteConnection(GetConnectionEndpoint()),
         m_engine(Core::ProxyType<RPC::InvokeServerType<1, 0, 4>>::Create()),
         m_client(Core::ProxyType<RPC::CommunicatorClient>::Create(m_remoteConnection, Core::ProxyType<Core::IIPCServer>(m_engine))),
         m_notification(),
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
      m_controller = m_client->Open<PluginHost::IShell>(_T("SamplePlugin"), ~0, 3000);
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
    * Return true if we connected to Thunder successfully and managed to
    * find the COM-RPC interface(s) we care about
    */
   bool ORBComRpcClient::IsValid()
   {
      return m_valid;
   }

   // actual calls

   std::string ORBComRpcClient::ExecuteBridgeRequest(std::string request)
   {
      std::string result = "";
      if (_orb)
      {
         ORB_LOG("Calling ExecuteBridgeRequest");
         result = _orb->ExecuteBridgeRequest(request);
      }
      return result;
   }

   std::string ORBComRpcClient::CreateToken(std::string uri)
   {
      std::string result = "";
      if (_orb)
      {
         ORB_LOG("Calling CreateToken");
         result = _orb->CreateToken(uri);
      }
      return result;
   }


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

}   // namespace orb