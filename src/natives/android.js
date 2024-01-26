/**
 * @fileOverview Android native
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

hbbtv.native = {
    name: 'android',
    initialise: function() {
        this.token = Object.assign({}, document.token);
    },
    request: function(method, params) {
        const body = {
            token: this.token,
            method: method,
            params: params || {},
        };
        const responseText = androidBridge.request(JSON.stringify(body));
        if (typeof responseText !== 'string') {
            console.debug('Invalid response');
            return false;
        }
        const response = JSON.parse(responseText);
        if (response.error !== undefined) {
            if (response.error === 'SecurityError') {
                throw new DOMException('', 'SecurityError');
            }
            console.debug('Error response from ' + method + ': ' + response.error);
            return false;
        }
        //console.log("Response from " + method + ": " + JSON.stringify(response));
        return response;
    },
    setDispatchEventCallback: function(callback) {
        // TEMPORARY Dispatch an event and properties. TODO Replace me in events part 2!
        window.dispatchBridgeEvent = document.dispatchBridgeEvent = (type, properties) => {
            callback(type, properties);
            for (const iframe of document.getElementsByTagName("iframe")) {
                if (typeof(iframe.contentWindow.dispatchBridgeEvent) === "function") {
                    iframe.contentWindow.dispatchBridgeEvent(type, properties);
                }
            }
        };
    },
    isDebugBuild: function() {
        return true; // TODO Move
    },
    isFeatureEnabled(name) {
        if (name === 'dash-scheme') {
            return true;
        }
        return false;
    },
};