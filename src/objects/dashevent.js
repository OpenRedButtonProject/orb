hbbtv.objects.DASHEvent = (function() {
    const prototype = { };
    const privates = new WeakMap();

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
                streamEvent.DASHEvent.data = streamEvent.data = textEncoder.encode(data);
            }
            else {
                try {
                    const parser = new DOMParser();
                    streamEvent.DASHEvent.data = streamEvent.data = parser.parseFromString(data, 'text/xml');
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