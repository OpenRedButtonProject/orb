// RDK/Linux native
hbbtv.native = {
    name: 'rdk',
    initialise: function() {
        this.token = Object.assign({}, document.token);
    },
    request: function(method, params) {
        const body = {
            token: this.token,
            method: method,
            params: params || {},
        };
        const responseText = wpeBridge.request(JSON.stringify(body));
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
        document.dispatchBridgeEvent = (type, properties) => {
            callback(type, properties);
        };
    },
    isDebugBuild: function() {
        return true; // TODO Move up. Note: This needs to be enabled to disable dash-scheme
    },
    isFeatureEnabled(name) {
        if (name === 'dash-scheme') {
            return false;
        }
        return false;
    },
};