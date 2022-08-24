/**
 * @fileOverview KeyEvent polyfill
 * @license ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

(function() {
    // Create or extend KeyEvent object
    if (typeof window.KeyEvent === 'undefined') {
        window.KeyEvent = {};
    }
    const keys = {
        VK_RED: 403,
        VK_GREEN: 404,
        VK_YELLOW: 405,
        VK_BLUE: 406,
        VK_LEFT: 37,
        VK_UP: 38,
        VK_RIGHT: 39,
        VK_DOWN: 40,
        VK_ENTER: 13,
        VK_BACK: 461,
        VK_0: 48,
        VK_1: 49,
        VK_2: 50,
        VK_3: 51,
        VK_4: 52,
        VK_5: 53,
        VK_6: 54,
        VK_7: 55,
        VK_8: 56,
        VK_9: 57,
        VK_STOP: 413,
        VK_PLAY: 415,
        VK_PAUSE: 19,
        VK_FAST_FWD: 417,
        VK_REWIND: 412,
    };
    for (const key in keys) {
        if (!window.KeyEvent.hasOwnProperty(key)) {
            Object.defineProperty(window.KeyEvent, key, {
                value: keys[key],
                writable: false,
            });
        }
    }
})();