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
 * CMS SignedData Verifier Interface
 *
 * Implements signature verification as per TS 103 606 Section 11.3.4.5
 */
#ifndef I_VERIFIER_H
#define I_VERIFIER_H

#include <string>
#include <filesystem>

namespace orb
{

/**
 * @brief Interface for CMS SignedData verification.
 *
 * Allows mock implementations for testing.
 */
class IVerifier {
public:
    virtual ~IVerifier() = default;

    /**
     * Verify a CMS SignedData file and extract the ZIP content.
     *
     * Implements TS 103 606 Section 11.3.4.5:
     * - Verifies certificate chain against Operator Signing Root CA
     * - Validates O= and CN= attributes match expected values
     * - Verifies message-digest matches content hash
     * - Extracts ZIP from encapContentInfo
     *
     * @param signedDataPath Path to the CMS SignedData file (DER encoded)
     * @param outZipPath Output path where the extracted ZIP is written
     * @param outError Output error message if verification fails
     * @return true if verification succeeded, false otherwise
     */
    virtual bool verify(
        const std::filesystem::path& signedDataPath,
        std::filesystem::path& outZipPath,
        std::string& outError) const = 0;

    /**
     * Check if the verifier is properly configured.
     *
     * @param outError Optional pointer to receive error message describing missing fields
     * @return true if all required fields are set
     */
    virtual bool isConfigured(std::string* outError = nullptr) const = 0;
};

} // namespace orb

#endif /* I_VERIFIER_H */

