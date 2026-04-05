hbbtv.objects.DASHEvent = (function() {
    const prototype = { };
    const privates = new WeakMap();

    function utf8StringToUpperHex(str) {
        const u8 = new TextEncoder().encode(str);
        let hex = "";
        for (let i = 0; i < u8.length; i += 1) {
            hex += ("0" + u8[i].toString(16)).slice(-2);
        }
        return hex.toUpperCase();
    }

    hbbtv.utils.defineGetterProperties(prototype, {
        pauseOnExit() {
            return false;
        },
        id() {
            return privates.get(this).eventData.id || "";
        },
        startTime() {
            return privates.get(this).eventData.startTime || 0;
        },
        endTime() {
            return privates.get(this).eventData.duration + this.startTime || Number.MAX_VALUE;
        },
        data() {
            return privates.get(this).eventData.data;
        }
    });

    // Initialise an instance of prototype
    function initialise(streamEvent) {
        privates.set(this, {
            eventData: streamEvent.DASHEvent,
        });
        const data = streamEvent.text;
        if (typeof(data) === "string") {
            if (streamEvent.DASHEvent.contentEncoding === "binary") {
                const textEncoder = new TextEncoder();
                const u8 = textEncoder.encode(data);
                streamEvent.DASHEvent.data = u8.buffer.slice(
                    u8.byteOffset,
                    u8.byteOffset + u8.byteLength);
                streamEvent.data = utf8StringToUpperHex(data);
            }
            else {
                try {
                    const parser = new DOMParser();
                    streamEvent.DASHEvent.data = parser.parseFromString(data, "text/xml");
                    streamEvent.data = utf8StringToUpperHex(data);
                }
                catch(e) {
                    console.warn(e.message);
                }
            }
        }
        streamEvent.DASHEvent = this;
    }

    return {
        prototype: prototype,
        initialise: initialise
    };
})();

hbbtv.objects.createDASHEvent = function(streamEvent) {
    const obj = Object.create(hbbtv.objects.DASHEvent.prototype);
    hbbtv.objects.DASHEvent.initialise.call(obj, streamEvent);
    return obj;
};