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
 * CMS SignedData Verifier
 *
 * Implements signature verification of CMS SignedData structures as per:
 * - ETSI TS 103 606 Section 11.3.4.5 (Application ZIP package signature verification)
 * - IETF RFC 5652 Section 5.1 (SignedData)
 * - IETF RFC 5280 Section 6 (Certificate Path Validation)
 *
 * Verification process:
 * 1. Parse CMS SignedData structure
 * 2. Verify certificate chain against Operator Signing Root CA
 * 3. Validate Operator Name (O=) and organisation_id (CN=) against bilateral agreement
 * 4. Verify message-digest matches hash of extracted content
 * 5. Verify signature over signed attributes
 * 6. Extract application ZIP from encapContentInfo
 */
#ifndef VERIFIER_H
#define VERIFIER_H

#include <filesystem>
#include <string>
#include <vector>
#include "IVerifier.h"

namespace orb
{

/**
 * @brief Configuration for the CMS SignedData Verifier.
 *
 * Contains the Operator Signing Root CA and expected operator identity
 * as required by TS 103 606 Section 11.3.4.5.
 *
 * All fields except workingDirectory and enableCRLCheck are REQUIRED.
 * The verifier will report "not configured" if any required field is empty.
 */
struct VerifierConfig {
    /** Path to the Operator Signing Root CA certificate (PEM format)
     *  Used to verify the certificate chain in the SignedData
     *  REQUIRED */
    std::filesystem::path operatorRootCAPath;

    /** Expected Operator Name from bilateral agreement
     *  Matched against the Organization (O=) attribute of the signer certificate subject
     *  REQUIRED */
    std::string expectedOperatorName;

    /** Expected organisation_id from bilateral agreement
     *  Matched against the CommonName (CN=) attribute of the signer certificate subject
     *  REQUIRED */
    std::string expectedOrganisationId;

    /** Working directory for extracted ZIP output */
    std::filesystem::path workingDirectory;

    /** Enable CRL checking for certificate revocation (default: false)
     *  NOTE: CRL checking may add latency due to network fetches */
    bool enableCRLCheck = false;
};

/**
 * @brief CMS SignedData Verifier implementation.
 *
 * Verifies signatures on CMS SignedData structures and extracts the content.
 *
 * Implementation notes:
 * - For Chromium builds (IS_CHROMIUM defined): Uses BoringSSL primitives with manual
 *   CMS ASN.1 parsing since BoringSSL does not support CMS.
 * - For non-Chromium builds: Uses OpenSSL's CMS_verify() API directly.
 */
class Verifier : public IVerifier {
public:
    /**
     * @brief Construct a Verifier with no configuration.
     *
     * Verification will fail until configuration is provided.
     */
    Verifier();

    /**
     * @brief Construct a Verifier with the given configuration.
     *
     * @param config Configuration containing Root CA and expected operator identity
     */
    explicit Verifier(const VerifierConfig& config);

    /**
     * @brief Set or update the verifier configuration.
     *
     * @param config Configuration containing Root CA and expected operator identity
     */
    void setConfig(const VerifierConfig& config);

    /**
     * @brief Check if the verifier is properly configured.
     *
     * @param outError Optional pointer to receive error message describing missing fields
     * @return true if all required fields are set
     */
    bool isConfigured(std::string* outError = nullptr) const override;

    /**
     * @brief Verify CMS SignedData and extract ZIP content.
     *
     * @param signedDataPath Path to the CMS SignedData file (DER encoded)
     * @param outZipPath Output path where the extracted ZIP is written
     * @param outError Error message if verification fails
     * @return true on success, false on failure
     */
    bool verify(
        const std::filesystem::path& signedDataPath,
        std::filesystem::path& outZipPath,
        std::string& outError) const override;

private:
    VerifierConfig m_Config;

#ifdef IS_CHROMIUM
    // BoringSSL implementation helpers
    bool verifyWithBoringSSL(
        const std::vector<uint8_t>& signedData,
        std::vector<uint8_t>& outContent,
        std::string& outError) const;

    bool parseSignedData(
        const uint8_t* data,
        size_t len,
        std::vector<std::vector<uint8_t>>& outCertificates,
        std::vector<uint8_t>& outContent,
        std::vector<uint8_t>& outSignature,
        std::vector<uint8_t>& outSignedAttrs,
        std::vector<uint8_t>& outMessageDigest,
        int& outDigestAlgorithm,
        std::string& outError) const;

    bool verifyCertificateChain(
        const std::vector<std::vector<uint8_t>>& certificates,
        std::string& outOperatorName,
        std::string& outOrganisationId,
        std::string& outError) const;

    bool verifySignature(
        const std::vector<uint8_t>& signedAttrs,
        const std::vector<uint8_t>& signature,
        const std::vector<uint8_t>& signerCertDer,
        int digestAlgorithm,
        std::string& outError) const;

    bool verifyMessageDigest(
        const std::vector<uint8_t>& content,
        const std::vector<uint8_t>& expectedDigest,
        int digestAlgorithm,
        std::string& outError) const;
#else
    // OpenSSL CMS implementation
    bool verifyWithOpenSSL(
        const std::vector<uint8_t>& signedData,
        std::vector<uint8_t>& outContent,
        std::string& outError) const;
#endif
};

} // namespace orb

#endif /* VERIFIER_H */

