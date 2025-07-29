#include <iostream>
#include <string>

#include "testing/gtest/include/gtest/gtest.h"
#include "StringUtil.h"

class StringUtilTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Test setup if needed
    }

    void TearDown() override {
        // Test cleanup if needed
    }
};

TEST_F(StringUtilTest, TestResolveMethod_ValidFormat) {
    // GIVEN: a string with valid component.method format
    std::string input = "Manager.getCapabilities";
    std::string component;
    std::string method;

    // WHEN: ResolveMethod is called
    bool result = orb::StringUtil::ResolveMethod(input, component, method);

    // THEN: true is returned and component and method are correctly parsed
    EXPECT_TRUE(result);
    EXPECT_EQ(component, "Manager");
    EXPECT_EQ(method, "getCapabilities");
}


TEST_F(StringUtilTest, TestResolveMethod_RealWorldExamples) {
    // GIVEN: various real-world examples
    std::vector<std::pair<std::string, std::pair<std::string, std::string>>> testCases = {
        {"Manager.getCapabilities", {"Manager", "getCapabilities"}},
        {"Network.getData", {"Network", "getData"}},
        {"MediaSynchroniser.getStatus", {"MediaSynchroniser", "getStatus"}},
        {"Configuration.getProfiles", {"Configuration", "getProfiles"}},
        {"Drm.getLicense", {"Drm", "getLicense"}},
        {"AppMgrInterface.executeRequest", {"AppMgrInterface", "executeRequest"}}
    };

    for (const auto& testCase : testCases) {
        std::string component;
        std::string method;

        // WHEN: ResolveMethod is called
        bool result = orb::StringUtil::ResolveMethod(testCase.first, component, method);

        // THEN: true is returned and component and method are correctly parsed
        EXPECT_TRUE(result) << "Failed for input: " << testCase.first;
        EXPECT_EQ(component, testCase.second.first) << "Failed for input: " << testCase.first;
        EXPECT_EQ(method, testCase.second.second) << "Failed for input: " << testCase.first;
    }
}

TEST_F(StringUtilTest, TestResolveMethod_EdgeCases) {
    // GIVEN: edge case strings
    std::vector<std::string> invalidInputs = {
        "",           // empty string
        ".",          // only dot
        "a",          // single character
        "a.",         // single character with dot
        ".a",         // dot with single character
        "a.b.c",      // multiple dots
        "..",         // multiple dots only
    };

    for (const auto& input : invalidInputs) {
        std::string component;
        std::string method;

        // WHEN: ResolveMethod is called
        bool result = orb::StringUtil::ResolveMethod(input, component, method);

        // THEN: false is returned
        EXPECT_FALSE(result) << "Failed for input: '" << input << "'";
    }
}