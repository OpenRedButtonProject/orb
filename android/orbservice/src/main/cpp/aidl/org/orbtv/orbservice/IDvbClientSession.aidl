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
/* IDvbClientSession.aidl - interface to DVB Integration from ORB Moderator */

package org.orbtv.orbservice;

interface IDvbClientSession {

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

}
