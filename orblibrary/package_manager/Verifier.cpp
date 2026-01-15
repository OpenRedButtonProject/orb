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
 */

#include "Verifier.h"
#include <fstream>
#include <sstream>
#include <cstring>

#ifdef IS_CHROMIUM
// BoringSSL headers
#include "third_party/boringssl/src/include/openssl/bio.h"
#include "third_party/boringssl/src/include/openssl/evp.h"
#include "third_party/boringssl/src/include/openssl/pem.h"
#include "third_party/boringssl/src/include/openssl/x509.h"
#include "third_party/boringssl/src/include/openssl/x509_vfy.h"
#include "third_party/boringssl/src/include/openssl/err.h"
#include "third_party/boringssl/src/include/openssl/bytestring.h"
#include "third_party/boringssl/src/include/openssl/obj.h"
#include "third_party/boringssl/src/include/openssl/sha.h"
#include "third_party/boringssl/src/include/openssl/rsa.h"
#else
// OpenSSL headers (with CMS support)
#include <openssl/bio.h>
#include <openssl/cms.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/x509_vfy.h>
#include <openssl/err.h>
#endif

namespace orb
{

namespace {
    std::string getOpenSSLError() {
        char buf[256];
        ERR_error_string_n(ERR_get_error(), buf, sizeof(buf));
        return std::string(buf);
    }

#ifdef IS_CHROMIUM
    // OID for signed-data: 1.2.840.113549.1.7.2
    const uint8_t OID_SIGNED_DATA[] = {0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x07, 0x02};

    // OID for sha256: 2.16.840.1.101.3.4.2.1
    const uint8_t OID_SHA256[] = {0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x01};

    // OID for sha384: 2.16.840.1.101.3.4.2.2
    const uint8_t OID_SHA384[] = {0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x02};

    // OID for sha512: 2.16.840.1.101.3.4.2.3
    const uint8_t OID_SHA512[] = {0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x03};

    // OID for messageDigest: 1.2.840.113549.1.9.4
    const uint8_t OID_MESSAGE_DIGEST[] = {0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x09, 0x04};

    // Digest algorithm constants
    constexpr int DIGEST_SHA256 = 1;
    constexpr int DIGEST_SHA384 = 2;
    constexpr int DIGEST_SHA512 = 3;

    bool compareOID(const uint8_t* oid1, size_t len1, const uint8_t* oid2, size_t len2) {
        if (len1 != len2) return false;
        return memcmp(oid1, oid2, len1) == 0;
    }
#endif // IS_CHROMIUM
} // anonymous namespace

Verifier::Verifier() = default;

Verifier::Verifier(const VerifierConfig& config)
    : m_Config(config)
{
}

void Verifier::setConfig(const VerifierConfig& config)
{
    m_Config = config;
}

bool Verifier::isConfigured(std::string* outError) const
{
    std::string missing;
    if (m_Config.operatorRootCAPath.empty()) {
        missing += "operatorRootCAPath, ";
    }
    if (m_Config.expectedOperatorName.empty()) {
        missing += "expectedOperatorName, ";
    }
    if (m_Config.expectedOrganisationId.empty()) {
        missing += "expectedOrganisationId, ";
    }

    if (!missing.empty()) {
        if (outError) {
            missing = missing.substr(0, missing.length() - 2); // Remove trailing ", "
            *outError = "Verifier not configured: missing " + missing;
        }
        return false;
    }
    return true;
}

bool Verifier::verify(
    const std::filesystem::path& signedDataPath,
    std::filesystem::path& outZipPath,
    std::string& outError) const
{
    // Validate configuration - all fields are required per TS 103 606 Section 11.3.4.5
    if (!isConfigured(&outError)) {
        return false;
    }

    // Check input file exists
    if (!std::filesystem::exists(signedDataPath)) {
        outError = "Input file does not exist: " + signedDataPath.string();
        return false;
    }

    // Check Root CA file exists
    if (!std::filesystem::exists(m_Config.operatorRootCAPath)) {
        outError = "Operator Root CA file does not exist: " + m_Config.operatorRootCAPath.string();
        return false;
    }

    // Read the SignedData file
    std::ifstream inFile(signedDataPath, std::ios::binary);
    if (!inFile) {
        outError = "Failed to open input file: " + signedDataPath.string();
        return false;
    }

    std::vector<uint8_t> signedData(
        (std::istreambuf_iterator<char>(inFile)),
        std::istreambuf_iterator<char>());
    inFile.close();

    if (signedData.empty()) {
        outError = "Input file is empty: " + signedDataPath.string();
        return false;
    }

    // Verify and extract content
    std::vector<uint8_t> content;
#ifdef IS_CHROMIUM
    bool result = verifyWithBoringSSL(signedData, content, outError);
#else
    bool result = verifyWithOpenSSL(signedData, content, outError);
#endif
    if (!result)
    {
        return false;
    }

    // Determine output path
    std::filesystem::path outputPath;
    std::string filename = signedDataPath.stem().string() + ".zip";
    if (!m_Config.workingDirectory.empty()) {
        std::filesystem::create_directories(m_Config.workingDirectory);
        outputPath = m_Config.workingDirectory / filename;
    } else {
        outputPath = signedDataPath.parent_path() / filename;
    }

    // Write extracted content to file
    std::ofstream outFileStream(outputPath, std::ios::binary);
    if (!outFileStream) {
        outError = "Failed to create output file: " + outputPath.string();
        return false;
    }

    outFileStream.write(reinterpret_cast<const char*>(content.data()), content.size());
    outFileStream.close();

    if (outFileStream.fail()) {
        outError = "Failed to write ZIP content to: " + outputPath.string();
        return false;
    }

    outZipPath = outputPath;
    return true;
}

#ifdef IS_CHROMIUM
// BoringSSL implementation - manual CMS SignedData parsing

bool Verifier::verifyWithBoringSSL(
    const std::vector<uint8_t>& signedData,
    std::vector<uint8_t>& outContent,
    std::string& outError) const
{
    std::vector<std::vector<uint8_t>> certificates;
    std::vector<uint8_t> content;
    std::vector<uint8_t> signature;
    std::vector<uint8_t> signedAttrs;
    std::vector<uint8_t> messageDigest;
    int digestAlgorithm = 0;

    // Parse the CMS SignedData structure
    if (!parseSignedData(signedData.data(), signedData.size(),
                        certificates, content, signature, signedAttrs,
                        messageDigest, digestAlgorithm, outError)) {
        return false;
    }

    if (certificates.empty()) {
        outError = "No certificates found in SignedData";
        return false;
    }

    // Verify certificate chain and extract operator identity
    std::string operatorName;
    std::string organisationId;
    if (!verifyCertificateChain(certificates, operatorName, organisationId, outError)) {
        return false;
    }

    // Validate operator identity against bilateral agreement (required)
    // Note: Error messages don't reveal expected values (security best practice)
    if (operatorName != m_Config.expectedOperatorName) {
        outError = "Operator Name mismatch: got '" + operatorName + "'";
        return false;
    }

    if (organisationId != m_Config.expectedOrganisationId) {
        outError = "Organisation ID mismatch: got '" + organisationId + "'";
        return false;
    }

    // Verify message digest
    if (!verifyMessageDigest(content, messageDigest, digestAlgorithm, outError)) {
        return false;
    }

    // Verify signature over signed attributes
    // Try each certificate until we find the signer (the certificate order may vary)
    if (!signedAttrs.empty() && !signature.empty()) {
        bool signatureVerified = false;
        std::string lastSigError;

        for (const auto& cert : certificates) {
            std::string sigError;
            if (verifySignature(signedAttrs, signature, cert, digestAlgorithm, sigError)) {
                signatureVerified = true;
                break;
            }
            lastSigError = sigError;
        }

        if (!signatureVerified) {
            outError = "Signature verification failed for all certificates: " + lastSigError;
            return false;
        }
    }

    outContent = std::move(content);
    return true;
}

bool Verifier::parseSignedData(
    const uint8_t* data,
    size_t len,
    std::vector<std::vector<uint8_t>>& outCertificates,
    std::vector<uint8_t>& outContent,
    std::vector<uint8_t>& outSignature,
    std::vector<uint8_t>& outSignedAttrs,
    std::vector<uint8_t>& outMessageDigest,
    int& outDigestAlgorithm,
    std::string& outError) const
{
    CBS cbs;
    CBS_init(&cbs, data, len);

    // Parse ContentInfo SEQUENCE
    CBS contentInfo;
    if (!CBS_get_asn1(&cbs, &contentInfo, CBS_ASN1_SEQUENCE)) {
        outError = "Failed to parse ContentInfo SEQUENCE";
        return false;
    }

    // Parse contentType OID
    CBS contentType;
    if (!CBS_get_asn1(&contentInfo, &contentType, CBS_ASN1_OBJECT)) {
        outError = "Failed to parse contentType OID";
        return false;
    }

    // Verify it's signed-data
    if (!compareOID(CBS_data(&contentType), CBS_len(&contentType),
                   OID_SIGNED_DATA, sizeof(OID_SIGNED_DATA))) {
        outError = "ContentInfo is not SignedData";
        return false;
    }

    // Parse [0] EXPLICIT content
    CBS contentWrapper;
    if (!CBS_get_asn1(&contentInfo, &contentWrapper, CBS_ASN1_CONTEXT_SPECIFIC | CBS_ASN1_CONSTRUCTED | 0)) {
        outError = "Failed to parse content wrapper [0]";
        return false;
    }

    // Parse SignedData SEQUENCE
    CBS signedDataSeq;
    if (!CBS_get_asn1(&contentWrapper, &signedDataSeq, CBS_ASN1_SEQUENCE)) {
        outError = "Failed to parse SignedData SEQUENCE";
        return false;
    }

    // Parse version INTEGER
    uint64_t version;
    if (!CBS_get_asn1_uint64(&signedDataSeq, &version)) {
        outError = "Failed to parse SignedData version";
        return false;
    }

    // Parse digestAlgorithms SET
    CBS digestAlgorithms;
    if (!CBS_get_asn1(&signedDataSeq, &digestAlgorithms, CBS_ASN1_SET)) {
        outError = "Failed to parse digestAlgorithms SET";
        return false;
    }

    // Get first digest algorithm
    outDigestAlgorithm = DIGEST_SHA256; // Default
    if (CBS_len(&digestAlgorithms) > 0) {
        CBS algId;
        if (CBS_get_asn1(&digestAlgorithms, &algId, CBS_ASN1_SEQUENCE)) {
            CBS algOid;
            if (CBS_get_asn1(&algId, &algOid, CBS_ASN1_OBJECT)) {
                if (compareOID(CBS_data(&algOid), CBS_len(&algOid),
                              OID_SHA256, sizeof(OID_SHA256))) {
                    outDigestAlgorithm = DIGEST_SHA256;
                } else if (compareOID(CBS_data(&algOid), CBS_len(&algOid),
                                     OID_SHA384, sizeof(OID_SHA384))) {
                    outDigestAlgorithm = DIGEST_SHA384;
                } else if (compareOID(CBS_data(&algOid), CBS_len(&algOid),
                                     OID_SHA512, sizeof(OID_SHA512))) {
                    outDigestAlgorithm = DIGEST_SHA512;
                }
            }
        }
    }

    // Parse encapContentInfo SEQUENCE
    CBS encapContentInfo;
    if (!CBS_get_asn1(&signedDataSeq, &encapContentInfo, CBS_ASN1_SEQUENCE)) {
        outError = "Failed to parse encapContentInfo SEQUENCE";
        return false;
    }

    // Parse eContentType OID
    CBS eContentType;
    if (!CBS_get_asn1(&encapContentInfo, &eContentType, CBS_ASN1_OBJECT)) {
        outError = "Failed to parse eContentType OID";
        return false;
    }

    // Parse eContent [0] EXPLICIT OCTET STRING (if present)
    if (CBS_peek_asn1_tag(&encapContentInfo, CBS_ASN1_CONTEXT_SPECIFIC | CBS_ASN1_CONSTRUCTED | 0)) {
        CBS eContentWrapper;
        if (!CBS_get_asn1(&encapContentInfo, &eContentWrapper,
                        CBS_ASN1_CONTEXT_SPECIFIC | CBS_ASN1_CONSTRUCTED | 0)) {
            outError = "Failed to parse eContent wrapper";
            return false;
        }

        CBS eContent;
        if (!CBS_get_asn1(&eContentWrapper, &eContent, CBS_ASN1_OCTETSTRING)) {
            outError = "Failed to parse eContent OCTET STRING";
            return false;
        }

        outContent.assign(CBS_data(&eContent), CBS_data(&eContent) + CBS_len(&eContent));
    }

    // Parse certificates [0] IMPLICIT (optional)
    if (CBS_peek_asn1_tag(&signedDataSeq, CBS_ASN1_CONTEXT_SPECIFIC | CBS_ASN1_CONSTRUCTED | 0)) {
        CBS certsSet;
        if (!CBS_get_asn1(&signedDataSeq, &certsSet,
                        CBS_ASN1_CONTEXT_SPECIFIC | CBS_ASN1_CONSTRUCTED | 0)) {
            outError = "Failed to parse certificates [0]";
            return false;
        }

        // Extract each certificate
        while (CBS_len(&certsSet) > 0) {
            CBS cert;
            if (!CBS_get_asn1_element(&certsSet, &cert, CBS_ASN1_SEQUENCE)) {
                break;
            }
            outCertificates.emplace_back(CBS_data(&cert), CBS_data(&cert) + CBS_len(&cert));
        }
    }

    // Skip crls [1] IMPLICIT if present
    if (CBS_peek_asn1_tag(&signedDataSeq, CBS_ASN1_CONTEXT_SPECIFIC | CBS_ASN1_CONSTRUCTED | 1)) {
        CBS crls;
        CBS_get_asn1(&signedDataSeq, &crls, CBS_ASN1_CONTEXT_SPECIFIC | CBS_ASN1_CONSTRUCTED | 1);
    }

    // Parse signerInfos SET
    CBS signerInfos;
    if (!CBS_get_asn1(&signedDataSeq, &signerInfos, CBS_ASN1_SET)) {
        outError = "Failed to parse signerInfos SET";
        return false;
    }

    // Parse first SignerInfo
    if (CBS_len(&signerInfos) > 0) {
        CBS signerInfo;
        if (!CBS_get_asn1(&signerInfos, &signerInfo, CBS_ASN1_SEQUENCE)) {
            outError = "Failed to parse SignerInfo SEQUENCE";
            return false;
        }

        // Parse version
        uint64_t siVersion;
        if (!CBS_get_asn1_uint64(&signerInfo, &siVersion)) {
            outError = "Failed to parse SignerInfo version";
            return false;
        }

        // Parse sid (SignerIdentifier)
        CBS sid;
        if (siVersion == 1) {
            // issuerAndSerialNumber
            if (!CBS_get_asn1(&signerInfo, &sid, CBS_ASN1_SEQUENCE)) {
                outError = "Failed to parse SignerIdentifier";
                return false;
            }
        } else if (siVersion == 3) {
            // subjectKeyIdentifier [0]
            if (!CBS_get_asn1(&signerInfo, &sid, CBS_ASN1_CONTEXT_SPECIFIC | 0)) {
                outError = "Failed to parse SignerIdentifier";
                return false;
            }
        }

        // Parse digestAlgorithm
        CBS digestAlg;
        if (!CBS_get_asn1(&signerInfo, &digestAlg, CBS_ASN1_SEQUENCE)) {
            outError = "Failed to parse SignerInfo digestAlgorithm";
            return false;
        }

        // Parse signedAttrs [0] IMPLICIT (optional)
        if (CBS_peek_asn1_tag(&signerInfo, CBS_ASN1_CONTEXT_SPECIFIC | CBS_ASN1_CONSTRUCTED | 0)) {
            CBS signedAttrsRaw;
            // Get the raw element including tag
            CBS temp;
            CBS_init(&temp, CBS_data(&signerInfo), CBS_len(&signerInfo));

            if (CBS_get_asn1_element(&temp, &signedAttrsRaw,
                                    CBS_ASN1_CONTEXT_SPECIFIC | CBS_ASN1_CONSTRUCTED | 0)) {
                // For signature verification, we need to re-encode with SET tag (0x31)
                // instead of [0] IMPLICIT (0xA0)
                outSignedAttrs.assign(CBS_data(&signedAttrsRaw),
                                     CBS_data(&signedAttrsRaw) + CBS_len(&signedAttrsRaw));
                // Replace the tag for signature verification
                if (!outSignedAttrs.empty()) {
                    outSignedAttrs[0] = 0x31; // SET tag
                }

                // Advance signerInfo past signedAttrs
                CBS_get_asn1(&signerInfo, &signedAttrsRaw,
                            CBS_ASN1_CONTEXT_SPECIFIC | CBS_ASN1_CONSTRUCTED | 0);

                // Extract messageDigest from signedAttrs
                CBS attrsContent;
                CBS_init(&attrsContent, outSignedAttrs.data() + 1, outSignedAttrs.size() - 1);
                // Skip length bytes
                size_t lenBytes = 1;
                if (outSignedAttrs.size() > 1 && (outSignedAttrs[1] & 0x80)) {
                    lenBytes = 1 + (outSignedAttrs[1] & 0x7F);
                }
                CBS_init(&attrsContent, outSignedAttrs.data() + 1 + lenBytes,
                        outSignedAttrs.size() - 1 - lenBytes);

                // Search for messageDigest attribute
                while (CBS_len(&attrsContent) > 0) {
                    CBS attr;
                    if (!CBS_get_asn1(&attrsContent, &attr, CBS_ASN1_SEQUENCE)) {
                        break;
                    }
                    CBS attrType;
                    if (!CBS_get_asn1(&attr, &attrType, CBS_ASN1_OBJECT)) {
                        continue;
                    }
                    if (compareOID(CBS_data(&attrType), CBS_len(&attrType),
                                  OID_MESSAGE_DIGEST, sizeof(OID_MESSAGE_DIGEST))) {
                        CBS attrValues;
                        if (CBS_get_asn1(&attr, &attrValues, CBS_ASN1_SET)) {
                            CBS digestValue;
                            if (CBS_get_asn1(&attrValues, &digestValue, CBS_ASN1_OCTETSTRING)) {
                                outMessageDigest.assign(CBS_data(&digestValue),
                                                       CBS_data(&digestValue) + CBS_len(&digestValue));
                            }
                        }
                        break;
                    }
                }
            }
        }

        // Parse signatureAlgorithm
        CBS sigAlg;
        if (!CBS_get_asn1(&signerInfo, &sigAlg, CBS_ASN1_SEQUENCE)) {
            outError = "Failed to parse signatureAlgorithm";
            return false;
        }

        // Parse signature OCTET STRING
        CBS sig;
        if (!CBS_get_asn1(&signerInfo, &sig, CBS_ASN1_OCTETSTRING)) {
            outError = "Failed to parse signature";
            return false;
        }
        outSignature.assign(CBS_data(&sig), CBS_data(&sig) + CBS_len(&sig));
    }

    return true;
}

bool Verifier::verifyCertificateChain(
    const std::vector<std::vector<uint8_t>>& certificates,
    std::string& outOperatorName,
    std::string& outOrganisationId,
    std::string& outError) const
{
    if (certificates.empty()) {
        outError = "No certificates to verify";
        return false;
    }

    // Load the Root CA
    FILE* caFile = fopen(m_Config.operatorRootCAPath.c_str(), "r");
    if (!caFile) {
        outError = "Failed to open Root CA file: " + m_Config.operatorRootCAPath.string();
        return false;
    }

    X509* rootCert = PEM_read_X509(caFile, nullptr, nullptr, nullptr);
    fclose(caFile);

    if (!rootCert) {
        outError = "Failed to read Root CA certificate: " + getOpenSSLError();
        return false;
    }

    // Create certificate store
    X509_STORE* store = X509_STORE_new();
    if (!store) {
        X509_free(rootCert);
        outError = "Failed to create X509_STORE";
        return false;
    }

    X509_STORE_add_cert(store, rootCert);

    // Parse all certificates from the SignedData
    std::vector<X509*> certChain;
    for (const auto& certDer : certificates) {
        const uint8_t* p = certDer.data();
        X509* cert = d2i_X509(nullptr, &p, certDer.size());
        if (cert) {
            certChain.push_back(cert);
        }
    }

    if (certChain.empty()) {
        X509_STORE_free(store);
        X509_free(rootCert);
        outError = "Failed to parse any certificates from SignedData";
        return false;
    }

    // The first certificate is typically the signer certificate
    X509* signerCert = certChain[0];

    // Create verification context
    X509_STORE_CTX* ctx = X509_STORE_CTX_new();
    if (!ctx) {
        for (auto* c : certChain) X509_free(c);
        X509_STORE_free(store);
        X509_free(rootCert);
        outError = "Failed to create X509_STORE_CTX";
        return false;
    }

    // Build untrusted chain (intermediate certs)
    STACK_OF(X509)* untrusted = sk_X509_new_null();
    for (size_t i = 1; i < certChain.size(); ++i) {
        sk_X509_push(untrusted, certChain[i]);
    }

    X509_STORE_CTX_init(ctx, store, signerCert, untrusted);

    // Verify the certificate chain
    int verifyResult = X509_verify_cert(ctx);

    if (verifyResult != 1) {
        int err = X509_STORE_CTX_get_error(ctx);
        outError = "Certificate chain verification failed: " +
                   std::string(X509_verify_cert_error_string(err));
        X509_STORE_CTX_free(ctx);
        sk_X509_free(untrusted);
        for (auto* c : certChain) X509_free(c);
        X509_STORE_free(store);
        X509_free(rootCert);
        return false;
    }

    // Extract O= and CN= from signer certificate subject
    X509_NAME* subject = X509_get_subject_name(signerCert);
    if (subject) {
        char buf[256];

        // Get Organization (O=)
        int idx = X509_NAME_get_text_by_NID(subject, NID_organizationName, buf, sizeof(buf));
        if (idx >= 0) {
            outOperatorName = buf;
        }

        // Get CommonName (CN=)
        idx = X509_NAME_get_text_by_NID(subject, NID_commonName, buf, sizeof(buf));
        if (idx >= 0) {
            outOrganisationId = buf;
        }
    }

    // Cleanup
    X509_STORE_CTX_free(ctx);
    sk_X509_free(untrusted);
    for (auto* c : certChain) X509_free(c);
    X509_STORE_free(store);
    X509_free(rootCert);

    return true;
}

bool Verifier::verifySignature(
    const std::vector<uint8_t>& signedAttrs,
    const std::vector<uint8_t>& signature,
    const std::vector<uint8_t>& signerCertDer,
    int digestAlgorithm,
    std::string& outError) const
{
    // Parse the signer certificate
    const uint8_t* p = signerCertDer.data();
    X509* cert = d2i_X509(nullptr, &p, signerCertDer.size());
    if (!cert) {
        outError = "Failed to parse signer certificate: " + getOpenSSLError();
        return false;
    }

    // Get public key from certificate
    EVP_PKEY* pkey = X509_get_pubkey(cert);
    if (!pkey) {
        X509_free(cert);
        outError = "Failed to get public key from certificate: " + getOpenSSLError();
        return false;
    }

    // Select digest algorithm
    const EVP_MD* md = nullptr;
    switch (digestAlgorithm) {
        case DIGEST_SHA256:
            md = EVP_sha256();
            break;
        case DIGEST_SHA384:
            md = EVP_sha384();
            break;
        case DIGEST_SHA512:
            md = EVP_sha512();
            break;
        default:
            md = EVP_sha256();
            break;
    }

    // Create verification context
    EVP_MD_CTX* mdCtx = EVP_MD_CTX_new();
    if (!mdCtx) {
        EVP_PKEY_free(pkey);
        X509_free(cert);
        outError = "Failed to create EVP_MD_CTX";
        return false;
    }

    // Initialize verification
    if (EVP_DigestVerifyInit(mdCtx, nullptr, md, nullptr, pkey) != 1) {
        EVP_MD_CTX_free(mdCtx);
        EVP_PKEY_free(pkey);
        X509_free(cert);
        outError = "Failed to initialize signature verification: " + getOpenSSLError();
        return false;
    }

    // Update with signed attributes
    if (EVP_DigestVerifyUpdate(mdCtx, signedAttrs.data(), signedAttrs.size()) != 1) {
        EVP_MD_CTX_free(mdCtx);
        EVP_PKEY_free(pkey);
        X509_free(cert);
        outError = "Failed to update signature verification: " + getOpenSSLError();
        return false;
    }

    // Verify signature
    int result = EVP_DigestVerifyFinal(mdCtx, signature.data(), signature.size());

    EVP_MD_CTX_free(mdCtx);
    EVP_PKEY_free(pkey);
    X509_free(cert);

    if (result != 1) {
        outError = "Signature verification failed: " + getOpenSSLError();
        return false;
    }

    return true;
}

bool Verifier::verifyMessageDigest(
    const std::vector<uint8_t>& content,
    const std::vector<uint8_t>& expectedDigest,
    int digestAlgorithm,
    std::string& outError) const
{
    if (expectedDigest.empty()) {
        // No messageDigest attribute to verify
        return true;
    }

    // Select digest algorithm
    const EVP_MD* md = nullptr;
    size_t digestLen = 0;
    switch (digestAlgorithm) {
        case DIGEST_SHA256:
            md = EVP_sha256();
            digestLen = 32;
            break;
        case DIGEST_SHA384:
            md = EVP_sha384();
            digestLen = 48;
            break;
        case DIGEST_SHA512:
            md = EVP_sha512();
            digestLen = 64;
            break;
        default:
            md = EVP_sha256();
            digestLen = 32;
            break;
    }

    // Calculate digest of content
    std::vector<uint8_t> calculatedDigest(digestLen);
    unsigned int actualLen = 0;

    EVP_MD_CTX* mdCtx = EVP_MD_CTX_new();
    if (!mdCtx) {
        outError = "Failed to create EVP_MD_CTX";
        return false;
    }

    if (EVP_DigestInit_ex(mdCtx, md, nullptr) != 1 ||
        EVP_DigestUpdate(mdCtx, content.data(), content.size()) != 1 ||
        EVP_DigestFinal_ex(mdCtx, calculatedDigest.data(), &actualLen) != 1) {
        EVP_MD_CTX_free(mdCtx);
        outError = "Failed to calculate message digest: " + getOpenSSLError();
        return false;
    }

    EVP_MD_CTX_free(mdCtx);
    calculatedDigest.resize(actualLen);

    // Compare digests
    if (calculatedDigest.size() != expectedDigest.size() ||
        memcmp(calculatedDigest.data(), expectedDigest.data(), calculatedDigest.size()) != 0) {
        outError = "Message digest mismatch: content hash does not match signed digest";
        return false;
    }

    return true;
}

#else
// OpenSSL implementation with CMS support

bool Verifier::verifyWithOpenSSL(
    const std::vector<uint8_t>& signedData,
    std::vector<uint8_t>& outContent,
    std::string& outError) const
{
    // Create BIO for input data
    BIO* inBio = BIO_new_mem_buf(signedData.data(), static_cast<int>(signedData.size()));
    if (!inBio) {
        outError = "Failed to create input BIO: " + getOpenSSLError();
        return false;
    }

    // Parse CMS structure
    CMS_ContentInfo* cms = d2i_CMS_bio(inBio, nullptr);
    BIO_free(inBio);

    if (!cms) {
        outError = "Failed to parse CMS structure: " + getOpenSSLError();
        return false;
    }

    // Load Root CA
    FILE* caFile = fopen(m_Config.operatorRootCAPath.c_str(), "r");
    if (!caFile) {
        CMS_ContentInfo_free(cms);
        outError = "Failed to open Root CA file: " + m_Config.operatorRootCAPath.string();
        return false;
    }

    X509* rootCert = PEM_read_X509(caFile, nullptr, nullptr, nullptr);
    fclose(caFile);

    if (!rootCert) {
        CMS_ContentInfo_free(cms);
        outError = "Failed to read Root CA certificate: " + getOpenSSLError();
        return false;
    }

    // Create certificate store
    X509_STORE* store = X509_STORE_new();
    if (!store) {
        X509_free(rootCert);
        CMS_ContentInfo_free(cms);
        outError = "Failed to create X509_STORE";
        return false;
    }

    X509_STORE_add_cert(store, rootCert);

    // Create output BIO
    BIO* outBio = BIO_new(BIO_s_mem());
    if (!outBio) {
        X509_STORE_free(store);
        X509_free(rootCert);
        CMS_ContentInfo_free(cms);
        outError = "Failed to create output BIO: " + getOpenSSLError();
        return false;
    }

    // Verify and extract content
    int flags = CMS_BINARY;
    if (CMS_verify(cms, nullptr, store, nullptr, outBio, flags) != 1) {
        BIO_free(outBio);
        X509_STORE_free(store);
        X509_free(rootCert);
        CMS_ContentInfo_free(cms);
        outError = "CMS verification failed: " + getOpenSSLError();
        return false;
    }

    // Validate operator identity against bilateral agreement (required)
    STACK_OF(X509)* signers = CMS_get0_signers(cms);
    if (!signers || sk_X509_num(signers) == 0) {
        BIO_free(outBio);
        X509_STORE_free(store);
        X509_free(rootCert);
        CMS_ContentInfo_free(cms);
        outError = "No signer certificates found in CMS";
        return false;
    }

    X509* signerCert = sk_X509_value(signers, 0);
    X509_NAME* subject = X509_get_subject_name(signerCert);

    if (!subject) {
        sk_X509_free(signers);
        BIO_free(outBio);
        X509_STORE_free(store);
        X509_free(rootCert);
        CMS_ContentInfo_free(cms);
        outError = "Failed to get signer certificate subject";
        return false;
    }

    char buf[256];

    // Note: Error messages don't reveal expected values (security best practice)
    int idx = X509_NAME_get_text_by_NID(subject, NID_organizationName, buf, sizeof(buf));
    if (idx < 0 || m_Config.expectedOperatorName != buf) {
        std::string got = (idx >= 0) ? buf : "(not found)";
        sk_X509_free(signers);
        BIO_free(outBio);
        X509_STORE_free(store);
        X509_free(rootCert);
        CMS_ContentInfo_free(cms);
        outError = "Operator Name mismatch: got '" + got + "'";
        return false;
    }

    idx = X509_NAME_get_text_by_NID(subject, NID_commonName, buf, sizeof(buf));
    if (idx < 0 || m_Config.expectedOrganisationId != buf) {
        std::string got = (idx >= 0) ? buf : "(not found)";
        sk_X509_free(signers);
        BIO_free(outBio);
        X509_STORE_free(store);
        X509_free(rootCert);
        CMS_ContentInfo_free(cms);
        outError = "Organisation ID mismatch: got '" + got + "'";
        return false;
    }

    sk_X509_free(signers);

    // Read verified content from BIO
    BUF_MEM* bufMem = nullptr;
    BIO_get_mem_ptr(outBio, &bufMem);

    outContent.assign(reinterpret_cast<uint8_t*>(bufMem->data),
                     reinterpret_cast<uint8_t*>(bufMem->data) + bufMem->length);

    // Cleanup
    BIO_free(outBio);
    X509_STORE_free(store);
    X509_free(rootCert);
    CMS_ContentInfo_free(cms);

    return true;
}

#endif // IS_CHROMIUM

} // namespace orb

