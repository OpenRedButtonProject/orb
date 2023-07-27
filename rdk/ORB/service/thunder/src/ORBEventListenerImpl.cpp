/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

#include "ORBEventListenerImpl.h"
#include "ORB.h"
#include "ORBImplementation.h"
#include "ORBLogging.h"

using namespace WPEFramework::Plugin;

namespace orb
{
/**
 * Constructor.
 */
ORBEventListenerImpl::ORBEventListenerImpl()
{
}

/**
 * Destructor.
 */
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
    ORB_LOG("PID=%d", getpid());
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
void ORBEventListenerImpl::OnDvbUrlLoaded(int requestId, std::vector<uint8_t> content, unsigned int
    contentLength)
{
    ORB_LOG("PID=%d", getpid());
    ORBImplementation::instance(nullptr)->DvbUrlLoaded(requestId, &content[0], contentLength);
}

/**
 * Trigger the DvbUrlLoadedNoData event.
 *
 * @param requestId     The original request identifier
 * @param contentLength The retrieved content length in number of bytes
 */
void ORBEventListenerImpl::OnDvbUrlLoadedNoData(int requestId, unsigned int contentLength)
{
    ORB_LOG("PID=%d", getpid());
    ORBImplementation::instance(nullptr)->DvbUrlLoadedNoData(requestId, contentLength);
}

/**
 * Trigger the InputKeyGenerated event.
 *
 * @param keyCode The JavaScript key code
 */
void ORBEventListenerImpl::OnInputKeyGenerated(int keyCode, uint8_t keyAction)
{
    ORB_LOG("PID=%d", getpid());
    ORBImplementation::instance(nullptr)->EventInputKeyGenerated(keyCode, keyAction);
}

/**
 * Trigger the ExitButtonPressed event.
 */
void ORBEventListenerImpl::OnExitButtonPressed()
{
    ORB_LOG_NO_ARGS();
    ORBImplementation::instance(nullptr)->ExitButtonPressed();
}
} // namespace orb
