/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
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
#pragma once

#include <string>

namespace orb
{
/**
 * @brief orb::SHA256
 *
 * Implementation of the SHA256 encryption algorithm.
 */
class SHA256 {
public:

    /**
     * @brief SHA256::Encrypt
     *
     * Encrypt the specified data using the SHA256 algorithm.
     *
     * @param data The data to be encrypted
     *
     * @return The encrypted data
     */
    static std::string Encrypt(std::string data);
}; // class SHA256
} // namespace orb
