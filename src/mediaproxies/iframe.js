(function () {
   let __orb_iframeId__ = -1;

   function onEvent(e) {
      window.parent.postMessage(JSON.stringify({iframeId: __orb_iframeId__, type: e.type}), "*");
   }

   window.addEventListener('message', function (e) {
      try {
         const msg = JSON.parse(e.data);
         const video = document.getElementById("orb_video");
         console.log("iframe: received message from MediaElementWrapper:", e.data);
         if (msg.name === "initialise") {
            __orb_iframeId__ = msg.args[0];
            console.log("iframe: initialised iframe with id", __orb_iframeId__);
         }
         else if (typeof video[msg.name] === "function") {
            if (msg.name === "addEventListener") {
               if (!video["on" + msg.args[0]]) {
                  video["on" + msg.args[0]] = onEvent;
               }
               window.parent.postMessage(JSON.stringify({callId: msg.callId}), '*');
            }
            else if (msg.name === "removeEventListener") {
               video["on" + msg.args[0]] = null;
               window.parent.postMessage(JSON.stringify({callId: msg.callId}), '*');
            }
            else {
               const result = video[msg.name](...msg.args);
               if (result instanceof Promise) {
                  result.then(() => {
                     window.parent.postMessage(JSON.stringify({callId: msg.callId}), '*');
                  })
                  .catch(e => {
                     window.parent.postMessage(JSON.stringify({callId: msg.callId}), '*');
                  });
               }
               else {
                  window.parent.postMessage(JSON.stringify({callId: msg.callId, result: result}), '*');
               }
            }
         }
         else {
            if (msg.args.length) {
               video[msg.name] = msg.args[0];
            }
            window.parent.postMessage(JSON.stringify({callId: msg.callId, result: video[msg.name]}), '*');
         }
      }
      catch (e) {
         console.warn(e);
      }
   });
})();
