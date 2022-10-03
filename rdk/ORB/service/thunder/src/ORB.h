/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

#pragma once

#include "Module.h"
#include <interfaces/json/JsonData_ORB.h>
#include "ORBImplementation.h"
#include <memory>

namespace WPEFramework {
namespace Plugin {

using namespace orb;
using namespace WPEFramework::JsonData::ORB;

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
                        public Exchange::IORB::INotification 
   {
   private:
      Notification() = delete;
      Notification(const Notification &) = delete;
      Notification &operator=(const Notification &) = delete;

public:

      explicit Notification(ORB *parent) : _parent(*parent)
      {
         ASSERT(parent != nullptr);
         fprintf(stderr, "NOTIFICATION CONSTUCTOR WITH NON NULL PARENT CALLED \n");
      }

      virtual ~Notification() override
      {
         fprintf(stderr, "NOTIFICATION DESTRUCTOR CALLED\n");
      }

public:
      void Activated(RPC::IRemoteConnection *) override
      {
         fprintf(stderr, "ORB NOTIFICATION ACTIVATED %p\n", this);
      }

      void Deactivated(RPC::IRemoteConnection *connection) override
      {
         _parent.Deactivated(connection);
      }

      void JavaScriptEventDispatchRequest(
         std::string name,
         std::string properties,
         bool broadcastRelated,
         std::string targetOrigin
      )
      {
         fprintf(stderr, "JavaScriptEventDispatchRequest\n");
      }

      void DvbUrlLoaded(
         int requestId,
         const uint8_t* fileContent, 
         const uint16_t fileContentLength
      )
      {
         fprintf(stderr, "DvbUrlLoaded\n");
      }

      void EventInputKeyGenerated(int keyCode)
      {
         fprintf(stderr, "EventInputKeyGenerated\n");
      }
  

      BEGIN_INTERFACE_MAP(Notification)
      INTERFACE_ENTRY(Exchange::IORB::INotification)
      INTERFACE_ENTRY(RPC::IRemoteConnection::INotification)
      END_INTERFACE_MAP
private:
      ORB &_parent;
   }; // class Notification
////////////////////////////////////////////////////////////////
public:
   
   ORB();
   ~ORB() override;

   // Dont allow copy/move constructors
   ORB(const ORB &) = delete;
   ORB &operator=(const ORB &) = delete;   

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

   BEGIN_INTERFACE_MAP(ORB)
   INTERFACE_ENTRY(PluginHost::IPlugin)
   INTERFACE_ENTRY(PluginHost::IDispatcher)
   INTERFACE_AGGREGATE(Exchange::IORB, _orb)
   END_INTERFACE_MAP

public:

   // IPlugin methods
   virtual const string Initialize(PluginHost::IShell *service) override;
   virtual void Deinitialize(PluginHost::IShell *service) override;
   virtual string Information() const override;

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
   uint32_t _connectionId;

   friend class Notification;
}; // class ORB
} // namespace Plugin
} // namespace WPEFramework
