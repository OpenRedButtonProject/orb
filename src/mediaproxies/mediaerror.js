/**
 * @fileOverview MediaError class
 * @license ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

hbbtv.objects.MediaError = (function() {
    const prototype = {};
    const privates = new WeakMap();

    hbbtv.utils.defineConstantProperties(prototype, {
        MEDIA_ERR_ABORTED: MediaError.MEDIA_ERR_ABORTED,
        MEDIA_ERR_NETWORK: MediaError.MEDIA_ERR_NETWORK,
        MEDIA_ERR_DECODE: MediaError.MEDIA_ERR_DECODE,
        MEDIA_ERR_SRC_NOT_SUPPORTED: MediaError.MEDIA_ERR_SRC_NOT_SUPPORTED,
    });

    hbbtv.utils.defineGetterProperties(prototype, {
        code() {
            return privates.get(this).code;
        },
        message() {
            return privates.get(this).message;
        },
    });

    function initialise(code, message) {
        privates.set(this, {});
        const p = privates.get(this);
        p.code = code;
        p.message = message;
    }

    return {
        prototype: prototype,
        initialise: initialise,
    };
})();

hbbtv.objects.createMediaError = function(code, message) {
    const mediaError = Object.create(hbbtv.objects.MediaError.prototype);
    hbbtv.objects.MediaError.initialise.call(mediaError, code, message);
    return mediaError;
};