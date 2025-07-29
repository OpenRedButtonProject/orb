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

TEST_F(ConfigurationUtilTest, TestCreateDefaultCapabilities_HbbTV) {
    // GIVEN: ApplicationType::HbbTV
    // WHEN: createDefaultCapabilities is called with HbbTV application type
    auto capabilities = orb::ConfigurationUtil::createDefaultCapabilities(orb::ApplicationType::APP_TYPE_HBBTV);

    // THEN: a valid Capabilities object is returned
    EXPECT_NE(capabilities, nullptr);
    EXPECT_FALSE(capabilities->m_optionStrings.empty());
    EXPECT_FALSE(capabilities->m_profileNameFragments.empty());
    EXPECT_FALSE(capabilities->m_parentalSchemes.empty());
    EXPECT_FALSE(capabilities->m_displaySizeWidth.empty());
    EXPECT_FALSE(capabilities->m_displaySizeHeight.empty());
    EXPECT_FALSE(capabilities->m_displaySizeMeasurementType.empty());

    // Verify JSON RPC server URL is set correctly for HbbTV
    EXPECT_FALSE(capabilities->m_jsonRpcServerUrl.empty());
    EXPECT_FALSE(capabilities->m_jsonRpcServerVersion.empty());
}

TEST_F(ConfigurationUtilTest, TestCreateDefaultCapabilities_OpApp) {
    // GIVEN: ApplicationType::OpApp
    // WHEN: createDefaultCapabilities is called with OpApp application type
    auto capabilities = orb::ConfigurationUtil::createDefaultCapabilities(orb::ApplicationType::APP_TYPE_OPAPP);

    // THEN: a valid Capabilities object is returned
    EXPECT_NE(capabilities, nullptr);
    EXPECT_FALSE(capabilities->m_optionStrings.empty());
    EXPECT_FALSE(capabilities->m_profileNameFragments.empty());
    EXPECT_FALSE(capabilities->m_parentalSchemes.empty());
    EXPECT_FALSE(capabilities->m_displaySizeWidth.empty());
    EXPECT_FALSE(capabilities->m_displaySizeHeight.empty());
    EXPECT_FALSE(capabilities->m_displaySizeMeasurementType.empty());

    // Verify JSON RPC server URL is set correctly for OpApp (different port)
    EXPECT_FALSE(capabilities->m_jsonRpcServerUrl.empty());
    EXPECT_FALSE(capabilities->m_jsonRpcServerVersion.empty());
}

TEST_F(ConfigurationUtilTest, TestCreateDefaultAudioProfiles) {
    // GIVEN: ConfigurationUtil class
    // WHEN: createDefaultAudioProfiles is called
    auto audioProfiles = orb::ConfigurationUtil::createDefaultAudioProfiles();

    // THEN: a non-empty vector of AudioProfile objects is returned
    EXPECT_FALSE(audioProfiles.empty());

    // Verify each audio profile has required fields
    for (const auto& profile : audioProfiles) {
        EXPECT_FALSE(profile.m_name.empty());
        EXPECT_FALSE(profile.m_type.empty());
    }
}

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

TEST_F(ConfigurationUtilTest, TestCreateDefaultVideoProfiles) {
    // GIVEN: ConfigurationUtil class
    // WHEN: createDefaultVideoProfiles is called
    auto videoProfiles = orb::ConfigurationUtil::createDefaultVideoProfiles();

    // THEN: a non-empty vector of VideoProfile objects is returned
    EXPECT_FALSE(videoProfiles.empty());

    // Verify each video profile has required fields
    for (const auto& profile : videoProfiles) {
        EXPECT_FALSE(profile.m_name.empty());
        EXPECT_FALSE(profile.m_type.empty());
    }
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

TEST_F(ConfigurationUtilTest, TestCreateDefaultVideoDisplayFormat) {
    // GIVEN: ConfigurationUtil class
    // WHEN: createDefaultVideoDisplayFormat is called
    auto videoDisplayFormat = orb::ConfigurationUtil::createDefaultVideoDisplayFormat();

    // THEN: a VideoDisplayFormat object is returned with default values
    EXPECT_EQ(videoDisplayFormat.m_width, 0);
    EXPECT_EQ(videoDisplayFormat.m_height, 0);
    EXPECT_EQ(videoDisplayFormat.m_frameRate, 0);
    EXPECT_EQ(videoDisplayFormat.m_bitDepth, 0);
    EXPECT_TRUE(videoDisplayFormat.m_colorimetry.empty());
}

TEST_F(ConfigurationUtilTest, TestCapabilitiesToJson) {
    // GIVEN: a Capabilities object
    auto capabilities = orb::ConfigurationUtil::createDefaultCapabilities(orb::ApplicationType::APP_TYPE_HBBTV);

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
    auto audioProfiles = orb::ConfigurationUtil::createDefaultAudioProfiles();

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
    auto videoProfiles = orb::ConfigurationUtil::createDefaultVideoProfiles();

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

TEST_F(ConfigurationUtilTest, TestConvertJsonToString) {
    // GIVEN: a JSON object
    Json::Value jsonObject;
    jsonObject["stringField"] = "test_value";
    jsonObject["intField"] = 42;
    jsonObject["boolField"] = true;
    jsonObject["arrayField"] = Json::Value(Json::arrayValue);
    jsonObject["arrayField"].append("item1");
    jsonObject["arrayField"].append("item2");

    // WHEN: convertJsonToString is called
    std::string result = orb::ConfigurationUtil::convertJsonToString(jsonObject);

    // THEN: a valid JSON string is returned
    EXPECT_FALSE(result.empty());

    // Verify the string can be parsed back to JSON
    Json::Value parsedJson;
    Json::CharReaderBuilder reader;
    std::string errors;
    std::istringstream resultStream(result);
    bool parseSuccess = Json::parseFromStream(reader, resultStream, &parsedJson, &errors);
    EXPECT_TRUE(parseSuccess) << "Failed to parse JSON string: " << errors;

    // Verify the parsed JSON matches the original
    EXPECT_EQ(parsedJson["stringField"].asString(), "test_value");
    EXPECT_EQ(parsedJson["intField"].asInt(), 42);
    EXPECT_EQ(parsedJson["boolField"].asBool(), true);
    EXPECT_TRUE(parsedJson["arrayField"].isArray());
    EXPECT_EQ(parsedJson["arrayField"].size(), (unsigned int)2);
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

TEST_F(ConfigurationUtilTest, TestConvertJsonToString_EmptyObject) {
    // GIVEN: an empty JSON object
    Json::Value jsonObject(Json::objectValue);

    // WHEN: convertJsonToString is called
    std::string result = orb::ConfigurationUtil::convertJsonToString(jsonObject);

    // THEN: a valid JSON string is returned
    EXPECT_FALSE(result.empty());
    EXPECT_EQ(result, "{}");
}

TEST_F(ConfigurationUtilTest, TestConvertJsonToString_ComplexObject) {
    // GIVEN: a complex JSON object
    Json::Value jsonObject;
    jsonObject["nested"] = Json::Value(Json::objectValue);
    jsonObject["nested"]["key1"] = "value1";
    jsonObject["nested"]["key2"] = 123;
    jsonObject["array"] = Json::Value(Json::arrayValue);
    jsonObject["array"].append("item1");
    jsonObject["array"].append("item2");
    jsonObject["array"].append(Json::Value(Json::objectValue));
    jsonObject["array"][2]["nestedKey"] = "nestedValue";

    // WHEN: convertJsonToString is called
    std::string result = orb::ConfigurationUtil::convertJsonToString(jsonObject);

    // THEN: a valid JSON string is returned
    EXPECT_FALSE(result.empty());

    // Verify the string can be parsed back to JSON
    Json::Value parsedJson;
    Json::CharReaderBuilder reader;
    std::string errors;
    std::istringstream resultStream(result);
    bool parseSuccess = Json::parseFromStream(reader, resultStream, &parsedJson, &errors);
    EXPECT_TRUE(parseSuccess) << "Failed to parse JSON string: " << errors;

    // Verify the structure is preserved
    EXPECT_TRUE(parsedJson.isMember("nested"));
    EXPECT_TRUE(parsedJson["nested"].isObject());
    EXPECT_EQ(parsedJson["nested"]["key1"].asString(), "value1");
    EXPECT_EQ(parsedJson["nested"]["key2"].asInt(), 123);
    EXPECT_TRUE(parsedJson.isMember("array"));
    EXPECT_TRUE(parsedJson["array"].isArray());
    EXPECT_EQ(parsedJson["array"].size(), (unsigned int)3);
}

TEST_F(ConfigurationUtilTest, TestMultipleCapabilitiesCreation) {
    // GIVEN: multiple application types
    // WHEN: createDefaultCapabilities is called for each type
    auto capabilities1 = orb::ConfigurationUtil::createDefaultCapabilities(orb::ApplicationType::APP_TYPE_HBBTV);
    auto capabilities2 = orb::ConfigurationUtil::createDefaultCapabilities(orb::ApplicationType::APP_TYPE_OPAPP);
    auto capabilities3 = orb::ConfigurationUtil::createDefaultCapabilities(orb::ApplicationType::APP_TYPE_HBBTV);

    // THEN: all capabilities objects are created successfully
    EXPECT_NE(capabilities1, nullptr);
    EXPECT_NE(capabilities2, nullptr);
    EXPECT_NE(capabilities3, nullptr);

    // Verify they have different JSON RPC server URLs (different ports)
    EXPECT_NE(capabilities1->m_jsonRpcServerUrl, capabilities2->m_jsonRpcServerUrl);
    EXPECT_EQ(capabilities1->m_jsonRpcServerUrl, capabilities3->m_jsonRpcServerUrl);
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