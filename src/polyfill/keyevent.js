/**
 * @fileOverview KeyEvent polyfill
 * @license ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
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
        VK_PLAY_PAUSE: 402,
        VK_FAST_FWD: 417,
        VK_REWIND: 412,
        VK_RECORD: 416,
    };
    for (const key in keys) {
        if (!window.KeyEvent.hasOwnProperty(key)) {
            Object.defineProperty(window.KeyEvent, key, {
                value: keys[key],
                writable: false,
                enumerable: true,
            });
        }
    }

    const keyLabels = {
        VK_BACK: "BrowserBack",
        VK_EXIT: "Exit",
        VK_CHANNELS: "ChannelList",
        VK_PLAY_PAUSE: "MediaPlayPause",
        VK_RED: "ColorF0Red",
        VK_GREEN: "ColorF1Green",
        VK_YELLOW: "ColorF2Yellow",
        VK_BLUE: "ColorF3Blue",
        VK_STOP: "MediaStop",
        VK_PLAY: "MediaPlay",
        VK_PAUSE: "MediaPause",
        VK_FAST_FWD: "MediaFastForward",
        VK_REWIND: "MediaRewind",
        VK_RECORD: "MediaRecord",
        VK_0: "Digit0",
        VK_1: "Digit1",
        VK_2: "Digit2",
        VK_3: "Digit3",
        VK_4: "Digit4",
        VK_5: "Digit5",
        VK_6: "Digit6",
        VK_7: "Digit7",
        VK_8: "Digit8",
        VK_9: "Digit9",
        VK_CHANNEL_UP: "ChannelUp",
        VK_CHANNEL_DOWN: "ChannelDown",
        VK_SUBTITLE: "Subtitle",
        VK_GUIDE: "Guide",
        VK_CHANNELS: "ChannelList",
        VK_INFO: "Info",
        VK_LEFT: "ArrowLeft",
        VK_RIGHT: "ArrowRight",
        VK_UP: "ArrowUp",
        VK_DOWN: "ArrowDown",
        VK_ENTER: "Enter",
        VK_MUTE: "AudioVolumeMute",
        VK_VOLUME_UP: "AudioVolumeUp",
        VK_VOLUME_DOWN: "AudioVolumeDown",
    };

    function keyHandler(originalEvent) {
        if(originalEvent.code === "") {
            originalEvent.preventDefault();
            originalEvent.stopImmediatePropagation();
            const keyLabel = Object.keys(keys).find(key => keys[key] === originalEvent.keyCode);
            const newEvent = new KeyboardEvent(originalEvent.type, {
                keyCode: originalEvent.keyCode,
                code: keyLabels[keyLabel] || "0",
                bubbles: true
            });

            window.dispatchEvent(newEvent);
        }
    }

    window.addEventListener('keydown', keyHandler);
    window.addEventListener('keyup', keyHandler);
    window.addEventListener('keypress', keyHandler);
})();