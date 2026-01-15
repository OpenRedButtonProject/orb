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
 *
 * CMS EnvelopedData Decryptor Interface
 *
 * Implements decryption as per TS 103 606 Section 11.3.4.4
 */
#ifndef I_DECRYPTOR_H
#define I_DECRYPTOR_H

#include <string>
#include <filesystem>

namespace orb
{

/**
 * @brief Interface for CMS EnvelopedData decryption.
 *
 * Allows mock implementations for testing.
 */
class IDecryptor {
public:
    virtual ~IDecryptor() = default;

    /**
     * Decrypt a CMS EnvelopedData package file.
     *
     * @param filePath Path to the encrypted package file (CMS EnvelopedData, DER encoded)
     * @param outFile Output decrypted file path (CMS SignedData)
     * @param outError Output error message if decryption fails
     * @return true if decryption succeeded, false otherwise
     */
    virtual bool decrypt(
        const std::filesystem::path& filePath,
        std::filesystem::path& outFile,
        std::string& outError) const = 0;
};

} // namespace orb

#endif /* I_DECRYPTOR_H */

