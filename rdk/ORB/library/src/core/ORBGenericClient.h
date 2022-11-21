/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */
#pragma once

#include <memory>
#include <string>
#include "ORBBrowserApi.h"

typedef void (*OnJavaScriptEventDispatchRequested_cb)(std::string name, std::string properties);
typedef void (*OnDvbUrlLoaded_cb)(int requestId, unsigned char *content, unsigned int contentLength);
typedef void (*OnInputKeyGenerated_cb)(int keyCode, unsigned char keyAction);

namespace orb
{
/**
 * Interface of the ORB client.
 */
class ORBGenericClient : public ORBBrowserApi
{
public:

   ORBGenericClient(
      OnJavaScriptEventDispatchRequested_cb onJavaScriptEventDispatchRequested_cb,
      OnDvbUrlLoaded_cb onDvbUrlLoaded_cb,
      OnInputKeyGenerated_cb onInputKeyGenerated_cb
      )
   {
      m_onJavaScriptEventDispatchRequested = onJavaScriptEventDispatchRequested_cb;
      m_onDvbUrlLoaded = onDvbUrlLoaded_cb;
      m_onInputKeyGenerated = onInputKeyGenerated_cb;
   }

   virtual ~ORBGenericClient()
   {
   }

public:

   // ORBBrowserApi

   virtual std::string ExecuteBridgeRequest(std::string jsonRequest) = 0;
   virtual std::string CreateToken(std::string uri) = 0;
   virtual void LoadDvbUrl(std::string url, int requestId) = 0;
   virtual void NotifyApplicationLoadFailed(std::string url, std::string errorDescription) = 0;
   virtual void NotifyApplicationPageChanged(std::string url) = 0;

public:
   // Events subscription
   virtual void SubscribeToJavaScriptEventDispatchRequestedEvent() = 0;
   virtual void SubscribeToDvbUrlLoadedEvent() = 0;
   virtual void SubscribeToInputKeyGeneratedEvent() = 0;

   virtual void UnsubscribeFromJavaScriptEventDispatchRequestedEvent() = 0;
   virtual void UnsubscribeFromDvbUrlLoadedEvent() = 0;
   virtual void UnsubscribeFromInputKeyGeneratedEvent() = 0;

protected:
   // callbacks
   OnJavaScriptEventDispatchRequested_cb m_onJavaScriptEventDispatchRequested;
   OnDvbUrlLoaded_cb m_onDvbUrlLoaded;
   OnInputKeyGenerated_cb m_onInputKeyGenerated;
}; // class ORBGenericClient


/**
 * Create a new ORB client instance.
 *
 * @param onJavaScriptEventDispatchRequested_cb The OnJavaScriptEventDispatchRequested callback
 * @param onDvbUrlLoaded_cb                     The OnDvbUrlLoaded callback
 * @param onInputKeyGenerated_cb                The OnInputKeyGenerated callback
 *
 * @return Pointer to the new ORB client instance
 */
std::shared_ptr<ORBGenericClient> CreateORBClient(
   OnJavaScriptEventDispatchRequested_cb onJavaScriptEventDispatchRequested_cb,
   OnDvbUrlLoaded_cb onDvbUrlLoaded_cb,
   OnInputKeyGenerated_cb onInputKeyGenerated_cb
   );
} // namespace orb
