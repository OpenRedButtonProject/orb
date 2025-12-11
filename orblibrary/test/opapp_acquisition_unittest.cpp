#include <string>
#include <vector>
#include <cstdint>

#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/orb/orblibrary/include/OpAppAcquisition.h"
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
// DNS Query Building Tests
// =============================================================================

TEST_F(OpAppAcquisitionTest, TestBuildDnsQuery_ValidName)
{
    // GIVEN: a test interface instance
    auto testInterface = OpAppAcquisitionTestInterface::create("example.com", true);

    // WHEN: building a DNS query for a valid service name
    std::vector<uint8_t> query = testInterface->buildDnsQuery("_hbbtv-ait._tcp.example.com", 0x1234);

    // THEN: the query should not be empty
    EXPECT_FALSE(query.empty());

    // AND: the query should have a valid DNS header (at least 12 bytes)
    EXPECT_GE(query.size(), size_t(12));

    // AND: the transaction ID should be correct (first 2 bytes)
    EXPECT_EQ(query[0], 0x12);
    EXPECT_EQ(query[1], 0x34);

    // AND: flags should be standard query with recursion desired (0x0100)
    EXPECT_EQ(query[2], 0x01);
    EXPECT_EQ(query[3], 0x00);

    // AND: QDCOUNT should be 1
    EXPECT_EQ(query[4], 0x00);
    EXPECT_EQ(query[5], 0x01);
}

TEST_F(OpAppAcquisitionTest, TestBuildDnsQuery_SimpleHostname)
{
    // GIVEN: a test interface instance
    auto testInterface = OpAppAcquisitionTestInterface::create("example.com", true);

    // WHEN: building a DNS query for a simple hostname
    std::vector<uint8_t> query = testInterface->buildDnsQuery("example.com", 0xABCD);

    // THEN: the query should contain the encoded domain name
    EXPECT_FALSE(query.empty());

    // The domain "example.com" should be encoded as:
    // 7 'e' 'x' 'a' 'm' 'p' 'l' 'e' 3 'c' 'o' 'm' 0
    // Starting after the 12-byte header
    EXPECT_GE(query.size(), size_t(12 + 13 + 4)); // header + name + type/class

    // Check the first label length
    EXPECT_EQ(query[12], 7); // "example" is 7 chars
}

// =============================================================================
// DNS Response Parsing Tests
// =============================================================================

TEST_F(OpAppAcquisitionTest, TestParseDnsResponse_TooShort)
{
    // GIVEN: a test interface instance
    auto testInterface = OpAppAcquisitionTestInterface::create("example.com", true);

    // WHEN: parsing a response that's too short
    uint8_t shortResponse[5] = {0x00, 0x01, 0x02, 0x03, 0x04};
    std::vector<SrvRecord> records = testInterface->parseDnsResponse(shortResponse, 5);

    // THEN: no records should be returned
    EXPECT_TRUE(records.empty());
}

TEST_F(OpAppAcquisitionTest, TestParseDnsResponse_ErrorResponse)
{
    // GIVEN: a test interface instance and a DNS response with NXDOMAIN error
    auto testInterface = OpAppAcquisitionTestInterface::create("example.com", true);

    // DNS response with RCODE=3 (NXDOMAIN)
    uint8_t errorResponse[12] = {
        0x12, 0x34,  // Transaction ID
        0x81, 0x83,  // Flags: Response, RCODE=3 (NXDOMAIN)
        0x00, 0x01,  // QDCOUNT: 1
        0x00, 0x00,  // ANCOUNT: 0
        0x00, 0x00,  // NSCOUNT: 0
        0x00, 0x00   // ARCOUNT: 0
    };

    // WHEN: parsing the error response
    std::vector<SrvRecord> records = testInterface->parseDnsResponse(errorResponse, 12);

    // THEN: no records should be returned
    EXPECT_TRUE(records.empty());
}

TEST_F(OpAppAcquisitionTest, TestParseDnsResponse_NoAnswers)
{
    // GIVEN: a test interface instance and a DNS response with no answers
    auto testInterface = OpAppAcquisitionTestInterface::create("example.com", true);

    // DNS response with ANCOUNT=0
    uint8_t noAnswerResponse[12] = {
        0x12, 0x34,  // Transaction ID
        0x81, 0x80,  // Flags: Response, no error
        0x00, 0x00,  // QDCOUNT: 0
        0x00, 0x00,  // ANCOUNT: 0
        0x00, 0x00,  // NSCOUNT: 0
        0x00, 0x00   // ARCOUNT: 0
    };

    // WHEN: parsing the response
    std::vector<SrvRecord> records = testInterface->parseDnsResponse(noAnswerResponse, 12);

    // THEN: no records should be returned
    EXPECT_TRUE(records.empty());
}

TEST_F(OpAppAcquisitionTest, TestParseDnsResponse_ValidSrvRecord)
{
    // GIVEN: a test interface instance and a valid DNS SRV response
    auto testInterface = OpAppAcquisitionTestInterface::create("example.com", true);

    // Construct a minimal valid SRV response
    // This is a simplified response for _srv._tcp.example.com -> target.example.com:8080
    uint8_t validResponse[] = {
        // Header (12 bytes)
        0x12, 0x34,  // Transaction ID
        0x81, 0x80,  // Flags: Response, no error
        0x00, 0x00,  // QDCOUNT: 0 (simplified - no question section)
        0x00, 0x01,  // ANCOUNT: 1
        0x00, 0x00,  // NSCOUNT: 0
        0x00, 0x00,  // ARCOUNT: 0

        // Answer section
        // NAME (using direct encoding for simplicity)
        0x04, '_', 's', 'r', 'v',
        0x04, '_', 't', 'c', 'p',
        0x07, 'e', 'x', 'a', 'm', 'p', 'l', 'e',
        0x03, 'c', 'o', 'm',
        0x00,  // null terminator

        // TYPE: SRV (33 = 0x0021)
        0x00, 0x21,

        // CLASS: IN (1)
        0x00, 0x01,

        // TTL: 300 seconds
        0x00, 0x00, 0x01, 0x2C,

        // RDLENGTH: 22 bytes (6 + 16 for target name)
        0x00, 0x16,

        // RDATA (SRV)
        // Priority: 10
        0x00, 0x0A,
        // Weight: 20
        0x00, 0x14,
        // Port: 8080
        0x1F, 0x90,

        // Target: target.example.com
        0x06, 't', 'a', 'r', 'g', 'e', 't',
        0x07, 'e', 'x', 'a', 'm', 'p', 'l', 'e',
        0x03, 'c', 'o', 'm',
        0x00  // null terminator
    };

    // WHEN: parsing the response
    std::vector<SrvRecord> records = testInterface->parseDnsResponse(validResponse, sizeof(validResponse));

    // THEN: one SRV record should be returned
    ASSERT_EQ(records.size(), size_t(1));

    // AND: the record should have the correct values
    EXPECT_EQ(records[0].priority, 10);
    EXPECT_EQ(records[0].weight, 20);
    EXPECT_EQ(records[0].port, 8080);
    EXPECT_EQ(records[0].target, "target.example.com");
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
// DNS SRV Lookup Integration Tests
// =============================================================================

TEST_F(OpAppAcquisitionTest, TestDoDnsSrvLookup_NetworkUnavailable)
{
    // GIVEN: a test interface with network unavailable
    auto testInterface = OpAppAcquisitionTestInterface::create("example.com", false);

    // WHEN: performing DNS SRV lookup
    std::string result = testInterface->doDnsSrvLookup();

    // THEN: the result should be empty due to network unavailability
    EXPECT_TRUE(result.empty());
}

TEST_F(OpAppAcquisitionTest, TestDoDnsSrvLookup_InvalidFqdn)
{
    // GIVEN: a test interface with invalid FQDN
    auto testInterface = OpAppAcquisitionTestInterface::create("invalid", true);

    // WHEN: performing DNS SRV lookup
    std::string result = testInterface->doDnsSrvLookup();

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

// Disabled - useful for manual testing
TEST_F(OpAppAcquisitionTest, DISABLED_TestDoDnsSrvLookup_ValidFqdn)
{
    // GIVEN: a test interface with a real-world FQDN
    const std::string fqdn = "test.freeviewplay.tv";
    auto testInterface = OpAppAcquisitionTestInterface::create(fqdn, true);

    // WHEN: performing DNS SRV lookup
    std::string result = testInterface->doDnsSrvLookup();

    // THEN: the result should not be empty
    EXPECT_FALSE(result.empty());

    // THEN: all fields should be set correctly
    // Care: this test is dependent on the actual
    // DNS server being used and the results it returns.
    EXPECT_EQ(result, "refplayer-dev.cloud.digitaluk.co.uk:443");
}
