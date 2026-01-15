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
 * Unit tests for the CMS SignedData Verifier
 *
 * Implements testing for TS 103 606 Section 11.3.4.5 signature verification.
 */

#include <iostream>
#include <string>
#include <filesystem>
#include <fstream>
#include <vector>
#include <cstdlib>

#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/orb/orblibrary/package_manager/Verifier.h"

#ifdef IS_CHROMIUM
#include "third_party/boringssl/src/include/openssl/bio.h"
#include "third_party/boringssl/src/include/openssl/evp.h"
#include "third_party/boringssl/src/include/openssl/pem.h"
#include "third_party/boringssl/src/include/openssl/x509.h"
#include "third_party/boringssl/src/include/openssl/rand.h"
#else
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/rand.h>
#include <openssl/cms.h>
#endif

using namespace orb;

namespace {

// Test fixture directory - uses a unique temp directory per test run
std::filesystem::path getTestDir() {
    static std::filesystem::path testDir;
    if (testDir.empty()) {
        testDir = std::filesystem::temp_directory_path() / "verifier_tests";
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

} // anonymous namespace

class VerifierTest : public ::testing::Test {
protected:
    void SetUp() override {
        m_TestDir = getTestDir() / "current_test";
        std::filesystem::create_directories(m_TestDir);
    }

    void TearDown() override {
        // Clean up test directory
        std::error_code ec;
        std::filesystem::remove_all(m_TestDir, ec);
    }

    std::filesystem::path m_TestDir;

    void createDummyFile(const std::filesystem::path& path, const std::vector<uint8_t>& content) {
        createTestFile(path, content);
    }

    void createDummyFile(const std::filesystem::path& path, const std::string& content) {
        createTestFile(path, content);
    }
};

// =============================================================================
// Configuration Tests
// =============================================================================

TEST_F(VerifierTest, DefaultConstructorNotConfigured)
{
    Verifier verifier;
    EXPECT_FALSE(verifier.isConfigured())
        << "Default constructed verifier should not be configured";
}

TEST_F(VerifierTest, ConfiguredWithRootCAOnly)
{
    // Create a dummy Root CA file
    std::filesystem::path rootCAPath = m_TestDir / "root_ca.pem";
    createDummyFile(rootCAPath, "-----BEGIN CERTIFICATE-----\ntest\n-----END CERTIFICATE-----\n");

    VerifierConfig config;
    config.operatorRootCAPath = rootCAPath;

    Verifier verifier(config);
    EXPECT_FALSE(verifier.isConfigured())
        << "Verifier with only Root CA path should NOT be configured (missing operator identity)";
}

TEST_F(VerifierTest, ConfiguredWithAllRequiredFields)
{
    // Create a dummy Root CA file
    std::filesystem::path rootCAPath = m_TestDir / "root_ca.pem";
    createDummyFile(rootCAPath, "-----BEGIN CERTIFICATE-----\ntest\n-----END CERTIFICATE-----\n");

    VerifierConfig config;
    config.operatorRootCAPath = rootCAPath;
    config.expectedOperatorName = "Test Operator Ltd";
    config.expectedOrganisationId = "org123";

    Verifier verifier(config);
    EXPECT_TRUE(verifier.isConfigured())
        << "Verifier with all required fields should be configured";
}

TEST_F(VerifierTest, SetConfigAfterConstruction)
{
    Verifier verifier;
    EXPECT_FALSE(verifier.isConfigured());

    // Create a dummy Root CA file
    std::filesystem::path rootCAPath = m_TestDir / "root_ca.pem";
    createDummyFile(rootCAPath, "-----BEGIN CERTIFICATE-----\ntest\n-----END CERTIFICATE-----\n");

    VerifierConfig config;
    config.operatorRootCAPath = rootCAPath;
    config.expectedOperatorName = "Test Operator Ltd";
    config.expectedOrganisationId = "org123";
    verifier.setConfig(config);

    EXPECT_TRUE(verifier.isConfigured())
        << "Verifier should be configured after setConfig()";
}

// =============================================================================
// Error Handling Tests
// =============================================================================

TEST_F(VerifierTest, VerifyFailsWhenNotConfigured)
{
    Verifier verifier;

    std::filesystem::path inputFile = m_TestDir / "test.cms";
    createDummyFile(inputFile, std::vector<uint8_t>{0x30, 0x00}); // Minimal DER

    std::filesystem::path outZipPath;
    std::string outError;

    bool result = verifier.verify(inputFile, outZipPath, outError);

    EXPECT_FALSE(result) << "Verify should fail when not configured";
    EXPECT_FALSE(outError.empty()) << "Error message should be set";
    EXPECT_NE(outError.find("not configured"), std::string::npos)
        << "Error should mention not configured";
}

TEST_F(VerifierTest, VerifyFailsWhenInputFileMissing)
{
    std::filesystem::path rootCAPath = m_TestDir / "root_ca.pem";
    createDummyFile(rootCAPath, "-----BEGIN CERTIFICATE-----\ntest\n-----END CERTIFICATE-----\n");

    VerifierConfig config;
    config.operatorRootCAPath = rootCAPath;
    config.expectedOperatorName = "Test Operator Ltd";
    config.expectedOrganisationId = "org123";

    Verifier verifier(config);

    std::filesystem::path inputFile = m_TestDir / "nonexistent.cms";
    std::filesystem::path outZipPath;
    std::string outError;

    bool result = verifier.verify(inputFile, outZipPath, outError);

    EXPECT_FALSE(result) << "Verify should fail when input file is missing";
    EXPECT_FALSE(outError.empty()) << "Error message should be set";
    EXPECT_NE(outError.find("does not exist"), std::string::npos)
        << "Error should mention file does not exist";
}

TEST_F(VerifierTest, VerifyFailsWhenRootCAFileMissing)
{
    VerifierConfig config;
    config.operatorRootCAPath = m_TestDir / "nonexistent_ca.pem";
    config.expectedOperatorName = "Test Operator Ltd";
    config.expectedOrganisationId = "org123";

    Verifier verifier(config);

    std::filesystem::path inputFile = m_TestDir / "test.cms";
    createDummyFile(inputFile, std::vector<uint8_t>{0x30, 0x00}); // Minimal DER

    std::filesystem::path outZipPath;
    std::string outError;

    bool result = verifier.verify(inputFile, outZipPath, outError);

    EXPECT_FALSE(result) << "Verify should fail when Root CA file is missing";
    EXPECT_FALSE(outError.empty()) << "Error message should be set";
    EXPECT_NE(outError.find("does not exist"), std::string::npos)
        << "Error should mention file does not exist";
}

TEST_F(VerifierTest, VerifyFailsWithEmptyInputFile)
{
    std::filesystem::path rootCAPath = m_TestDir / "root_ca.pem";
    createDummyFile(rootCAPath, "-----BEGIN CERTIFICATE-----\ntest\n-----END CERTIFICATE-----\n");

    VerifierConfig config;
    config.operatorRootCAPath = rootCAPath;
    config.expectedOperatorName = "Test Operator Ltd";
    config.expectedOrganisationId = "org123";

    Verifier verifier(config);

    std::filesystem::path inputFile = m_TestDir / "empty.cms";
    createDummyFile(inputFile, std::vector<uint8_t>{}); // Empty file

    std::filesystem::path outZipPath;
    std::string outError;

    bool result = verifier.verify(inputFile, outZipPath, outError);

    EXPECT_FALSE(result) << "Verify should fail with empty input file";
    EXPECT_FALSE(outError.empty()) << "Error message should be set";
}

TEST_F(VerifierTest, VerifyFailsWithInvalidCMS)
{
    std::filesystem::path rootCAPath = m_TestDir / "root_ca.pem";
    createDummyFile(rootCAPath, "-----BEGIN CERTIFICATE-----\ntest\n-----END CERTIFICATE-----\n");

    VerifierConfig config;
    config.operatorRootCAPath = rootCAPath;
    config.expectedOperatorName = "Test Operator Ltd";
    config.expectedOrganisationId = "org123";

    Verifier verifier(config);

    // Create invalid CMS data (just random bytes)
    std::filesystem::path inputFile = m_TestDir / "invalid.cms";
    createDummyFile(inputFile, std::vector<uint8_t>{0x01, 0x02, 0x03, 0x04, 0x05});

    std::filesystem::path outZipPath;
    std::string outError;

    bool result = verifier.verify(inputFile, outZipPath, outError);

    EXPECT_FALSE(result) << "Verify should fail with invalid CMS data";
    EXPECT_FALSE(outError.empty()) << "Error message should be set";
}

// =============================================================================
// Working Directory Tests
// =============================================================================

TEST_F(VerifierTest, OutputFileInWorkingDirectory)
{
    std::filesystem::path rootCAPath = m_TestDir / "root_ca.pem";
    createDummyFile(rootCAPath, "-----BEGIN CERTIFICATE-----\ntest\n-----END CERTIFICATE-----\n");

    std::filesystem::path workingDir = m_TestDir / "output";
    std::filesystem::create_directories(workingDir);

    VerifierConfig config;
    config.operatorRootCAPath = rootCAPath;
    config.expectedOperatorName = "Test Operator Ltd";
    config.expectedOrganisationId = "org123";
    config.workingDirectory = workingDir;

    Verifier verifier(config);

    // This test just verifies the Verifier is configured correctly
    // Actual verification requires valid CMS SignedData
    EXPECT_TRUE(verifier.isConfigured());
}

// =============================================================================
// Real File Verification Test (Using Environment Variables)
// =============================================================================

/**
 * This test allows verification with real CMS SignedData files.
 * Set these environment variables to run:
 *   VERIFIER_TEST_ROOT_CA          - Path to Operator Signing Root CA (PEM)
 *   VERIFIER_TEST_CMS              - Path to CMS SignedData file to verify (DER)
 *   VERIFIER_TEST_OPERATOR_NAME    - Expected O= from signer cert (REQUIRED)
 *   VERIFIER_TEST_ORGANISATION_ID  - Expected CN= from signer cert (REQUIRED)
 *
 * Example:
 *   VERIFIER_TEST_ROOT_CA=/path/to/EveryoneTV-Root-CA-2024.pem \
 *   VERIFIER_TEST_CMS=/path/to/opapp_decrypted.cms \
 *   VERIFIER_TEST_OPERATOR_NAME="EveryoneTV Ltd" \
 *   VERIFIER_TEST_ORGANISATION_ID="EveryoneTV MAV CA 2024.1" \
 *   ./test_orb_all --gtest_filter=VerifierTest.RealFileVerification
 */
TEST_F(VerifierTest, RealFileVerification)
{
    // Get environment variables
    const char* rootCAEnv = std::getenv("VERIFIER_TEST_ROOT_CA");
    const char* cmsEnv = std::getenv("VERIFIER_TEST_CMS");
    const char* operatorNameEnv = std::getenv("VERIFIER_TEST_OPERATOR_NAME");
    const char* organisationIdEnv = std::getenv("VERIFIER_TEST_ORGANISATION_ID");

    // Skip if required environment variables are not set
    if (!rootCAEnv || !cmsEnv || !operatorNameEnv || !organisationIdEnv) {
        GTEST_SKIP() << "Skipping RealFileVerification test. "
                     << "Set VERIFIER_TEST_ROOT_CA, VERIFIER_TEST_CMS, "
                     << "VERIFIER_TEST_OPERATOR_NAME, and VERIFIER_TEST_ORGANISATION_ID "
                     << "environment variables to run this test.";
    }

    std::filesystem::path rootCAPath(rootCAEnv);
    std::filesystem::path cmsFilePath(cmsEnv);

    // Validate files exist
    if (!std::filesystem::exists(rootCAPath)) {
        GTEST_SKIP() << "Root CA file does not exist: " << rootCAPath;
    }
    if (!std::filesystem::exists(cmsFilePath)) {
        GTEST_SKIP() << "CMS file does not exist: " << cmsFilePath;
    }

    // Configure verifier - all fields are required
    VerifierConfig config;
    config.operatorRootCAPath = rootCAPath;
    config.expectedOperatorName = operatorNameEnv;
    config.expectedOrganisationId = organisationIdEnv;
    config.workingDirectory = cmsFilePath.parent_path();

    Verifier verifier(config);

    std::filesystem::path outZipPath;
    std::string outError;

    std::cout << "Verifying: " << cmsFilePath << std::endl;
    std::cout << "Using Root CA: " << rootCAPath << std::endl;
    std::cout << "Expected Operator Name (O=): " << operatorNameEnv << std::endl;
    std::cout << "Expected Organisation ID (CN=): " << organisationIdEnv << std::endl;

    bool result = verifier.verify(cmsFilePath, outZipPath, outError);

    if (result) {
        std::cout << "SUCCESS: Verified and extracted to " << outZipPath << std::endl;
        EXPECT_TRUE(std::filesystem::exists(outZipPath))
            << "Extracted ZIP file should exist at: " << outZipPath;
        auto fileSize = std::filesystem::file_size(outZipPath);
        std::cout << "Extracted ZIP size: " << fileSize << " bytes" << std::endl;
    } else {
        std::cout << "FAILED: " << outError << std::endl;
    }

    EXPECT_TRUE(result) << "Verification failed: " << outError;
}

// =============================================================================
// Operator Identity Validation Tests
// =============================================================================

TEST_F(VerifierTest, ConfigWithOperatorIdentity)
{
    std::filesystem::path rootCAPath = m_TestDir / "root_ca.pem";
    createDummyFile(rootCAPath, "-----BEGIN CERTIFICATE-----\ntest\n-----END CERTIFICATE-----\n");

    VerifierConfig config;
    config.operatorRootCAPath = rootCAPath;
    config.expectedOperatorName = "Test Operator Ltd";
    config.expectedOrganisationId = "org123";

    Verifier verifier(config);
    EXPECT_TRUE(verifier.isConfigured())
        << "Verifier should be configured with operator identity fields";
}

// =============================================================================
// IVerifier Interface Tests
// =============================================================================

TEST_F(VerifierTest, ImplementsIVerifierInterface)
{
    std::filesystem::path rootCAPath = m_TestDir / "root_ca.pem";
    createDummyFile(rootCAPath, "-----BEGIN CERTIFICATE-----\ntest\n-----END CERTIFICATE-----\n");

    VerifierConfig config;
    config.operatorRootCAPath = rootCAPath;
    config.expectedOperatorName = "Test Operator Ltd";
    config.expectedOrganisationId = "org123";

    // Verify Verifier can be used through IVerifier interface
    std::unique_ptr<IVerifier> verifier = std::make_unique<Verifier>(config);

    std::filesystem::path inputFile = m_TestDir / "test.cms";
    createDummyFile(inputFile, std::vector<uint8_t>{0x30, 0x00}); // Invalid but tests interface

    std::filesystem::path outZipPath;
    std::string outError;

    // This will fail due to invalid CMS, but it tests the interface
    bool result = verifier->verify(inputFile, outZipPath, outError);

    // We expect failure because the CMS data is invalid
    EXPECT_FALSE(result);
    EXPECT_FALSE(outError.empty());
}

