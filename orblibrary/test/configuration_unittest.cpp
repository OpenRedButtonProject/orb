#include <iostream>
#include <string>
#include <json/json.h>

#include "testing/gtest/include/gtest/gtest.h"
#include "Configuration.hpp"
#include "ConfigurationUtil.h"
#include "JsonUtil.h"
#include "log.hpp"

using namespace orb;

//  mock IOrbBrowser
class MockOrbBrowser : public orb::IOrbBrowser {
   public:
   void loadApplication(std::string app_id, std::string url) override {
    // TODO: implement
   }
   void showApplication() override {
    // TODO: implement
   }
   void hideApplication() override {
    // TODO: implement
   }
   std::string sendRequestToClient(std::string jsonRequest) override {
    // get the method from the jsonRequest
    Json::Value jsonRequestVal;
    if (!JsonUtil::decodeJson(jsonRequest, &jsonRequestVal)) {
        return "{\"result\":{\"error\":\"Invalid JSON request\"}}";
    }
    std::string method = jsonRequestVal["method"].asString();
    if (method == "Configuration.getCapabilities") {
        return "{\"result\":{\"jsonRpcServerEndpoint\":\"/hbbtv/jsonrpc/\",\"jsonRpcServerPort\":8080}}";
    }
    else if (method == "Configuration.getAudioProfiles") {
        return "{\"result\":{\"AudioProfiles\":[{\"name\":\"AudioProfile1\",\"id\":1},{\"name\":\"AudioProfile2\",\"id\":2}]}}";
    }
    else if (method == "Configuration.getVideoProfiles") {
        return "{\"result\":{\"VideoProfiles\":[{\"name\":\"VideoProfile1\",\"id\":1},{\"name\":\"VideoProfile2\",\"id\":2}]}}";
    }
    return "{\"result\":{\"error\":\"Not implemented\"}}";
   }
   void dispatchEvent(const std::string& etype, const std::string& properties) override {
    // TODO: implement
   }
   void notifyKeySetChange(uint16_t keyset, std::vector<uint16_t> otherkeys) override {
   }
};

class ConfigurationTest : public ::testing::Test {
protected:
    void SetUp() override {
        m_mockOrbBrowser = std::make_unique<MockOrbBrowser>();
        // Create a Configuration instance for testing
        m_configuration = std::make_unique<orb::Configuration>(orb::ApplicationType::APP_TYPE_HBBTV, m_mockOrbBrowser.get());
    }

    void TearDown() override {
        m_configuration.reset();
    }

    std::unique_ptr<orb::IOrbBrowser> m_mockOrbBrowser;
    std::unique_ptr<orb::Configuration> m_configuration;
};

TEST_F(ConfigurationTest, TestGetCapabilities) {
    std::string capabilities = m_configuration->executeRequest("getCapabilities", Json::Value(), Json::Value());
    EXPECT_EQ(capabilities, "{\"result\":{\"jsonRpcServerEndpoint\":\"/hbbtv/jsonrpc/\",\"jsonRpcServerPort\":8080}}");
}

TEST_F(ConfigurationTest, TestGetAudioProfiles) {
    std::string audioProfiles = m_configuration->executeRequest("getAudioProfiles", Json::Value(), Json::Value());
    EXPECT_EQ(audioProfiles, "{\"result\":{\"AudioProfiles\":[{\"name\":\"AudioProfile1\",\"id\":1},{\"name\":\"AudioProfile2\",\"id\":2}]}}");
}

TEST_F(ConfigurationTest, TestGetVideoProfiles) {
    std::string videoProfiles = m_configuration->executeRequest("getVideoProfiles", Json::Value(), Json::Value());
    EXPECT_EQ(videoProfiles, "{\"result\":{\"VideoProfiles\":[{\"name\":\"VideoProfile1\",\"id\":1},{\"name\":\"VideoProfile2\",\"id\":2}]}}");
}


