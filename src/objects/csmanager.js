/**
 * @fileOverview application/hbbtvCSManager object
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