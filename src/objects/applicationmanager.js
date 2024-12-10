/**
 * @fileOverview OIPF application manager object.
 * See: {@link https://web.archive.org/web/20200219165053/http://www.oipf.tv/web-spec/volume5.html#application-oipfApplicationManager}
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

hbbtv.objects.ApplicationManager = (function() {
    // Create new instance that extends HTMLObjectElement
    const prototype = Object.create(HTMLObjectElement.prototype);
    const privates = new WeakMap();
    const gGarbageCollectionBlocked = new Set();

    prototype.getOwnerApplication = function(page) {
        return hbbtv.objects.Application.getOwnerApplication(page);
    };

    // DOM level 1 event methods
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

    // DOM level 0 event properties
    Object.defineProperty(prototype, 'onLowMemory', {
        get() {
            return privates.get(this).onLowMemoryDomLevel0;
        },
        set(listener) {
            const p = privates.get(this);
            if (p.onLowMemoryDomLevel0) {
                this.removeEventListener('LowMemory', p.onLowMemoryDomLevel0);
            }
            p.onLowMemoryDomLevel0 = listener;
            if (listener) {
                this.addEventListener('LowMemory', p.onLowMemoryDomLevel0);
            }
        },
    });

    Object.defineProperty(prototype, 'onApplicationLoadError', {
        get() {
            return privates.get(this).onApplicationLoadErrorDomLevel0;
        },
        set(listener) {
            const p = privates.get(this);
            if (p.onApplicationLoadErrorDomLevel0) {
                this.removeEventListener('ApplicationLoadError', p.onApplicationLoadErrorDomLevel0);
            }
            p.onApplicationLoadErrorDomLevel0 = listener;
            if (listener) {
                this.addEventListener('ApplicationLoadError', p.onApplicationLoadErrorDomLevel0);
            }
        },
    });

    // Internal listeners
    function addBridgeEventListeners() {
        const p = privates.get(this);
        p.onLowMemory = (event) => {
            console.log('The system is running low on memory!');
            const lowMemoryevent = new Event('LowMemory');
            privates.get(this).eventDispatcher.dispatchEvent(lowMemoryevent);
        };
        hbbtv.bridge.addWeakEventListener('LowMemory', p.onLowMemory);

        p.onApplicationLoadError = (event) => {
            console.log('Application load error!');
            const ev = new Event('ApplicationLoadError');
            // TODO: assign the actual application that triggered the event (by its id maybe?)
            Object.assign(ev, {
                appl: hbbtv.objects.createApplication({
                    disabled: true,
                }),
            });
            privates.get(this).eventDispatcher.dispatchEvent(ev);
        };
        hbbtv.bridge.addWeakEventListener('ApplicationLoadError', p.onApplicationLoadError);
    }

    function initialise() {
        privates.set(this, {});
        const p = privates.get(this);
        p.eventDispatcher = new hbbtv.utils.EventDispatcher(this);
        addBridgeEventListeners.call(this);
    }

    return {
        prototype: prototype,
        initialise: initialise,
    };
})();

hbbtv.objects.upgradeToApplicationManager = function(object) {
    Object.setPrototypeOf(object, hbbtv.objects.ApplicationManager.prototype);
    hbbtv.objects.ApplicationManager.initialise.call(object);
};

hbbtv.objectManager.registerObject({
    name: 'application/oipfapplicationmanager',
    mimeTypes: ['application/oipfapplicationmanager'],
    oipfObjectFactoryMethodName: 'createApplicationManagerObject',
    upgradeObject: function(object) {
        hbbtv.objects.upgradeToApplicationManager(object);
    },
});
