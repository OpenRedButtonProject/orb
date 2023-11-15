/**
 * @fileOverview Special ORB methods for debug builds.
 * @license ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
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
        window.orbDebug.isFeatureEnabled = function(name) {
            return hbbtv.native.isFeatureEnabled(name);
        };
    };

    return exported;
})();