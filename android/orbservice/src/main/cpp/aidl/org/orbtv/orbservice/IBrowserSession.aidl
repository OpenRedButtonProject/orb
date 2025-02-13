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
/* IDvbiSession.aidl - interface to DVB Integration from ORB C++ */

package org.orbtv.orbservice;

interface IBrowserSession {

    /**
     * Called to Tell the browser to dispatch an key press event.
     *
     * @param action, Key action
     * @param a_code The android Key Code
     * @param tv_code The TV Key Code
     */
    boolean dispatchKeyEvent(int action, int a_code, int tv_code);

    /**
     * Tell the browser to load an application. If the entry page fails to load, the browser
     * should call IBridgeSession.onLoadApplicationFailed.
     *
     * @param appId The application ID.
     * @param entryUrl The entry page URL.
     * @param graphics The list of the co-ordinate graphics supported by the application (HbbTV-204)
     */
    void loadApplication(int appid, in byte[] url, in int[] graphic_ids);

    /**
     * Tell the browser to show the application.
     */
    void showApplication();

    /**
     * Tell the browser to hide the application.
     */
    void hideApplication();

    /**
     * Tell the browser to dispatch an event
     *
     * @param type The event type.
     * @param properties A name/value map of event properties in a JSON string.
     */
    void dispatchEvent(in byte[] type, in byte[] properties);

    /**
     * Called to Tell the browser to dispatch an text input.
     *
     * @param text The content of the text input
     */
    void dispatchTextInput(in byte[] text);

    /**
     * Provide received content of DSM-CC file requested by IBridgeSession.LoadDsmccDvbUrl
     *
     * @param requestId The distinctive request id
     * @param content The file data
     */
    void receiveDsmccContent(int requestId, in byte[] content);

}
