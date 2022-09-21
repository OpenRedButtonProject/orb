/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

#include "ORBEventListenerImpl.h"
#include "ORB.h"

using namespace WPEFramework::Plugin;

namespace orb
{
ORBEventListenerImpl::ORBEventListenerImpl()
{
}

ORBEventListenerImpl::~ORBEventListenerImpl()
{
}

/**
 * Trigger the JavaScriptEventDispatchRequested event.
 *
 * @param eventName        The JavaScript event name
 * @param eventProperties  The JavaScript event properties
 * @param targetOrigin     The target origin
 * @param broadcastRelated Indicates whether the JavaScript event is broadcast-related or not
 */
void ORBEventListenerImpl::OnJavaScriptEventDispatchRequested(
   std::string eventName,
   std::string eventProperties,
   std::string targetOrigin,
   bool broadcastRelated
   )
{
   JsonObject properties;
   properties.FromString(eventProperties);

   ORB::instance(nullptr)->NotifyJavaScriptEventDispatchRequested(
      eventName,
      properties,
      broadcastRelated,
      targetOrigin
      );
}

/**
 * Trigger the DvbUrlLoaded event.
 *
 * @param requestId     The original request identifier
 * @param content       The retrieved content
 * @param contentLength The retrieved content length in number of bytes
 */
void ORBEventListenerImpl::OnDvbUrlLoaded(int requestId, unsigned short int *content, unsigned int contentLength)
{
   ORB::instance(nullptr)->NotifyDvbUrlLoaded(requestId, contentLength);
}

/**
 * Trigger the InputKeyGenerated event.
 *
 * @param keyCode The JavaScript key code
 */
void ORBEventListenerImpl::OnInputKeyGenerated(int keyCode)
{
   ORB::instance(nullptr)->NotifyInputKeyGenerated(keyCode);
}
} // namespace orb