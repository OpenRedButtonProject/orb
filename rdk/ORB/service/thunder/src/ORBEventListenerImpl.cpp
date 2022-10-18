/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

#include "ORBEventListenerImpl.h"
#include "ORB.h"
#include "ORBImplementation.h"

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
   fprintf(stderr, "[ORBEventListenerImpl::OnJavaScriptEventDispatchRequested] Calling COMRPC JavaScriptEventDispatchRequest - PID: %d\n", getpid());
   ORBImplementation::instance(nullptr)->JavaScriptEventDispatchRequest(
      eventName,
      eventProperties,
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
void ORBEventListenerImpl::OnDvbUrlLoaded(int requestId, unsigned char *content, unsigned int contentLength)
{
   fprintf(stderr, "[ORBEventListenerImpl::OnDvbUrlLoaded] Calling COMRPC OnDvbUrlLoaded - PID: %d\n", getpid());
   ORBImplementation::instance(nullptr)->DvbUrlLoaded(requestId, content, contentLength);
}

/**
 * Trigger the InputKeyGenerated event.
 *
 * @param keyCode The JavaScript key code
 */
void ORBEventListenerImpl::OnInputKeyGenerated(int keyCode)
{
   fprintf(stderr, "[ORBEventListenerImpl::OnInputKeyGenerated] Calling COMRPC OnInputKeyGenerated - PID: %d\n", getpid());
   ORBImplementation::instance(nullptr)->EventInputKeyGenerated(keyCode);
}
} // namespace orb