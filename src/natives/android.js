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
        const FUNCTION_CALL_TYPE = "orb_functionCall";
        // TEMPORARY Dispatch an event and properties. TODO Replace me in events part 2!
        document.dispatchBridgeEvent = (type, properties) => {
            callback(type, properties);
            for (const iframe of document.getElementsByTagName("iframe")) {
                iframe.contentWindow.postMessage(JSON.stringify({
                    type: FUNCTION_CALL_TYPE,
                    name: "dispatchBridgeEvent",
                    args: [type, properties]
                }), '*');
            }
        };
        window.addEventListener("message", (e) => {
            try {
                const msg = JSON.parse(e.data);
                if (msg.type === FUNCTION_CALL_TYPE && msg.name === "dispatchBridgeEvent") {
                    document.dispatchBridgeEvent(msg.args[0], msg.args[1]);
                }
            } catch (error) {
                console.warn('native: error parsing message data:', e.data);
            }
        })
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