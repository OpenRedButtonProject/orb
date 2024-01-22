const orbtest = (function (){
   const exported = {};
   let testsuite_name = "";
   let testsuite_started;
   let testcases = [];
   let testcase_started;
   let testcase_index = -1;
   let testcases_passed = 0;
   let testcases_failed = 0;
   let xml_document = document.implementation.createDocument(null, "testsuite");
   let xml_testsuite = xml_document.getElementsByTagName("testsuite")[0];

   exported.add = function(testcase) {
      testcase.number = testcases.length + 1;
      testcases.push(testcase);
   }

   exported.run = function(name) {
      if (testcase_index != -1) {
         return;
      }
      testsuite_name = name;
      console.log(`Run testsuite: "${testsuite_name}" (total: ${testcases.length})`);
      xml_testsuite.setAttribute("name", testsuite_name);
      xml_testsuite.setAttribute("tests", testcases.length);
      testsuite_started = performance.now();
      step();
   }

   function step() {
      if (++testcase_index >= testcases.length) {
         return false;
      }
      testcase_started = performance.now();
      const testcase = testcases[testcase_index];
      try {
         console.log(`[${testcase.number}/${testcases.length}] Run testcase: "${testcase.name}", ` +
            `description: "${testcase.description}"`);
         testcase.test({
            pass: (message) => {
               result(testcase, {pass: true, message});
            },
            fail: (message) => {
               result(testcase, {pass: false, message});
            }
         });
      } catch (err) {
         result(testcase, {pass: false, message: err.message});
      }
      return true;
   }
   
   function result(testcase, result) {
      const ms = performance.now() - testcase_started;
      const xml_testcase = xml_document.createElement("testcase");
      xml_testcase.setAttribute("classname", testsuite_name);
      xml_testcase.setAttribute("name", testcase.name);
      if (result.pass) {
         testcases_passed++;
         console.log(`[${testcase.number}/${testcases.length}] Passed: "${testcase.name}", ` +
            `message: "${result.message}" (time ${ms}ms)`);
      } else {
         testcases_failed++;
         console.log(`[${testcase.number}/${testcases.length}] Failed: "${testcase.name}", ` +
            `message: "${result.message}" (time ${ms}ms)`);
         const xml_failure = xml_document.createElement("failure");
         xml_failure.setAttribute("type", "Failed");
         xml_failure.textContent = result.message;
         xml_testcase.appendChild(xml_failure);
      }
      xml_testsuite.appendChild(xml_testcase);
      if (!step()) {
         finish();
      }
   }

   function finish() {
      const ms = performance.now() - testsuite_started;
      console.log(`Finished testsuite "${testsuite_name}" ` +
         `(passed: ${testcases_passed}, failed: ${testcases_failed}, total time: ${ms}ms)`);
      const xml = new XMLSerializer().serializeToString(xml_document);
      if (window.orbDebug !== undefined) {
         window.orbDebug.publishTestReport(testsuite_name, xml);
      }
   }

   return exported;
})();
