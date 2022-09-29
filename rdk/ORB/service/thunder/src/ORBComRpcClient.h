#pragma once

#include "Module.h"
#include <interfaces/IORB.h>

#include "ORBGenericClient.h"

using namespace WPEFramework;

namespace orb
{
class ORBComRpcClient
{
   // We want do run our own custom code when the plugin raises a notification
   // Implement the INotification class to do what we want
   class NotificationHandler : public Exchange::IORB::INotification
   {
   
      // Must define an interface map since we are implementing an interface on the exchange
      // so Thunder knows what type we are
      BEGIN_INTERFACE_MAP(NotificationHandler)
      INTERFACE_ENTRY(Exchange::IORB::INotification)
      END_INTERFACE_MAP
   };

   public:
      ORBComRpcClient();
      ~ORBComRpcClient();

   public:

      // actual calls
      std::string ExecuteBridgeRequest(std::string request);
      std::string CreateToken(std::string uri);
      void NotifyApplicationLoadFailed(std::string url, std::string errorDescription);
      void NotifyApplicationPageChanged(std::string url);
      bool SendKeyEvent(int keyCode);
      void LoadDvbUrl(std::string url, int requestId);
   
      bool IsValid();

   private:
    Core::NodeId GetConnectionEndpoint();

   private:
      Core::NodeId m_remoteConnection;
      
      // An engine that can serialize/deserialize the COMRPC messages. Can be configured
      // to tune performance:
      // 1 = Number of threads allocated to this connection
      // 0 = Stack size per thread
      // 4 = Message slots. 4 which means that if 4 messages have
      // been queued, the submission of the 5th element will be a
      // blocking call until there is a free slot again
      Core::ProxyType<RPC::InvokeServerType<1, 0, 4>> m_engine;

      Core::ProxyType<RPC::CommunicatorClient> m_client;

      // The implementation of the ISamplePlugin interface - this could be an in-process, out-of-process
      // or distributed plugin, but as a client we don't need to worry about that
      Exchange::IORB *_orb;
      PluginHost::IShell *m_controller;

      // used to store if valid after connection
      bool m_valid;

      // Instance of our notification handler
      Core::Sink<NotificationHandler> m_notification;

}; // class ORBComRpcClient 
}  // namespace orb