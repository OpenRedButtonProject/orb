// create unit for all static methods in JsonRpcServiceUtil.cpp
#include "testing/gtest/include/gtest/gtest.h"
#include "JsonRpcServiceUtil.h"

using namespace orb::networkServices;

TEST(JsonRpcServiceUtil, TestEncodeJsonId) {
    // GIVEN: a JSON value representing an id
    Json::Value idValue("12345");

    // WHEN: encoding the JSON id
    std::string encodedId = JsonRpcServiceUtil::EncodeJsonId(idValue);

    // THEN: it should return the string representation of the id
    EXPECT_EQ(encodedId, "\"12345\"");
}

TEST(JsonRpcServiceUtil, TestDecodeJsonId) {
    // GIVEN: a string representation of a JSON id
    std::string idString = "\"12345\"";

    // WHEN: decoding the JSON id
    Json::Value decodedId = JsonRpcServiceUtil::DecodeJsonId(idString);

    // THEN: it should return the JSON value representing the id
    EXPECT_EQ(decodedId.asString(), "12345");
}

TEST(JsonRpcServiceUtil, TestCreateFeatureSettingsQuery) {
    // GIVEN: a feature name and value
    std::string feature = "highContrast";
    Json::Value value;
    value["enabled"] = true;

    // WHEN: creating a feature settings query
    Json::Value query = JsonRpcServiceUtil::CreateFeatureSettingsQuery(feature, value);

    // THEN: the query should contain the feature and value
    EXPECT_EQ(query["feature"].asString(), feature);
    EXPECT_EQ(query["value"]["enabled"].asBool(), true);
}

TEST(JsonRpcServiceUtil, TestCreateNotifyRequest) {
    // GIVEN: a JSON value for parameters
    Json::Value params;
    params["key"] = "value";

    // WHEN: creating a notify request
    Json::Value notifyRequest = JsonRpcServiceUtil::CreateNotifyRequest(params);

    // THEN: the notify request should contain the method and parameters
    EXPECT_EQ(notifyRequest["method"].asString(), MD_NOTIFY);
    EXPECT_EQ(notifyRequest["params"]["key"].asString(), "value");
}

TEST(JsonRpcServiceUtil, TestCreateClientRequest) {
    // GIVEN: an id, method, and parameters
    std::string id = "12345";
    std::string method = "testMethod";
    Json::Value params;
    params["param1"] = "value1";

    // WHEN: creating a client request
    Json::Value clientRequest = JsonRpcServiceUtil::CreateClientRequest(id, method, params);

    // THEN: the client request should contain the id, method, and parameters
    EXPECT_EQ(clientRequest["id"].asString(), "12345");
    EXPECT_EQ(clientRequest["method"].asString(), method);
    EXPECT_EQ(clientRequest["params"]["param1"].asString(), "value1");
}

TEST(JsonRpcServiceUtil, TestCreateJsonResponse) {
    // GIVEN: an id and result
    std::string id = "12345";
    Json::Value result;
    result["key"] = "value";

    // WHEN: creating a JSON response
    Json::Value jsonResponse = JsonRpcServiceUtil::CreateJsonResponse(id, result);

    // THEN: the JSON response should contain the id and result
    EXPECT_EQ(jsonResponse["id"].asString(), "12345");
    EXPECT_EQ(jsonResponse["result"]["key"].asString(), "value");
}

TEST(JsonRpcServiceUtil, TestCreateJsonErrorResponse) {
    // GIVEN: an id and error
    std::string id = "12345";
    Json::Value error;
    error["code"] = -32600; // Invalid Request
    error["message"] = "Invalid JSON format";

    // WHEN: creating a JSON error response
    Json::Value jsonErrorResponse = JsonRpcServiceUtil::CreateJsonErrorResponse(id, error);

    // THEN: the JSON error response should contain the id and error
    EXPECT_EQ(jsonErrorResponse["id"].asString(), "12345");
    EXPECT_EQ(jsonErrorResponse["error"]["code"].asInt(), -32600);
    EXPECT_EQ(jsonErrorResponse["error"]["message"].asString(), "Invalid JSON format");
}

TEST(JsonRpcServiceUtil, TestGetErrorMessage) {
    // GIVEN: a JSON RPC status
    JsonRpcService::JsonRpcStatus status = JsonRpcService::JsonRpcStatus::INVALID_PARAMS;

    // WHEN: getting the error message
    std::string errorMessage = JsonRpcServiceUtil::GetErrorMessage(status);

    // THEN: it should return the corresponding error message
    EXPECT_EQ(errorMessage, "Invalid params");
}

TEST(JsonRpcServiceUtil, TestGetAccessibilityFeatureName) {
    // GIVEN: an accessibility feature id
    int featureId = 1; // Assuming 1 corresponds to a valid feature

    // WHEN: getting the feature name
    std::string featureName = JsonRpcServiceUtil::GetAccessibilityFeatureName(featureId);

    // THEN: it should return the correct feature name
    EXPECT_EQ(featureName, "dialogueEnhancement"); // Replace with actual expected name
}

TEST(JsonRpcServiceUtil, TestGetAccessibilityFeatureId) {
    // GIVEN: a feature name
    std::string featureName = "dialogueEnhancement"; // Replace with actual feature name

    // WHEN: getting the feature id
    int featureId = JsonRpcServiceUtil::GetAccessibilityFeatureId(featureName);

    // THEN: it should return the correct feature id
    EXPECT_EQ(featureId, 1); // Replace with actual expected id
}

TEST(JsonRpcServiceUtil, TestConvertISO8601ToSecond) {
    // GIVEN: an ISO 8601 date string
    std::string isoDate = "2023-10-01T12:00:00Z";

    // WHEN: converting to seconds
    std::time_t seconds = JsonRpcServiceUtil::ConvertISO8601ToSecond(isoDate);

    // THEN: it should return the correct time in seconds
    EXPECT_GT(seconds, 0); // Check that the conversion was successful
}

TEST(JsonRpcServiceUtil, TestConvertSecondToISO8601) {
    // GIVEN: a time in seconds
    int seconds = 1696156800; // Corresponds to "2023-10-01T10:40:00Z"

    // WHEN: converting to ISO 8601 format
    std::string isoDate = JsonRpcServiceUtil::ConvertSecondToISO8601(seconds);

    // THEN: it should return the correct ISO 8601 date string
    EXPECT_EQ(isoDate, "2023-10-01T10:40:00Z");
}

TEST(JsonRpcServiceUtil, TestGetId) {
    // GIVEN: a JSON object with an id
    Json::Value json;
    json["id"] = "12345";

    // WHEN: getting the id from the JSON object
    std::string id = JsonRpcServiceUtil::GetId(json);

    // THEN: it should return the correct id
    EXPECT_EQ(id, "\"12345\"");
}

TEST(JsonRpcServiceUtil, TestGetAccessibilityFeatureIdFromJson) {
    // GIVEN: a JSON object with a feature parameter
    Json::Value json;
    json["params"]["feature"] = "dialogueEnhancement"; // Replace with actual feature name

    // WHEN: getting the accessibility feature id from the JSON object
    int featureId = JsonRpcServiceUtil::GetAccessibilityFeatureId(json);

    // THEN: it should return the correct feature id
    EXPECT_EQ(featureId, 1); // Replace with actual expected id
}

TEST(JsonRpcServiceUtil, TestGetAccessibilityFeatureIdFromJsonInvalid) {
    // GIVEN: a JSON object without a feature parameter
    Json::Value json;

    // WHEN: getting the accessibility feature id from the JSON object
    int featureId = JsonRpcServiceUtil::GetAccessibilityFeatureId(json);

    // THEN: it should return -1 indicating an invalid feature
    EXPECT_EQ(featureId, -1);
}

TEST(JsonRpcServiceUtil, TestGetIdFromJson) {
    // GIVEN: a JSON object with an id
    Json::Value json;
    json["id"] = "12345";

    // WHEN: getting the id from the JSON object
    std::string id = JsonRpcServiceUtil::GetId(json);

    // THEN: it should return the correct id
    EXPECT_EQ(id, "\"12345\"");

    // AND WHEN: the id is not present
    json.removeMember("id");
    id = JsonRpcServiceUtil::GetId(json);

    // THEN: it should return an empty string
    EXPECT_EQ(id, "");
}
