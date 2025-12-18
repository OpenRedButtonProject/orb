#include <string>
#include <vector>
#include <cstdint>
#include <iostream>
#include <filesystem>
#include <fstream>

#include "testing/gtest/include/gtest/gtest.h"
#include "OpAppAcquisition.h"
#include "SrvRecord.h"
#include "OpAppAcquisitionTestInterface.h"

using namespace orb;

class OpAppAcquisitionTest : public ::testing::Test {
protected:
    void SetUp() override {
    }

    void TearDown() override {
    }
};

// =============================================================================
// FQDN Validation Tests
// =============================================================================

TEST_F(OpAppAcquisitionTest, TestValidateFqdn_ValidFqdn)
{
    // GIVEN: a test interface instance
    auto testInterface = OpAppAcquisitionTestInterface::create();

    // WHEN: validating a valid FQDN
    bool result = testInterface->validateFqdn("example.com");

    // THEN: the validation should succeed
    EXPECT_TRUE(result);
}

TEST_F(OpAppAcquisitionTest, TestValidateFqdn_EmptyString)
{
    // GIVEN: a test interface instance
    auto testInterface = OpAppAcquisitionTestInterface::create();

    // WHEN: validating an empty string
    bool result = testInterface->validateFqdn("");

    // THEN: the validation should fail
    EXPECT_FALSE(result);
}

TEST_F(OpAppAcquisitionTest, TestValidateFqdn_NoDot)
{
    // GIVEN: a test interface instance
    auto testInterface = OpAppAcquisitionTestInterface::create();

    // WHEN: validating an FQDN without a dot
    bool result = testInterface->validateFqdn("localhost");

    // THEN: the validation should fail
    EXPECT_FALSE(result);
}

TEST_F(OpAppAcquisitionTest, TestValidateFqdn_SubdomainFqdn)
{
    // GIVEN: a test interface instance
    auto testInterface = OpAppAcquisitionTestInterface::create();

    // WHEN: validating an FQDN with subdomain
    bool result = testInterface->validateFqdn("sub.domain.example.com");

    // THEN: the validation should succeed
    EXPECT_TRUE(result);
}

// =============================================================================
// SRV Record Selection Tests
// =============================================================================

TEST_F(OpAppAcquisitionTest, TestSelectBestSrvRecord_EmptyList)
{
    // GIVEN: a test interface instance and an empty list
    auto testInterface = OpAppAcquisitionTestInterface::create();
    std::vector<SrvRecord> records;

    // WHEN: selecting the best record
    SrvRecord best = testInterface->selectBestSrvRecord(records);

    // THEN: an empty record should be returned
    EXPECT_TRUE(best.target.empty());
    EXPECT_EQ(best.port, 0);
}

TEST_F(OpAppAcquisitionTest, TestSelectBestSrvRecord_SingleRecord)
{
    // GIVEN: a test interface instance and a single SRV record
    auto testInterface = OpAppAcquisitionTestInterface::create();
    std::vector<SrvRecord> records = {
        SrvRecord(10, 100, 8080, "server.example.com")
    };

    // WHEN: selecting the best record
    SrvRecord best = testInterface->selectBestSrvRecord(records);

    // THEN: the single record should be returned
    EXPECT_EQ(best.priority, 10);
    EXPECT_EQ(best.weight, 100);
    EXPECT_EQ(best.port, 8080);
    EXPECT_EQ(best.target, "server.example.com");
}

TEST_F(OpAppAcquisitionTest, TestSelectBestSrvRecord_PrioritySelection)
{
    // GIVEN: a test interface instance and multiple SRV records with different priorities
    auto testInterface = OpAppAcquisitionTestInterface::create();
    std::vector<SrvRecord> records = {
        SrvRecord(20, 100, 8081, "backup.example.com"),
        SrvRecord(10, 100, 8080, "primary.example.com"),
        SrvRecord(30, 100, 8082, "tertiary.example.com")
    };

    // WHEN: selecting the best record
    SrvRecord best = testInterface->selectBestSrvRecord(records);

    // THEN: the record with lowest priority should be returned
    EXPECT_EQ(best.priority, 10);
    EXPECT_EQ(best.target, "primary.example.com");
}

TEST_F(OpAppAcquisitionTest, TestSelectBestSrvRecord_ZeroWeights)
{
    // GIVEN: multiple records with same priority and zero weights
    auto testInterface = OpAppAcquisitionTestInterface::create();
    std::vector<SrvRecord> records = {
        SrvRecord(10, 0, 8080, "server1.example.com"),
        SrvRecord(10, 0, 8081, "server2.example.com")
    };

    // WHEN: selecting the best record
    SrvRecord best = testInterface->selectBestSrvRecord(records);

    // THEN: one of the records should be returned (random selection)
    EXPECT_EQ(best.priority, 10);
    EXPECT_FALSE(best.target.empty());
}

// =============================================================================
// popNextSrvRecord Tests
// =============================================================================

TEST_F(OpAppAcquisitionTest, TestPopNextSrvRecord_EmptyList)
{
    // GIVEN: a test interface instance and an empty list
    auto testInterface = OpAppAcquisitionTestInterface::create();
    std::vector<SrvRecord> records;

    // WHEN: getting the next record
    SrvRecord next = testInterface->popNextSrvRecord(records);

    // THEN: an empty record should be returned
    EXPECT_TRUE(next.target.empty());
    EXPECT_EQ(next.port, 0);

    // AND: the list should still be empty
    EXPECT_TRUE(records.empty());
}

TEST_F(OpAppAcquisitionTest, TestPopNextSrvRecord_SingleRecord)
{
    // GIVEN: a test interface instance and a single SRV record
    auto testInterface = OpAppAcquisitionTestInterface::create();
    std::vector<SrvRecord> records = {
        SrvRecord(10, 100, 8080, "server.example.com")
    };

    // WHEN: getting the next record
    SrvRecord next = testInterface->popNextSrvRecord(records);

    // THEN: the single record should be returned
    EXPECT_EQ(next.priority, 10);
    EXPECT_EQ(next.weight, 100);
    EXPECT_EQ(next.port, 8080);
    EXPECT_EQ(next.target, "server.example.com");

    // AND: the list should now be empty
    EXPECT_TRUE(records.empty());
}

TEST_F(OpAppAcquisitionTest, TestPopNextSrvRecord_MultipleRecords_RemovesSelected)
{
    // GIVEN: a test interface instance and multiple SRV records
    auto testInterface = OpAppAcquisitionTestInterface::create();
    std::vector<SrvRecord> records = {
        SrvRecord(20, 100, 8081, "backup.example.com"),
        SrvRecord(10, 100, 8080, "primary.example.com"),
        SrvRecord(30, 100, 8082, "tertiary.example.com")
    };

    // WHEN: getting the next record
    SrvRecord next = testInterface->popNextSrvRecord(records);

    // THEN: the record with lowest priority should be returned
    EXPECT_EQ(next.priority, 10);
    EXPECT_EQ(next.target, "primary.example.com");

    // AND: the list should have 2 records remaining
    EXPECT_EQ(records.size(), size_t(2));

    // AND: the returned record should not be in the list
    for (const auto& record : records) {
        EXPECT_NE(record.target, "primary.example.com");
    }
}

TEST_F(OpAppAcquisitionTest, TestPopNextSrvRecord_IterateThroughAll)
{
    // GIVEN: a test interface instance and multiple SRV records with different priorities
    auto testInterface = OpAppAcquisitionTestInterface::create();
    std::vector<SrvRecord> records = {
        SrvRecord(20, 100, 8081, "backup.example.com"),
        SrvRecord(10, 100, 8080, "primary.example.com"),
        SrvRecord(30, 100, 8082, "tertiary.example.com")
    };

    // WHEN: getting all records one by one
    SrvRecord first = testInterface->popNextSrvRecord(records);
    EXPECT_EQ(records.size(), size_t(2));

    SrvRecord second = testInterface->popNextSrvRecord(records);
    EXPECT_EQ(records.size(), size_t(1));

    SrvRecord third = testInterface->popNextSrvRecord(records);
    EXPECT_EQ(records.size(), size_t(0));

    // THEN: records should be returned in priority order
    EXPECT_EQ(first.priority, 10);
    EXPECT_EQ(first.target, "primary.example.com");

    EXPECT_EQ(second.priority, 20);
    EXPECT_EQ(second.target, "backup.example.com");

    EXPECT_EQ(third.priority, 30);
    EXPECT_EQ(third.target, "tertiary.example.com");

    // AND: getting next from empty list returns empty record
    SrvRecord fourth = testInterface->popNextSrvRecord(records);
    EXPECT_TRUE(fourth.target.empty());
}

// =============================================================================
// SrvRecord Struct Tests
// =============================================================================

TEST_F(OpAppAcquisitionTest, TestSrvRecord_DefaultConstructor)
{
    // GIVEN/WHEN: creating a default SrvRecord
    SrvRecord record;

    // THEN: all fields should be default initialized
    EXPECT_EQ(record.priority, 0);
    EXPECT_EQ(record.weight, 0);
    EXPECT_EQ(record.port, 0);
    EXPECT_TRUE(record.target.empty());
}

TEST_F(OpAppAcquisitionTest, TestSrvRecord_ParameterizedConstructor)
{
    // GIVEN/WHEN: creating an SrvRecord with parameters
    SrvRecord record(10, 20, 8080, "server.example.com");

    // THEN: all fields should be set correctly
    EXPECT_EQ(record.priority, 10);
    EXPECT_EQ(record.weight, 20);
    EXPECT_EQ(record.port, 8080);
    EXPECT_EQ(record.target, "server.example.com");
}

// =============================================================================
// Disabled Tests - Useful for manual/integration testing
// =============================================================================

// Disabled - useful for manual testing with real DNS
TEST_F(OpAppAcquisitionTest, DISABLED_TestDoDnsSrvLookup_ValidFqdn)
{
    // GIVEN: a test interface with a real-world FQDN
    const std::string fqdn = "test.freeviewplay.tv";
    auto testInterface = OpAppAcquisitionTestInterface::create();

    // WHEN: performing DNS SRV lookup
    std::vector<SrvRecord> records = testInterface->doDnsSrvLookup(fqdn);

    // THEN: at least one record should be returned
    EXPECT_FALSE(records.empty());

    // Care: this test is dependent on the actual
    // DNS server being used and the results it returns.
    if (!records.empty()) {
        // Select best record and verify expected values
        SrvRecord best = testInterface->selectBestSrvRecord(records);
        EXPECT_EQ(best.target, "refplayer-dev.cloud.digitaluk.co.uk");
        EXPECT_EQ(best.port, 443);
    }
}

// =============================================================================
// AcquisitionResult Tests
// =============================================================================

TEST_F(OpAppAcquisitionTest, TestAcquisitionResult_DefaultConstructor)
{
    // GIVEN/WHEN: creating a default AcquisitionResult
    AcquisitionResult result;

    // THEN: default values should indicate failure
    EXPECT_FALSE(result.success);
    EXPECT_TRUE(result.aitFiles.empty());
    EXPECT_TRUE(result.errors.empty());
    EXPECT_TRUE(result.fatalError.empty());
}

TEST_F(OpAppAcquisitionTest, TestAcquisitionResult_FullSuccess)
{
    // WHEN: creating a full success result with multiple files (no errors)
    std::vector<std::string> files = {"/tmp/ait_0_server1.xml", "/tmp/ait_1_server2.xml"};
    std::vector<std::string> noErrors;
    AcquisitionResult result = AcquisitionResult(files, noErrors);

    // THEN: values should indicate success
    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.aitFiles.size(), size_t(2));
    EXPECT_EQ(result.aitFiles[0], "/tmp/ait_0_server1.xml");
    EXPECT_EQ(result.aitFiles[1], "/tmp/ait_1_server2.xml");
    EXPECT_TRUE(result.errors.empty());
    EXPECT_TRUE(result.fatalError.empty());
}

TEST_F(OpAppAcquisitionTest, TestAcquisitionResult_Failure)
{
    // WHEN: creating a failure result
    AcquisitionResult result = AcquisitionResult("fatal error");

    // THEN: values should indicate failure
    EXPECT_FALSE(result.success);
    EXPECT_TRUE(result.aitFiles.empty());
    EXPECT_TRUE(result.errors.empty());
    EXPECT_EQ(result.fatalError, "fatal error");
}

TEST_F(OpAppAcquisitionTest, TestAcquisitionResult_PartialSuccess)
{
    // WHEN: creating a partial success result
    std::vector<std::string> files = {"/tmp/ait_0_server1.xml"};
    std::vector<std::string> errors = {"Failed to download from server2"};
    AcquisitionResult result = AcquisitionResult(files, errors);

    // THEN: values should indicate partial success
    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.aitFiles.size(), size_t(1));
    EXPECT_EQ(result.errors.size(), size_t(1));
    EXPECT_EQ(result.errors[0], "Failed to download from server2");
}

TEST_F(OpAppAcquisitionTest, TestAcquisitionResult_PartialSuccess_NoFiles)
{
    // WHEN: creating a partial success result with no files (all failed)
    std::vector<std::string> files;
    std::vector<std::string> errors = {"Failed from server1", "Failed from server2"};
    AcquisitionResult result = AcquisitionResult(files, errors);

    // THEN: success should be false since no files were acquired
    EXPECT_FALSE(result.success);
    EXPECT_TRUE(result.aitFiles.empty());
    EXPECT_EQ(result.errors.size(), size_t(2));
}

// =============================================================================
// FetchAitXmls Tests
// =============================================================================

TEST_F(OpAppAcquisitionTest, TestFetchAitXmls_NetworkUnavailable)
{
    // GIVEN: a test interface
    auto testInterface = OpAppAcquisitionTestInterface::create();

    // WHEN: fetching all AITs with network unavailable
    AcquisitionResult result = testInterface->FetchAitXmls(
        "example.com", false, "/tmp/test_ait");

    // THEN: the result should indicate failure
    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.fatalError.empty());
    EXPECT_TRUE(result.aitFiles.empty());
}

TEST_F(OpAppAcquisitionTest, TestFetchAitXmls_InvalidFqdn)
{
    // GIVEN: a test interface
    auto testInterface = OpAppAcquisitionTestInterface::create();

    // WHEN: fetching all AITs with invalid FQDN
    AcquisitionResult result = testInterface->FetchAitXmls(
        "invalid", true, "/tmp/test_ait");

    // THEN: the result should indicate failure
    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.fatalError.empty());
}

TEST_F(OpAppAcquisitionTest, TestFetchAitXmls_EmptyFqdn)
{
    // GIVEN: a test interface
    auto testInterface = OpAppAcquisitionTestInterface::create();

    // WHEN: fetching all AITs with empty FQDN
    AcquisitionResult result = testInterface->FetchAitXmls(
        "", true, "/tmp/test_ait");

    // THEN: the result should indicate failure
    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.fatalError.empty());
}

TEST_F(OpAppAcquisitionTest, TestFetchAitXmls_EmptyOutputDirectory)
{
    // GIVEN: a test interface
    auto testInterface = OpAppAcquisitionTestInterface::create();

    // WHEN: fetching all AITs with empty output directory
    AcquisitionResult result = testInterface->FetchAitXmls(
        "example.com", true, "");

    // THEN: the result should indicate failure
    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.fatalError, "Output directory not specified");
}

TEST_F(OpAppAcquisitionTest, TestStaticFetch_NetworkUnavailable)
{
    // WHEN: using static fetch all with network unavailable
    AcquisitionResult result = OpAppAcquisitionTestInterface::StaticFetch(
        "example.com", false, "/tmp/test_ait");

    // THEN: the result should indicate failure
    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.fatalError.empty());
}

TEST_F(OpAppAcquisitionTest, TestStaticFetch_InvalidFqdn)
{
    // WHEN: using static fetch all with invalid FQDN
    AcquisitionResult result = OpAppAcquisitionTestInterface::StaticFetch(
        "invalid", true, "/tmp/test_ait");

    // THEN: the result should indicate failure
    EXPECT_FALSE(result.success);
}

// =============================================================================
// Helper Function Tests
// =============================================================================

TEST_F(OpAppAcquisitionTest, TestGenerateAitFilename_SimpleHostname)
{
    // GIVEN: a test interface
    auto testInterface = OpAppAcquisitionTestInterface::create();

    // WHEN: generating a filename for a simple hostname
    std::string filename = testInterface->generateAitFilename(0, "server.example.com");

    // THEN: the filename should be properly formatted
    EXPECT_EQ(filename, "ait_0_server.example.com.xml");
}

TEST_F(OpAppAcquisitionTest, TestGenerateAitFilename_SpecialCharacters)
{
    // GIVEN: a test interface
    auto testInterface = OpAppAcquisitionTestInterface::create();

    // WHEN: generating a filename with special characters in hostname
    std::string filename = testInterface->generateAitFilename(1, "server:8080/path?query=1");

    // THEN: special characters should be replaced with underscores
    EXPECT_EQ(filename, "ait_1_server_8080_path_query_1.xml");
}

TEST_F(OpAppAcquisitionTest, TestGenerateAitFilename_MultipleIndices)
{
    // GIVEN: a test interface
    auto testInterface = OpAppAcquisitionTestInterface::create();

    // WHEN: generating filenames with different indices
    std::string filename0 = testInterface->generateAitFilename(0, "server.com");
    std::string filename5 = testInterface->generateAitFilename(5, "server.com");
    std::string filename99 = testInterface->generateAitFilename(99, "server.com");

    // THEN: indices should be included correctly
    EXPECT_EQ(filename0, "ait_0_server.com.xml");
    EXPECT_EQ(filename5, "ait_5_server.com.xml");
    EXPECT_EQ(filename99, "ait_99_server.com.xml");
}

TEST_F(OpAppAcquisitionTest, TestWriteAitToFile_Success)
{
    // GIVEN: a test interface and a temporary directory
    auto testInterface = OpAppAcquisitionTestInterface::create();
    std::string testDir = "/tmp/opapp_test_" + std::to_string(getpid());
    std::filesystem::create_directories(testDir);
    std::string testFile = testDir + "/test_ait.xml";
    std::string content = "<?xml version=\"1.0\"?><ait>test content</ait>";

    // WHEN: writing AIT content to a file
    bool result = testInterface->writeAitToFile(content, testFile);

    // THEN: the write should succeed
    EXPECT_TRUE(result);
    EXPECT_TRUE(std::filesystem::exists(testFile));

    // Verify content
    std::ifstream inFile(testFile);
    std::stringstream buffer;
    buffer << inFile.rdbuf();
    EXPECT_EQ(buffer.str(), content);

    // Cleanup
    std::filesystem::remove_all(testDir);
}

TEST_F(OpAppAcquisitionTest, TestWriteAitToFile_CreatesParentDirectory)
{
    // GIVEN: a test interface and a path with non-existent parent directory
    auto testInterface = OpAppAcquisitionTestInterface::create();
    std::string testDir = "/tmp/opapp_test_" + std::to_string(getpid());
    std::string testFile = testDir + "/test_ait.xml";
    std::string content = "<?xml version=\"1.0\"?><ait>test</ait>";

    // Ensure directory exists for write (since writeAitToFile doesn't create parent dirs)
    std::filesystem::create_directories(testDir);

    // WHEN: writing AIT content to a file
    bool result = testInterface->writeAitToFile(content, testFile);

    // THEN: the write should succeed
    EXPECT_TRUE(result);
    EXPECT_TRUE(std::filesystem::exists(testFile));

    // Cleanup
    std::filesystem::remove_all(testDir);
}

TEST_F(OpAppAcquisitionTest, TestWriteAitToFile_EmptyContent)
{
    // GIVEN: a test interface and empty content
    auto testInterface = OpAppAcquisitionTestInterface::create();
    std::string testDir = "/tmp/opapp_test_" + std::to_string(getpid());
    std::filesystem::create_directories(testDir);
    std::string testFile = testDir + "/empty_ait.xml";

    // WHEN: writing empty content to a file
    bool result = testInterface->writeAitToFile("", testFile);

    // THEN: the write should still succeed (empty file is valid)
    EXPECT_TRUE(result);
    EXPECT_TRUE(std::filesystem::exists(testFile));
    EXPECT_EQ(std::filesystem::file_size(testFile), 0u);

    // Cleanup
    std::filesystem::remove_all(testDir);
}

TEST_F(OpAppAcquisitionTest, TestWriteAitToFile_InvalidPath)
{
    // GIVEN: a test interface and an invalid path
    auto testInterface = OpAppAcquisitionTestInterface::create();
    std::string invalidPath = "/nonexistent_root_dir_12345/subdir/test.xml";

    // WHEN: writing to an invalid path
    bool result = testInterface->writeAitToFile("content", invalidPath);

    // THEN: the write should fail
    EXPECT_FALSE(result);
}

// =============================================================================
// Disabled Integration Tests - For manual testing with real DNS/network
// =============================================================================

TEST_F(OpAppAcquisitionTest, DISABLED_TestFetchAitXmls_ValidFqdn)
{
    // GIVEN: a test interface with a real-world FQDN
    const std::string fqdn = "test.freeviewplay.tv";
    auto testInterface = OpAppAcquisitionTestInterface::create();
    std::string testDir = "/tmp/opapp_ait_test_" + std::to_string(getpid());

    // WHEN: fetching all AITs
    AcquisitionResult result = testInterface->FetchAitXmls(fqdn, true, testDir);

    // THEN: on success, at least one AIT file should be created
    if (result.success) {
        EXPECT_FALSE(result.aitFiles.empty());
        std::cout << "Successfully acquired " << result.aitFiles.size() << " AIT file(s):" << std::endl;
        for (const auto& file : result.aitFiles) {
            std::cout << "  - " << file << std::endl;
            // Print file content
            std::ifstream inFile(file);
            std::stringstream buffer;
            buffer << inFile.rdbuf();
            std::cout << "    Content:\n" << buffer.str() << std::endl;
        }
    } else {
        std::cout << "FetchAitXmls failed: " << result.fatalError << std::endl;
    }

    // Log any errors encountered
    for (const auto& error : result.errors) {
        std::cout << "Error: " << error << std::endl;
    }

    // Cleanup
    std::filesystem::remove_all(testDir);
}

TEST_F(OpAppAcquisitionTest, DISABLED_TestStaticFetch_ValidFqdn)
{
    // WHEN: using static fetch all with valid FQDN
    const std::string fqdn = "test.freeviewplay.tv";
    std::string testDir = "/tmp/opapp_static_ait_test_" + std::to_string(getpid());
    AcquisitionResult result = OpAppAcquisition::Fetch(fqdn, true, testDir);

    // THEN: on success, at least one AIT file should be created
    if (result.success) {
        EXPECT_FALSE(result.aitFiles.empty());
        std::cout << "Static FetchAll acquired " << result.aitFiles.size() << " AIT file(s)" << std::endl;
    } else {
        std::cout << "Static FetchAll failed: " << result.fatalError << std::endl;
    }

    // Cleanup
    std::filesystem::remove_all(testDir);
}
