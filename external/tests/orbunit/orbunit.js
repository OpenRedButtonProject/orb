const orbunit = (function (){
   let gBanner = null;
   let gApplicationManager = null;
   let gCompletedCallback = null;
   let gTests = [];
   let gIndex = -1;
   let gResolved = 0;
   let gParallelize = false;

   function init(props = {}) {
      gBanner = document.createElement("div");
      gBanner.className = "orbunit-banner";
      gBanner.innerHTML =
         "<p class='results'></p>" +
         "<hr/>" +
         "<p><strong class='title'></strong></p>" +
         "<p class='description'></p>";

      banner(props);
      document.body.prepend(gBanner);
      
      gApplicationManager = document.createElement("object")
      gApplicationManager.type = "application/oipfapplicationmanager";
      const app = gApplicationManager.getOwnerApplication(document);
      if (props.keyset) {
         app.privateData.keyset.setValue(props.keyset);
      }
      if (props.showApp) {
         app.show();
      }
   }

   function banner(props = {}) {
      if (props.title) {
         gBanner.querySelector(".title").innerText = props.title;
      }
      if (props.description) {
         gBanner.querySelector(".description").innerText = props.description;
      }
   }

   function add(test) {
      test.index = gTests.length;
      test.elem = document.createElement("p");
      test.elem.style.color = "#ffffff";
      test.elem.innerText = "[" + test.index + "] [....] " + test.title + " []";
      gBanner.querySelector(".results").append(test.elem);
      gTests.push(test);
   }

   function run(props = {}) {
      if (gIndex != -1) {
         return;
      }
      if (props.parallelize) {
         gParallelize = true;
         gBanner.querySelector(".title").innerText = "Running " + gTests.length +
            " tests in parallel.";
         gBanner.querySelector(".description").innerText = "";
         while (step()) {
         }
      } else {
         step();
      }
   }

   function step() {
      if (++gIndex < gTests.length) {
         const test = gTests[gIndex];
         if (gParallelize) {
            gBanner.querySelector(".description").innerHTML += "<p>[" + test.index + "] "
               + test.title + " [" + test.description + "]</p>";
         } else {
            gBanner.querySelector(".title").innerText = "Running: " + test.title;
            gBanner.querySelector(".description").innerText = test.description;
         }
         try {
            test.test({
               pass: (message) => {
                  result(test, {pass: true, message});
               },
               fail: (message) => {
                  result(test, {pass: false, message});
               }
            });
         } catch (err) {
            result(test, {pass: false, message: err.message});
         }
      }
      return (gIndex < gTests.length);
   }
   
   function result(test, result) {
      gResolved++;
      test.elem.style.backgroundColor = result.pass ? "#008800" : "#880000";
      if (result.pass) {
         test.elem.innerText = "[" + test.index + "] [PASS] " + test.title +
            " [" + result.message + "]";
      } else {
         test.elem.innerText = "[" + test.index + "] [FAIL] " + test.title +
            " [" + result.message + "]";
      }
      if (gResolved == gTests.length && gCompletedCallback) {
         gCompletedCallback();
      }

      if (!gParallelize) {
         step();
      }
   }

   const api = {
      init: init,
      banner: banner,
      add: add,
      run: run,
      KEYSET_RED: 0x1,
      KEYSET_GREEN: 0x2,
      KEYSET_YELLOW: 0x4,
      KEYSET_BLUE: 0x8,
      KEYSET_NAVIGATION: 0x10,
      KEYSET_VCR: 0x20,
      KEYSET_NUMERIC: 0x100,
      KEYSET_OTHER: 0x400
   }

   Object.defineProperty(api, "oncompleted", {
      get() { return gCompletedCallback; },
      set(callback) { gCompletedCallback = callback; },
      enumerable: true
    });

   return api;
})();
