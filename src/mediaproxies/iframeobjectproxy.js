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
   const MSG_TYPE_DISPATCH_EVENT = "orb_iframe_dispatch_event";
   const MSG_TYPE_SET_PROPERTIES = "orb_iframe_set_properties";
   const MSG_TYPE_METHOD_REQUEST = "orb_iframe_method_request";
   const MSG_TYPE_ASYNC_METHOD_REQUEST = "orb_iframe_async_method_request";
   const MSG_TYPE_ASYNC_CALL_RESPONSE = "orb_iframe_async_method_response";
   let asyncIDs = 0;
   
   // This may not be ideal in case the iframe content initiates the handshake,
   // because in case we have multiple video elements, for all of them, sessionIDs
   // variable would be initially set to 0. At the moment we know that the main
   // window initiates the handshake, so no problems there.
   let sessionIDs = 0;

   prototype.initiateHandshake = function (remoteWindow) {
      const p = privates.get(this);
      const thiz = this;
      p.sessionId = sessionIDs++;
      p.remoteWindow = remoteWindow;
      console.log("IFrameObjectProxy: Requesting handshake for object type", p.objectType, "with sessionId", p.sessionId + "...");
      return makeAsyncCall.call(this, MSG_TYPE_HANDSHAKE_REQUEST, {objectType: p.objectType}).then(() => {
         while (p.pending.length) {
            const pending = p.pending.shift();
            postMessage.call(thiz, pending.type, pending.data);
         }
      });
   }

   prototype.invalidate = function () {
      const p = privates.get(this);
      p.remoteWindow = undefined;
      p.sessionId = undefined;
   }

   prototype.callMethod = function (name, args = []) {
      postMessage.call(this, MSG_TYPE_METHOD_REQUEST, {name: name, args: args});
      console.log("IFrameObjectProxy: Requested call to method", name, "with arguments", args);
   }

   prototype.callAsyncMethod = function (name, args = []) {
      console.log("IFrameObjectProxy: Requested call to async method", name, "with arguments", args);
      return makeAsyncCall.call(this, MSG_TYPE_ASYNC_METHOD_REQUEST, {name: name, args: args});
   }

   prototype.setRemoteObjectProperties = function (properties) {
      postMessage.call(this, MSG_TYPE_SET_PROPERTIES, properties);
   }

   prototype.dispatchEvent = function (e) {
      postMessage.call(this, MSG_TYPE_DISPATCH_EVENT, {eventName: e.type, eventData: e});
   }

   function makeAsyncCall(type, data) {
      const thiz = this;
      return new Promise((resolve, reject) => {
         data.callId = asyncIDs++;
         const callback = (e) => {
            let msg;
            try {
               msg = JSON.parse(e.data);
            }
            catch(error) {
               console.warn("IFrameObjectProxy: error parsing message data:", e.data);
               return;
            }
            if (msg.type === MSG_TYPE_ASYNC_CALL_RESPONSE && msg.data.callId === data.callId && msg.sessionId === privates.get(thiz).sessionId) {
               console.log("IFrameObjectProxy: received response from async call with callId", data.callId);
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
            reject("IFrameObjectProxy: Async call with callId " + data.callId +  " timed out without receiving a response.");
         }, 10000);
         window.addEventListener("message", callback);
         console.log("IFrameObjectProxy: Making an async call with callId " + data.callId);
         postMessage.call(thiz, type, data);
      });
   }

   function postMessage(type, data) {
      const p = privates.get(this);
      if (p.remoteWindow) {
         p.remoteWindow.postMessage(JSON.stringify({
            sessionId: p.sessionId,
            type: type,
            data: data
         }), '*');
      }
      else {
         p.pending.push({type: type, data: data});
      }
   }

   // objectType is the only way to know which object to bind from the main window
   // with the remote window. It also works at the moment, because we know that
   // on each iframe we have unique object types, and the MediaElementWrapper from
   // the main window is the one that will initiate a handshake to a targeted iframe.
   // TODO: find a proper solution as this does not feel right
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
               p.sessionId = msg.sessionId;
               console.log("IFrameObjectProxy: Received handshake request with sessionId", p.sessionId + ". Responding back...");
               postMessage.call(thiz, MSG_TYPE_ASYNC_CALL_RESPONSE, { 
                  callId: msg.data.callId
               });
               while (p.pending.length) {
                  const pending = p.pending.shift();
                  postMessage.call(thiz, pending.type, pending.data);
               }
               e.stopImmediatePropagation();
            }
         }
         else if (p.sessionId === msg.sessionId && p.remoteWindow === e.source) {
            switch (msg.type) {
               case MSG_TYPE_DISPATCH_EVENT:
                  const evt = new Event(msg.data.eventName)
                  for (const key in msg.data.eventData) {
                     if (key !== "isTrusted") { // Uncaught TypeError: Cannot set property isTrusted of #<Event> which has only a getter
                        evt[key] = msg.data.eventData[key];
                     }
                  }
                  localObject.dispatchEvent(evt);
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
                        postMessage.call(thiz, MSG_TYPE_ASYNC_CALL_RESPONSE, { 
                           callId: msg.data.callId
                        });
                     })
                     .catch(e => {
                        postMessage.call(thiz, MSG_TYPE_ASYNC_CALL_RESPONSE, { 
                           callId: msg.data.callId,
                           error: e
                        });
                     });
                  }
                  break;
               default:
                  return; // return for all other cases in order to prevent stopImmediatePropagation from being called
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