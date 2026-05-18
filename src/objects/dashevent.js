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
        const raw = streamEvent.DASHEvent;
        console.log(
            '[ORB-DASHEvent] initialise: raw id=' + (raw && raw.id) +
                ' startTime=' + (raw && raw.startTime) +
                ' duration=' + (raw && raw.duration) +
                ' textType=' + typeof streamEvent.text
        );
        privates.set(this, {
            eventData: streamEvent.DASHEvent,
        });
        const data = streamEvent.text;
        if (typeof(data) === "string") {
            /* TS 102 796 §9.3.2.2: DataCue.data is UTF-8 bytes in ArrayBuffer, not XMLDocument. */
            try {
                const textEncoder = new TextEncoder();
                const u8 = textEncoder.encode(data);
                streamEvent.DASHEvent.data = u8.buffer.slice(
                    u8.byteOffset,
                    u8.byteOffset + u8.byteLength);
                streamEvent.data = utf8StringToUpperHex(data);
            }
            catch(e) {
                console.warn(e.message);
            }
        }
        streamEvent.DASHEvent = this;
        console.log(
            '[ORB-DASHEvent] initialise: after wrap id=' + streamEvent.DASHEvent.id +
                ' startTime=' + streamEvent.DASHEvent.startTime +
                ' endTime=' + streamEvent.DASHEvent.endTime
        );
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