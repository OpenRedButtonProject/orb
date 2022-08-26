/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */
#pragma once

#include <WPE/WebKit.h>
#include <core/core.h>
#include <tracing/tracing.h>
#include <com/com.h>
#include <websocket/websocket.h>
#include "OrbUtils.h"

namespace orb {
class ORBClient {
public:

   static ORBClient& GetSharedInstance()
   {
      static ORBClient s_ORBClient;
      return s_ORBClient;
   }

   ~ORBClient();

   void SubscribeWithJavaScriptEventDispatchRequestedEvent();
   void SubscribeWithDvbUrlLoadedEvent();

   // ORB api
   JsonObject CreateToken(std::string uri);
   std::string ExecuteWpeBridgeRequest(std::string request);
   void LoadDvbUrl(std::string url, int requestId);
   void ApplicationLoadFailed(std::string url, std::string errorDescription);
   void ApplicationPageChanged(std::string url);

   void DispatchEvent(std::string type, JsonObject properties, bool broadcastRelated, std::string targetOrigin);
   void SetJavaScriptContext(JSContextRef jsContextRef);

   void AddDsmccCaller(int requestId, void *caller);
   void AddDsmccCallback(int requestId, OnDvbUrlLoaded callback);

   void* GetDsmccCaller(int requestId);
   OnDvbUrlLoaded GetDsmccCallback(int requestId);

   void RemoveDsmccCaller(int requestId);
   void RemoveDsmccCallback(int requestId);

private:

   ORBClient();
   ORBClient(ORBClient const&) = delete;
   void operator=(ORBClient const&) = delete;

   // member variables
   WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement> m_remoteObject;
   JSContextRef m_javaScriptContext;
   std::map<int, void *> m_dsmccCallers;
   std::map<int, OnDvbUrlLoaded> m_dsmccCallbacks;

   std::map<std::string, bool> m_subscribedEvents;
}; // class ORBClient
} // namespace orb
