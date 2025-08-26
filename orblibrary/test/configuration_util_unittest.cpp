#include <iostream>
#include <string>
#include <vector>
#include <json/json.h>

#include "testing/gtest/include/gtest/gtest.h"
#include "ConfigurationUtil.h"

class ConfigurationUtilTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Test setup if needed
    }

    void TearDown() override {
        // Test cleanup if needed
    }
};

TEST_F(ConfigurationUtilTest, TestGenerateRequestWithApplicationType) {
    std::string request = ConfigurationUtil::generateRequest("Configuration.getCapabilities", APP_TYPE_HBBTV);
    EXPECT_EQ(request, "{\"method\":\"Configuration.getCapabilities\",\"params\":{\"applicationType\":\"0\"}}");
}