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
hbbtv.mediaManager = (function() {
    let objectHandlers = {};
    let fallbackHandlers = undefined;

    function initialise() {
        addMutationIntercept();
        addSourceManipulationIntercept();

        const __play = HTMLMediaElement.prototype.play;
        const __load = HTMLMediaElement.prototype.load;
        HTMLMediaElement.prototype.load = function() {
            let tracks = this.textTracks;
            if (tracks) {
                for (const track of tracks) {
                    track.mode = 'hidden';
                }
            }

            __load.call(this);
        };

        // we override play() for the DASH playback as we end up receiving
        // Uncaught (in promise) DOMException: The play() request was interrupted by a new load request.
        // when calling play() immediately after setting the src attribute
        HTMLMediaElement.prototype.play = function() {
            const thiz = this;

            return new Promise((resolve, reject) => {
                if (thiz.readyState < 2) {
                    const playFcn = function() {
                        thiz.removeEventListener('loadeddata', playFcn, true);
                        thiz.removeEventListener('progress', playFcn, true);
                        __play.call(thiz).then(resolve, reject);
                    };
                    thiz.addEventListener('loadeddata', playFcn, true);
                    thiz.addEventListener('progress', playFcn, true);
                } else {
                    __play.call(thiz).then(resolve, reject);
                }
            });
        };

        console.log("[MediaManager]: Initialized Media manager.");
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

    function setObjectHandler(nextHandler, src) {
        this.__objectType = nextHandler.getName();
        nextHandler.initialise(this, src);
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
                    setObjectHandler.call(object, objectHandlers[key], src);
                    return Promise.resolve();
                }
            }
        }

        // if no supported extension is found, request the content-type of the source
        return new Promise((resolve, reject) => {
            const request = new XMLHttpRequest();
            request.onabort = request.onerror = () => {
                reject('An error occurred while requesting the content type.');
            };
            let timeoutId = -1;
            request.onload = () => {
                clearTimeout(timeoutId);
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
                        setObjectHandler.call(object, nextHandler, src);
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
            timeoutId = setTimeout(request.onload, 5000);
            request.open('HEAD', src);
            request.send();
        });
    }

    function upgradeToFallback(node, src, err) {
        console.warn(
            'MediaManager: Failed to upgrade object. Fallback to native proxy. Error: ' + err
        );
        if (fallbackHandlers) {
            node.__objectType = fallbackHandlers.getName();
            fallbackHandlers.initialise(node, src);
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

        HTMLMediaElement.prototype.setAttribute = function(name, value) {
            if (name === 'src' && !this.__objectType) {
                const thiz = this;
                console.log('MediaManager: intercepted src manipulation. new src: ' + value);
                upgradeObject.call(this, value).catch((e) => upgradeToFallback(thiz, value, e));
                return;
            }
            Element.prototype.setAttribute.call(this, name, value);
        };
    }

    // Mutation observer
    function addMutationIntercept() {
        const observer = new MutationObserver(function(mutationsList) {
            for (const mutation of mutationsList) {
                for (const node of mutation.addedNodes) {
                    if (node.nodeName) {
                        if (
                            (node.nodeName.toLowerCase() === 'video' ||
                            node.nodeName.toLowerCase() === 'audio')
                        ) {
                            if (node.src) {
                                upgradeObject.call(node, node.src).catch((e) => upgradeToFallback(node, node.src, e));
                            }
                            else if (!node.src) {
                                const source = node.getElementsByTagName("source")[0];
                                if (source) {
                                    node.src = source.src;
                                }
                            }
                        }
                        else if (
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
                                setObjectHandler.call(node.parentNode, nextHandler, node.src);
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
