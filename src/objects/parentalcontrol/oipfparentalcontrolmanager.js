/**
 * @fileOverview OIPF application/oipfParentalControlManager object
 * See: {@link https://web.archive.org/web/20200219165053/http://www.oipf.tv/web-spec/volume5.html#application-oipfparentalcontrolmanager}
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

hbbtv.objects.OipfParentalControlManager = (function() {
    const prototype = Object.create(HTMLObjectElement.prototype);
    const privates = new WeakMap();

    Object.defineProperty(prototype, 'parentalRatingSchemes', {
        get() {
            return privates.get(this).parentalRatingSchemes;
        },
    });

    function initialise() {
        privates.set(this, {});
        const p = privates.get(this);
        p.parentalRatingSchemes = hbbtv.objects.createParentalRatingSchemeCollection();
    }

    return {
        prototype: prototype,
        initialise: initialise,
    };
})();

hbbtv.objects.upgradeToOipfParentalControlManager = function(object) {
    Object.setPrototypeOf(object, hbbtv.objects.OipfParentalControlManager.prototype);
    hbbtv.objects.OipfParentalControlManager.initialise.call(object);
};

hbbtv.objectManager.registerObject({
    name: 'application/oipfparentalcontrolmanager',
    mimeTypes: ['application/oipfparentalcontrolmanager'],
    oipfObjectFactoryMethodName: 'createParentalControlManagerObject',
    upgradeObject: function(object) {
        hbbtv.objects.upgradeToOipfParentalControlManager(object);
    },
});