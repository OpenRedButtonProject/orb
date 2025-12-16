#include <string>
#include <vector>
#include <cstdint>
#include <iostream>

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
// FetchAitXml Tests (new simplified interface)
// =============================================================================

TEST_F(OpAppAcquisitionTest, TestFetchAitXml_NetworkUnavailable)
{
    // GIVEN: a test interface
    auto testInterface = OpAppAcquisitionTestInterface::create();

    // WHEN: fetching AIT XML with network unavailable
    AcquisitionResult result = testInterface->FetchAitXml("example.com", false);

    // THEN: the result should indicate failure
    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.errorMessage.empty());
    EXPECT_TRUE(result.content.empty());
}

TEST_F(OpAppAcquisitionTest, TestFetchAitXml_InvalidFqdn)
{
    // GIVEN: a test interface
    auto testInterface = OpAppAcquisitionTestInterface::create();

    // WHEN: fetching AIT XML with invalid FQDN
    AcquisitionResult result = testInterface->FetchAitXml("invalid", true);

    // THEN: the result should indicate failure
    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.errorMessage.empty());
}

TEST_F(OpAppAcquisitionTest, TestFetchAitXml_EmptyFqdn)
{
    // GIVEN: a test interface
    auto testInterface = OpAppAcquisitionTestInterface::create();

    // WHEN: fetching AIT XML with empty FQDN
    AcquisitionResult result = testInterface->FetchAitXml("", true);

    // THEN: the result should indicate failure
    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.errorMessage.empty());
}

TEST_F(OpAppAcquisitionTest, TestStaticFetch_NetworkUnavailable)
{
    // WHEN: using static fetch with network unavailable
    AcquisitionResult result = OpAppAcquisitionTestInterface::StaticFetch("example.com", false);

    // THEN: the result should indicate failure
    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.errorMessage.empty());
}

TEST_F(OpAppAcquisitionTest, TestStaticFetch_InvalidFqdn)
{
    // WHEN: using static fetch with invalid FQDN
    AcquisitionResult result = OpAppAcquisitionTestInterface::StaticFetch("invalid", true);

    // THEN: the result should indicate failure
    EXPECT_FALSE(result.success);
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
    EXPECT_TRUE(result.content.empty());
    EXPECT_TRUE(result.errorMessage.empty());
    EXPECT_EQ(result.statusCode, -1);
}

TEST_F(OpAppAcquisitionTest, TestAcquisitionResult_Success)
{
    // WHEN: creating a success result
    AcquisitionResult result = AcquisitionResult::Success("test content", 200);

    // THEN: values should indicate success
    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.content, "test content");
    EXPECT_TRUE(result.errorMessage.empty());
    EXPECT_EQ(result.statusCode, 200);
}

TEST_F(OpAppAcquisitionTest, TestAcquisitionResult_Failure)
{
    // WHEN: creating a failure result
    AcquisitionResult result = AcquisitionResult::Failure("error message");

    // THEN: values should indicate failure
    EXPECT_FALSE(result.success);
    EXPECT_TRUE(result.content.empty());
    EXPECT_EQ(result.errorMessage, "error message");
    EXPECT_EQ(result.statusCode, -1);
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

TEST_F(OpAppAcquisitionTest, DISABLED_TestFetchAitXml_ValidFqdn)
{
    // GIVEN: a test interface with a real-world FQDN
    const std::string fqdn = "test.freeviewplay.tv";
    auto testInterface = OpAppAcquisitionTestInterface::create();

    // WHEN: fetching AIT XML
    AcquisitionResult result = testInterface->FetchAitXml(fqdn, true);

    // THEN: on success, content should not be empty
    if (result.success) {
        EXPECT_FALSE(result.content.empty());
        std::cout << "Content:\n\n" << result.content << std::endl;
    } else {
        std::cout << "Fetch failed: " << result.errorMessage << std::endl;
    }
}

TEST_F(OpAppAcquisitionTest, DISABLED_TestStaticFetch_ValidFqdn)
{
    // WHEN: using static fetch with valid FQDN
    const std::string fqdn = "test.freeviewplay.tv";
    AcquisitionResult result = OpAppAcquisition::Fetch(fqdn, true);

    // THEN: on success, content should not be empty
    if (result.success) {
        EXPECT_FALSE(result.content.empty());
        std::cout << "Static fetch content:\n\n" << result.content << std::endl;
    } else {
        std::cout << "Static fetch failed: " << result.errorMessage << std::endl;
    }
}
