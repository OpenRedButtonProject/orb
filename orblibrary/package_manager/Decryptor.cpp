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

#include "Decryptor.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <cstring>

#ifdef IS_CHROMIUM
// BoringSSL headers
#include "third_party/boringssl/src/include/openssl/bio.h"
#include "third_party/boringssl/src/include/openssl/evp.h"
#include "third_party/boringssl/src/include/openssl/pem.h"
#include "third_party/boringssl/src/include/openssl/rsa.h"
#include "third_party/boringssl/src/include/openssl/err.h"
#include "third_party/boringssl/src/include/openssl/bytestring.h"
#include "third_party/boringssl/src/include/openssl/x509.h"
#include "third_party/boringssl/src/include/openssl/obj.h"
#include "third_party/boringssl/src/include/openssl/aes.h"
#else
// OpenSSL headers (with CMS support)
#include <openssl/bio.h>
#include <openssl/cms.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/x509.h>
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
    // OID constants used for CMS EnvelopedData parsing with BoringSSL
    // OID for enveloped-data: 1.2.840.113549.1.7.3
    const uint8_t OID_ENVELOPED_DATA[] = {0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x07, 0x03};

    // OID for aes128-CBC: 2.16.840.1.101.3.4.1.2
    const uint8_t OID_AES_128_CBC[] = {0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x01, 0x02};

    // OID for aes256-CBC: 2.16.840.1.101.3.4.1.42
    const uint8_t OID_AES_256_CBC[] = {0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x01, 0x2A};

    // OID for rsaEncryption: 1.2.840.113549.1.1.1
    const uint8_t OID_RSA_ENCRYPTION[] = {0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x01, 0x01};

    // OID for rsaesOaepEncryption: 1.2.840.113549.1.1.7
    const uint8_t OID_RSAES_OAEP[] = {0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x01, 0x07};

    bool compareOID(const uint8_t* oid1, size_t len1, const uint8_t* oid2, size_t len2) {
        if (len1 != len2) return false;
        return memcmp(oid1, oid2, len1) == 0;
    }
#endif // IS_CHROMIUM
}

Decryptor::Decryptor() = default;

Decryptor::Decryptor(const DecryptorConfig& config)
    : m_Config(config)
{
}

void Decryptor::setConfig(const DecryptorConfig& config)
{
    m_Config = config;
}

bool Decryptor::isConfigured() const
{
    return !m_Config.privateKeyPath.empty() && !m_Config.certificatePath.empty();
}

bool Decryptor::decrypt(
    const std::filesystem::path& filePath,
    std::filesystem::path& outFile,
    std::string& outError) const
{
    // Validate configuration
    if (!isConfigured()) {
        outError = "Decryptor not configured: missing private key or certificate path";
        return false;
    }

    // Check input file exists
    if (!std::filesystem::exists(filePath)) {
        outError = "Input file does not exist: " + filePath.string();
        return false;
    }

    // Check key and certificate files exist
    if (!std::filesystem::exists(m_Config.privateKeyPath)) {
        outError = "Private key file does not exist: " + m_Config.privateKeyPath.string();
        return false;
    }
    if (!std::filesystem::exists(m_Config.certificatePath)) {
        outError = "Certificate file does not exist: " + m_Config.certificatePath.string();
        return false;
    }

    // Read the encrypted CMS file
    std::ifstream inFile(filePath, std::ios::binary);
    if (!inFile) {
        outError = "Failed to open input file: " + filePath.string();
        return false;
    }

    std::vector<uint8_t> cmsData(
        (std::istreambuf_iterator<char>(inFile)),
        std::istreambuf_iterator<char>());
    inFile.close();

    if (cmsData.empty()) {
        outError = "Input file is empty: " + filePath.string();
        return false;
    }

    // Decrypt the CMS data
    std::vector<uint8_t> decrypted;
#ifdef IS_CHROMIUM
    if (!decryptWithBoringSSL(cmsData, decrypted, outError)) {
        return false;
    }
#else
    if (!decryptWithOpenSSL(cmsData, decrypted, outError)) {
        return false;
    }
#endif

    // Determine output path
    std::filesystem::path outputPath;
    if (!m_Config.workingDirectory.empty()) {
        std::filesystem::create_directories(m_Config.workingDirectory);
        outputPath = m_Config.workingDirectory / (filePath.stem().string() + "_decrypted.cms");
    } else {
        outputPath = filePath.parent_path() / (filePath.stem().string() + "_decrypted.cms");
    }

    // Write decrypted content to file
    std::ofstream outFileStream(outputPath, std::ios::binary);
    if (!outFileStream) {
        outError = "Failed to create output file: " + outputPath.string();
        return false;
    }

    outFileStream.write(reinterpret_cast<const char*>(decrypted.data()), decrypted.size());
    outFileStream.close();

    if (outFileStream.fail()) {
        outError = "Failed to write decrypted content to: " + outputPath.string();
        return false;
    }

    outFile = outputPath;
    return true;
}

#ifdef IS_CHROMIUM
// BoringSSL implementation - manual CMS EnvelopedData parsing

bool Decryptor::decryptWithBoringSSL(
    const std::vector<uint8_t>& cmsData,
    std::vector<uint8_t>& outDecrypted,
    std::string& outError) const
{
    std::vector<uint8_t> encryptedKey;
    std::vector<uint8_t> encryptedContent;
    std::vector<uint8_t> iv;
    int keySize = 0;

    // Parse the CMS EnvelopedData structure
    if (!parseEnvelopedData(cmsData.data(), cmsData.size(),
                           encryptedKey, encryptedContent, iv, keySize, outError)) {
        return false;
    }

    // Decrypt the symmetric key using RSA
    std::vector<uint8_t> decryptedKey;
    if (!decryptKey(encryptedKey, decryptedKey, outError)) {
        return false;
    }

    // Verify key size matches expected
    if (static_cast<int>(decryptedKey.size()) != keySize) {
        outError = "Decrypted key size (" + std::to_string(decryptedKey.size()) +
                   ") does not match expected (" + std::to_string(keySize) + ")";
        return false;
    }

    // Decrypt the content using AES-CBC
    if (!decryptContent(encryptedContent, decryptedKey, iv, outDecrypted, outError)) {
        return false;
    }

    return true;
}

bool Decryptor::parseEnvelopedData(
    const uint8_t* data,
    size_t len,
    std::vector<uint8_t>& outEncryptedKey,
    std::vector<uint8_t>& outEncryptedContent,
    std::vector<uint8_t>& outIV,
    int& outKeySize,
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

    // Verify it's enveloped-data
    if (!compareOID(CBS_data(&contentType), CBS_len(&contentType),
                   OID_ENVELOPED_DATA, sizeof(OID_ENVELOPED_DATA))) {
        outError = "ContentInfo is not EnvelopedData";
        return false;
    }

    // Parse [0] EXPLICIT content
    CBS contentWrapper;
    if (!CBS_get_asn1(&contentInfo, &contentWrapper, CBS_ASN1_CONTEXT_SPECIFIC | CBS_ASN1_CONSTRUCTED | 0)) {
        outError = "Failed to parse content wrapper [0]";
        return false;
    }

    // Parse EnvelopedData SEQUENCE
    CBS envelopedData;
    if (!CBS_get_asn1(&contentWrapper, &envelopedData, CBS_ASN1_SEQUENCE)) {
        outError = "Failed to parse EnvelopedData SEQUENCE";
        return false;
    }

    // Parse version INTEGER
    uint64_t version;
    if (!CBS_get_asn1_uint64(&envelopedData, &version)) {
        outError = "Failed to parse EnvelopedData version";
        return false;
    }

    // Skip optional originatorInfo [0] if present
    CBS originatorInfo;
    CBS_get_asn1(&envelopedData, &originatorInfo, CBS_ASN1_CONTEXT_SPECIFIC | CBS_ASN1_CONSTRUCTED | 0);

    // Parse recipientInfos SET
    CBS recipientInfos;
    if (!CBS_get_asn1(&envelopedData, &recipientInfos, CBS_ASN1_SET)) {
        outError = "Failed to parse recipientInfos SET";
        return false;
    }

    // Parse first RecipientInfo (KeyTransRecipientInfo)
    // For now we just take the first one - in production should match against certificate
    CBS recipientInfo;
    if (!CBS_get_asn1(&recipientInfos, &recipientInfo, CBS_ASN1_SEQUENCE)) {
        outError = "Failed to parse RecipientInfo";
        return false;
    }

    // Parse KeyTransRecipientInfo version
    uint64_t ktriVersion;
    if (!CBS_get_asn1_uint64(&recipientInfo, &ktriVersion)) {
        outError = "Failed to parse KeyTransRecipientInfo version";
        return false;
    }

    // Parse RecipientIdentifier (issuerAndSerialNumber or subjectKeyIdentifier)
    // We skip this for now - just advance past it
    CBS rid;
    if (ktriVersion == 0) {
        // issuerAndSerialNumber SEQUENCE
        if (!CBS_get_asn1(&recipientInfo, &rid, CBS_ASN1_SEQUENCE)) {
            outError = "Failed to parse RecipientIdentifier (issuerAndSerialNumber)";
            return false;
        }
    } else if (ktriVersion == 2) {
        // subjectKeyIdentifier [0] IMPLICIT
        if (!CBS_get_asn1(&recipientInfo, &rid, CBS_ASN1_CONTEXT_SPECIFIC | 0)) {
            outError = "Failed to parse RecipientIdentifier (subjectKeyIdentifier)";
            return false;
        }
    } else {
        outError = "Unsupported KeyTransRecipientInfo version: " + std::to_string(ktriVersion);
        return false;
    }

    // Parse keyEncryptionAlgorithm AlgorithmIdentifier
    CBS keyEncAlg;
    if (!CBS_get_asn1(&recipientInfo, &keyEncAlg, CBS_ASN1_SEQUENCE)) {
        outError = "Failed to parse keyEncryptionAlgorithm";
        return false;
    }

    // Parse algorithm OID
    CBS keyEncAlgOID;
    if (!CBS_get_asn1(&keyEncAlg, &keyEncAlgOID, CBS_ASN1_OBJECT)) {
        outError = "Failed to parse keyEncryptionAlgorithm OID";
        return false;
    }

    // Verify RSA encryption algorithm
    bool isRSA = compareOID(CBS_data(&keyEncAlgOID), CBS_len(&keyEncAlgOID),
                           OID_RSA_ENCRYPTION, sizeof(OID_RSA_ENCRYPTION));
    bool isOAEP = compareOID(CBS_data(&keyEncAlgOID), CBS_len(&keyEncAlgOID),
                            OID_RSAES_OAEP, sizeof(OID_RSAES_OAEP));
    if (!isRSA && !isOAEP) {
        outError = "Unsupported key encryption algorithm (not RSA or RSAES-OAEP)";
        return false;
    }

    // Parse encryptedKey OCTET STRING
    CBS encryptedKeyOctet;
    if (!CBS_get_asn1(&recipientInfo, &encryptedKeyOctet, CBS_ASN1_OCTETSTRING)) {
        outError = "Failed to parse encryptedKey";
        return false;
    }

    outEncryptedKey.assign(CBS_data(&encryptedKeyOctet),
                          CBS_data(&encryptedKeyOctet) + CBS_len(&encryptedKeyOctet));

    // Parse encryptedContentInfo SEQUENCE
    CBS encryptedContentInfo;
    if (!CBS_get_asn1(&envelopedData, &encryptedContentInfo, CBS_ASN1_SEQUENCE)) {
        outError = "Failed to parse encryptedContentInfo";
        return false;
    }

    // Parse contentType OID (should be id-data)
    CBS encContentType;
    if (!CBS_get_asn1(&encryptedContentInfo, &encContentType, CBS_ASN1_OBJECT)) {
        outError = "Failed to parse encryptedContentInfo contentType";
        return false;
    }

    // Parse contentEncryptionAlgorithm AlgorithmIdentifier
    CBS contentEncAlg;
    if (!CBS_get_asn1(&encryptedContentInfo, &contentEncAlg, CBS_ASN1_SEQUENCE)) {
        outError = "Failed to parse contentEncryptionAlgorithm";
        return false;
    }

    // Parse algorithm OID
    CBS contentEncAlgOID;
    if (!CBS_get_asn1(&contentEncAlg, &contentEncAlgOID, CBS_ASN1_OBJECT)) {
        outError = "Failed to parse contentEncryptionAlgorithm OID";
        return false;
    }

    // Check for AES-128-CBC or AES-256-CBC
    bool isAES128 = compareOID(CBS_data(&contentEncAlgOID), CBS_len(&contentEncAlgOID),
                              OID_AES_128_CBC, sizeof(OID_AES_128_CBC));
    bool isAES256 = compareOID(CBS_data(&contentEncAlgOID), CBS_len(&contentEncAlgOID),
                              OID_AES_256_CBC, sizeof(OID_AES_256_CBC));

    if (!isAES128 && !isAES256) {
        outError = "Unsupported content encryption algorithm (not AES-128-CBC or AES-256-CBC)";
        return false;
    }

    outKeySize = isAES128 ? 16 : 32;

    // Parse IV (parameters should be OCTET STRING containing IV)
    CBS ivOctet;
    if (!CBS_get_asn1(&contentEncAlg, &ivOctet, CBS_ASN1_OCTETSTRING)) {
        outError = "Failed to parse IV from contentEncryptionAlgorithm parameters";
        return false;
    }

    if (CBS_len(&ivOctet) != 16) {
        outError = "Invalid IV length: expected 16, got " + std::to_string(CBS_len(&ivOctet));
        return false;
    }

    outIV.assign(CBS_data(&ivOctet), CBS_data(&ivOctet) + CBS_len(&ivOctet));

    // Parse encryptedContent [0] IMPLICIT OCTET STRING
    CBS encryptedContentOctet;
    if (!CBS_get_asn1(&encryptedContentInfo, &encryptedContentOctet,
                     CBS_ASN1_CONTEXT_SPECIFIC | 0)) {
        outError = "Failed to parse encryptedContent";
        return false;
    }

    outEncryptedContent.assign(CBS_data(&encryptedContentOctet),
                              CBS_data(&encryptedContentOctet) + CBS_len(&encryptedContentOctet));

    return true;
}

bool Decryptor::decryptKey(
    const std::vector<uint8_t>& encryptedKey,
    std::vector<uint8_t>& outKey,
    std::string& outError) const
{
    // Load private key from file
    FILE* keyFile = fopen(m_Config.privateKeyPath.c_str(), "r");
    if (!keyFile) {
        outError = "Failed to open private key file: " + m_Config.privateKeyPath.string();
        return false;
    }

    EVP_PKEY* pkey = PEM_read_PrivateKey(keyFile, nullptr, nullptr, nullptr);
    fclose(keyFile);

    if (!pkey) {
        outError = "Failed to read private key: " + getOpenSSLError();
        return false;
    }

    // Create decryption context
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(pkey, nullptr);
    if (!ctx) {
        EVP_PKEY_free(pkey);
        outError = "Failed to create EVP_PKEY_CTX: " + getOpenSSLError();
        return false;
    }

    if (EVP_PKEY_decrypt_init(ctx) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(pkey);
        outError = "Failed to initialize decryption: " + getOpenSSLError();
        return false;
    }

    // Set RSA padding to PKCS1 (most common for CMS)
    if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_PADDING) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(pkey);
        outError = "Failed to set RSA padding: " + getOpenSSLError();
        return false;
    }

    // Determine output buffer size
    size_t outLen = 0;
    if (EVP_PKEY_decrypt(ctx, nullptr, &outLen, encryptedKey.data(), encryptedKey.size()) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(pkey);
        outError = "Failed to determine decrypted key size: " + getOpenSSLError();
        return false;
    }

    outKey.resize(outLen);

    // Perform decryption
    if (EVP_PKEY_decrypt(ctx, outKey.data(), &outLen, encryptedKey.data(), encryptedKey.size()) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(pkey);
        outError = "Failed to decrypt key: " + getOpenSSLError();
        return false;
    }

    outKey.resize(outLen);
    EVP_PKEY_CTX_free(ctx);
    EVP_PKEY_free(pkey);

    return true;
}

bool Decryptor::decryptContent(
    const std::vector<uint8_t>& encryptedContent,
    const std::vector<uint8_t>& key,
    const std::vector<uint8_t>& iv,
    std::vector<uint8_t>& outContent,
    std::string& outError) const
{
    // Select cipher based on key size
    const EVP_CIPHER* cipher = nullptr;
    if (key.size() == 16) {
        cipher = EVP_aes_128_cbc();
    } else if (key.size() == 32) {
        cipher = EVP_aes_256_cbc();
    } else {
        outError = "Invalid key size: " + std::to_string(key.size());
        return false;
    }

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        outError = "Failed to create cipher context: " + getOpenSSLError();
        return false;
    }

    if (EVP_DecryptInit_ex(ctx, cipher, nullptr, key.data(), iv.data()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        outError = "Failed to initialize AES decryption: " + getOpenSSLError();
        return false;
    }

    // Allocate output buffer (encrypted size + block size for padding)
    outContent.resize(encryptedContent.size() + EVP_CIPHER_block_size(cipher));

    int outLen = 0;
    if (EVP_DecryptUpdate(ctx, outContent.data(), &outLen,
                         encryptedContent.data(), encryptedContent.size()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        outError = "Failed to decrypt content: " + getOpenSSLError();
        return false;
    }

    int finalLen = 0;
    if (EVP_DecryptFinal_ex(ctx, outContent.data() + outLen, &finalLen) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        outError = "Failed to finalize decryption (padding error?): " + getOpenSSLError();
        return false;
    }

    outContent.resize(outLen + finalLen);
    EVP_CIPHER_CTX_free(ctx);

    return true;
}

#else
// OpenSSL implementation with CMS support

bool Decryptor::decryptWithOpenSSL(
    const std::vector<uint8_t>& cmsData,
    std::vector<uint8_t>& outDecrypted,
    std::string& outError) const
{
    // Create BIO for input data
    BIO* inBio = BIO_new_mem_buf(cmsData.data(), static_cast<int>(cmsData.size()));
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

    // Load private key
    FILE* keyFile = fopen(m_Config.privateKeyPath.c_str(), "r");
    if (!keyFile) {
        CMS_ContentInfo_free(cms);
        outError = "Failed to open private key file: " + m_Config.privateKeyPath.string();
        return false;
    }

    EVP_PKEY* pkey = PEM_read_PrivateKey(keyFile, nullptr, nullptr, nullptr);
    fclose(keyFile);

    if (!pkey) {
        CMS_ContentInfo_free(cms);
        outError = "Failed to read private key: " + getOpenSSLError();
        return false;
    }

    // Load certificate
    FILE* certFile = fopen(m_Config.certificatePath.c_str(), "r");
    if (!certFile) {
        EVP_PKEY_free(pkey);
        CMS_ContentInfo_free(cms);
        outError = "Failed to open certificate file: " + m_Config.certificatePath.string();
        return false;
    }

    X509* cert = PEM_read_X509(certFile, nullptr, nullptr, nullptr);
    fclose(certFile);

    if (!cert) {
        EVP_PKEY_free(pkey);
        CMS_ContentInfo_free(cms);
        outError = "Failed to read certificate: " + getOpenSSLError();
        return false;
    }

    // Create output BIO
    BIO* outBio = BIO_new(BIO_s_mem());
    if (!outBio) {
        X509_free(cert);
        EVP_PKEY_free(pkey);
        CMS_ContentInfo_free(cms);
        outError = "Failed to create output BIO: " + getOpenSSLError();
        return false;
    }

    // Perform decryption
    if (CMS_decrypt(cms, pkey, cert, nullptr, outBio, 0) != 1) {
        BIO_free(outBio);
        X509_free(cert);
        EVP_PKEY_free(pkey);
        CMS_ContentInfo_free(cms);
        outError = "CMS decryption failed: " + getOpenSSLError();
        return false;
    }

    // Read decrypted data from BIO
    BUF_MEM* bufMem = nullptr;
    BIO_get_mem_ptr(outBio, &bufMem);

    outDecrypted.assign(reinterpret_cast<uint8_t*>(bufMem->data),
                       reinterpret_cast<uint8_t*>(bufMem->data) + bufMem->length);

    // Cleanup
    BIO_free(outBio);
    X509_free(cert);
    EVP_PKEY_free(pkey);
    CMS_ContentInfo_free(cms);

    return true;
}

#endif // IS_CHROMIUM

} // namespace orb
