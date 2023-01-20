/**
 * @fileOverview Utiltiies for hole punching.
 * @license ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

hbbtv.holePuncher = (function() {
    let gRect = {
        x: 0,
        y: 0,
        width: 1280,
        height: 720,
        fullScreen: false,
    };

    let gObject = null;
    let gObjectInDocument = false;

    const gPunchHoleObserver = new MutationObserver(function(mutations) {
        mutations.forEach(function(mutation) {
            if (mutation.attributeName === 'style') {
                matchVideoRectangleToObject(mutation.target);
            }
        });
    });

    function observeObject(object) {
        gPunchHoleObserver.observe(object, {
            attributes: true,
            attributeFilter: ['style'],
        });
    }

    function unobserveObject(object) {
        gPunchHoleObserver.disconnect();
    }

    function setVideoRectangle(x, y, width, height, widescreen, fullScreen) {
        if (
            gRect.x == x &&
            gRect.y == y &&
            gRect.width == width &&
            gRect.height == height &&
            gRect.fullScreen == fullScreen
        ) {
            return;
        }
        gRect = {
            x,
            y,
            width,
            height,
            fullScreen,
        };

        const w = Math.min(width, (height / (widescreen ? 9 : 3)) * (widescreen ? 16 : 4));
        const h = Math.min(height, (width / (widescreen ? 16 : 4)) * (widescreen ? 9 : 3));
        x = x + (width - w) / 2;
        y = y + (height - h) / 2;

        if (fullScreen) {
            hbbtv.bridge.broadcast.setVideoRectangle(0, 0, 1280, 720);
        } else {
            hbbtv.bridge.broadcast.setVideoRectangle(
                Math.round(x),
                Math.round(y),
                Math.round(w),
                Math.round(h)
            );
        }
    }

    function matchVideoRectangleToObject(object) {
        if (object.fullScreen) {
            object.style.position = 'absolute';
            object.style.left = '0px';
            object.style.top = '0px';
            object.style.width = '1280px';
            object.style.height = '720px';
            setVideoRectangle(0, 0, 1280, 720, false, true);
            return;
        } else {
            // TODO Restore from full screen
        }

        if (!(gObjectInDocument || object.offsetWidth || object.offsetHeight)) {
            hideVideoRectangle();
            return;
        }

        const bounds = object.getBoundingClientRect();
        const rect = {
            top: bounds.top,
            right: bounds.right,
            bottom: bounds.bottom,
            left: bounds.left,
        };
        const style = window.getComputedStyle(object);
        for (const side in rect) {
            rect[side] =
                rect[side] -
                (parseFloat(style.getPropertyValue('padding-' + side)) +
                    parseFloat(style.getPropertyValue('border-' + side + '-width')));
        }
        const width = rect.right - rect.left;
        const height = rect.bottom - rect.top;
        setVideoRectangle(rect.left, rect.top, width, height, object.widescreen, false);
    }

    function hideVideoRectangle() {
        setVideoRectangle(-1280, -720, 1280, 720, true, false);
    }

    function startPunchHole(object) {
        object.setAttribute('noshade', true);
        if (hbbtv.native.name === 'rdk') {
            let video = hbbtv.objectManager.createRdkVideoElement();
            let source = document.createElement('source');
            video.setAttribute('style', 'transform: scale(1); width:100%; height:100%');
            video.setAttribute('id', 'vid_elem');
            source.setAttribute('src', '/var/www/html/LayoutTests/media/content/long-test.mp4');
            source.setAttribute('type', 'video/holepunch');
            source.setAttribute('id', 'src_elem');
            video.appendChild(source);
            object.appendChild(video);
        }
        if (object.parentNode) {
            observeObject(object);
            gObjectInDocument = true;
        } else {
            gObjectInDocument = false;
        }
        matchVideoRectangleToObject(object);
    }

    function stopPunchHole(object) {
        object.removeAttribute('noshade');
        if (hbbtv.native.name === 'rdk') {
            let video = document.getElementById('vid_elem');
            let source = document.getElementById('src_elem');
            video.removeChild(source);
            object.removeChild(video);
        }
        unobserveObject(object);
        setVideoRectangle(0, 0, 1280, 720, true, false);
    }

    function setBroadcastVideoObject(object) {
        const oldObject = hbbtv.utils.weakDeref(gObject);
        if (object != null) {
            if (oldObject != null) {
                if (oldObject == object) {
                    return;
                }
                stopPunchHole(oldObject);
            }
            gObject = hbbtv.utils.weakRef(object);
            startPunchHole(object);
        } else {
            if (oldObject != null) {
                stopPunchHole(oldObject);
            }
            gObject = null;
        }
    }

    function notifyFullScreenChanged(object) {
        const activeObject = hbbtv.utils.weakDeref(gObject);
        if (object != null && object == activeObject) {
            matchVideoRectangleToObject(activeObject);
        }
    }

    function notifyObjectAddedToDocument(objectAdded) {
        if (!objectAdded.hasAttribute('noshade')) {
            return;
        }
        const object = hbbtv.utils.weakDeref(gObject);
        if (object == objectAdded) {
            observeObject(object);
            gObjectInDocument = true;
            matchVideoRectangleToObject(object);
        }
    }

    function notifyObjectRemovedFromDocument(objectRemoved) {
        if (!objectRemoved.hasAttribute('noshade')) {
            return;
        }
        const object = hbbtv.utils.weakDeref(gObject);
        if (object == objectRemoved) {
            unobserveObject(object);
            gObjectInDocument = false;
            matchVideoRectangleToObject(object);
        }
    }

    return {
        setBroadcastVideoObject: setBroadcastVideoObject,
        notifyFullScreenChanged: notifyFullScreenChanged,
        notifyObjectAddedToDocument: notifyObjectAddedToDocument,
        notifyObjectRemovedFromDocument: notifyObjectRemovedFromDocument,
    };
})();