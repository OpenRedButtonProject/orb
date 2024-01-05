hbbtv.objects.DASHEvent = (function() {
    const prototype = { };
    const privates = new WeakMap();

    hbbtv.utils.defineGetterProperties(prototype, {
        pauseOnExit() {
            return false;
        },
        Id() {
            return privates.get(this).eventData.Id || "";
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
    function initialise(eventData) {
        privates.set(this, { eventData });
        const data = eventData.data;
        if (data) {
            if (eventData.contentEncoding === "arrayBuffer") {
                const uint8Array = new Uint8Array(data.length / 2);
                for (let i = 0; i < data.length; i += 2) {
                    uint8Array[i / 2] = parseInt(data.substr(i, 2), 16);
                }
                eventData.data = uint8Array;
            }
            else {
                try {
                    const parser = new DOMParser();
                    const xmlDoc = parser.parseFromString(data, "text/xml");
                    const parseErrors = xmlDoc.getElementsByTagName("parsererror");
                    if (parseErrors.length === 0) {
                        eventData.data = xmlDoc;
                    }
                  } 
                  catch (e) { }
            }
        }
        console.log(eventData.data);

    }

    return {
        prototype: prototype,
        initialise: initialise
    };
})();

hbbtv.objects.createDASHEvent = function(eventData) {
    const obj = Object.create(hbbtv.objects.DASHEvent.prototype);
    hbbtv.objects.DASHEvent.initialise.call(obj, eventData);
    return obj;
};