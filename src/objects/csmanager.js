/**
 * @fileOverview application/hbbtvCSManager object
 * @license ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

hbbtv.objects.CSManager = (function() {
    const prototype = Object.create(HTMLObjectElement.prototype);

    prototype.discoverTerminals = function() {
        // unsupported
        return false;
    };

    prototype.getApp2AppLocalBaseURL = function() {
        return hbbtv.bridge.csManager.getApp2AppLocalBaseURL();
    };

    prototype.getInterDevSyncURL = function() {
        return hbbtv.bridge.csManager.getInterDevSyncURL();
    };

    prototype.getApp2AppRemoteBaseURL = function() {
        return hbbtv.bridge.csManager.getApp2AppRemoteBaseURL();
    };

    function initialise() {}

    return {
        prototype: prototype,
        initialise: initialise,
    };
})();

hbbtv.objects.upgradeToCSManager = function(object) {
    Object.setPrototypeOf(object, hbbtv.objects.CSManager.prototype);
    hbbtv.objects.CSManager.initialise.call(object);
};

hbbtv.objectManager.registerObject({
    name: 'application/hbbtvCSManager',
    mimeTypes: ['application/hbbtvcsmanager'],
    oipfObjectFactoryMethodName: 'createCSManager',
    upgradeObject: function(object) {
        hbbtv.objects.upgradeToCSManager(object);
    },
});