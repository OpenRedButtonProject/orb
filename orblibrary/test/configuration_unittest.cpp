#include <iostream>
#include <string>
#include <json/json.h>

#include "testing/gtest/include/gtest/gtest.h"
#include "Configuration.h"
#include "ConfigurationUtil.h"
#include "third_party/orb/orblibrary/moderator/PlatformAndroid.h"

class ConfigurationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a Configuration instance for testing
        m_platform = std::make_shared<orb::AndroidPlatform>(orb::ApplicationType::APP_TYPE_VIDEO);
        m_configuration = std::make_unique<orb::Configuration>(m_platform);
    }

    void TearDown() override {
        m_configuration.reset();
    }

    std::unique_ptr<orb::Configuration> m_configuration;
    std::shared_ptr<orb::IPlatform> m_platform;
};

TEST_F(ConfigurationTest, TestExecuteRequest_GetCapabilities) {
    // GIVEN: a Configuration object
    // WHEN: executeRequest is called with "getCapabilities" method
    Json::Value token;
    Json::Value params;
    std::string response = m_configuration->executeRequest("getCapabilities", token, params);

    // THEN: a valid JSON response is returned
    EXPECT_FALSE(response.empty());

    // Parse the response to verify it's valid JSON
    Json::Value jsonResponse;
    Json::CharReaderBuilder reader;
    std::string errors;
    std::istringstream responseStream(response);
    bool parseSuccess = Json::parseFromStream(reader, responseStream, &jsonResponse, &errors);
    EXPECT_TRUE(parseSuccess) << "Failed to parse response JSON: " << errors;
}

TEST_F(ConfigurationTest, TestExecuteRequest_GetAudioProfiles) {
    // GIVEN: a Configuration object
    // WHEN: executeRequest is called with "getAudioProfiles" method
    Json::Value token;
    Json::Value params;
    std::string response = m_configuration->executeRequest("getAudioProfiles", token, params);

    // THEN: a valid JSON response is returned
    EXPECT_FALSE(response.empty());

    // Parse the response to verify it's valid JSON
    Json::Value jsonResponse;
    Json::CharReaderBuilder reader;
    std::string errors;
    std::istringstream responseStream(response);
    bool parseSuccess = Json::parseFromStream(reader, responseStream, &jsonResponse, &errors);
    EXPECT_TRUE(parseSuccess) << "Failed to parse response JSON: " << errors;
}

TEST_F(ConfigurationTest, TestExecuteRequest_GetVideoProfiles) {
    // GIVEN: a Configuration object
    // WHEN: executeRequest is called with "getVideoProfiles" method
    Json::Value token;
    Json::Value params;
    std::string response = m_configuration->executeRequest("getVideoProfiles", token, params);

    // THEN: a valid JSON response is returned
    EXPECT_FALSE(response.empty());

    // Parse the response to verify it's valid JSON
    Json::Value jsonResponse;
    Json::CharReaderBuilder reader;
    std::string errors;
    std::istringstream responseStream(response);
    bool parseSuccess = Json::parseFromStream(reader, responseStream, &jsonResponse, &errors);
    EXPECT_TRUE(parseSuccess) << "Failed to parse response JSON: " << errors;
}

TEST_F(ConfigurationTest, TestExecuteRequest_UnknownMethod) {
    // GIVEN: a Configuration object
    // WHEN: executeRequest is called with an unknown method
    Json::Value token;
    Json::Value params;
    std::string response = m_configuration->executeRequest("unknownMethod", token, params);

    // THEN: a valid JSON response is returned
    EXPECT_FALSE(response.empty());

    // Parse the response to verify it's valid JSON
    Json::Value jsonResponse;
    Json::CharReaderBuilder reader;
    std::string errors;
    std::istringstream responseStream(response);
    bool parseSuccess = Json::parseFromStream(reader, responseStream, &jsonResponse, &errors);
    EXPECT_TRUE(parseSuccess) << "Failed to parse response JSON: " << errors;
}

TEST_F(ConfigurationTest, TestExecuteRequest_EmptyMethod) {
    // GIVEN: a Configuration object
    // WHEN: executeRequest is called with an empty method
    Json::Value token;
    Json::Value params;
    std::string response = m_configuration->executeRequest("", token, params);

    // THEN: a valid JSON response is returned
    EXPECT_FALSE(response.empty());

    // Parse the response to verify it's valid JSON
    Json::Value jsonResponse;
    Json::CharReaderBuilder reader;
    std::string errors;
    std::istringstream responseStream(response);
    bool parseSuccess = Json::parseFromStream(reader, responseStream, &jsonResponse, &errors);
    EXPECT_TRUE(parseSuccess) << "Failed to parse response JSON: " << errors;
}

TEST_F(ConfigurationTest, TestExecuteRequest_WithTokenAndParams) {
    // GIVEN: a Configuration object with token and params
    Json::Value token;
    token["appId"] = "testApp";
    token["sessionId"] = "testSession";

    Json::Value params;
    params["param1"] = "value1";
    params["param2"] = 42;

    // WHEN: executeRequest is called with "getCapabilities" method
    std::string response = m_configuration->executeRequest("getCapabilities", token, params);

    // THEN: a valid JSON response is returned
    EXPECT_FALSE(response.empty());

    // Parse the response to verify it's valid JSON
    Json::Value jsonResponse;
    Json::CharReaderBuilder reader;
    std::string errors;
    std::istringstream responseStream(response);
    bool parseSuccess = Json::parseFromStream(reader, responseStream, &jsonResponse, &errors);
    EXPECT_TRUE(parseSuccess) << "Failed to parse response JSON: " << errors;
}

TEST_F(ConfigurationTest, TestExecuteRequest_WithComplexParams) {
    // GIVEN: a Configuration object with complex params
    Json::Value token;
    token["appId"] = "testApp";

    Json::Value params;
    params["stringParam"] = "testString";
    params["intParam"] = 123;
    params["boolParam"] = true;
    params["arrayParam"] = Json::Value(Json::arrayValue);
    params["arrayParam"].append("item1");
    params["arrayParam"].append("item2");
    params["objectParam"] = Json::Value(Json::objectValue);
    params["objectParam"]["nestedKey"] = "nestedValue";

    // WHEN: executeRequest is called with "getCapabilities" method
    std::string response = m_configuration->executeRequest("getCapabilities", token, params);

    // THEN: a valid JSON response is returned
    EXPECT_FALSE(response.empty());

    // Parse the response to verify it's valid JSON
    Json::Value jsonResponse;
    Json::CharReaderBuilder reader;
    std::string errors;
    std::istringstream responseStream(response);
    bool parseSuccess = Json::parseFromStream(reader, responseStream, &jsonResponse, &errors);
    EXPECT_TRUE(parseSuccess) << "Failed to parse response JSON: " << errors;
}

TEST_F(ConfigurationTest, TestExecuteRequest_AllMethods) {
    // GIVEN: a Configuration object
    std::vector<std::string> methods = {
        "getCapabilities",
        "getAudioProfiles",
        "getVideoProfiles",
        "getVideoDisplayFormats",
        "getCleanAudioEnabled"
    };

    Json::Value token;
    Json::Value params;

    // WHEN: executeRequest is called with each method
    for (const auto& method : methods) {
        std::string response = m_configuration->executeRequest(method, token, params);

        // THEN: a valid JSON response is returned for each method
        EXPECT_FALSE(response.empty()) << "Empty response for method: " << method;

        // Parse the response to verify it's valid JSON
        Json::Value jsonResponse;
        Json::CharReaderBuilder reader;
        std::string errors;
        std::istringstream responseStream(response);
        bool parseSuccess = Json::parseFromStream(reader, responseStream, &jsonResponse, &errors);
        EXPECT_TRUE(parseSuccess) << "Failed to parse response JSON for method " << method << ": " << errors;
    }
}