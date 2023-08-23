/**
 * @fileOverview BBC API polyfill
 * @license ORB Software. Copyright (c) 2023 Ocean Blue Software Limited
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

(function() {
    // Define/override window.getPrimaryDisplay()
    window.getPrimaryDisplay = function() {
        return hbbtv.bridge.configuration.getPrimaryDisplay();
    };
})();