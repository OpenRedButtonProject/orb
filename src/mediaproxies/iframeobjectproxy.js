/**
 * @fileOverview IFrameObjectProxy class
 * @license ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

hbbtv.objects.IFrameObjectProxy = (function() {
   const prototype = {};
   const privates = new WeakMap();
   const MSG_TYPE_HANDSHAKE_REQUEST = "orb_iframe_handshake_request";
   const MSG_TYPE_HANDSHAKE_RESPONSE = "orb_iframe_handshake_response";
   const MSG_TYPE_DISPATCH_EVENT = "orb_iframe_dispatch_event";
   const MSG_TYPE_SET_PROPERTIES = "orb_iframe_set_properties";

   prototype.initiateHandshake = function (uuid, remoteWindow) {
      if (uuid === undefined) {
         throw "IFrameObjectProxy: undefined uuid.";
      }
      const p = privates.get(this);
      console.log("IFrameObjectProxy: Requesting handshake for object type", p.objectType, "with uuid", uuid + "...");
      p.uuid = uuid;
      remoteWindow.postMessage(JSON.stringify({
         type: MSG_TYPE_HANDSHAKE_REQUEST,
         uuid: uuid,
         objectType: p.objectType
      }), '*');
      p.remoteWindow = remoteWindow;
   }

   prototype.invalidate = function () {
      privates.get(this).remoteWindow = undefined;
   }

   prototype.callMethod = function (name, args = []) {
      this.setRemoteObjectProperties({[name]: args});
   }

   prototype.setRemoteObjectProperties = function (properties) {
      const p = privates.get(this);
      if (p.remoteWindow) {
         p.remoteWindow.postMessage(JSON.stringify({
            uuid: p.uuid,
            type: MSG_TYPE_SET_PROPERTIES,
            properties: properties
         }), '*');
      }
      else {
         p.pending.push(properties);
      }
   }

   prototype.dispatchEvent = function (name) {
      const p = privates.get(this);
      if (p.remoteWindow) {
         p.remoteWindow.postMessage(JSON.stringify({
            uuid: p.uuid,
            type: MSG_TYPE_DISPATCH_EVENT,
            name: name
         }), '*');
      }
   }

   function initialise(localObject, objectType) {
      privates.set(this, {
         objectType: objectType,
         pending: []
      });
      
      const thiz = this;
      const p = privates.get(this);

      p.onMessage = (e) => {
         let msg;
         try {
            msg = JSON.parse(e.data);
         }
         catch(error) {
            console.error("IFrameObjectProxy:", error);
            return;
         }
         if (msg.type === MSG_TYPE_HANDSHAKE_REQUEST) {
            if (msg.objectType === objectType) {
               p.remoteWindow = e.source;
               p.uuid = msg.uuid;
               console.log("IFrameObjectProxy: Received handshake request with uuid", p.uuid + ". Responding back...");
               e.source.postMessage(JSON.stringify({
                  type: MSG_TYPE_HANDSHAKE_RESPONSE,
                  uuid: p.uuid
               }), '*');
               while (p.pending.length) {
                  prototype.setRemoteObjectProperties.call(thiz, p.pending.shift());
               }
               e.stopImmediatePropagation();
            }
         }
         else if (p.uuid === msg.uuid && p.remoteWindow === e.source) {
            switch (msg.type) {
               case MSG_TYPE_HANDSHAKE_RESPONSE:
                  while (p.pending.length) {
                     prototype.setRemoteObjectProperties.call(thiz, p.pending.shift());
                  }
                  console.log("IFrameObjectProxy: Received handshake response with uuid", msg.uuid + ".");
                  break;
               case MSG_TYPE_DISPATCH_EVENT:
                  localObject.dispatchEvent(new Event(msg.name));
                  break;
               case MSG_TYPE_SET_PROPERTIES:
                  for (const key in msg.properties) {
                     if (typeof localObject[key] === "function") {
                        localObject[key](...msg.properties[key]);
                     }
                     else if (localObject[key] !== msg.properties[key]) { // prevent recursive setting of properties
                        localObject[key] = msg.properties[key];
                     }
                  }
                  break;
               default:
                  return; // return in order to prevent stopImmediatePropagation from being called
            }
            e.stopImmediatePropagation();
         }
      };
      window.addEventListener("message", p.onMessage);
      console.log("IFrameObjectProxy: initialised for type", objectType);
   }

   return {
      prototype: prototype,
      initialise: initialise
   };
})();

hbbtv.objects.createIFrameObjectProxy = function(localObject, objectType) {
   const iframeObjectProxy = Object.create(hbbtv.objects.IFrameObjectProxy.prototype);
   hbbtv.objects.IFrameObjectProxy.initialise.call(iframeObjectProxy, localObject, objectType);
   return iframeObjectProxy;
};