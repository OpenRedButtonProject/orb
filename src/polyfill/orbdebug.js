/**
 * @fileOverview Special ORB methods for debug builds.
 * @license ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

 hbbtv.orbDebug = (function() {
   const exported = {};

   exported.polyfill = function() {
      if (!hbbtv.native.isDebugBuild()) {
         return;
      }
      window.orbDebug = {};
      window.orbDebug.publishTestReport = function(testSuite, xml) {
         hbbtv.bridge.orbDebug.publishTestReport(testSuite, xml);
      };
   };

   return exported;
 })();