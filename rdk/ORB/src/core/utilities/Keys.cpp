/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

#include "Keys.h"

#define HBBTV_CODE_RED        0x1
#define HBBTV_CODE_GREEN      0x2
#define HBBTV_CODE_YELLOW     0x4
#define HBBTV_CODE_BLUE       0x8
#define HBBTV_CODE_NAVIGATION 0x10
#define HBBTV_CODE_VCR        0x20
#define HBBTV_CODE_SCROLL     0x40
#define HBBTV_CODE_INFO       0x80
#define HBBTV_CODE_NUMERIC    0x100
#define HBBTV_CODE_ALPHA      0x200
#define HBBTV_CODE_OTHER      0x400

#define VK_YELLOW     403
#define VK_BLUE       404
#define VK_RED        405
#define VK_GREEN      406
#define VK_BACK       8
#define VK_ENTER      13
#define VK_UP         38
#define VK_LEFT       37
#define VK_RIGHT      39
#define VK_DOWN       40
#define VK_STOP       3
#define VK_PLAY_PAUSE 179
#define VK_REWIND     227
#define VK_FAST_FWD   228
#define VK_PAGE_UP    33
#define VK_PAGE_DOWN  34
#define VK_ZERO       48
#define VK_NINE       57
#define VK_A          65
#define VK_Z          90

namespace orb {
/**
 * @brief Keys::ResolveKeyEvent
 *
 * Get the OIPF/HbbTV key event that corresponds to the given JavaScript key code.
 *
 * @param keyCode The JavaScript key code
 *
 * @return The corresponding OIPF/HbbTV key event code or 0
 */
uint16_t Keys::ResolveKeyEvent(uint16_t keyCode)
{
    uint16_t keyEventCode = 0;

    if (keyCode >= VK_LEFT && keyCode <= VK_DOWN)
    {
        keyEventCode = HBBTV_CODE_NAVIGATION;
    }
    else if (keyCode >= VK_PAGE_UP && keyCode <= VK_PAGE_DOWN)
    {
        keyEventCode = HBBTV_CODE_SCROLL;
    }
    else if (keyCode >= VK_ZERO && keyCode <= VK_NINE)
    {
        keyEventCode = HBBTV_CODE_NUMERIC;
    }
    else if (keyCode >= VK_A && keyCode <= VK_Z)
    {
        keyEventCode = HBBTV_CODE_ALPHA;
    }
    else
    {
        switch (keyCode)
        {
            case VK_STOP:
            case VK_FAST_FWD:
            case VK_REWIND:
            case VK_PLAY_PAUSE:
                keyEventCode = HBBTV_CODE_VCR;
                break;
            case VK_ENTER:
            case VK_BACK:
                keyEventCode = HBBTV_CODE_NAVIGATION;
                break;
            case VK_RED:
                keyEventCode = HBBTV_CODE_RED;
                break;
            case VK_GREEN:
                keyEventCode = HBBTV_CODE_GREEN;
                break;
            case VK_YELLOW:
                keyEventCode = HBBTV_CODE_YELLOW;
                break;
            case VK_BLUE:
                keyEventCode = HBBTV_CODE_BLUE;
                break;
            default:
                keyEventCode = HBBTV_CODE_OTHER;
                break;
        }
    }

    return keyEventCode;
}
} // namespace orb
