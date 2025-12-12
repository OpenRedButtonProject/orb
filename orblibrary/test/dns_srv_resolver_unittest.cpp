#include <string>
#include <vector>
#include <cstdint>

#include "testing/gtest/include/gtest/gtest.h"
#include "DnsSrvResolver.h"
#include "SrvRecord.h"

namespace orb {

// Test interface - must be in orb namespace to match friend declaration
class DnsSrvResolverTestInterface {
public:
    explicit DnsSrvResolverTestInterface(const std::string& dnsServer = "8.8.8.8",
                                          int timeoutMs = 5000)
        : m_resolver(dnsServer, timeoutMs)
    {
    }

    std::vector<uint8_t> BuildDnsQuery(const std::string& name, uint16_t transactionId) {
        return m_resolver.BuildDnsQuery(name, transactionId);
    }

    std::vector<SrvRecord> ParseDnsResponse(const uint8_t* response, size_t length) {
        return m_resolver.ParseDnsResponse(response, length);
    }

    std::vector<SrvRecord> Query(const std::string& serviceName) {
        return m_resolver.Query(serviceName);
    }

private:
    DnsSrvResolver m_resolver;
};

} // namespace orb

using namespace orb;

class DnsSrvResolverTest : public ::testing::Test {
protected:
    void SetUp() override {
    }

    void TearDown() override {
    }
};

// =============================================================================
// DNS Query Building Tests
// =============================================================================

TEST_F(DnsSrvResolverTest, TestBuildDnsQuery_ValidName)
{
    // GIVEN: a resolver test interface
    DnsSrvResolverTestInterface resolver;

    // WHEN: building a DNS query for a valid service name
    std::vector<uint8_t> query = resolver.BuildDnsQuery("_hbbtv-ait._tcp.example.com", 0x1234);

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

TEST_F(DnsSrvResolverTest, TestBuildDnsQuery_SimpleHostname)
{
    // GIVEN: a resolver test interface
    DnsSrvResolverTestInterface resolver;

    // WHEN: building a DNS query for a simple hostname
    std::vector<uint8_t> query = resolver.BuildDnsQuery("example.com", 0xABCD);

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

TEST_F(DnsSrvResolverTest, TestParseDnsResponse_TooShort)
{
    // GIVEN: a resolver test interface
    DnsSrvResolverTestInterface resolver;

    // WHEN: parsing a response that's too short
    uint8_t shortResponse[5] = {0x00, 0x01, 0x02, 0x03, 0x04};
    std::vector<SrvRecord> records = resolver.ParseDnsResponse(shortResponse, 5);

    // THEN: no records should be returned
    EXPECT_TRUE(records.empty());
}

TEST_F(DnsSrvResolverTest, TestParseDnsResponse_ErrorResponse)
{
    // GIVEN: a resolver test interface and a DNS response with NXDOMAIN error
    DnsSrvResolverTestInterface resolver;

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
    std::vector<SrvRecord> records = resolver.ParseDnsResponse(errorResponse, 12);

    // THEN: no records should be returned
    EXPECT_TRUE(records.empty());
}

TEST_F(DnsSrvResolverTest, TestParseDnsResponse_NoAnswers)
{
    // GIVEN: a resolver test interface and a DNS response with no answers
    DnsSrvResolverTestInterface resolver;

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
    std::vector<SrvRecord> records = resolver.ParseDnsResponse(noAnswerResponse, 12);

    // THEN: no records should be returned
    EXPECT_TRUE(records.empty());
}

TEST_F(DnsSrvResolverTest, TestParseDnsResponse_ValidSrvRecord)
{
    // GIVEN: a resolver test interface and a valid DNS SRV response
    DnsSrvResolverTestInterface resolver;

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
    std::vector<SrvRecord> records = resolver.ParseDnsResponse(validResponse, sizeof(validResponse));

    // THEN: one SRV record should be returned
    ASSERT_EQ(records.size(), size_t(1));

    // AND: the record should have the correct values
    EXPECT_EQ(records[0].priority, 10);
    EXPECT_EQ(records[0].weight, 20);
    EXPECT_EQ(records[0].port, 8080);
    EXPECT_EQ(records[0].target, "target.example.com");
}

// =============================================================================
// Constructor Tests
// =============================================================================

TEST_F(DnsSrvResolverTest, TestConstruction_DefaultParameters)
{
    // GIVEN/WHEN: creating a resolver with default parameters
    DnsSrvResolver resolver;

    // THEN: should be constructed successfully
    SUCCEED();
}

TEST_F(DnsSrvResolverTest, TestConstruction_CustomParameters)
{
    // GIVEN/WHEN: creating a resolver with custom parameters
    DnsSrvResolver resolver("1.1.1.1", 3000);

    // THEN: should be constructed successfully
    SUCCEED();
}

// =============================================================================
// Query Tests - Connection failures
// =============================================================================

TEST_F(DnsSrvResolverTest, TestQuery_InvalidDnsServer)
{
    // GIVEN: a resolver with an invalid DNS server
    DnsSrvResolver resolver("999.999.999.999", 1000);

    // WHEN: querying for SRV records
    std::vector<SrvRecord> records = resolver.Query("_hbbtv-ait._tcp.example.com");

    // THEN: no records should be returned
    EXPECT_TRUE(records.empty());
}

// =============================================================================
// Disabled Tests - Useful for manual/integration testing
// =============================================================================

// Disabled - useful for manual testing with real DNS
TEST_F(DnsSrvResolverTest, DISABLED_TestQuery_RealDns)
{
    // GIVEN: a resolver
    DnsSrvResolver resolver;

    // WHEN: querying for a real service
    std::vector<SrvRecord> records = resolver.Query("_hbbtv-ait._tcp.test.freeviewplay.tv");

    // THEN: records should be returned
    EXPECT_FALSE(records.empty());

    // Note: Results depend on actual DNS configuration
    if (!records.empty()) {
        for (const auto& record : records) {
            EXPECT_FALSE(record.target.empty());
            EXPECT_GT(record.port, 0);
        }
    }
}
