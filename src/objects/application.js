/**
 * @fileOverview OIPF Application class
 * See: {@link https://web.archive.org/web/20200219165053/http://www.oipf.tv/web-spec/volume5.html#application-class}
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

hbbtv.objects.Application = (function() {
    const prototype = {};
    const privates = new WeakMap();

    prototype.createApplication = function(uri) {
        if (privates.get(this).disabled) {
            return null;
        }
        const url = new defaultEntities.URL(uri, document.location.href);
        if (hbbtv.bridge.manager.createApplication(url.href) === true) {
            return hbbtv.objects.createApplication({
                disabled: true,
            });
        }
        return null;
    };

    prototype.destroyApplication = function() {
        if (privates.get(this).disabled) {
            return;
        }
        hbbtv.bridge.manager.destroyApplication();
    };

    prototype.show = function() {
        if (privates.get(this).disabled) {
            return;
        }
        try {
            hbbtv.bridge.manager.showApplication();
        } catch (e) {
            if (e.message !== 'NotRunning') {
                throw e;
            }
        }
    };

    prototype.hide = function() {
        if (privates.get(this).disabled) {
            return;
        }
        try {
            hbbtv.bridge.manager.hideApplication();
        } catch (e) {
            if (e.message !== 'NotRunning') {
                throw e;
            }
        }
    };

    Object.defineProperty(prototype, 'privateData', {
        get() {
            return privates.get(this).privateData;
        },
    });

    function initialise(data) {
        privates.set(this, {});
        const p = privates.get(this);
        p.disabled = data.disabled;
        p.privateData = hbbtv.objects.createPrivateData({
            disabled: p.disabled,
        });
    }

    return {
        prototype: prototype,
        initialise: initialise,
    };
})();

hbbtv.objects.createApplication = function(data) {
    // Create new instance of hbbtv.objects.Application
    const application = Object.create(hbbtv.objects.Application.prototype);
    hbbtv.objects.Application.initialise.call(application, data);
    return application;
};
