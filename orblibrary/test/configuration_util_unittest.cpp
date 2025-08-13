#include <iostream>
#include <string>
#include <vector>
#include <json/json.h>

#include "testing/gtest/include/gtest/gtest.h"
#include "ConfigurationUtil.h"
#include "third_party/orb/orblibrary/moderator/PlatformAndroid.h"

class ConfigurationUtilTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Test setup if needed
        mPlatform = std::make_shared<orb::AndroidPlatform>(orb::ApplicationType::APP_TYPE_HBBTV);
    }

    void TearDown() override {
        // Test cleanup if needed
    }
    std::shared_ptr<orb::IPlatform> mPlatform;
};

TEST_F(ConfigurationUtilTest, TestCreateAudioProfile) {
    // GIVEN: audio profile parameters
    std::string name = "test_profile";
    std::string type = "audio/mpeg";
    std::string transport = "dash";
    std::string syncTl = "dash_pr";
    std::string drmSystemId = "test_drm_id";

    // WHEN: createAudioProfile is called with the parameters
    auto audioProfile = orb::ConfigurationUtil::createAudioProfile(name, type, transport, syncTl, drmSystemId);

    // THEN: an AudioProfile object is returned with correct values
    EXPECT_EQ(audioProfile.m_name, name);
    EXPECT_EQ(audioProfile.m_type, type);
    EXPECT_EQ(audioProfile.m_transport, transport);
    EXPECT_EQ(audioProfile.m_syncTl, syncTl);
    EXPECT_EQ(audioProfile.m_drmSystemId, drmSystemId);
}

TEST_F(ConfigurationUtilTest, TestCreateVideoProfile) {
    // GIVEN: video profile parameters
    std::string name = "test_video_profile";
    std::string type = "video/mp4";
    std::string transport = "dash";
    std::string syncTl = "dash_pr";
    std::string drmSystemId = "test_drm_id";
    std::string hdr = "test_hdr";

    // WHEN: createVideoProfile is called with the parameters
    auto videoProfile = orb::ConfigurationUtil::createVideoProfile(name, type, transport, syncTl, drmSystemId, hdr);

    // THEN: a VideoProfile object is returned with correct values
    EXPECT_EQ(videoProfile.m_name, name);
    EXPECT_EQ(videoProfile.m_type, type);
    EXPECT_EQ(videoProfile.m_transport, transport);
    EXPECT_EQ(videoProfile.m_syncTl, syncTl);
    EXPECT_EQ(videoProfile.m_drmSystemId, drmSystemId);
    EXPECT_EQ(videoProfile.m_hdr, hdr);
}

TEST_F(ConfigurationUtilTest, TestCapabilitiesToJson) {
    // GIVEN: a Capabilities object
    auto capabilities = mPlatform->Configuration_GetCapabilities();

    // WHEN: capabilitiesToJson is called
    Json::Value jsonResult = orb::ConfigurationUtil::capabilitiesToJson(*capabilities);

    // THEN: a valid JSON object is returned
    EXPECT_TRUE(jsonResult.isObject());

    // Verify required fields are present
    EXPECT_TRUE(jsonResult.isMember("optionStrings"));
    EXPECT_TRUE(jsonResult.isMember("profileNameFragments"));
    EXPECT_TRUE(jsonResult.isMember("parentalSchemes"));
    EXPECT_TRUE(jsonResult.isMember("displaySizeWidth"));
    EXPECT_TRUE(jsonResult.isMember("displaySizeHeight"));
    EXPECT_TRUE(jsonResult.isMember("displaySizeMeasurementType"));
    EXPECT_TRUE(jsonResult.isMember("passThroughStatus"));
    EXPECT_TRUE(jsonResult.isMember("jsonRpcServerUrl"));
    EXPECT_TRUE(jsonResult.isMember("jsonRpcServerVersion"));

    // Verify arrays are not empty
    EXPECT_TRUE(jsonResult["optionStrings"].isArray());
    EXPECT_TRUE(jsonResult["profileNameFragments"].isArray());
    EXPECT_TRUE(jsonResult["parentalSchemes"].isArray());
    EXPECT_FALSE(jsonResult["optionStrings"].empty());
    EXPECT_FALSE(jsonResult["profileNameFragments"].empty());
    EXPECT_FALSE(jsonResult["parentalSchemes"].empty());
    EXPECT_FALSE(jsonResult["jsonRpcServerUrl"].empty());
    EXPECT_FALSE(jsonResult["jsonRpcServerVersion"].empty());
}

TEST_F(ConfigurationUtilTest, TestAudioProfilesToJson) {
    // GIVEN: a vector of AudioProfile objects
    auto audioProfiles = mPlatform->Configuration_GetAudioProfiles();

    // WHEN: audioProfilesToJson is called
    Json::Value jsonResult = orb::ConfigurationUtil::audioProfilesToJson(audioProfiles);

    // THEN: a valid JSON array is returned
    EXPECT_TRUE(jsonResult.isArray());
    EXPECT_FALSE(jsonResult.empty());

    // Verify each element in the array is an object with required fields
    for (const auto& profileJson : jsonResult) {
        EXPECT_TRUE(profileJson.isObject());
        EXPECT_TRUE(profileJson.isMember("name"));
        EXPECT_TRUE(profileJson.isMember("type"));
        EXPECT_FALSE(profileJson["name"].asString().empty());
        EXPECT_FALSE(profileJson["type"].asString().empty());
    }
}

TEST_F(ConfigurationUtilTest, TestVideoProfilesToJson) {
    // GIVEN: a vector of VideoProfile objects
    auto videoProfiles = mPlatform->Configuration_GetVideoProfiles();

    // WHEN: videoProfilesToJson is called
    Json::Value jsonResult = orb::ConfigurationUtil::videoProfilesToJson(videoProfiles);

    // THEN: a valid JSON array is returned
    EXPECT_TRUE(jsonResult.isArray());
    EXPECT_FALSE(jsonResult.empty());

    // Verify each element in the array is an object with required fields
    for (const auto& profileJson : jsonResult) {
        EXPECT_TRUE(profileJson.isObject());
        EXPECT_TRUE(profileJson.isMember("name"));
        EXPECT_TRUE(profileJson.isMember("type"));
        EXPECT_FALSE(profileJson["name"].asString().empty());
        EXPECT_FALSE(profileJson["type"].asString().empty());
    }
}

TEST_F(ConfigurationUtilTest, TestGetJsonRpcServerUrl) {
    // GIVEN: a port number
    int port = 8910;

    // WHEN: getJsonRpcServerUrl is called
    std::string url = orb::ConfigurationUtil::getJsonRpcServerUrl(port);

    // THEN: a valid URL string is returned
    EXPECT_FALSE(url.empty());
    EXPECT_TRUE(url.find("ws://localhost:") == 0);
    EXPECT_TRUE(url.find(std::to_string(port)) != std::string::npos);
    EXPECT_TRUE(url.find("/hbbtv/") != std::string::npos);
}

TEST_F(ConfigurationUtilTest, TestGetJsonRpcServerEndpoint) {
    // GIVEN: ConfigurationUtil class
    // WHEN: getJsonRpcServerEndpoint is called
    std::string endpoint = orb::ConfigurationUtil::getJsonRpcServerEndpoint();

    // THEN: a valid endpoint string is returned
    EXPECT_FALSE(endpoint.empty());
    EXPECT_TRUE(endpoint.find("/hbbtv/") == 0);
}

TEST_F(ConfigurationUtilTest, TestGetJsonRpcServerPort) {
    // GIVEN: ConfigurationUtil class
    // WHEN: getJsonRpcServerPort is called
    int port = orb::ConfigurationUtil::getJsonRpcServerPort(orb::ApplicationType::APP_TYPE_HBBTV);

    // THEN: a valid port number is returned
    EXPECT_EQ(port, 8911);
}

TEST_F(ConfigurationUtilTest, TestCapabilitiesToJson_EmptyCapabilities) {
    // GIVEN: an empty Capabilities object
    auto capabilities = std::make_shared<orb::Capabilities>();

    // WHEN: capabilitiesToJson is called
    Json::Value jsonResult = orb::ConfigurationUtil::capabilitiesToJson(*capabilities);

    // THEN: a valid JSON object is returned
    EXPECT_TRUE(jsonResult.isObject());

    // Verify required fields are present (even if empty)
    EXPECT_TRUE(jsonResult.isMember("optionStrings"));
    EXPECT_TRUE(jsonResult.isMember("profileNameFragments"));
    EXPECT_TRUE(jsonResult.isMember("parentalSchemes"));
    EXPECT_TRUE(jsonResult.isMember("displaySizeWidth"));
    EXPECT_TRUE(jsonResult.isMember("displaySizeHeight"));
    EXPECT_TRUE(jsonResult.isMember("displaySizeMeasurementType"));
    EXPECT_TRUE(jsonResult.isMember("passThroughStatus"));

    // Verify arrays are empty
    EXPECT_TRUE(jsonResult["optionStrings"].isArray());
    EXPECT_TRUE(jsonResult["profileNameFragments"].isArray());
    EXPECT_TRUE(jsonResult["parentalSchemes"].isArray());
    EXPECT_TRUE(jsonResult["optionStrings"].empty());
    EXPECT_TRUE(jsonResult["profileNameFragments"].empty());
    EXPECT_TRUE(jsonResult["parentalSchemes"].empty());
}

TEST_F(ConfigurationUtilTest, TestAudioProfilesToJson_EmptyVector) {
    // GIVEN: an empty vector of AudioProfile objects
    std::vector<orb::AudioProfile> audioProfiles;

    // WHEN: audioProfilesToJson is called
    Json::Value jsonResult = orb::ConfigurationUtil::audioProfilesToJson(audioProfiles);

    // THEN: an empty JSON array is returned
    EXPECT_TRUE(jsonResult.isArray());
    EXPECT_TRUE(jsonResult.empty());
}

TEST_F(ConfigurationUtilTest, TestVideoProfilesToJson_EmptyVector) {
    // GIVEN: an empty vector of VideoProfile objects
    std::vector<orb::VideoProfile> videoProfiles;

    // WHEN: videoProfilesToJson is called
    Json::Value jsonResult = orb::ConfigurationUtil::videoProfilesToJson(videoProfiles);

    // THEN: an empty JSON array is returned
    EXPECT_TRUE(jsonResult.isArray());
    EXPECT_TRUE(jsonResult.empty());
}

TEST_F(ConfigurationUtilTest, TestJsonRpcServerUrlPorts) {
    // GIVEN: different port numbers
    // WHEN: getJsonRpcServerUrl is called with different ports
    std::string url1 = orb::ConfigurationUtil::getJsonRpcServerUrl(8910);
    std::string url2 = orb::ConfigurationUtil::getJsonRpcServerUrl(8911);

    // THEN: different URLs are returned
    EXPECT_NE(url1, url2);
    EXPECT_TRUE(url1.find(":8910") != std::string::npos);
    EXPECT_TRUE(url2.find(":8911") != std::string::npos);
}