/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */
#pragma once

#include "ORBGenericClient.h"

#include <core/core.h>
#include <tracing/tracing.h>
#include <com/com.h>
#include <websocket/websocket.h>
#include <interfaces/json/JsonData_ORB.h>

using namespace WPEFramework::JsonData::ORB;
using namespace WPEFramework;

namespace orb
{
class ORBJsonRpcClient : public ORBGenericClient
{
public:

   /**
    * Constructor.
    */
   ORBJsonRpcClient(
      OnJavaScriptEventDispatchRequested_cb onJavaScriptEventDispatchRequested_cb,
      OnDvbUrlLoaded_cb onDvbUrlLoaded_cb,
      OnInputKeyGenerated_cb onInputKeyGenerated_cb
      );

   /**
    * Destructor.
    */
   virtual ~ORBJsonRpcClient();

public:

   // ORBGenericClient

   virtual std::string ExecuteBridgeRequest(std::string jsonRequest) override;
   virtual std::string CreateToken(std::string uri) override;
   virtual void LoadDvbUrl(std::string url, int requestId) override;
   virtual void NotifyApplicationLoadFailed(std::string url, std::string errorDescription) override;
   virtual void NotifyApplicationPageChanged(std::string url) override;

   virtual void SubscribeToJavaScriptEventDispatchRequestedEvent() override;
   virtual void SubscribeToDvbUrlLoadedEvent() override;
   virtual void SubscribeToInputKeyGeneratedEvent() override;

   virtual void UnsubscribeFromJavaScriptEventDispatchRequestedEvent() override;
   virtual void UnsubscribeFromDvbUrlLoadedEvent() override;
   virtual void UnsubscribeFromInputKeyGeneratedEvent() override;

private:

   // event handlers

   void JavaScriptEventDispatchRequested(const JsonObject& params);
   void DvbUrlLoaded(const JsonObject& params);
   void InputKeyGenerated(const Core::JSON::DecSInt32 keyCode);

   // member variables

   WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement> m_remoteObject;
   std::map<std::string, bool> m_subscribedEvents;
}; // class ORBJsonRpcClient
} // namespace orb
