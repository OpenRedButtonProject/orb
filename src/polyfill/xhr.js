/**
 * @fileOverview XHR polyfill
 * @license ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

(function() {
    const _getResponseHeader = window.XMLHttpRequest.prototype.getResponseHeader;
    window.XMLHttpRequest.prototype.getResponseHeader = function(headerName) {
        return new URL(this.responseURL).protocol === 'dvb:' ?
            null :
            _getResponseHeader.call(this, headerName);
    };
    const _getAllResponseHeaders = window.XMLHttpRequest.prototype.getAllResponseHeaders;
    window.XMLHttpRequest.prototype.getAllResponseHeaders = function() {
        return new URL(this.responseURL).protocol === 'dvb:' ?
            '' :
            _getAllResponseHeaders.call(this);
    };
})();