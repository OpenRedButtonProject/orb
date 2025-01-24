/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 * <p>
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
/* IOrbcSession.aidl - interface to ORB C++ from DVB integration */

package org.orbtv.orbservice;

import org.orbtv.orbservice.IBrowserSession;

interface IBridgeSession {

    /**
     * Iniitialise interface and supply the callback interface
     */
    void initialise(in IBrowserSession browser);

    /**
     * Execute the given bridge request.
     * The request is a string representation of a JSON object with the following form:
     * {
     *    "token": <token>
     *    "method": <method>
     *    "params": <params>
     * }
     * The response is also a string representation of a JSON object containing the results, if any.
     * @param jsonRequest byte array as a string representation of the JSON request
     * @return A byte array as a string representation of the JSON response
     */
    byte[] executeRequest(in byte[] jsonRequest);

    /**
     * Convert Android key code to Tv Browser key code, and
     * check whether the TV key code is in the key set for the application.
     *
     * @param androidKeyCode The Android key code (KeyEvent.KEYCODE_*)
     * @param appId   The application ID.
     * @return Tv Browser Key code (VK_*), or zero (VK_INVALID) if key not in App KeySet
     */
    int getTvKeyCodeForApp(int androidKeyCode, int appId);

    /**
     * Notify the application manager that a call to loadApplication failed.
     *
     * @param appId The application ID of the application that failed to load.
     */
    void notifyLoadApplicationFailed(int appId);

    /**
     * Notify the application manager of application page changed, before the new page is
     * loaded. For example, when the user follows a link.
     *
     * @param appId The application ID.
     * @param url   The URL of the new page.
     */
    void notifyApplicationPageChanged(int appId, in byte[] url);

    /**
     * Load DSM-CC file with specified DVB URL.
     * Content is returned IBrowserSession.receiveDsmccContent
     *
     * @param url The DVB URL
     * @param requestId The distinctive request id
     */
    void LoadDsmccDvbUrl(in byte[] url, int requestId);
}
