//(function () {
   function onDocumentLoaded() {
      console.log("iframe: loaded...");
     // window.document.removeEventListener("load", onDocumentLoaded);
      const video = document.getElementById("orb_video");
      video.audioTracks = hbbtv.objects.createAudioTrackList();
      video.videoTracks = hbbtv.objects.createVideoTrackList();
      const videoProxy = hbbtv.objects.createIFrameObjectProxy(video, "video");
      video.addEventListener("loadeddata", () => {
         videoProxy.setRemoteObjectProperty("duration", video.duration);
      });
      video.addEventListener("durationchanged", () => {
         videoProxy.setRemoteObjectProperty("duration", video.duration);
      });
   }
   //window.document.addEventListener("load", onDocumentLoaded);
   // let __orb_iframeId__ = -1;

   // function onEvent(e) {
   //    window.parent.postMessage(JSON.stringify({callId: -1, iframeId: __orb_iframeId__, type: e.type}), "*");
   // }

   // function makePropertiesUpdateCallback(video, propertyNames) {
   //    return function() {
   //       const properties = [];
   //       for (const prop of propertyNames) {
   //          const value = typeof video[prop].toJSON === "function" ? video[prop].toJSON() : video[prop];
   //          properties.push({name: prop, value: value});
   //       }
   //       window.parent.postMessage(JSON.stringify({
   //          callId: -2,
   //          iframeId: __orb_iframeId__,
   //          properties: properties
   //       }), '*');
   //    }
   // }

   // window.addEventListener('message', function (e) {
   //    try {
   //       const msg = JSON.parse(e.data);
   //       const video = document.getElementById("orb_video");
   //       console.log("iframe: received message from MediaElementWrapper:", e.data);
   //       if (msg.name === "initialise") {
   //          __orb_iframeId__ = msg.args[0];
   //          console.log("iframe: initialised iframe with id", __orb_iframeId__);
   //          const propsUpdateCallback = makePropertiesUpdateCallback(video, ["paused", "ended", "duration"]);
   //          video.addEventListener("loadeddata", makePropertiesUpdateCallback(video, ["paused", "ended", "duration", "videoTracks", "audioTracks", "textTracks"]));
   //          video.addEventListener("timeupdate", makePropertiesUpdateCallback(video, ["currentTime"]));
   //          video.addEventListener("durationchanged", makePropertiesUpdateCallback(video, ["duration"]));
   //          video.addEventListener("play", propsUpdateCallback);
   //          video.addEventListener("ended", propsUpdateCallback);
   //          video.addEventListener("pause", propsUpdateCallback);
   //       }
   //       else if (typeof video[msg.name] === "function") {
   //          if (msg.name === "addEventListener") {
   //             if (!video["on" + msg.args[0]]) {
   //                video["on" + msg.args[0]] = onEvent;
   //             }
   //          }
   //          else if (msg.name === "removeEventListener") {
   //             video["on" + msg.args[0]] = null;
   //          }
   //          else if (msg.name === "appendChild") {
   //             video.innerHTML += msg.args[0];
   //          }
   //          else {
   //             const result = video[msg.name](...msg.args);
   //             if (result instanceof Promise) {
   //                result.then(() => {
   //                   window.parent.postMessage(JSON.stringify({callId: msg.callId}), '*');
   //                })
   //                .catch(e => {
   //                   window.parent.postMessage(JSON.stringify({callId: msg.callId, error: e}), '*');
   //                });
   //             }
   //             else if (msg.name === "setAttribute") {
   //                const val = video.getAttribute(msg.args[0]);
   //                // in case setAttribute fails to set the requested value,
   //                // send back the actual value for the same attribute 
   //                if (msg.args[1] !== val) {
   //                   window.parent.postMessage(JSON.stringify({
   //                      callId: -3,
   //                      iframeId: __orb_iframeId__,
   //                      attribute: {name: msg.args[0], value: val}
   //                   }), '*');
   //                }
   //             }
   //          }
   //       }
   //       else {
   //          switch (msg.name) {
   //             case "videoTracks":
   //             case "audioTracks":
   //             case "textTracks":
   //                video[msg.name][msg.args[0]][msg.args[1]] = msg.args[2];
   //                break;
   //             default:
   //                video[msg.name] = msg.args[0];
   //                if (video[msg.name] !== msg.args[0]) {
   //                   // in case the property fails to update to the requested value,
   //                   // send back the actual value for the this property 
   //                   window.parent.postMessage(JSON.stringify({
   //                      callId: -2,
   //                      iframeId: __orb_iframeId__,
   //                      properties: [{name: msg.name, value: video[msg.name]}]
   //                   }), '*');
   //                }
   //                break;
   //          }
   //       }
   //    }
   //    catch (e) {
   //       console.warn(e);
   //    }
   // });
//})();
