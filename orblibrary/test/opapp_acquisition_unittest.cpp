#include <string>
#include <vector>
#include <cstdint>

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
    auto testInterface = OpAppAcquisitionTestInterface::create("test.example.com", true);

    // WHEN: validating a valid FQDN
    bool result = testInterface->validateFqdn("example.com");

    // THEN: the validation should succeed
    EXPECT_TRUE(result);
}

TEST_F(OpAppAcquisitionTest, TestValidateFqdn_EmptyString)
{
    // GIVEN: a test interface instance
    auto testInterface = OpAppAcquisitionTestInterface::create("test.example.com", true);

    // WHEN: validating an empty string
    bool result = testInterface->validateFqdn("");

    // THEN: the validation should fail
    EXPECT_FALSE(result);
}

TEST_F(OpAppAcquisitionTest, TestValidateFqdn_NoDot)
{
    // GIVEN: a test interface instance
    auto testInterface = OpAppAcquisitionTestInterface::create("test.example.com", true);

    // WHEN: validating an FQDN without a dot
    bool result = testInterface->validateFqdn("localhost");

    // THEN: the validation should fail
    EXPECT_FALSE(result);
}

TEST_F(OpAppAcquisitionTest, TestValidateFqdn_SubdomainFqdn)
{
    // GIVEN: a test interface instance
    auto testInterface = OpAppAcquisitionTestInterface::create("test.example.com", true);

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
    auto testInterface = OpAppAcquisitionTestInterface::create("example.com", true);
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
    auto testInterface = OpAppAcquisitionTestInterface::create("example.com", true);
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
    auto testInterface = OpAppAcquisitionTestInterface::create("example.com", true);
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
    auto testInterface = OpAppAcquisitionTestInterface::create("example.com", true);
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
    auto testInterface = OpAppAcquisitionTestInterface::create("example.com", true);
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
    auto testInterface = OpAppAcquisitionTestInterface::create("example.com", true);
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
    auto testInterface = OpAppAcquisitionTestInterface::create("example.com", true);
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
    auto testInterface = OpAppAcquisitionTestInterface::create("example.com", true);
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
// DNS SRV Lookup Integration Tests
// =============================================================================

TEST_F(OpAppAcquisitionTest, TestDoDnsSrvLookup_NetworkUnavailable)
{
    // GIVEN: a test interface with network unavailable
    auto testInterface = OpAppAcquisitionTestInterface::create("example.com", false);

    // WHEN: performing DNS SRV lookup
    std::vector<SrvRecord> records = testInterface->doDnsSrvLookup();

    // THEN: the result should be empty due to network unavailability
    EXPECT_TRUE(records.empty());
}

TEST_F(OpAppAcquisitionTest, TestDoDnsSrvLookup_InvalidFqdn)
{
    // GIVEN: a test interface with invalid FQDN
    auto testInterface = OpAppAcquisitionTestInterface::create("invalid", true);

    // WHEN: performing DNS SRV lookup
    std::vector<SrvRecord> records = testInterface->doDnsSrvLookup();

    // THEN: the result should be empty due to invalid FQDN
    EXPECT_TRUE(records.empty());
}

// =============================================================================
// retrieveOpAppAitXml Tests
// =============================================================================

TEST_F(OpAppAcquisitionTest, TestRetrieveOpAppAitXml_NetworkUnavailable)
{
    // GIVEN: a test interface with network unavailable
    auto testInterface = OpAppAcquisitionTestInterface::create("example.com", false);

    // WHEN: retrieving AIT XML
    std::string result = testInterface->retrieveOpAppAitXml();

    // THEN: the result should be empty due to network unavailability
    EXPECT_TRUE(result.empty());
}

TEST_F(OpAppAcquisitionTest, TestRetrieveOpAppAitXml_InvalidFqdn)
{
    // GIVEN: a test interface with invalid FQDN
    auto testInterface = OpAppAcquisitionTestInterface::create("invalid", true);

    // WHEN: retrieving AIT XML
    std::string result = testInterface->retrieveOpAppAitXml();

    // THEN: the result should be empty due to invalid FQDN
    EXPECT_TRUE(result.empty());
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
    auto testInterface = OpAppAcquisitionTestInterface::create(fqdn, true);

    // WHEN: performing DNS SRV lookup
    std::vector<SrvRecord> records = testInterface->doDnsSrvLookup();

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
