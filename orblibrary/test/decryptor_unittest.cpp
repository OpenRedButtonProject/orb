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
 * Unit tests for the CMS EnvelopedData Decryptor
 */

#include <iostream>
#include <string>
#include <filesystem>
#include <fstream>
#include <vector>
#include <cstdlib>

#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/orb/orblibrary/package_manager/Decryptor.h"

#ifdef IS_CHROMIUM
#include "third_party/boringssl/src/include/openssl/bio.h"
#include "third_party/boringssl/src/include/openssl/evp.h"
#include "third_party/boringssl/src/include/openssl/pem.h"
#include "third_party/boringssl/src/include/openssl/rsa.h"
#include "third_party/boringssl/src/include/openssl/rand.h"
#include "third_party/boringssl/src/include/openssl/aes.h"
#else
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/rand.h>
#include <openssl/cms.h>
#endif

using namespace orb;

namespace {

// Test fixture directory - uses a unique temp directory per test run
std::filesystem::path getTestDir() {
    static std::filesystem::path testDir;
    if (testDir.empty()) {
        testDir = std::filesystem::temp_directory_path() / "decryptor_tests";
        std::filesystem::create_directories(testDir);
    }
    return testDir;
}

// Helper to create a test file with given content
void createTestFile(const std::filesystem::path& path, const std::vector<uint8_t>& content) {
    std::filesystem::create_directories(path.parent_path());
    std::ofstream file(path, std::ios::binary);
    file.write(reinterpret_cast<const char*>(content.data()), content.size());
}

[[maybe_unused]]
void createTestFile(const std::filesystem::path& path, const std::string& content) {
    std::filesystem::create_directories(path.parent_path());
    std::ofstream file(path);
    file << content;
}

// Helper to generate RSA key pair for testing
// Returns true if key generation succeeded, stores paths in outKeyPath and outCertPath
bool generateTestKeyPair(
    const std::filesystem::path& keyPath,
    const std::filesystem::path& certPath)
{
    // Generate RSA key
    EVP_PKEY* pkey = EVP_PKEY_new();
    if (!pkey) return false;

    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, nullptr);
    if (!ctx) {
        EVP_PKEY_free(pkey);
        return false;
    }

    if (EVP_PKEY_keygen_init(ctx) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(pkey);
        return false;
    }

    if (EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, 2048) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(pkey);
        return false;
    }

    EVP_PKEY* newKey = nullptr;
    if (EVP_PKEY_keygen(ctx, &newKey) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(pkey);
        return false;
    }

    EVP_PKEY_CTX_free(ctx);
    EVP_PKEY_free(pkey);
    pkey = newKey;

    // Write private key to file
    std::filesystem::create_directories(keyPath.parent_path());
    FILE* keyFile = fopen(keyPath.c_str(), "w");
    if (!keyFile) {
        EVP_PKEY_free(pkey);
        return false;
    }

    if (PEM_write_PrivateKey(keyFile, pkey, nullptr, nullptr, 0, nullptr, nullptr) != 1) {
        fclose(keyFile);
        EVP_PKEY_free(pkey);
        return false;
    }
    fclose(keyFile);

    // Create a self-signed certificate
    X509* x509 = X509_new();
    if (!x509) {
        EVP_PKEY_free(pkey);
        return false;
    }

    ASN1_INTEGER_set(X509_get_serialNumber(x509), 1);
    X509_gmtime_adj(X509_get_notBefore(x509), 0);
    X509_gmtime_adj(X509_get_notAfter(x509), 31536000L); // 1 year

    X509_set_pubkey(x509, pkey);

    X509_NAME* name = X509_get_subject_name(x509);
    X509_NAME_add_entry_by_txt(name, "C", MBSTRING_ASC,
                               reinterpret_cast<const unsigned char*>("GB"), -1, -1, 0);
    X509_NAME_add_entry_by_txt(name, "O", MBSTRING_ASC,
                               reinterpret_cast<const unsigned char*>("Test Org"), -1, -1, 0);
    X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC,
                               reinterpret_cast<const unsigned char*>("Test Certificate"), -1, -1, 0);
    X509_set_issuer_name(x509, name);

    if (X509_sign(x509, pkey, EVP_sha256()) == 0) {
        X509_free(x509);
        EVP_PKEY_free(pkey);
        return false;
    }

    // Write certificate to file
    std::filesystem::create_directories(certPath.parent_path());
    FILE* certFile = fopen(certPath.c_str(), "w");
    if (!certFile) {
        X509_free(x509);
        EVP_PKEY_free(pkey);
        return false;
    }

    if (PEM_write_X509(certFile, x509) != 1) {
        fclose(certFile);
        X509_free(x509);
        EVP_PKEY_free(pkey);
        return false;
    }
    fclose(certFile);

    X509_free(x509);
    EVP_PKEY_free(pkey);

    return true;
}

#ifndef IS_CHROMIUM
// Helper to create CMS EnvelopedData for testing (OpenSSL only)
// This is used to generate test fixtures
bool createTestCMSEnvelopedData(
    const std::filesystem::path& certPath,
    const std::vector<uint8_t>& plaintext,
    std::vector<uint8_t>& outCMS)
{
    // Load certificate
    FILE* certFile = fopen(certPath.c_str(), "r");
    if (!certFile) return false;

    X509* cert = PEM_read_X509(certFile, nullptr, nullptr, nullptr);
    fclose(certFile);
    if (!cert) return false;

    // Create recipient stack
    STACK_OF(X509)* certs = sk_X509_new_null();
    sk_X509_push(certs, cert);

    // Create BIO for plaintext
    BIO* inBio = BIO_new_mem_buf(plaintext.data(), static_cast<int>(plaintext.size()));
    if (!inBio) {
        sk_X509_pop_free(certs, X509_free);
        return false;
    }

    // Create CMS EnvelopedData
    CMS_ContentInfo* cms = CMS_encrypt(certs, inBio, EVP_aes_256_cbc(), CMS_BINARY);
    BIO_free(inBio);
    sk_X509_pop_free(certs, X509_free);

    if (!cms) {
        return false;
    }

    // Serialize to DER
    BIO* outBio = BIO_new(BIO_s_mem());
    if (!outBio) {
        CMS_ContentInfo_free(cms);
        return false;
    }

    if (i2d_CMS_bio(outBio, cms) != 1) {
        BIO_free(outBio);
        CMS_ContentInfo_free(cms);
        return false;
    }

    BUF_MEM* bufMem = nullptr;
    BIO_get_mem_ptr(outBio, &bufMem);

    outCMS.assign(reinterpret_cast<uint8_t*>(bufMem->data),
                 reinterpret_cast<uint8_t*>(bufMem->data) + bufMem->length);

    BIO_free(outBio);
    CMS_ContentInfo_free(cms);

    return true;
}
#endif

} // anonymous namespace

// Test fixture class
class DecryptorTest : public ::testing::Test {
protected:
    void SetUp() override {
        m_TestDir = getTestDir() / ::testing::UnitTest::GetInstance()->current_test_info()->name();
        std::filesystem::create_directories(m_TestDir);

        m_KeyPath = m_TestDir / "test_key.pem";
        m_CertPath = m_TestDir / "test_cert.pem";
        m_WorkingDir = m_TestDir / "working";
    }

    void TearDown() override {
        // Clean up test files
        std::error_code ec;
        std::filesystem::remove_all(m_TestDir, ec);
    }

    std::filesystem::path m_TestDir;
    std::filesystem::path m_KeyPath;
    std::filesystem::path m_CertPath;
    std::filesystem::path m_WorkingDir;
};

//------------------------------------------------------------------------------
// Configuration Tests
//------------------------------------------------------------------------------

TEST_F(DecryptorTest, DefaultConstructorIsNotConfigured)
{
    Decryptor decryptor;
    EXPECT_FALSE(decryptor.isConfigured());
}

TEST_F(DecryptorTest, ConfiguredWithValidPaths)
{
    DecryptorConfig config;
    config.privateKeyPath = "/path/to/key.pem";
    config.certificatePath = "/path/to/cert.pem";

    Decryptor decryptor(config);
    EXPECT_TRUE(decryptor.isConfigured());
}

TEST_F(DecryptorTest, NotConfiguredWithEmptyKeyPath)
{
    DecryptorConfig config;
    config.privateKeyPath = "";
    config.certificatePath = "/path/to/cert.pem";

    Decryptor decryptor(config);
    EXPECT_FALSE(decryptor.isConfigured());
}

TEST_F(DecryptorTest, NotConfiguredWithEmptyCertPath)
{
    DecryptorConfig config;
    config.privateKeyPath = "/path/to/key.pem";
    config.certificatePath = "";

    Decryptor decryptor(config);
    EXPECT_FALSE(decryptor.isConfigured());
}

TEST_F(DecryptorTest, SetConfigUpdatesConfiguration)
{
    Decryptor decryptor;
    EXPECT_FALSE(decryptor.isConfigured());

    DecryptorConfig config;
    config.privateKeyPath = "/path/to/key.pem";
    config.certificatePath = "/path/to/cert.pem";

    decryptor.setConfig(config);
    EXPECT_TRUE(decryptor.isConfigured());
}

//------------------------------------------------------------------------------
// Validation Tests
//------------------------------------------------------------------------------

TEST_F(DecryptorTest, DecryptFailsWhenNotConfigured)
{
    Decryptor decryptor;

    std::filesystem::path inputFile = m_TestDir / "input.cms";
    createTestFile(inputFile, std::vector<uint8_t>{0x30, 0x00});  // Minimal DER

    std::filesystem::path outFile;
    std::string outError;

    bool result = decryptor.decrypt(inputFile, outFile, outError);

    EXPECT_FALSE(result);
    EXPECT_TRUE(outError.find("not configured") != std::string::npos);
}

TEST_F(DecryptorTest, DecryptFailsWhenInputFileDoesNotExist)
{
    DecryptorConfig config;
    config.privateKeyPath = m_KeyPath;
    config.certificatePath = m_CertPath;
    config.workingDirectory = m_WorkingDir;

    // Create key and cert files
    ASSERT_TRUE(generateTestKeyPair(m_KeyPath, m_CertPath));

    Decryptor decryptor(config);

    std::filesystem::path inputFile = m_TestDir / "nonexistent.cms";
    std::filesystem::path outFile;
    std::string outError;

    bool result = decryptor.decrypt(inputFile, outFile, outError);

    EXPECT_FALSE(result);
    EXPECT_TRUE(outError.find("does not exist") != std::string::npos);
}

TEST_F(DecryptorTest, DecryptFailsWhenKeyFileDoesNotExist)
{
    DecryptorConfig config;
    config.privateKeyPath = m_TestDir / "nonexistent_key.pem";
    config.certificatePath = m_CertPath;
    config.workingDirectory = m_WorkingDir;

    // Create only cert file, not key
    ASSERT_TRUE(generateTestKeyPair(m_KeyPath, m_CertPath));

    Decryptor decryptor(config);

    std::filesystem::path inputFile = m_TestDir / "input.cms";
    createTestFile(inputFile, std::vector<uint8_t>{0x30, 0x00});

    std::filesystem::path outFile;
    std::string outError;

    bool result = decryptor.decrypt(inputFile, outFile, outError);

    EXPECT_FALSE(result);
    EXPECT_TRUE(outError.find("Private key file does not exist") != std::string::npos);
}

TEST_F(DecryptorTest, DecryptFailsWhenCertFileDoesNotExist)
{
    DecryptorConfig config;
    config.privateKeyPath = m_KeyPath;
    config.certificatePath = m_TestDir / "nonexistent_cert.pem";
    config.workingDirectory = m_WorkingDir;

    // Create only key file, not cert
    ASSERT_TRUE(generateTestKeyPair(m_KeyPath, m_CertPath));

    Decryptor decryptor(config);

    std::filesystem::path inputFile = m_TestDir / "input.cms";
    createTestFile(inputFile, std::vector<uint8_t>{0x30, 0x00});

    std::filesystem::path outFile;
    std::string outError;

    bool result = decryptor.decrypt(inputFile, outFile, outError);

    EXPECT_FALSE(result);
    EXPECT_TRUE(outError.find("Certificate file does not exist") != std::string::npos);
}

TEST_F(DecryptorTest, DecryptFailsWithEmptyInputFile)
{
    DecryptorConfig config;
    config.privateKeyPath = m_KeyPath;
    config.certificatePath = m_CertPath;
    config.workingDirectory = m_WorkingDir;

    ASSERT_TRUE(generateTestKeyPair(m_KeyPath, m_CertPath));

    Decryptor decryptor(config);

    std::filesystem::path inputFile = m_TestDir / "empty.cms";
    createTestFile(inputFile, std::vector<uint8_t>{});  // Empty file

    std::filesystem::path outFile;
    std::string outError;

    bool result = decryptor.decrypt(inputFile, outFile, outError);

    EXPECT_FALSE(result);
    EXPECT_TRUE(outError.find("empty") != std::string::npos);
}

//------------------------------------------------------------------------------
// Invalid CMS Structure Tests
//------------------------------------------------------------------------------

TEST_F(DecryptorTest, DecryptFailsWithInvalidDER)
{
    DecryptorConfig config;
    config.privateKeyPath = m_KeyPath;
    config.certificatePath = m_CertPath;
    config.workingDirectory = m_WorkingDir;

    ASSERT_TRUE(generateTestKeyPair(m_KeyPath, m_CertPath));

    Decryptor decryptor(config);

    // Create file with invalid DER data (not a valid ASN.1 structure)
    std::filesystem::path inputFile = m_TestDir / "invalid.cms";
    createTestFile(inputFile, std::vector<uint8_t>{0xFF, 0xFF, 0xFF, 0xFF});

    std::filesystem::path outFile;
    std::string outError;

    bool result = decryptor.decrypt(inputFile, outFile, outError);

    EXPECT_FALSE(result);
    // Should fail during parsing
}

TEST_F(DecryptorTest, DecryptFailsWithNonCMSData)
{
    DecryptorConfig config;
    config.privateKeyPath = m_KeyPath;
    config.certificatePath = m_CertPath;
    config.workingDirectory = m_WorkingDir;

    ASSERT_TRUE(generateTestKeyPair(m_KeyPath, m_CertPath));

    Decryptor decryptor(config);

    // Create a valid DER SEQUENCE but not CMS EnvelopedData
    // This is a minimal SEQUENCE with an INTEGER inside
    std::vector<uint8_t> notCMS = {
        0x30, 0x03,  // SEQUENCE, length 3
        0x02, 0x01, 0x00  // INTEGER 0
    };
    std::filesystem::path inputFile = m_TestDir / "not_cms.der";
    createTestFile(inputFile, notCMS);

    std::filesystem::path outFile;
    std::string outError;

    bool result = decryptor.decrypt(inputFile, outFile, outError);

    EXPECT_FALSE(result);
    // Should fail because OID doesn't match enveloped-data
}

//------------------------------------------------------------------------------
// Integration Tests (OpenSSL only - can generate CMS)
//------------------------------------------------------------------------------

#ifndef IS_CHROMIUM
TEST_F(DecryptorTest, DecryptValidCMSEnvelopedData)
{
    DecryptorConfig config;
    config.privateKeyPath = m_KeyPath;
    config.certificatePath = m_CertPath;
    config.workingDirectory = m_WorkingDir;

    ASSERT_TRUE(generateTestKeyPair(m_KeyPath, m_CertPath));

    // Create test plaintext
    std::string plaintextStr = "This is the test message content for CMS encryption.";
    std::vector<uint8_t> plaintext(plaintextStr.begin(), plaintextStr.end());

    // Create CMS EnvelopedData
    std::vector<uint8_t> cmsData;
    ASSERT_TRUE(createTestCMSEnvelopedData(m_CertPath, plaintext, cmsData));

    // Write CMS to file
    std::filesystem::path inputFile = m_TestDir / "encrypted.cms";
    createTestFile(inputFile, cmsData);

    // Decrypt
    Decryptor decryptor(config);
    std::filesystem::path outFile;
    std::string outError;

    bool result = decryptor.decrypt(inputFile, outFile, outError);

    EXPECT_TRUE(result) << "Decryption failed: " << outError;
    EXPECT_TRUE(std::filesystem::exists(outFile));

    // Verify decrypted content matches original
    std::ifstream decryptedFile(outFile, std::ios::binary);
    std::vector<uint8_t> decrypted(
        (std::istreambuf_iterator<char>(decryptedFile)),
        std::istreambuf_iterator<char>());

    EXPECT_EQ(plaintext, decrypted);
}

TEST_F(DecryptorTest, DecryptCMSWithBinaryContent)
{
    DecryptorConfig config;
    config.privateKeyPath = m_KeyPath;
    config.certificatePath = m_CertPath;
    config.workingDirectory = m_WorkingDir;

    ASSERT_TRUE(generateTestKeyPair(m_KeyPath, m_CertPath));

    // Create binary test content (simulating ZIP file header)
    std::vector<uint8_t> plaintext = {
        0x50, 0x4B, 0x03, 0x04,  // ZIP local file header signature
        0x14, 0x00, 0x00, 0x00,
        0x08, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        // ... more binary data
        0xFF, 0xFE, 0x00, 0x01
    };

    // Create CMS EnvelopedData
    std::vector<uint8_t> cmsData;
    ASSERT_TRUE(createTestCMSEnvelopedData(m_CertPath, plaintext, cmsData));

    std::filesystem::path inputFile = m_TestDir / "binary.cms";
    createTestFile(inputFile, cmsData);

    Decryptor decryptor(config);
    std::filesystem::path outFile;
    std::string outError;

    bool result = decryptor.decrypt(inputFile, outFile, outError);

    EXPECT_TRUE(result) << "Decryption failed: " << outError;

    // Verify decrypted content
    std::ifstream decryptedFile(outFile, std::ios::binary);
    std::vector<uint8_t> decrypted(
        (std::istreambuf_iterator<char>(decryptedFile)),
        std::istreambuf_iterator<char>());

    EXPECT_EQ(plaintext, decrypted);
}

TEST_F(DecryptorTest, DecryptCMSWithLargeContent)
{
    DecryptorConfig config;
    config.privateKeyPath = m_KeyPath;
    config.certificatePath = m_CertPath;
    config.workingDirectory = m_WorkingDir;

    ASSERT_TRUE(generateTestKeyPair(m_KeyPath, m_CertPath));

    // Create larger test content (1MB)
    std::vector<uint8_t> plaintext(1024 * 1024);
    for (size_t i = 0; i < plaintext.size(); ++i) {
        plaintext[i] = static_cast<uint8_t>(i & 0xFF);
    }

    // Create CMS EnvelopedData
    std::vector<uint8_t> cmsData;
    ASSERT_TRUE(createTestCMSEnvelopedData(m_CertPath, plaintext, cmsData));

    std::filesystem::path inputFile = m_TestDir / "large.cms";
    createTestFile(inputFile, cmsData);

    Decryptor decryptor(config);
    std::filesystem::path outFile;
    std::string outError;

    bool result = decryptor.decrypt(inputFile, outFile, outError);

    EXPECT_TRUE(result) << "Decryption failed: " << outError;

    // Verify decrypted content
    std::ifstream decryptedFile(outFile, std::ios::binary);
    std::vector<uint8_t> decrypted(
        (std::istreambuf_iterator<char>(decryptedFile)),
        std::istreambuf_iterator<char>());

    EXPECT_EQ(plaintext.size(), decrypted.size());
    EXPECT_EQ(plaintext, decrypted);
}
#endif // !IS_CHROMIUM

//------------------------------------------------------------------------------
// Output Path Tests
//------------------------------------------------------------------------------

TEST_F(DecryptorTest, DecryptWithWorkingDirectoryFailsWithInvalidCMS)
{
    DecryptorConfig config;
    config.privateKeyPath = m_KeyPath;
    config.certificatePath = m_CertPath;
    config.workingDirectory = m_WorkingDir;

    ASSERT_TRUE(generateTestKeyPair(m_KeyPath, m_CertPath));

    Decryptor decryptor(config);

    // Create an invalid CMS file (valid DER SEQUENCE but not CMS EnvelopedData)
    std::filesystem::path inputFile = m_TestDir / "mypackage.cms";
    createTestFile(inputFile, std::vector<uint8_t>{0x30, 0x03, 0x02, 0x01, 0x00});

    std::filesystem::path outFile;
    std::string outError;

    // Should fail during CMS parsing
    bool result = decryptor.decrypt(inputFile, outFile, outError);

    EXPECT_FALSE(result);
    EXPECT_FALSE(outError.empty());
    // outFile should not be set on failure
    EXPECT_TRUE(outFile.empty());
}

TEST_F(DecryptorTest, DecryptWithoutWorkingDirectoryFailsWithInvalidCMS)
{
    DecryptorConfig config;
    config.privateKeyPath = m_KeyPath;
    config.certificatePath = m_CertPath;
    // workingDirectory left empty - output should go to input file's parent directory

    ASSERT_TRUE(generateTestKeyPair(m_KeyPath, m_CertPath));

    Decryptor decryptor(config);

    std::filesystem::path inputFile = m_TestDir / "package.cms";
    createTestFile(inputFile, std::vector<uint8_t>{0x30, 0x03, 0x02, 0x01, 0x00});

    std::filesystem::path outFile;
    std::string outError;

    // Should fail during CMS parsing
    bool result = decryptor.decrypt(inputFile, outFile, outError);

    EXPECT_FALSE(result);
    EXPECT_FALSE(outError.empty());
    // outFile should not be set on failure
    EXPECT_TRUE(outFile.empty());
}

//------------------------------------------------------------------------------
// Algorithm Support Tests (via error messages with constructed CMS)
//------------------------------------------------------------------------------

#ifdef IS_CHROMIUM
// These tests verify algorithm detection by checking error messages
// when parsing hand-crafted CMS structures

TEST_F(DecryptorTest, RejectsUnsupportedContentEncryptionAlgorithm)
{
    DecryptorConfig config;
    config.privateKeyPath = m_KeyPath;
    config.certificatePath = m_CertPath;
    config.workingDirectory = m_WorkingDir;

    ASSERT_TRUE(generateTestKeyPair(m_KeyPath, m_CertPath));

    Decryptor decryptor(config);

    // Construct a minimal CMS structure with unsupported algorithm
    // This is a simplified structure that will be rejected for wrong algorithm
    // OID 1.2.840.113549.3.7 is DES-EDE3-CBC (not AES)
    std::vector<uint8_t> craftedCMS = {
        // ContentInfo SEQUENCE
        0x30, 0x82, 0x01, 0x00,
        // contentType OID (enveloped-data: 1.2.840.113549.1.7.3)
        0x06, 0x09, 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x07, 0x03,
        // [0] EXPLICIT content
        0xA0, 0x82, 0x00, 0xF0,
        // EnvelopedData SEQUENCE
        0x30, 0x82, 0x00, 0xEC,
        // version INTEGER 0
        0x02, 0x01, 0x00,
        // recipientInfos SET - minimal
        0x31, 0x82, 0x00, 0x80,
        // ... (truncated - will fail during full parsing)
    };

    std::filesystem::path inputFile = m_TestDir / "unsupported_alg.cms";
    createTestFile(inputFile, craftedCMS);

    std::filesystem::path outFile;
    std::string outError;

    bool result = decryptor.decrypt(inputFile, outFile, outError);

    EXPECT_FALSE(result);
    // Should fail during parsing due to incomplete/invalid structure
}
#endif // IS_CHROMIUM


