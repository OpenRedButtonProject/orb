/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
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
