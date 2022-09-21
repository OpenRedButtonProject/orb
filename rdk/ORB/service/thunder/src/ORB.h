/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

#pragma once

#include "Module.h"
#include "ORBEventListenerImpl.h"
#include <interfaces/json/JsonData_ORB.h>
#include "ORBImplementation.h"
#include <memory>

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

/**
    * @brief ORB::Notification
    *
    * Used to receive activation/deactivation events.
    */
   class Notification : public RPC::IRemoteConnection::INotification, 
                        public Exchange::IORB::INotification {
private:
      Notification() = delete;
      Notification(const Notification&) = delete;
      Notification& operator=(const Notification&) = delete;

public:

      explicit Notification(ORB *parent) : _parent(*parent)
      {
         ASSERT(parent != nullptr);
      }

      virtual ~Notification()
      {
      }

public:
      virtual void Activated(RPC::IRemoteConnection *)
      {
      }

      virtual void Deactivated(RPC::IRemoteConnection *connection)
      {
         _parent.Deactivated(connection);
      }

      BEGIN_INTERFACE_MAP(Notification)
      INTERFACE_ENTRY(Exchange::IORB::INotification)
      INTERFACE_ENTRY(RPC::IRemoteConnection::INotification)
      END_INTERFACE_MAP
private:
      ORB& _parent;
   }; // class Notification


   BEGIN_INTERFACE_MAP(ORB)
   INTERFACE_ENTRY(PluginHost::IPlugin)
   INTERFACE_ENTRY(PluginHost::IDispatcher)
   INTERFACE_AGGREGATE(Exchange::IORB, _orb)
   END_INTERFACE_MAP

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
      , _orbEventListener(std::make_shared<ORBEventListenerImpl>())
   {
      ORB::instance(this);
      //RegisterAll();
      SYSLOG(Logging::Startup, (_T("ORB service instance constructed")));
   }

   /**
    * Destructor.
    */
   ~ORB()
   {
      //UnregisterAll();
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

   // IPlugin methods
   virtual const string Initialize(PluginHost::IShell *service);
   virtual void Deinitialize(PluginHost::IShell *service);
   virtual string Information() const;


// disallow copying of plugin
private:
   ORB(const ORB&) = delete;
   ORB& operator=(const ORB&) = delete;   
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

   // event notifications
   void NotifyJavaScriptEventDispatchRequested(std::string name, JsonObject properties, bool broadcastRelated, std::string targetOrigin);
   void NotifyDvbUrlLoaded(int requestId, unsigned int fileContentLength);
   void NotifyInputKeyGenerated(int keyCode);

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
   void EventInputKeyGenerated(Core::JSON::DecSInt32 keyCode);

private:

   // member variables
   PluginHost::IShell *_service;
   Exchange::IORB* _orb;
   Core::Sink<Notification> _notification;
   uint8_t _skipURL;
   uint32_t _connectionId;
   std::shared_ptr<ORBEventListenerImpl> _orbEventListener;
}; // class ORB
} // namespace Plugin
} // namespace WPEFramework
