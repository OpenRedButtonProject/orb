/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

#include "ORBComRpcServer.h"

ORBComRpcServer::ORBComRpcServer(const Core::NodeId& socket,
                                 Exchange::IORB *parentInterface,
                                 PluginHost::IShell *shell,
                                 const string& proxyStubPath,
                                 const Core::ProxyType<RPC::InvokeServer>& engine)
   : RPC::Communicator(socket, proxyStubPath, Core::ProxyType<Core::IIPCServer>(engine))
   , _orb(parentInterface)
   , _shell(shell)
{
   uint32_t result = Open(Core::infinite);

   engine->Announcements(RPC::Communicator::Announcement());

   if (result != Core::ERROR_NONE)
   {
      TRACE(Trace::Error, (_T("Failed to open COM-RPC server for SamplePlugin with error %d (%s)"), result, Core::ErrorToString(result)));
   }
   else
   {
      TRACE(Trace::Initialisation, (_T("Successfully opened COM-RPC server for SamplePlugin @ '%s'"), RPC::Communicator::Connector().c_str()));
   }
}

ORBComRpcServer::~ORBComRpcServer()
{
   TRACE(Trace::Information, (_T("Shutting down COM-RPC server for SamplePlugin")));

   Close(Core::infinite);
}

/**
 * @brief If the connecting client wants an interface we provide, return it to them
 *
 * We can only return either the IORB or IShell interfaces provided to us in the
 * constructor since we don't know about anything else
 */
void * ORBComRpcServer::Aquire(const string& className VARIABLE_IS_NOT_USED, const uint32_t interfaceId,
   const uint32_t versionId)
{
   void *result = nullptr;

   // Currently we only support version 1
   if ((versionId == 1) || (versionId == static_cast<uint32_t>(~0)))
   {
      if (interfaceId == Exchange::IORB::ID && _orb != nullptr)
      {
         result = _orb->QueryInterface(interfaceId);
      }
      else if (interfaceId == PluginHost::IShell::ID && _shell != nullptr)
      {
         result = _shell->QueryInterface(interfaceId);
      }
      else
      {
         TRACE(Trace::Error, (_T("Cannot only acquire the ISamplePlugin or IShell interface from this server!")));
      }
   }

   return result;
}