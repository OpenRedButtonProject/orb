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
   const MSG_TYPE_METHOD_REQUEST = "orb_iframe_method_request";
   const MSG_TYPE_ASYNC_METHOD_REQUEST = "orb_iframe_async_method_request";
   const MSG_TYPE_ASYNC_METHOD_RESPONSE = "orb_iframe_async_method_response";
   let asyncIDs = 0;

   prototype.initiateHandshake = function (uuid, remoteWindow) {
      if (uuid === undefined) {
         reject("IFrameObjectProxy: undefined uuid.");
      }
      else {
         const p = privates.get(this);
         console.log("IFrameObjectProxy: Requesting handshake for object type", p.objectType, "with uuid", uuid + "...");
         p.uuid = uuid;
         p.remoteWindow = remoteWindow;
         postMessage.call(this, MSG_TYPE_HANDSHAKE_REQUEST, {objectType: p.objectType});
      }
   }

   prototype.invalidate = function () {
      const p = privates.get(this);
      p.remoteWindow = undefined;
      p.uuid = undefined;
   }

   prototype.callMethod = function (name, args = []) {
      postMessage.call(this, MSG_TYPE_METHOD_REQUEST, {name: name, args: args});
      console.log("IFrameObjectProxy: Requested call to method", name, "with arguments", args);
   }

   prototype.callAsyncMethod = function (name, args = []) {
      const thiz = this;
      return new Promise((resolve, reject) => {
         const id = asyncIDs++;
         console.log("IFrameObjectProxy: Requested call to async method", name, "with arguments", args);
         const callback = (e) => {
            let msg;
            try {
               msg = JSON.parse(e.data);
            }
            catch(error) {
               console.warn("IFrameObjectProxy: error parsing message data:", e.data);
               return;
            }
            if (msg.type === MSG_TYPE_ASYNC_METHOD_RESPONSE && msg.data.id === id && msg.uuid === privates.get(thiz).uuid) {
               console.log("IFrameObjectProxy: received response from async method", name);
               window.removeEventListener("message", callback);
               clearTimeout(timeoutId);
               if (msg.data.error) {
                  reject(msg.data.error);
               }
               else {
                  resolve();
               }
               e.stopImmediatePropagation();
            }
         };
         const timeoutId = setTimeout(() => {
            window.removeEventListener("message", callback);
            reject("IFrameObjectProxy: Call to async method " + name + " timed out without receiving a response.");
         }, 10000);
         window.addEventListener("message", callback);
         postMessage.call(this, MSG_TYPE_ASYNC_METHOD_REQUEST, {name: name, args: args, id: id});
      });
   }

   prototype.setRemoteObjectProperties = function (properties) {
      postMessage.call(this, MSG_TYPE_SET_PROPERTIES, properties);
   }

   prototype.dispatchEvent = function (name) {
      postMessage.call(this, MSG_TYPE_DISPATCH_EVENT, {event: name});
   }

   function postMessage(type, data) {
      const p = privates.get(this);
      if (p.remoteWindow) {
         p.remoteWindow.postMessage(JSON.stringify({
            uuid: p.uuid,
            type: type,
            data: data
         }), '*');
      }
      else {
         p.pending.push({type: type, data: data});
      }
   }

   // objectType is the only way to know which object to bind from the main window
   // with the remote window. TODO: find a proper solution as this does not feel right
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
            console.warn("IFrameObjectProxy: error parsing message data:", e.data);
            return;
         }
         if (msg.type === MSG_TYPE_HANDSHAKE_REQUEST) {
            if (msg.data.objectType === objectType) {
               p.remoteWindow = e.source;
               p.uuid = msg.uuid;
               console.log("IFrameObjectProxy: Received handshake request with uuid", p.uuid + ". Responding back...");
               postMessage.call(thiz, MSG_TYPE_HANDSHAKE_RESPONSE);
               while (p.pending.length) {
                  const pending = p.pending.shift();
                  postMessage.call(thiz, pending.type, pending.data);
               }
               e.stopImmediatePropagation();
            }
         }
         else if (p.uuid === msg.uuid && p.remoteWindow === e.source) {
            switch (msg.type) {
               case MSG_TYPE_HANDSHAKE_RESPONSE:
                  while (p.pending.length) {
                     const pending = p.pending.shift();
                     postMessage.call(thiz, pending.type, pending.data);
                  }
                  console.log("IFrameObjectProxy: Received handshake response with uuid", msg.uuid + ".");
                  break;
               case MSG_TYPE_DISPATCH_EVENT:
                  localObject.dispatchEvent(new Event(msg.data.event));
                  break;
               case MSG_TYPE_SET_PROPERTIES:
                  for (const key in msg.data) {
                     if (typeof localObject[key] !== "function" && localObject[key] !== msg.data[key]) {
                        localObject[key] = msg.data[key];
                     }
                  }
                  break;
               case MSG_TYPE_METHOD_REQUEST:
                  if (typeof localObject[msg.data.name] === "function") {
                     localObject[msg.data.name](...msg.data.args);
                  }
                  break;
               case MSG_TYPE_ASYNC_METHOD_REQUEST:
                  if (typeof localObject[msg.data.name] === "function") {
                     localObject[msg.data.name](...msg.data.args)
                     .then(() => {
                        postMessage.call(thiz, MSG_TYPE_ASYNC_METHOD_RESPONSE, { 
                           id: msg.data.id
                        });
                     })
                     .catch(e => {
                        postMessage.call(thiz, MSG_TYPE_ASYNC_METHOD_RESPONSE, { 
                           id: msg.data.id,
                           error: e
                        });
                     });
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