/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
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
