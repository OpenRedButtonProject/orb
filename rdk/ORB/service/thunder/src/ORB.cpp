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
      _orb->Register(&_notification);
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
      _orb->Unregister(&_notification);

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

/**
 * @brief ORB::NotifyJavaScriptEventDispatchRequested
 *
 * @param name             The name of the event to be dispatched
 * @param properties       The properties of the event to be dispatched
 * @param broadcastRelated Indicates whether the event is broadcast-related or not
 * @param targetOrigin     The event's target origin
 */
void ORB::NotifyJavaScriptEventDispatchRequested(
   std::string name,
   JsonObject properties,
   bool broadcastRelated,
   std::string targetOrigin
   )
{
   std::string propertiesAsString;
   properties.ToString(propertiesAsString);
   fprintf(stderr, "[ORB::NotifyJavaScriptEventDispatchRequested] name=%s properties=%s\n", name.c_str(), propertiesAsString.c_str());
   fprintf(stderr, "[NotifyJavaScriptEventDispatchRequested] PID: %d\n", getpid());
   JavaScriptEventDispatchRequestedParamsData params;
   params.EventName = name;
   params.EventProperties = propertiesAsString;
   params.BroadcastRelated = broadcastRelated;
   params.TargetOrigin = targetOrigin;

   EventJavaScriptEventDispatchRequested(params);
}

/**
 * @brief ORB::NotifyDvbUrlLoaded
 *
 * @param requestId         The request identifier
 * @param fileContentLength The DVB file content length in number of bytes
 */
void ORB::NotifyDvbUrlLoaded(int requestId, unsigned int fileContentLength)
{
   fprintf(stderr, "[ORB::NotifyDvbUrlLoaded] requestId=%d fileContentLength=%d\n", requestId, fileContentLength);

   DvbUrlLoadedParamsData params;
   params.RequestId = requestId;
   params.FileContentLength = fileContentLength;

   EventDvbUrlLoaded(params);
}

/**
 * @brief ORB::NotifyInputKeyGenerated
 *
 * @param keyCode The JavaScript key code
 */
void ORB::NotifyInputKeyGenerated(int keyCode)
{
   fprintf(stderr, "[ORB::NotifyInputKeyGenerated] keyCode=%d\n", keyCode);

   EventInputKeyGenerated(keyCode);
}

/**
 * @brief ORB::Notification::JavaScriptEventDispatchRequest
 * 
 * This event comes from COMRPC side. In this handler we will dispatch
 * a JSONRPC event for 'JavaScriptEventDispatchRequest'
 * 
 * @param name 
 * @param properties 
 * @param broadcastRelated 
 * @param targetOrigin 
 */
void ORB::Notification::JavaScriptEventDispatchRequest(
   std::string name, 
   std::string properties, 
   bool broadcastRelated, 
   std::string targetOrigin
   )
{
   fprintf(stderr, "[ORB::Notification::JavaScriptEventDispatchRequest] - COMRPC %d\n", getpid());
   JsonObject propertiesAsObject;

   propertiesAsObject.FromString(properties);
   _parent.NotifyJavaScriptEventDispatchRequested(
      name, 
      propertiesAsObject, 
      broadcastRelated, 
      targetOrigin
   );
}

} // namespace Plugin
} // namespace WPEFramework
