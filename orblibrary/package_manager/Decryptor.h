/**
 * ORB Software. Copyright (c) 2026 Ocean Blue Software Limited
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
 * CMS EnvelopedData Decryptor
 *
 * Implements decryption of CMS EnvelopedData structures as per:
 * - ETSI TS 103 606 Section 11.3.4.4 (Process for decrypting an application package)
 * - IETF RFC 5652 Section 6.2 (EnvelopedData)
 *
 * Supported algorithms:
 * - Key encryption: RSA (RSAES-PKCS1-v1_5 or RSAES-OAEP)
 * - Content encryption: AES-128-CBC, AES-256-CBC
 */
#ifndef DECRYPTOR_H
#define DECRYPTOR_H

#include "OpAppPackageManager.h"
#include <filesystem>
#include <string>
#include <vector>

namespace orb
{

/**
 * @brief Configuration for the CMS Decryptor.
 *
 * Contains paths to the Terminal Packaging Certificate and its private key
 * as required by TS 103 606 Section 11.3.4.4.
 */
struct DecryptorConfig {
    /** Path to the Terminal Packaging Certificate private key (PEM format) */
    std::filesystem::path privateKeyPath;

    /** Path to the Terminal Packaging Certificate (PEM or DER format) */
    std::filesystem::path certificatePath;

    /** Working directory for temporary decrypted files */
    std::filesystem::path workingDirectory;
};

/**
 * @brief CMS EnvelopedData Decryptor
 *
 * Decrypts CMS EnvelopedData structures containing encrypted operator application packages.
 *
 * Implementation notes:
 * - For Chromium builds (IS_CHROMIUM defined): Uses BoringSSL primitives with manual
 *   CMS ASN.1 parsing since BoringSSL does not support CMS.
 * - For non-Chromium builds: Uses OpenSSL's CMS API directly.
 */
class Decryptor : public IDecryptor {
public:
    /**
     * @brief Construct a Decryptor with no configuration.
     *
     * Decryption will fail until configuration is provided.
     */
    Decryptor();

    /**
     * @brief Construct a Decryptor with the given configuration.
     *
     * @param config Configuration containing key and certificate paths
     */
    explicit Decryptor(const DecryptorConfig& config);

    /**
     * @brief Set or update the decryptor configuration.
     *
     * @param config Configuration containing key and certificate paths
     */
    void setConfig(const DecryptorConfig& config);

    /**
     * @brief Check if the decryptor is properly configured.
     *
     * @return true if private key and certificate paths are set
     */
    bool isConfigured() const;

    /**
     * @brief Decrypt a CMS EnvelopedData package.
     *
     * Implements the decryption process defined in TS 103 606 Section 11.3.4.4:
     * 1. Parse the CMS EnvelopedData structure
     * 2. Find the RecipientInfo matching our certificate
     * 3. Decrypt the EncryptedKey using the private key
     * 4. Decrypt the encryptedContent using the decrypted key
     *
     * @param filePath Path to the encrypted CMS package file (DER encoded)
     * @param outFile Output path where the decrypted content is written
     * @param outError Error message if decryption fails
     * @return true on success, false on failure
     */
    bool decrypt(
        const std::filesystem::path& filePath,
        std::filesystem::path& outFile,
        std::string& outError) const override;

private:
    DecryptorConfig m_Config;

#ifdef IS_CHROMIUM
    // BoringSSL implementation helpers
    bool decryptWithBoringSSL(
        const std::vector<uint8_t>& cmsData,
        std::vector<uint8_t>& outDecrypted,
        std::string& outError) const;

    bool parseEnvelopedData(
        const uint8_t* data,
        size_t len,
        std::vector<uint8_t>& outEncryptedKey,
        std::vector<uint8_t>& outEncryptedContent,
        std::vector<uint8_t>& outIV,
        int& outKeySize,
        std::string& outError) const;

    bool decryptKey(
        const std::vector<uint8_t>& encryptedKey,
        std::vector<uint8_t>& outKey,
        std::string& outError) const;

    bool decryptContent(
        const std::vector<uint8_t>& encryptedContent,
        const std::vector<uint8_t>& key,
        const std::vector<uint8_t>& iv,
        std::vector<uint8_t>& outContent,
        std::string& outError) const;
#else
    // OpenSSL CMS implementation
    bool decryptWithOpenSSL(
        const std::vector<uint8_t>& cmsData,
        std::vector<uint8_t>& outDecrypted,
        std::string& outError) const;
#endif
};

} // namespace orb

#endif /* DECRYPTOR_H */
