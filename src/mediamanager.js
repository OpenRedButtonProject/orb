/**
 * @fileOverview Monitors the document and certain JavaScript mechanisms for objects. Builds or
 * upgrades those objects if there is a registered handler.
 * @license ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

hbbtv.mediaManager = (function() {
    let objectHandlers = {};
    let fallbackHandlers = undefined;

    function initialise() {
        addSourceManipulationIntercept();
        addMutationIntercept();

        function polyfillDocumentHead() {
            document.removeEventListener('DOMContentLoaded', polyfillDocumentHead);

            // add a predifined style to the subtitles div
            document.head.innerHTML +=
                '<style>#__obs_subsPH__ div { position: static; overflow: auto; pointer-events:none; }</style>';
        }
        if (document.head) {
            polyfillDocumentHead();
        } else {
            document.addEventListener('DOMContentLoaded', polyfillDocumentHead);
        }

        const __play = HTMLMediaElement.prototype.play;
        let lastMediaElement = undefined;

        // we override play() for the DASH playback as we end up receiving
        // Uncaught (in promise) DOMException: The play() request was interrupted by a new load request.
        // when calling play() immediately after setting the src attribute
        HTMLMediaElement.prototype.play = function() {
            const thiz = this;
            if (!this.__added_to_media_sync__) {
                // check if the HTMLMediaElement is provided to MediaSynchroniser.addMediaObject() before we pause it
                if (lastMediaElement && lastMediaElement !== this && !lastMediaElement.paused) {
                    lastMediaElement.pause();
                }
                lastMediaElement = this;
            }
            return new Promise((resolve, reject) => {
                if (thiz.readyState < 2) {
                    const playFcn = function() {
                        thiz.removeEventListener('loadeddata', playFcn);
                        __play.call(thiz).then(resolve, reject);
                    };
                    thiz.addEventListener('loadeddata', playFcn, true);
                } else {
                    __play.call(thiz).then(resolve, reject);
                }
            });
        };
    }

    function registerObject(handlers) {
        objectHandlers[handlers.getName()] = handlers;
        if (handlers.getName() === 'native') {
            fallbackHandlers = handlers;
        }
    }

    function getHandlerByContentType(type) {
        for (const key in objectHandlers) {
            const supportedTypes = objectHandlers[key].getSupportedContentTypes();
            for (const t of supportedTypes) {
                if (type.includes(t)) {
                    return objectHandlers[key];
                }
            }
        }
        return undefined;
    }

    function switchObjectHandler(nextHandler, src) {
        let objType = this.__objectType;
        if (objType) {
            if (objectHandlers[objType] !== nextHandler) {
                this.__objectType = nextHandler.getName();
                nextHandler.initialise(this, src);
            } else {
                nextHandler.onSourceAboutToChange(this, src);
            }
        } else {
            this.__objectType = nextHandler.getName();
            nextHandler.initialise(this, src);
        }
    }

    function upgradeObject(src) {
        const object = this;
        let objType = object.__objectType;
        if (!src) {
            return Promise.reject('Playback source is not defined.');
        } else if (
            (object.getAttribute('src') === src && objType) ||
            src.toLowerCase().startsWith('blob:')
        ) {
            return Promise.resolve();
        }

        // consider playback anchors with url
        let ext = src.split('#')[0];
        ext = ext.split('?')[0];
        // first check each objectHandler supported extensions
        if (ext) {
            ext = ext.split('.').pop().toLowerCase();
            console.log('MediaManager: Checking extension support for .' + ext + '...');
            for (const key in objectHandlers) {
                if (objectHandlers[key].getSupportedExtensions().indexOf(ext) >= 0) {
                    switchObjectHandler.call(object, objectHandlers[key], src);
                    return Promise.resolve();
                }
            }
        }

        // if no supported extension is found, request the content-type of the source
        const request = new XMLHttpRequest();
        request.open('HEAD', src);
        request.send();
        return new Promise((resolve, reject) => {
            request.onabort = request.onerror = () => {
                reject('An error occurred while requesting the content type.');
            };
            request.onload = () => {
                try {
                    const contentType = request
                        .getAllResponseHeaders()
                        .split('\n')
                        .find((header) => header.toLowerCase().startsWith('content-type'))
                        .split(':')[1]
                        .trim();

                    console.log('MediaManager: Requested content of type ' + contentType);
                    const nextHandler = getHandlerByContentType(contentType);
                    if (nextHandler) {
                        switchObjectHandler.call(object, nextHandler, src);
                        resolve();
                    } else {
                        reject(
                            'Failed to find a registered playback proxy for the content type ' +
                            contentType
                        );
                    }
                } catch (e) {
                    reject(e);
                }
            };
        });
    }

    function upgradeToFallback(node, err) {
        console.warn(
            'MediaManager: Failed to upgrade object. Fallback to native proxy. Error: ' + err
        );
        if (fallbackHandlers) {
            let type = node.__objectType;
            if (type) {
                const handler = getHandlerByContentType(type);
                if (handler !== fallbackHandlers) {
                    node.__objectType = fallbackHandlers.getName();
                    fallbackHandlers.initialise(node, node.src);
                }
            } else {
                node.__objectType = fallbackHandlers.getName();
                fallbackHandlers.initialise(node, node.src);
            }
        }
    }

    function addSourceManipulationIntercept() {
        const ownProperty = Object.getOwnPropertyDescriptor(HTMLMediaElement.prototype, 'src');
        Object.defineProperty(HTMLMediaElement.prototype, 'src', {
            set(val) {
                this.setAttribute('src', val);
            },
            get() {
                return ownProperty.get.call(this);
            },
        });

        HTMLMediaElement.prototype.removeAttribute = function(name) {
            if (this.getAttribute(name)) {
                Element.prototype.removeAttribute.apply(this, arguments);
                if (name === 'src') {
                    console.log('MediaManager: removing src attribute.');
                    let type = this.__objectType;
                    if (type) {
                        const handler = getHandlerByContentType(type) || fallbackHandlers;
                        handler.onSourceAboutToChange(this, null);
                    } else {
                        this.__objectType = fallbackHandlers.getName();
                        fallbackHandlers.initialise(this, undefined);
                    }
                }
            }
        };

        HTMLMediaElement.prototype.setAttribute = function(name, value) {
            if (name === 'src') {
                const thiz = this;
                console.log('MediaManager: intercepted src manipulation. new src: ' + value);
                upgradeObject.call(this, value).catch((e) => upgradeToFallback(thiz, e));
            }
            Element.prototype.setAttribute.apply(this, arguments);
        };

        HTMLMediaElement.prototype.appendChild = function(node) {
            if (node.nodeName) {
                if (node.nodeName.toLowerCase() === 'source' && !this.src) {
                    console.log(
                        'MediaManager: intercepted source element addition with src ' +
                        node.src +
                        ' and type ' +
                        node.type
                    );
                    if (node.type) {
                        const nextHandler = getHandlerByContentType(node.type) || fallbackHandlers;
                        switchObjectHandler.call(this, nextHandler, node.src);
                        Element.prototype.setAttribute.call(this, 'src', node.src);
                    } else {
                        const thiz = this;
                        upgradeObject.call(this, node.src).catch((e) => upgradeToFallback(thiz, e));
                    }
                }
            }
            Element.prototype.appendChild.apply(this, arguments);
        };
    }

    // Mutation observer
    function addMutationIntercept() {
        const observer = new MutationObserver(function(mutationsList) {
            for (const mutation of mutationsList) {
                for (const node of mutation.addedNodes) {
                    if (node.nodeName) {
                        if (
                            node.nodeName.toLowerCase() === 'video' ||
                            node.nodeName.toLowerCase() === 'audio'
                        ) {
                            try {
                                if (node.src) {
                                    upgradeObject
                                        .call(node, node.src)
                                        .catch((e) => upgradeToFallback(node, e));
                                } else {
                                    let found = false;
                                    for (const nd of node.children) {
                                        if (nd.nodeName && nd.nodeName.toLowerCase() === 'source') {
                                            console.log(
                                                'MediaManager: intercepted source element addition with src ' +
                                                nd.src +
                                                ' and type ' +
                                                nd.type
                                            );
                                            if (nd.type) {
                                                const nextHandler =
                                                    getHandlerByContentType(nd.type) ||
                                                    fallbackHandlers;
                                                switchObjectHandler.call(node, nextHandler, nd.src);
                                                Element.prototype.setAttribute.call(
                                                    node,
                                                    'src',
                                                    nd.src
                                                );
                                            } else {
                                                upgradeObject
                                                    .call(node, nd.src)
                                                    .catch((e) => upgradeToFallback(node, e));
                                            }
                                            found = true;
                                            break;
                                        }
                                    }
                                    if (!found) {
                                        upgradeToFallback(node, 'Source is not defined.');
                                    }
                                }
                            } catch (e) {
                                if (fallbackHandlers) {
                                    console.warn(
                                        'MediaManager: Failed to upgrade object. Fallback to native proxy. Error: ' +
                                        e
                                    );
                                    let type = node.__objectType;
                                    if (type) {
                                        const handler = getHandlerByContentType(type);
                                        if (handler !== fallbackHandlers) {
                                            node.__objectType = fallbackHandlers.getName();
                                            fallbackHandlers.initialise(node, node.src);
                                        }
                                    } else {
                                        node.__objectType = fallbackHandlers.getName();
                                        fallbackHandlers.initialise(node, node.src);
                                    }
                                }
                            }
                        } else if (
                            node.nodeName.toLowerCase() === 'source' &&
                            node.parentNode &&
                            !node.parentNode.src
                        ) {
                            console.log(
                                'MediaManager: intercepted source element addition with src ' +
                                node.src +
                                ' and type ' +
                                node.type
                            );
                            if (node.type) {
                                const nextHandler =
                                    getHandlerByContentType(node.type) || fallbackHandlers;
                                switchObjectHandler.call(node.parentNode, nextHandler, node.src);
                                Element.prototype.setAttribute.call(
                                    node.parentNode,
                                    'src',
                                    node.src
                                );
                            } else {
                                upgradeObject
                                    .call(node.parentNode, node.src)
                                    .catch((e) => upgradeToFallback(node.parentNode, e));
                            }
                        }
                    }
                }
            }
        });

        observer.observe(document.documentElement || document.body, {
            childList: true,
            subtree: true,
        });
    }

    return {
        initialise: initialise,
        registerObject: registerObject,
    };
})();