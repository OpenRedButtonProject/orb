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
      console.log("IFrameObjectProxy: Requesting handshake with sessionId", p.sessionId + "...");
      return makeAsyncCall.call(this, MSG_TYPE_HANDSHAKE_REQUEST).then(() => {
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

   prototype.registerObserver = function(observerId, observer) {
      console.log("IFrameObjectProxy: Registered observer", observer, "with id", observerId);
      privates.get(this).observers[observerId] = observer;
   };

   prototype.unregisterObserver = function(observerId) {
      console.log("IFrameObjectProxy: Unregistered observer with id", observerId);
      delete privates.get(this).observers[observerId];
   }

   prototype.callObserverMethod = function (observerId, name, args = []) {
      postMessage.call(this, MSG_TYPE_METHOD_REQUEST, {observerId: observerId, name: name, args: args});
      console.log("IFrameObjectProxy: Requested call to method", name, "with arguments", args, "to observer with id", observerId);
   }

   prototype.callAsyncObserverMethod = function (observerId, name, args = []) {
      console.log("IFrameObjectProxy: Requested call to async method", name, "with arguments", args, "to observer with id", observerId);
      return makeAsyncCall.call(this, MSG_TYPE_ASYNC_METHOD_REQUEST, {observerId: observerId, name: name, args: args});
   }

   prototype.updateObserverProperties = function (observerId, properties) {
      postMessage.call(this, MSG_TYPE_SET_PROPERTIES, {observerId: observerId, properties: properties});
      //console.log("IFrameObjectProxy: Requested update of properties for observer with id", observerId, properties);
   }

   prototype.dispatchEvent = function (observerId, e) {
      postMessage.call(this, MSG_TYPE_DISPATCH_EVENT, {observerId: observerId, eventName: e.type, eventData: e});
   }

   function makeAsyncCall(type, data = {}) {
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

   function initialise() {
      privates.set(this, {
         observers: {},
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
         else if (p.sessionId === msg.sessionId && p.remoteWindow === e.source) {
            const observer = p.observers[msg.data.observerId];
            if (observer) {
               switch (msg.type) {
                  case MSG_TYPE_DISPATCH_EVENT:
                     const evt = new Event(msg.data.eventName)
                     for (const key in msg.data.eventData) {
                        if (key !== "isTrusted") { // Uncaught TypeError: Cannot set property isTrusted of #<Event> which has only a getter
                           evt[key] = msg.data.eventData[key];
                        }
                     }
                     observer.dispatchEvent(evt);
                     break;
                  case MSG_TYPE_SET_PROPERTIES:
                     for (const key in msg.data.properties) {
                        if (typeof observer[key] !== "function" && observer[key] !== msg.data.properties[key]) {
                           observer[key] = msg.data.properties[key];
                        }
                     }
                     break;
                  case MSG_TYPE_METHOD_REQUEST:
                     if (typeof observer[msg.data.name] === "function") {
                        observer[msg.data.name](...msg.data.args);
                     }
                     break;
                  case MSG_TYPE_ASYNC_METHOD_REQUEST:
                     if (typeof observer[msg.data.name] === "function") {
                        observer[msg.data.name](...msg.data.args)
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
         }
      };
      window.addEventListener("message", p.onMessage);
      console.log("IFrameObjectProxy: initialised");
   }

   return {
      prototype: prototype,
      initialise: initialise
   };
})();

hbbtv.objects.createIFrameObjectProxy = function() {
   const iframeObjectProxy = Object.create(hbbtv.objects.IFrameObjectProxy.prototype);
   hbbtv.objects.IFrameObjectProxy.initialise.call(iframeObjectProxy);
   return iframeObjectProxy;
};
