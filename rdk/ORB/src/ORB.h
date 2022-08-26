/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

#pragma once

#include "Module.h"
#include "application_manager.h"
#include "BroadcastRequestHandler.h"
#include "ConfigurationRequestHandler.h"
#include "ManagerRequestHandler.h"
#include "ProgrammeRequestHandler.h"
#include "ParentalControlRequestHandler.h"
#include "TokenManager.h"
#include "ORBPlatform.h"
#include "ORBPlatformLoader.h"
#include "MetadataSearchTask.h"

#include <map>
#include <memory>
#include <interfaces/json/JsonData_ORB.h>

using namespace orb;
using namespace WPEFramework::JsonData::ORB;

namespace WPEFramework {
namespace Plugin {
/**
 * @brief orb::ORB
 *
 * Implementation of the ORB plugin
 */
class ORB
   : public PluginHost::IPlugin
   , public PluginHost::JSONRPC {
private:

   ORB(const ORB&) = delete;
   ORB& operator=(const ORB&) = delete;

   /**
    * @brief ORB::Notification
    *
    * Used to receive activation/deactivation events.
    */
   class Notification : public RPC::IRemoteConnection::INotification {
private:
      Notification();
      Notification(const Notification&);
      Notification& operator=(const Notification&);

public:

      explicit Notification(ORB *parent) : _parent(*parent)
      {
         ASSERT(parent != nullptr);
      }

      ~Notification()
      {
      }

      BEGIN_INTERFACE_MAP(Notification)
      INTERFACE_ENTRY(RPC::IRemoteConnection::INotification)
      END_INTERFACE_MAP

      virtual void Activated(RPC::IRemoteConnection *)
      {
      }

      virtual void Deactivated(RPC::IRemoteConnection *connection)
      {
         _parent.Deactivated(connection);
      }

private:
      ORB& _parent;
   }; // class Notification

   /**
    * @brief ORB::Config
    *
    * Used to map the plugin configuration.
    */
   class Config : public Core::JSON::Container {
private:
      Config(const Config&);
      Config& operator=(const Config&);

public:
      Config()
         : Core::JSON::Container()
         , OutOfProcess(true)
      {
         Add(_T("outofprocess"), &OutOfProcess);
      }

      ~Config()
      {
      }

public:
      Core::JSON::Boolean OutOfProcess;
   }; // class Config

public:

   /**
    * Default constructor.
    */
   ORB()
      : _service(nullptr)
      , _orb(nullptr)
      , _skipURL(0)
      , _connectionId(0)
      , _notification(this)
      , _orbPlatformLoader(std::make_shared<ORBPlatformLoader>())
      , _tokenManager(std::make_shared<TokenManager>())
      , _broadcastRequestHandler(std::make_shared<BroadcastRequestHandler>())
      , _configurationRequestHandler(std::make_shared<ConfigurationRequestHandler>())
      , _managerRequestHandler(std::make_shared<ManagerRequestHandler>())
      , _programmeRequestHandler(std::make_shared<ProgrammeRequestHandler>())
      , _parentalControlRequestHandler(std::make_shared<ParentalControlRequestHandler>())
      , _currentAppId(UINT16_MAX)
      , _orbPlatform(nullptr)
   {
      ORB::instance(this);
      RegisterAll();
      SYSLOG(Logging::Startup, (_T("ORB service instance constructed")));
   }

   /**
    * Destructor.
    */
   ~ORB()
   {
      UnregisterAll();
      _metadataSearchTasks.clear();
      SYSLOG(Logging::Shutdown, (_T("ORB service instance destructed")));
   }

   /**
    * Singleton.
    */
   static ORB* instance(ORB *orb = nullptr)
   {
      static ORB *orb_instance = nullptr;
      if (orb != nullptr)
      {
         orb_instance = orb;
      }
      return orb_instance;
   }

public:

   BEGIN_INTERFACE_MAP(ORB)
   INTERFACE_ENTRY(PluginHost::IPlugin)
   INTERFACE_ENTRY(PluginHost::IDispatcher)
   END_INTERFACE_MAP

public:

   // IPlugin methods
   virtual const string Initialize(PluginHost::IShell *service);
   virtual void Deinitialize(PluginHost::IShell *service);
   virtual string Information() const;

public:

   // orb component getters
   std::shared_ptr<ApplicationManager> GetApplicationManager()
   {
      return _applicationManager;
   }

   std::shared_ptr<TokenManager> GetTokenManager()
   {
      return _tokenManager;
   }

   std::shared_ptr<BroadcastRequestHandler> GetBroadcastRequestHandler()
   {
      return _broadcastRequestHandler;
   }

   std::shared_ptr<ConfigurationRequestHandler> GetConfigurationRequestHandler()
   {
      return _configurationRequestHandler;
   }

   std::shared_ptr<ManagerRequestHandler> GetManagerRequestHandler()
   {
      return _managerRequestHandler;
   }

   std::shared_ptr<ProgrammeRequestHandler> GetProgrammeRequestHandler()
   {
      return _programmeRequestHandler;
   }

   std::shared_ptr<ParentalControlRequestHandler> GetParentalControlRequestHandler()
   {
      return _parentalControlRequestHandler;
   }

   ORBPlatform* GetORBPlatform()
   {
      return _orbPlatform;
   }

   // orb state setters
   void SetCurrentAppId(uint16_t appId)
   {
      _currentAppId = appId;
   }

   // orb metadata search task pool handling
   void AddMetadataSearchTask(int queryId, std::shared_ptr<MetadataSearchTask> searchTask)
   {
      _metadataSearchTasks[queryId] = searchTask;
   }

   void RemoveMetadataSearchTask(int queryId)
   {
      _metadataSearchTasks.erase(queryId);
   }

   std::shared_ptr<MetadataSearchTask> GetMetadataSearchTask(int queryId)
   {
      return _metadataSearchTasks[queryId];
   }

   // event notifications
   void NotifyJavaScriptEventDispatchRequested(std::string name, JsonObject properties, bool broadcastRelated, std::string targetOrigin);
   void NotifyDvbUrlLoaded(int requestId, unsigned int fileContentLength);

private:

   void Deactivated(RPC::IRemoteConnection *connection);

   // JsonRpc
   void RegisterAll();
   void UnregisterAll();

   // JsonRpc methods
   uint32_t ExecuteWpeBridgeRequest(JsonObject request, JsonObject& response);
   uint32_t CreateToken(Core::JSON::String uri, JsonObject& token);
   uint32_t ApplicationLoadFailed(const ApplicationLoadFailedParamsData& params);
   uint32_t ApplicationPageChanged(Core::JSON::String url);
   uint32_t LoadDvbUrl(const LoadDvbUrlParamsData& params);
   uint32_t SendKeyEvent(Core::JSON::DecUInt16 keyCode, Core::JSON::Boolean& response);

   // JsonRpc events
   void EventJavaScriptEventDispatchRequested(JavaScriptEventDispatchRequestedParamsData& params);
   void EventDvbUrlLoaded(DvbUrlLoadedParamsData& params);

private:

   // member variables
   PluginHost::IShell *_service;
   Core::IUnknown *_orb;
   uint8_t _skipURL;
   uint32_t _connectionId;
   Core::Sink<Notification> _notification;

   // orb component member variables
   std::shared_ptr<ORBPlatformLoader> _orbPlatformLoader;
   std::shared_ptr<ApplicationManager> _applicationManager;
   std::shared_ptr<TokenManager> _tokenManager;
   std::shared_ptr<BroadcastRequestHandler> _broadcastRequestHandler;
   std::shared_ptr<ConfigurationRequestHandler> _configurationRequestHandler;
   std::shared_ptr<ManagerRequestHandler> _managerRequestHandler;
   std::shared_ptr<ProgrammeRequestHandler> _programmeRequestHandler;
   std::shared_ptr<ParentalControlRequestHandler> _parentalControlRequestHandler;

   // other member variables
   uint16_t _currentAppId;
   ORBPlatform *_orbPlatform;
   std::map<int, std::shared_ptr<MetadataSearchTask> > _metadataSearchTasks;
}; // class ORB
} // namespace Plugin
} // namespace WPEFramework
