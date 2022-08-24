/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

#pragma once

#include <core/core.h>

namespace orb {
/**
 * @brief orb::Keys
 *
 * Container of input key-related methods.
 */
class Keys {
public:

    /**
     * @brief Keys::ResolveKeyEvent
     *
     * Get the OIPF/HbbTV key event that corresponds to the given JavaScript key code.
     *
     * @param keyCode The JavaScript key code
     *
     * @return The corresponding OIPF/HbbTV key event code or 0
     */
    static uint16_t ResolveKeyEvent(uint16_t keyCode);
}; // class Keys
} // namespace orb
