/**
 * @license ORB Software. Copyright (c) 2026 Ocean Blue Software Limited
 *
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

(function() {
    function onTextInputFocused(e) {
        hbbtv.native.request('SoftKeyboard.show');
    }
    const observer = new MutationObserver(function(mutationsList) {
        for (const mutation of mutationsList) {
            for (const node of mutation.removedNodes) {
                if (node.nodeName && node.nodeName.toLowerCase() === 'input') {
                    node.removeEventListener('focus', onTextInputFocused);
                }
            }
            for (const node of mutation.addedNodes) {
                if (node.nodeName && node.nodeName.toLowerCase() === 'input' && node.type === 'text') {
                    node.addEventListener('focus', onTextInputFocused);
                }
            }
        }
    });
    const config = {
        childList: true,
        subtree: true,
    };
    observer.observe(document.documentElement || document.body, config);
})();
