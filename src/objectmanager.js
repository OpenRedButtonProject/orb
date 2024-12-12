/**
 * @fileOverview Monitors the document and certain JavaScript mechanisms for objects. Builds or
 * upgrades those objects if there is a registered handler.
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

hbbtv.objects = {};

hbbtv.objectManager = (function() {
    const __createElement = document.createElement;
    const __getElementById = document.getElementById;
    
    // general-purpose storage for information during the application's runtime. 
    // It can be used to hold flags, intermediate results, or contextual data 
    // required by specific functions or processes.
    const __context = {};

    let objectMimeTypeTable = [];
    let objectFactoryMethodTable = [];
    let objectUpgradeHandlers = [];

    // Keep references to the original add/removeEventListener methods
    const _addEventListener = EventTarget.prototype.addEventListener;
    const _removeEventListener = EventTarget.prototype.removeEventListener;

    const callbackWrappers = new WeakMap();

    EventTarget.prototype.addEventListener = function (type, listener, options) {
        // if listener is not a function, let the browser handle it
        if (typeof listener !== "function") {
            return _addEventListener.call(this, type, listener, options);
        }
    
        // wrapper for the incoming callback to update event context
        const wrapper = function (...args) {
            __context.event = { type, args };
            listener.call(this, ...args);
            __context.event = undefined;
        };

        // First call the original addEventListener and pass the wrapper as handler.
        // We call it first in case some exception is thrown.
        _addEventListener.call(this, type, wrapper, options);
    
        // get a reference to the events objects that hold information
        // per event
        let events = callbackWrappers.get(this);
        if (!events) {
            // if this is the first time addEventListener is called on
            // this object, create an entry in the WeakMap with it as key
            events = {};
            callbackWrappers.set(this, events);
        }

        if (!events[type]) {
            // if this is the first time addEventListener is called on
            // this object for type, create a new Map entry with type as key.
            events[type] = new Map();
        }
    
        // create an array of objects that will store the wrappers
        // and the options for the combination of type/listener
        const listenerInfo = events[type].get(listener) || [];
        listenerInfo.push({
            wrapper,
            options: options || {}
        });

        // create an entry in the events[type] Map with the listener as key
        events[type].set(listener, listenerInfo);

        console.log(`[ObjectManager]: Called addEventListener("${type}, ${listener.name}, ${options})`);
    };
    
    EventTarget.prototype.removeEventListener = function (type, listener, options) {
        const events = callbackWrappers.get(this);

        if (events?.[type]?.has(listener)) {
            const listenerInfo = events[type].get(listener);
            const opt = options || {};

            // find the index in the added listeners for event type that
            // matches the listener and options parameters
            const index = listenerInfo.findIndex((info) => {
                const infoOptions = info.options;
                if (Object.keys(opt).length !== Object.keys(infoOptions).length) {
                    return false;
                }
                for (const key in opt) {
                    if (opt[key] !== infoOptions[key]) {
                        return false;
                    }
                }
                return true;
            });
    
            if (index !== -1) {
                // if the index is found, call the original removeEventListener
                // with the wrapper created when addEventListener was called as
                // argument
                _removeEventListener.call(this, type, listenerInfo[index].wrapper, options);

                // clean up
                listenerInfo.splice(index, 1);
                if (listenerInfo.length === 0) {
                    events[type].delete(listener);
                    if (events[type].size === 0) {
                        delete events[type];
                    }
                }
            }
            console.log(`[ObjectManager]: Called removeEventListener("${type}, ${listener.name}, ${options})`);
        } else {
            _removeEventListener.call(this, type, listener, options);
        }
    };
    

    function initialise() {
        // Override getElementById while app is loading to upgrade HbbTV objects before they are used.
        // This is a WORKAROUND for apps that incorrectly use 'getElementById' before 'window.onload'.
        document.getElementById = getElementByIdOverride;

        addMetaViewportElement();
        addOipfObjectFactory();
        addHTMLManipulationIntercept(function(manipulatedElement) {
            upgradeDescendantObjects(manipulatedElement);
        });
        addCreateElementTypeIntercept(function(object, type) {
            upgradeObject(object, type);
        });
        addMutationIntercept(
            function(addedObject) {
                hbbtv.holePuncher.notifyObjectAddedToDocument(addedObject);
                upgradeObject(addedObject, addedObject.getAttribute('type'));
            },
            function(removedObject) {
                hbbtv.holePuncher.notifyObjectRemovedFromDocument(removedObject);
            }
        );
        upgradeDescendantObjects(document);
        document.addEventListener('DOMContentLoaded', function() {
            if (document.getElementById === getElementByIdOverride) {
                document.getElementById = __getElementById;
            }
            upgradeDescendantObjects(document);
        });
    }

    function registerObject(options) {
        //console.debug("Register object " + options.name + " with mime types " + options.mimeTypes);
        for (const mimeType of options.mimeTypes) {
            objectMimeTypeTable[mimeType] = options.name;
        }
        if (options.oipfObjectFactoryMethodName.length > 0) {
            objectFactoryMethodTable[options.oipfObjectFactoryMethodName] = options.name;
        }
        objectUpgradeHandlers[options.name] = options.upgradeObject;
    }

    function resolveMimeType(mimeType) {
        if (mimeType in objectMimeTypeTable) {
            return objectMimeTypeTable[mimeType];
        }
        console.debug('Could not resolve mime type ' + mimeType);
        return undefined;
    }

    function upgradeDescendantObjects(target) {
        for (const object of target.getElementsByTagName('object')) {
            if (object.hasAttribute('type')) {
                upgradeObject(object, object.getAttribute('type'));
            }
        }
    }

    function upgradeObject(object, mimeType) {
        if (!mimeType) {
            return;
        }
        mimeType = mimeType.toLowerCase();
        if (object.hasAttribute('__mimeType') && object.getAttribute('__mimeType') === mimeType) {
            // Already done
            return;
        }
        const objectName = resolveMimeType(mimeType);
        if (objectName !== undefined) {
            let mimeTypeChanged = false;
            if (object.hasAttribute('__objectType')) {
                if (object.getAttribute('__objectType') !== objectName) {
                    // Ignore changes to the object type (but not the MIME type) [OIPF 5 4.4.4]
                    return;
                }
                mimeTypeChanged = true;
            }
            object.setAttribute('__mimeType', mimeType);
            object.setAttribute('__objectType', objectName);
            if (mimeTypeChanged) {
                // TODO
            } else {
                objectUpgradeHandlers[objectName](object);
            }
        }
    }

    // Mutation observer
    function addMutationIntercept(callbackObjectAdded, callbackObjectRemoved) {
        const observer = new MutationObserver(function(mutationsList) {
            for (const mutation of mutationsList) {
                if (mutation.type === 'childList') {
                    for (const node of mutation.addedNodes) {
                        if (node.nodeName && node.nodeName.toLowerCase() === 'object') {
                            callbackObjectAdded(node);
                        }
                    }
                    for (const node of mutation.removedNodes) {
                        if (node.nodeName && node.nodeName.toLowerCase() === 'object') {
                            callbackObjectRemoved(node);
                        }
                    }
                }
            }
        });
        const config = {
            childList: true,
            subtree: true,
        };
        observer.observe(document.documentElement || document.body, config);
    }

    // Override createElement install a proxy to monitor and intercept objects. Needed for the
    // case where a page script creates and uses an object before adding it to the document.
    function addCreateElementTypeIntercept(callbackTypeSet) {
        const ownProperty = Object.getOwnPropertyDescriptor(HTMLObjectElement.prototype, 'type');
        Object.defineProperty(HTMLObjectElement.prototype, 'type', {
            set(val) {
                this.setAttribute('type', val);
            },
            get() {
                return ownProperty.get.call(this);
            },
        });

        HTMLObjectElement.prototype.setAttribute = function(name, value) {
            if (name === 'type') {
                callbackTypeSet(this, value);
            }
            Element.prototype.setAttribute.apply(this, arguments);
        };

        document.createElement = function(tagname, options) {
            let element = __createElement.apply(document, arguments);
            return element;
        };
    }

    // Intercept direct HTML manipulation. Needed for the (possibly esoteric) case where a page
    // script directly modifies the HTML to create an object and immediately uses it (i.e. before
    // the event loop processes mutation events).
    function addHTMLManipulationIntercept(interceptCallback) {
        Object.defineProperty(HTMLElement.prototype, 'innerHTML', {
            set: function(val) {
                const ownProperty = Object.getOwnPropertyDescriptor(Element.prototype, 'innerHTML');
                ownProperty.set.call(this, val);
                interceptCallback(this);
            },
            get: function() {
                const ownProperty = Object.getOwnPropertyDescriptor(Element.prototype, 'innerHTML');
                return ownProperty.get.call(this);
            },
        });
        Object.defineProperty(HTMLElement.prototype, 'outerHTML', {
            set: function(val) {
                const parent = this.parentElement;
                const ownProperty = Object.getOwnPropertyDescriptor(Element.prototype, 'outerHTML');
                ownProperty.set.call(this, val);
                interceptCallback(parent);
            },
            get: function() {
                const ownProperty = Object.getOwnPropertyDescriptor(Element.prototype, 'outerHTML');
                return ownProperty.get.call(this);
            },
        });
        HTMLElement.prototype.insertAdjacentHTML = function(position, text) {
            const result = Element.prototype.insertAdjacentHTML.call(this, position, text);
            interceptCallback(this.parentElement);
            return result;
        };
    }

    function addOipfObjectFactory() {
        if (window.oipfObjectFactory !== undefined) {
            console.error('Cannot redefine oipfObjectFactory!');
            return;
        }

        window.oipfObjectFactory = {
            isObjectSupported: function(objectType) {
                return resolveMimeType(objectType.toLowerCase()) !== undefined;
            },
        };

        for (let objectFactoryMethod in objectFactoryMethodTable) {
            console.log(objectFactoryMethod);
            const objectName = objectFactoryMethodTable[objectFactoryMethod];
            window.oipfObjectFactory[objectFactoryMethod] = function() {
                const object = __createElement.call(document, 'object');
                object.setAttribute('type', objectName);
                objectUpgradeHandlers[objectName](object);
                return object;
            };
        }
    }

    function createRdkVideoElement() {
        console.log('[RDK] Creating video element for holepunch');
        const video = __createElement.call(document, 'video');
        video._rdkHolepunch = true;
        return video;
    }

    function addMetaViewportElement() {
        if (!document.querySelector('meta[name=viewport]')) {
            let meta = document.createElement('meta');
            meta.name = 'viewport';
            meta.content = 'width=device-width, initial-scale=1.0';
            document.getElementsByTagName('head')[0] ?.appendChild(meta);
        }
    }

    function getElementByIdOverride(id) {
        const element = __getElementById.call(document, id);
        if (element) {
            const objectType = element.getAttribute('type');
            if (objectType) {
                upgradeObject(element, objectType);
            }
        }
        return element;
    }

    return {
        initialise: initialise,
        registerObject: registerObject,
        createRdkVideoElement: createRdkVideoElement,
        context: __context,
    };
})();
