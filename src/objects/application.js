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
    const LINKED_APP_SCHEME_1_2 = "urn:dvb:metadata:cs:LinkedApplicationCS:2019:1.2";
    const LINKED_APP_SCHEME_2 = "urn:dvb:metadata:cs:LinkedApplicationCS:2019:2";
    const prototype = {};
    const privates = new WeakMap();
    const gGarbageCollectionBlocked = new Set();
    
    // store created apps by their url as key
    const gApps = {};
    const gEvents = [
        'OperatorApplicationStateChange',
        'OperatorApplicationStateChangeCompleted',
        'OperatorApplicationContextChange',
        'OpAppUpdate'
    ];

    // define intrinsic events
    for (const event of gEvents) {
        var onEvent = 'on' + event;
        Object.defineProperty(prototype, onEvent, {
            set(value) {
                const p = privates.get(this);
                if (p[onEvent]) {
                    this.removeEventListener(event, p[onEvent]);
                }
                p[onEvent] = value;
                if (value) {
                    this.addEventListener(event, p[onEvent]);
                }
            },
            get() {
                return privates.get(this)[onEvent];
            },
        });
    }

    prototype.createApplication = function(uri, createChild, runAsOpApp) {
        if (privates.get(this).disabled) {
            return null;
        }
        const url = new defaultEntities.URL(uri, document.location.href);
        if (hbbtv.bridge.manager.createApplication(url.href, runAsOpApp) === true) {
            return hbbtv.objects.createApplication({
                url: url.href,
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
        delete gApps[p.url];
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

    prototype.opAppRequestTransient = function() {
        // Allowed events in which their handler called this method from.
        // ETSI TS 103 606 V1.2.1 (2024-03) 6.3.3.4
        const allowedEvents = ["ChannelChangeSucceeded", "ChannelChangeError",
            "keydown", "keyup", "keypress", "click", "OperatorApplicationContextChange",
            "load", "PlayStateChange", "onProgrammesChanged", "ApplicationUnloaded",
            "onMessage"];
        if (hbbtv.objectManager.context.event?.type in allowedEvents)
        {
            return hbbtv.bridge.manager.opAppRequestTransient();
        }
        return false;
    }

    prototype.opAppRequestForeground = function() {
        // Allowed events in which their handler called this method from.
        // ETSI TS 103 606 V1.2.1 (2024-03) 6.3.3.2
        const allowedEvents = ["keydown", "keyup", "keypress", "click",
            "OperatorApplicationContextChange", "load", "PlayStateChange",
            "ApplicationUnloaded", "onMessage"];
        if (hbbtv.objectManager.context.event?.type in allowedEvents)
        {
            return hbbtv.bridge.manager.opAppRequestForeground();
        }
        return false;
    }

    prototype.opAppRequestBackground = function() {
        hbbtv.bridge.manager.opAppRequestBackground();
    }

    prototype.opAppRequestUpdate = function(immediate, params) {
        hbbtv.bridge.manager.opAppRequestBackground(immediate, params);
    }

    prototype.opAppUpdateStatus = function() {
        return hbbtv.bridge.manager.opAppUpdateStatus();
    }

    prototype.opAppUninstall = function() {
        return hbbtv.bridge.manager.opAppUninstall();
    }

    prototype.getPrivateLocalStorage = function() {
        // TODO: return Web Storage object
        return undefined;
    }

    prototype.getOpApp2AppBaseURL = function() {
        return hbbtv.bridge.manager.getOpApp2AppBaseURL();
    }

    prototype.getApp2OpAppBaseURL = function() {
        return hbbtv.bridge.manager.getApp2OpAppBaseURL();
    }

    prototype.addEventListener = function(type, listener) {
        if (privates.get(this).eventDispatcher.addCountedEventListener(type, listener) > 0) {
            gGarbageCollectionBlocked.add(this);
        }
    };

    prototype.removeEventListener = function(type, listener) {
        if (privates.get(this).eventDispatcher.removeCountedEventListener(type, listener) == 0) {
            gGarbageCollectionBlocked.delete(this);
        }
    };

    Object.defineProperty(prototype, 'privateData', {
        get() {
            return privates.get(this).privateData;
        },
    });

    Object.defineProperty(prototype, 'opAppState', {
        get() {            
            return hbbtv.bridge.manager.getOpAppState();
        }
    });

    // Internal listeners
    function addBridgeEventListeners() {
        var p = privates.get(this);
        p.onApplicationSchemeUpdated = (event) => {
            console.log('onApplicationSchemeUpdated', event.scheme);
            const currentURL = new URL(window.location.href);
            switch (event.scheme) {
                case LINKED_APP_SCHEME_1_2:
                    currentURL.searchParams.set("lloc", "service");
                    break;
                case LINKED_APP_SCHEME_2:
                    currentURL.searchParams.set("lloc", "availability");
                    break;
                default:
                    return;
            }
            window.history.replaceState(null, null, currentURL);
        };
        hbbtv.bridge.addWeakEventListener('ApplicationSchemeUpdated', p.onApplicationSchemeUpdated);
    }

    function getOwnerApplication(doc) {
        try {
            console.log("Application: Request owner application with url:", doc.documentURI);
            if (doc.documentURI in gApps) {
                return gApps[doc.documentURI];
            }
            // TODO: Applications should be populated by the application manager
            // upon its construction. Move the following to the application manager
            // initialization when the logic of retrieving running applications
            // from native code is done.
            return hbbtv.objects.createApplication({
                disabled: false,
                url: doc.documentURI
            });
        }
        catch(e) {
            console.error("Application:", e);
        }
        return null;
    }

    function initialise(data) {
        privates.set(this, {
            id: data.id,
            url: data.url,
            disabled: data.disabled,
            eventDispatcher: new hbbtv.utils.EventDispatcher(this),
            privateData: hbbtv.objects.createPrivateData({
                disabled: data.disabled,
            })
        });
        addBridgeEventListeners.call(this);
        gApps[data.url] = this;
    }

    return {
        prototype: prototype,
        initialise: initialise,
        getOwnerApplication: getOwnerApplication,
    };
})();

hbbtv.objects.createApplication = function(data) {
    // Create new instance of hbbtv.objects.Application
    const application = Object.create(hbbtv.objects.Application.prototype);
    hbbtv.objects.Application.initialise.call(application, data);
    return application;
};
