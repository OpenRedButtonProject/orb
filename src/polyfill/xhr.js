/**
 * @fileOverview XHR polyfill
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
    const _getResponseHeader = window.XMLHttpRequest.prototype.getResponseHeader;
    window.XMLHttpRequest.prototype.getResponseHeader = function(headerName) {
        return new __URL(this.responseURL).protocol === 'dvb:' ?
            null :
            _getResponseHeader.call(this, headerName);
    };
    const _getAllResponseHeaders = window.XMLHttpRequest.prototype.getAllResponseHeaders;
    window.XMLHttpRequest.prototype.getAllResponseHeaders = function() {
        return new __URL(this.responseURL).protocol === 'dvb:' ?
            '' :
            _getAllResponseHeaders.call(this);
    };

    if (hbbtv.native.name === "rdk") {
        const _open = window.XMLHttpRequest.prototype.open;
        window.XMLHttpRequest.prototype.open = function(method, url, async, user, password) {
            const baseUrl = (window.document.getElementsByTagName('base')[0] ?? document.createElement('base')).href;
            if(baseUrl.length > 0) {
                if(url.startsWith("dvb:/") && !url.startsWith("dvb://")) {
                    let newbase;
                    const dvbTag="dvb:";
                    if(baseUrl.startsWith("dvb://")) {
                        newbase = baseUrl.substring(0, baseUrl.lastIndexOf("/"));
                        url = newbase + url.substring(dvbTag.length);
                    } else {
                        const dvburlTag = "dvburl=";
                        const urlIdx = baseUrl.lastIndexOf(dvburlTag);
                        if(urlIdx) {
                            newbase = baseUrl.substring(urlIdx + dvburlTag.length);
                            url = newbase + url.substring(dvbTag.length);
                        }
                    }
                }
            }
            return _open.call(this,method,url,async,user,password);
        };
    }
})();
