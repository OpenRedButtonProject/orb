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
    const INVALID_APP_ID = 0;
    
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
        const id = privates.get(this).id;
        if (id in gApps) {
            const url = new defaultEntities.URL(uri, document.location.href);
            const appId = hbbtv.bridge.manager.createApplication(id, url.href, runAsOpApp || false);
            console.log("Application: creating application... url:", url.href, "id:", appId);
            if (appId !== INVALID_APP_ID) {
                return hbbtv.objects.createApplication({
                    id: appId
                });
            }
        }
        return null;
    };

    prototype.destroyApplication = function() {
        const id = privates.get(this).id;
        if (id in gApps) {
            hbbtv.bridge.manager.destroyApplication(id);
            delete gApps[p.url];
        }
    };

    prototype.show = function() {
        const id = privates.get(this).id;
        if (id in gApps) {
            try {
                hbbtv.bridge.manager.showApplication(id);
            } catch (e) {
                if (e.message !== 'NotRunning') {
                    throw e;
                }
            }
        }
    };

    prototype.hide = function() {
        const id = privates.get(this).id;
        if (id in gApps) {
            try {
                hbbtv.bridge.manager.hideApplication(id);
            } catch (e) {
                if (e.message !== 'NotRunning') {
                    throw e;
                }
            }
        }
    };

    prototype.opAppRequestTransient = function() {
        const id = privates.get(this).id;
        if (id in gApps) {
            // Allowed events in which their handler called this method from.
            // ETSI TS 103 606 V1.2.1 (2024-03) 6.3.3.4
            const allowedEvents = ["ChannelChangeSucceeded", "ChannelChangeError",
                "keydown", "keyup", "keypress", "click", "OperatorApplicationContextChange",
                "load", "PlayStateChange", "onProgrammesChanged", "ApplicationUnloaded",
                "onMessage"];
            if (hbbtv.objectManager.context.event?.type in allowedEvents)
            {
                return hbbtv.bridge.manager.opAppRequestTransient(id);
            }
        }
        return false;
    }

    prototype.opAppRequestForeground = function() {
        const id = privates.get(this).id;
        if (id in gApps) {
            // Allowed events in which their handler called this method from.
            // ETSI TS 103 606 V1.2.1 (2024-03) 6.3.3.2
            const allowedEvents = ["keydown", "keyup", "keypress", "click",
                "OperatorApplicationContextChange", "load", "PlayStateChange",
                "ApplicationUnloaded", "onMessage"];
            if (hbbtv.objectManager.context.event?.type in allowedEvents)
            {
                return hbbtv.bridge.manager.opAppRequestForeground(id);
            }
        }
        return false;
    }

    prototype.opAppRequestBackground = function() {
        const id = privates.get(this).id;
        if (id in gApps) {
            hbbtv.bridge.manager.opAppRequestBackground(id);
        }
    }

    prototype.opAppRequestUpdate = function(immediate, params) {
        const id = privates.get(this).id;
        if (id in gApps) {
            hbbtv.bridge.manager.opAppRequestBackground(id, immediate, params);
        }
    }

    prototype.opAppUpdateStatus = function() {
        const id = privates.get(this).id;
        if (id in gApps) {
            return hbbtv.bridge.manager.opAppUpdateStatus(id);
        }
        return -2; // ETSI TS 103 606 V1.2.1 (2024-03) page 128
    }

    prototype.opAppUninstall = function() {
        const id = privates.get(this).id;
        if (id in gApps) {
            return hbbtv.bridge.manager.opAppUninstall(id);
        }
        return false;
    }

    prototype.getPrivateLocalStorage = function() {
        // TODO: return Web Storage object
        return undefined;
    }

    prototype.getOpApp2AppBaseURL = function() {
        const id = privates.get(this).id;
        if (id in gApps) {
            return hbbtv.bridge.manager.getOpApp2AppBaseURL(id);
        }
        return null; // ETSI TS 103 606 V1.2.1 (2024-03) page 128
    }

    prototype.getApp2OpAppBaseURL = function() {
        const id = privates.get(this).id;
        if (id in gApps) {
            return hbbtv.bridge.manager.getApp2OpAppBaseURL(id);
        }
        return null; // ETSI TS 103 606 V1.2.1 (2024-03) page 129
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
            const id = privates.get(this).id; 
            if (id in gApps) {     
                return hbbtv.bridge.manager.getOpAppState(id);
            }
            return null; // ETSI TS 103 606 V1.2.1 (2024-03) page 125
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
            const app = Object.values(gApps).find(app => hbbtv.bridge.manager.getApplicationUrl(privates.get(app).id) === doc.documentURI);
            return app ? app : null;
        }
        catch(e) {
            console.error("Application:", e);
        }
        return null;
    }

    function initialise(data) {
        data.id = data.id || INVALID_APP_ID;
        privates.set(this, {
            id: data.id,
            eventDispatcher: new hbbtv.utils.EventDispatcher(this),
            privateData: hbbtv.objects.createPrivateData({
                disabled: data.id === INVALID_APP_ID,
            })
        });
        if (data.id !== INVALID_APP_ID)
        {
            addBridgeEventListeners.call(this);
            gApps[data.id] = this;
        }
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
