// create unit for all static methods in JsonRpcServiceUtil.cpp
#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/orb/components/network_services/json_rpc/JsonRpcServiceUtil.h"

using namespace orb::networkServices;

TEST(JsonRpcServiceUtil, TestGetMethodsInJsonArray) {
    // GIVEN: an unordered set of methods
    std::unordered_set<std::string> methods = {"method1", "method2", "method3"};
    
    // WHEN: converting to JSON array
    Json::Value jsonArray = JsonRpcServiceUtil::GetMethodsInJsonArray(methods);
    
    // THEN: the JSON array should contain the methods
    EXPECT_EQ(static_cast<int>(jsonArray.size()), 3);
    EXPECT_TRUE(JsonRpcServiceUtil::IsMethodInJsonArray(jsonArray, "method1"));
    EXPECT_TRUE(JsonRpcServiceUtil::IsMethodInJsonArray(jsonArray, "method2"));
    EXPECT_TRUE(JsonRpcServiceUtil::IsMethodInJsonArray(jsonArray, "method3"));
}

TEST(JsonRpcServiceUtil, TestIsMethodInJsonArray) {
    // GIVEN: a JSON array and a method
    Json::Value jsonArray(Json::arrayValue);
    jsonArray.append("method1");
    jsonArray.append("method2");
    
    // WHEN: checking if the method is in the JSON array
    bool exists = JsonRpcServiceUtil::IsMethodInJsonArray(jsonArray, "method1");
    
    // THEN: it should return true
    EXPECT_TRUE(exists);
    
    // AND WHEN: checking for a method that does not exist
    exists = JsonRpcServiceUtil::IsMethodInJsonArray(jsonArray, "method3");
    
    // THEN: it should return false
    EXPECT_FALSE(exists);
}

TEST(JsonRpcServiceUtil, TestIsMethodInSet) {
    // GIVEN: an unordered set of methods
    std::unordered_set<std::string> methods = {"method1", "method2", "method3"};
    
    // WHEN: checking if a method exists in the set
    bool exists = JsonRpcServiceUtil::IsMethodInSet(methods, "method1");
    
    // THEN: it should return true
    EXPECT_TRUE(exists);
    
    // AND WHEN: checking for a method that does not exist
    exists = JsonRpcServiceUtil::IsMethodInSet(methods, "method4");
    
    // THEN: it should return false
    EXPECT_FALSE(exists);
}

TEST(JsonRpcServiceUtil, TestHasParam) {
    // GIVEN: a JSON object with parameters
    Json::Value json;
    json["param1"] = "value1";
    json["param2"] = 42;
    
    // WHEN: checking for an existing parameter
    bool hasParam = JsonRpcServiceUtil::HasParam(json, "param1", Json::stringValue);
    
    // THEN: it should return true
    EXPECT_TRUE(hasParam);
    
    // AND WHEN: checking for a non-existing parameter
    hasParam = JsonRpcServiceUtil::HasParam(json, "param3", Json::stringValue);
    
    // THEN: it should return false
    EXPECT_FALSE(hasParam);
}

TEST(JsonRpcServiceUtil, TestHasJsonParam) {
    // GIVEN: a JSON object with parameters
    Json::Value json;
    // Adding a parameter to the JSON object
    Json::Value param1;
    param1["key"] = "value";
    json["param1"] = param1;
    
    // WHEN: checking for an existing JSON parameter
    bool hasJsonParam = JsonRpcServiceUtil::HasJsonParam(json, "param1");
    
    // THEN: it should return true
    EXPECT_TRUE(hasJsonParam);
    
    // AND WHEN: checking for a non-existing JSON parameter
    hasJsonParam = JsonRpcServiceUtil::HasJsonParam(json, "param2");
    
    // THEN: it should return false
    EXPECT_FALSE(hasJsonParam);
}

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

TEST(JsonRpcServiceUtil, TestAddArrayToJson) {
    // GIVEN: a JSON object and an array of integers
    Json::Value json;
    std::vector<int> intArray = {1, 2, 3};
    
    // WHEN: adding the integer array to the JSON object
    JsonRpcServiceUtil::AddArrayToJson(json, "numbers", intArray);
    
    // THEN: the JSON object should contain the array under the specified key
    EXPECT_EQ(static_cast<int>(json["numbers"].size()), 3);
    EXPECT_EQ(json["numbers"][0].asInt(), 1);
    EXPECT_EQ(json["numbers"][1].asInt(), 2);
    EXPECT_EQ(json["numbers"][2].asInt(), 3);
}

TEST(JsonRpcServiceUtil, TestGetStringValueFromJson) {
    // GIVEN: a JSON object with a string value
    Json::Value json;
    json["key"] = "value";
    
    // WHEN: getting the string value from the JSON object
    std::string value = JsonRpcServiceUtil::GetStringValueFromJson(json, "key");
    
    // THEN: it should return the correct string value
    EXPECT_EQ(value, "value");
    
    // AND WHEN: trying to get a non-existing key
    value = JsonRpcServiceUtil::GetStringValueFromJson(json, "nonExistingKey");
    
    // THEN: it should return an empty string
    EXPECT_EQ(value, OPTIONAL_STR_NOT_SET);
}

TEST(JsonRpcServiceUtil, TestGetIntValueFromJson) {
    // GIVEN: a JSON object with an integer value
    Json::Value json;
    json["key"] = 42;
    
    // WHEN: getting the integer value from the JSON object
    int value = JsonRpcServiceUtil::GetIntValueFromJson(json, "key");
    
    // THEN: it should return the correct integer value
    EXPECT_EQ(value, 42);
    
    // AND WHEN: trying to get a non-existing key
    value = JsonRpcServiceUtil::GetIntValueFromJson(json, "nonExistingKey");
    
    // THEN: it should return OPTIONAL_INT_NOT_SET
    EXPECT_EQ(value, OPTIONAL_INT_NOT_SET);
}

TEST(JsonRpcServiceUtil, TestGetBoolValueFromJson) {
    // GIVEN: a JSON object with a boolean value
    Json::Value json;
    json["key"] = true;
    
    // WHEN: getting the boolean value from the JSON object
    bool value = JsonRpcServiceUtil::GetBoolValueFromJson(json, "key");
    
    // THEN: it should return the correct boolean value
    EXPECT_TRUE(value);
    
    // AND WHEN: trying to get a non-existing key
    value = JsonRpcServiceUtil::GetBoolValueFromJson(json, "nonExistingKey");
    
    // THEN: it should return false
    EXPECT_FALSE(value);
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
