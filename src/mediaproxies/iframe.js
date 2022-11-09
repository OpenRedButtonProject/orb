function onDocumentLoaded() {
   console.log("iframe: loaded...");
   // window.document.removeEventListener("load", onDocumentLoaded);
   const video = document.getElementById("orb_video");
   const videoProxy = hbbtv.objects.createIFrameObjectProxy(video, "video");
   const genericEvents = [
      "loadstart", "progress", "suspend", "abort", "error", "emptied", "stalled", "loadedmetadata", "canplay",
      "canplaythrough", "playing", "waiting", "seeking", "seeked", "resize"
   ];
   const genericHandler = (e) => {
      videoProxy.dispatchEvent(e.type);
   };
   for (const evt of genericEvents) {
      video.addEventListener(evt, genericHandler);
   }
   const propsUpdateCallback = function (e) {
      const props = { };
      const keys = Object.getOwnPropertyNames(HTMLMediaElement.prototype);
      for (const key of keys) {
         if (typeof video[key] !== "function") {
            props[key] = video[key];
         }
      }
      videoProxy.setRemoteObjectProperties(props);
      videoProxy.dispatchEvent(e.type);
      console.log("iframe: update properties", props);
   };
   const makeCallback = function(property) {
      return function (e) {
         videoProxy.setRemoteObjectProperties({[property]: video[property]});
         videoProxy.dispatchEvent(e.type);
      }
   }
   video.addEventListener("loadeddata", propsUpdateCallback);
   video.addEventListener("play", propsUpdateCallback);
   video.addEventListener("ended", propsUpdateCallback);
   video.addEventListener("pause", propsUpdateCallback);
   video.addEventListener("durationchanged", makeCallback("duration"));
   video.addEventListener("timeupdate", makeCallback("currentTime"));
   video.addEventListener("ratechange", makeCallback("playbackRate"));
   video.addEventListener("volumechange", makeCallback("volume"));
}
