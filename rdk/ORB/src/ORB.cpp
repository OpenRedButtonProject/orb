/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

#include "ORB.h"
#include "SessionCallbackImpl.h"
#include "ORBPlatformLoader.h"

namespace WPEFramework {
namespace Plugin {
SERVICE_REGISTRATION(ORB, 1, 0);


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
   SYSLOG(Logging::Startup, (_T("ORB Initialisation started")));
   string message;

   _connectionId = 0;
   _service = service;
   _skipURL = _service->WebPrefix().length();

   _service->Register(&_notification);

   _orb = service->Root<Core::IUnknown>(_connectionId, 2000, _T("ORB"));

   // Check if ORB plugin initialisation failed
   if (_orb == nullptr)
   {
      message = _T("ORB plugin could not be initialised");
      _service->Unregister(&_notification);
      _service = nullptr;
      return message;
   }

   _orbPlatform = _orbPlatformLoader->Load();

   // Check if ORB platform implementation library load failed
   if (!_orbPlatform)
   {
      fprintf(stderr, "[ORB::Initialize] ORBPlatform could not be loaded\n");
      message = _T("ORB plugin could not be initialised");
      _service->Unregister(&_notification);
      _service = nullptr;
      return message;
   }

   fprintf(stderr, "[ORB::Initialize] ORBPlatform loaded\n");

   // Initialise platform
   if (_orbPlatform)
   {
      fprintf(stderr, "[ORB::Initialize] Initialising platform...\n");
      _orbPlatform->Platform_Initialise();
      fprintf(stderr, "[ORB::Initialize] Platform initialised\n");
   }

   // Initialise application manager
   std::unique_ptr<SessionCallbackImpl> sessionCallback = std::make_unique<SessionCallbackImpl>();
   _applicationManager = std::make_shared<ApplicationManager>(std::move(sessionCallback));

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
      if (_orbPlatform)
      {
         _orbPlatform->Platform_Finalise();
         _orbPlatformLoader->Unload(_orbPlatform);
      }
      _orb->Release();
      _orb = nullptr;
   }

   _service = nullptr;

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
} // namespace Plugin
} // namespace WPEFramework
