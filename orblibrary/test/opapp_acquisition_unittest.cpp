#include <string>

#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/orb/orblibrary/include/OpAppAcquisition.h"
#include "OpAppAcquisitionTestInterface.h"

class OpAppAcquisitionTest : public ::testing::Test {
protected:
    void SetUp() override {
    }

    void TearDown() override {
    }
};

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

TEST_F(OpAppAcquisitionTest, TestDoDnsSrvLookup_ValidFqdnAndNetwork)
{
    // GIVEN: a test interface with valid FQDN and network available
    const std::string fqdn = "example.com";
    auto testInterface = OpAppAcquisitionTestInterface::create(fqdn, true);

    // WHEN: performing DNS SRV lookup
    std::string result = testInterface->doDnsSrvLookup();

    // THEN: the result should be the FQDN (current implementation returns FQDN)
    EXPECT_EQ(result, fqdn);
}

