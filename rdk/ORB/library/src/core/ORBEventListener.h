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
#pragma once

#include <string>
#include <vector>

namespace orb
{
/**
 * Interface defining the events triggered by the ORB engine.
 */
class ORBEventListener
{
public:

    /**
     * Trigger the JavaScriptEventDispatchRequested event.
     *
     * @param eventName        The JavaScript event name
     * @param eventProperties  The JavaScript event properties
     * @param targetOrigin     The target origin
     * @param broadcastRelated Indicates whether the JavaScript event is broadcast-related or not
     */
    virtual void OnJavaScriptEventDispatchRequested(
        std::string eventName,
        std::string eventProperties,
        std::string targetOrigin,
        bool broadcastRelated
        ) = 0;

    /**
     * Trigger the DvbUrlLoaded event.
     *
     * @param requestId     The original request identifier
     * @param content       The retrieved content
     * @param contentLength The retrieved content length in number of bytes
     */
    virtual void OnDvbUrlLoaded(int requestId, std::vector<uint8_t> content, unsigned int
        contentLength) = 0;

    /**
     * Trigger the DvbUrlLoaded event.
     *
     * @param requestId     The original request identifier
     * @param contentLength The retrieved content length in number of bytes
     */
    virtual void OnDvbUrlLoadedNoData(int requestId, unsigned int contentLength) = 0;

    /**
     * Trigger the InputKeyGenerated event.
     *
     * @param keyCode   The JavaScript key code
     * @param keyAction The JavaScript key action (0 = keyup , 1 = keydown)
     */
    virtual void OnInputKeyGenerated(int keyCode, uint8_t keyAction) = 0;

    /**
     * Trigger the ExitButtonPressed event.
     */
    virtual void OnExitButtonPressed() = 0;
}; // class ORBEventListener
} // namespace orb
