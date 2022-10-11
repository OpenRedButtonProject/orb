/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

#include "ORB.h"
#include <chrono>

using namespace std::chrono_literals;

namespace WPEFramework {
namespace Plugin {

SERVICE_REGISTRATION(ORB, 1, 0);

/**
 * Constructor
 */
ORB::ORB()
   : _service(nullptr)
   , _orb(nullptr)
   , _connectionId(0)
   , _notification(this)
{
   SYSLOG(Logging::Startup, (_T("ORB service instance constructed")));
}

/**
 * Destructor.
 */
ORB::~ORB()
{
   SYSLOG(Logging::Shutdown, (_T("ORB service instance destructed")));
}

/**
 * @brief ORB::Initialize
 *
 * Initialise the ORB plugin.
 *
 * @param service
 *
 * @return An empty string in success, or else an error message
 */
const string ORB::Initialize(PluginHost::IShell *service)
{
   string message;
   
   ASSERT(_service == nullptr);
   ASSERT(_orb == nullptr);

   SYSLOG(Logging::Startup, (_T("ORB Initialisation started in process %d"), Core::ProcessInfo().Id()));
   
   // Register Connection::Notification
   _service = service;
   _service->Register(&_notification);
   instance(this);

   fprintf(stderr, "READY TO CALL\n");
   _orb = service->Root<Exchange::IORB>(_connectionId, 2000, _T("ORBImplementation"));

   // Check if ORB plugin initialisation failed
   if (_orb != nullptr)
   {
      _orb->LoadPlatform();
      RegisterAll();
   }
   else
   {
      message = _T("ORB plugin could not be initialised");
      _service->Unregister(&_notification);
      _service = nullptr;
   }
   
   // Reached successful initialisation
   SYSLOG(Logging::Startup, (_T("ORB Initialisation finished")));
   return message;
}

/**
 * @brief ORB::Deinitialize
 *
 * @param service
 */
void ORB::Deinitialize(PluginHost::IShell *service)
{
   ASSERT(_service == service);

   SYSLOG(Logging::Shutdown, (_T("ORB Deinitialisation started")));

   _service->Unregister(&_notification);

   if (_orb != nullptr)
   {
      _service->Unregister(&_notification);
      _orb->UnLoadPlatform();

      UnregisterAll();
      _orb->Release();
   }

   // Set everything back to default
   _connectionId = 0;
   _service = nullptr;
   _orb = nullptr;

   SYSLOG(Logging::Shutdown, (_T("ORB Deinitialisation finished")));
}

/**
 * @brief ORB::Information
 *
 * @return
 */
string ORB::Information() const
{
   return string();
}

/**
 * @brief ORB::Deactivated
 *
 * @param connection
 */
void ORB::Deactivated(RPC::IRemoteConnection *connection)
{
   SYSLOG(Logging::Notification, (_T("ORB Deactivation started")));
   if (connection->Id() == _connectionId)
   {
      ASSERT(_service != nullptr);
      PluginHost::WorkerPool::Instance().Submit(PluginHost::IShell::Job::Create(_service,
         PluginHost::IShell::DEACTIVATED, PluginHost::IShell::FAILURE));
   }
   SYSLOG(Logging::Notification, (_T("ORB Deactivation finished")));
}

} // namespace Plugin
} // namespace WPEFramework
