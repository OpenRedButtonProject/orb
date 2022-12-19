/**
 * @fileOverview Common utiltiies used by objects.
 * @license ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

hbbtv.utils = (function() {
    const HAS_WEAKREF_SUPPORT = (function() {
        try {
            const test = new WeakRef({});
            return true;
        } catch (e) {
            console.log(
                'Warning: For development on non-production browsers only (WeakRef unsupported)'
            );
        }
        return false;
    })();

    function weakRef(ref) {
        return ref ? (HAS_WEAKREF_SUPPORT ? new WeakRef(ref) : ref) : ref;
    }

    function weakDeref(ref) {
        return ref ? (HAS_WEAKREF_SUPPORT ? ref.deref() : ref) : ref;
    }

    function createFinalizationRegistry(callback) {
        return HAS_WEAKREF_SUPPORT ?
            new FinalizationRegistry(callback) :
            {
                register: () => {},
                unregister: () => {},
            };
    }

    function runOnMainLooper(thiz, runnable) {
        setTimeout(runnable.bind(thiz), 0);
    }

    function defineConstantProperties(prototype, constants) {
        for (const [key, value] of Object.entries(constants)) {
            Object.defineProperty(prototype, key, {
                value: value,
            });
        }
    }

    function defineGetterProperties(prototype, getters) {
        for (const [key, getter] of Object.entries(getters)) {
            Object.defineProperty(prototype, key, {
                get: getter,
            });
        }
    }

    function defineGetterSetterProperties(prototype, props) {
        for (const [key, prop] of Object.entries(props)) {
            Object.defineProperty(prototype, key, {
                set: prop.set,
                get: prop.get,
            });
        }
    }

    function preventDefaultMediaHandling(target) {
        const parent = target.parentNode;
        target.setAttribute('classid', 'hbbtv');
        if (parent) {
            // Chrome ignores classid changes once object is already in the tree, so remove and add it back
            const sibling = target.nextSibling;
            parent.removeChild(target);
            if (sibling) {
                parent.insertBefore(target, sibling);
            } else {
                parent.appendChild(target);
            }
        }
    }

    function base64Encode(str) {
        const table = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=';
        let result = '';
        let length = str.length;
        let i = 0;
        while (i < length) {
            let output0 = (str.charCodeAt(i) & 0b11111100) >> 2;
            let output1 = (str.charCodeAt(i) & 0b00000011) << 4;
            let output2 = 64;
            let output3 = 64;
            if (++i < length) {
                output1 |= (str.charCodeAt(i) & 0b11110000) >> 4;
                output2 = (str.charCodeAt(i) & 0b00001111) << 2;
                if (++i < length) {
                    output2 |= (str.charCodeAt(i) & 0b11000000) >> 6;
                    output3 = str.charCodeAt(i) & 0b00111111;
                    ++i;
                }
            }
            result += table[output0];
            result += table[output1];
            result += table[output2];
            result += table[output3];
        }
        return result;
    }

    function insertAfter(thiz, child, sibling) {
        if (thiz) {
            const nextSibling = sibling.nextSibling;
            if (nextSibling) {
                thiz.insertBefore(child, nextSibling);
            } else {
                thiz.appendChild(child);
            }
        }
    }

    function matchElementStyle(thiz, element) {
        if (thiz) {
            const style = window.getComputedStyle(element);
            Array.from(style).forEach((key) =>
                thiz.style.setProperty(
                    key,
                    style.getPropertyValue(key),
                    style.getPropertyPriority(key)
                )
            );
            if (thiz.style.position !== 'fixed') {
                const bounds = element.getBoundingClientRect();
                thiz.style.left = bounds.left + 'px';
                thiz.style.top = bounds.top + 'px';
                thiz.style.position = 'absolute';
            }
        }
    }

    const gListenerFinalization = createFinalizationRegistry(({
        target,
        type,
        wrapper
    }) => {
        target.removeEventListener(type, wrapper);
    });

    class EventDispatcher {
        constructor(target) {
            this.delegate = document.createDocumentFragment();
            this.wrappers = {};
            this.count = 0;
            this.shadow = {};
            this.target = target;
        }

        /**
         * Add a listener that will be called for this event type, unless it was collected.
         *
         * This method does not create a strong reference from the EventDispatcher to the listener,
         * so the listener (and its references) can be collected once they become otherwise
         * unreachable. Useful for events where no explicit removal method is guaranteed to be called
         * later.
         *
         * Note: Passing in an anonymous function (for example) will probably result in listener being
         * collected immediately! A reference to listener needs to be held by the caller, for example
         * in an object with an associated lifetime.
         */
        addWeakEventListener(type, listener) {
            if (!HAS_WEAKREF_SUPPORT) {
                return this.addEventListener(type, listener);
            }

            if (!(type in this.wrappers)) {
                this.wrappers[type] = new WeakMap();
            }
            if (!this.wrappers[type].has(listener)) {
                const weak = new WeakRef(listener);
                const wrapper = (event) => {
                    const listener = weak.deref();
                    if (listener) {
                        listener(event);
                    }
                };
                this.wrappers[type].set(listener, wrapper);
                gListenerFinalization.register(listener, {
                    target: this,
                    type,
                    wrapper,
                });
                this.addEventListener(type, wrapper);
            }
        }

        /**
         * Remove a previously added weak event listener.
         */
        removeWeakEventListener(type, listener) {
            if (!HAS_WEAKREF_SUPPORT) {
                return this.removeEventListener(type, listener);
            }

            if (type in this.wrappers) {
                const wrapper = this.wrappers[type].get(listener);
                if (wrapper) {
                    this.wrappers[type].delete(listener);
                    gListenerFinalization.unregister(listener);
                    this.removeEventListener(type, wrapper);
                }
            }
        }

        /**
         * Add a counted listener that will be called for this event type.
         *
         * Returns the total number of counted listeners currently added to this EventDispatcher.
         */
        addCountedEventListener(type, listener) {
            this.addEventListener(type, listener);
            if (!(type in this.shadow)) {
                this.shadow[type] = new WeakSet([listener]);
                this.count++;
            } else if (!this.shadow[type].has(listener)) {
                this.shadow[type].add(listener);
                this.count++;
            }
            return this.count;
        }

        /**
         * Remove a previously added counted listener.
         *
         * Returns the total number of counted listeners currently added to this EventDispatcher.
         */
        removeCountedEventListener(type, listener) {
            this.removeEventListener(type, listener);
            if (type in this.shadow) {
                if (this.shadow[type].has(listener)) {
                    this.shadow[type].delete(listener);
                    this.count--;
                }
            }
            return this.count;
        }

        addEventListener(type, listener) {
            this.delegate.addEventListener(type, listener);
        }

        removeEventListener(type, listener) {
            this.delegate.removeEventListener(type, listener);
        }

        dispatchEvent(event) {
            if (this.target) {
                Object.defineProperty(event, 'target', {
                    writable: false,
                    value: this.target,
                });
            }
            this.delegate.dispatchEvent(event);
        }
    }

    return {
        HAS_WEAKREF_SUPPORT: HAS_WEAKREF_SUPPORT,
        weakRef: weakRef,
        weakDeref: weakDeref,
        createFinalizationRegistry: createFinalizationRegistry,
        runOnMainLooper: runOnMainLooper,
        defineConstantProperties: defineConstantProperties,
        defineGetterProperties: defineGetterProperties,
        defineGetterSetterProperties: defineGetterSetterProperties,
        preventDefaultMediaHandling: preventDefaultMediaHandling,
        base64Encode: base64Encode,
        insertAfter: insertAfter,
        matchElementStyle: matchElementStyle,
        EventDispatcher: EventDispatcher,
    };
})();