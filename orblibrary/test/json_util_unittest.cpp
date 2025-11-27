#include <iostream>
#include <string>
#include <json/json.h>

#include "testing/gtest/include/gtest/gtest.h"
#include "JsonUtil.h"

class JsonUtilTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Test setup if needed
    }

    void TearDown() override {
        // Test cleanup if needed
    }
};

TEST_F(JsonUtilTest, TestHasParam_StringValue_Exists) {
    // GIVEN: a JSON object with a string parameter
    Json::Value jsonObject;
    jsonObject["testParam"] = "testValue";

    // WHEN: HasParam is called with stringValue type
    bool result = orb::JsonUtil::HasParam(jsonObject, "testParam", Json::stringValue);

    // THEN: true is returned
    EXPECT_TRUE(result);
}

TEST_F(JsonUtilTest, TestHasParam_StringValue_NotExists) {
    // GIVEN: a JSON object without the parameter
    Json::Value jsonObject;
    jsonObject["otherParam"] = "otherValue";

    // WHEN: HasParam is called with stringValue type
    bool result = orb::JsonUtil::HasParam(jsonObject, "testParam", Json::stringValue);

    // THEN: false is returned
    EXPECT_FALSE(result);
}

TEST_F(JsonUtilTest, TestHasParam_StringValue_WrongType) {
    // GIVEN: a JSON object with a parameter of wrong type
    Json::Value jsonObject;
    jsonObject["testParam"] = 42; // integer instead of string

    // WHEN: HasParam is called with stringValue type
    bool result = orb::JsonUtil::HasParam(jsonObject, "testParam", Json::stringValue);

    // THEN: false is returned
    EXPECT_FALSE(result);
}

TEST_F(JsonUtilTest, TestHasParam_IntValue_Exists) {
    // GIVEN: a JSON object with an integer parameter
    Json::Value jsonObject;
    jsonObject["testParam"] = 42;

    // WHEN: HasParam is called with intValue type
    bool result = orb::JsonUtil::HasParam(jsonObject, "testParam", Json::intValue);

    // THEN: true is returned
    EXPECT_TRUE(result);
}

TEST_F(JsonUtilTest, TestHasParam_BoolValue_Exists) {
    // GIVEN: a JSON object with a boolean parameter
    Json::Value jsonObject;
    jsonObject["testParam"] = true;

    // WHEN: HasParam is called with booleanValue type
    bool result = orb::JsonUtil::HasParam(jsonObject, "testParam", Json::booleanValue);

    // THEN: true is returned
    EXPECT_TRUE(result);
}

TEST_F(JsonUtilTest, TestHasParam_ArrayValue_Exists) {
    // GIVEN: a JSON object with an array parameter
    Json::Value jsonObject;
    jsonObject["testParam"] = Json::Value(Json::arrayValue);
    jsonObject["testParam"].append("item1");
    jsonObject["testParam"].append("item2");

    // WHEN: HasParam is called with arrayValue type
    bool result = orb::JsonUtil::HasParam(jsonObject, "testParam", Json::arrayValue);

    // THEN: true is returned
    EXPECT_TRUE(result);
}

TEST_F(JsonUtilTest, TestHasJsonParam_ObjectExists) {
    // GIVEN: a JSON object with a nested object parameter
    Json::Value jsonObject;
    jsonObject["testParam"] = Json::Value(Json::objectValue);
    jsonObject["testParam"]["nestedKey"] = "nestedValue";

    // WHEN: HasJsonParam is called
    bool result = orb::JsonUtil::HasJsonParam(jsonObject, "testParam");

    // THEN: true is returned
    EXPECT_TRUE(result);
}

TEST_F(JsonUtilTest, TestHasJsonParam_ObjectNotExists) {
    // GIVEN: a JSON object without the parameter
    Json::Value jsonObject;
    jsonObject["otherParam"] = "otherValue";

    // WHEN: HasJsonParam is called
    bool result = orb::JsonUtil::HasJsonParam(jsonObject, "testParam");

    // THEN: false is returned
    EXPECT_FALSE(result);
}

TEST_F(JsonUtilTest, TestHasJsonParam_ParameterNotObject) {
    // GIVEN: a JSON object with a parameter that is not an object
    Json::Value jsonObject;
    jsonObject["testParam"] = "stringValue"; // string instead of object

    // WHEN: HasJsonParam is called
    bool result = orb::JsonUtil::HasJsonParam(jsonObject, "testParam");

    // THEN: false is returned
    EXPECT_FALSE(result);
}

TEST_F(JsonUtilTest, TestHasJsonParam_EmptyObject) {
    // GIVEN: a JSON object with an empty object parameter
    Json::Value jsonObject;
    jsonObject["testParam"] = Json::Value(Json::objectValue);

    // WHEN: HasJsonParam is called
    bool result = orb::JsonUtil::HasJsonParam(jsonObject, "testParam");

    // THEN: true is returned (empty object is still an object)
    EXPECT_TRUE(result);
}

TEST_F(JsonUtilTest, TestGetIntegerArray) {
    // GIVEN: a minimal JSON string where 'key' is an integer array
    const std::string jsonString = R"({"key":[458,65535, -1, 0]})";

    Json::Value root;
    ASSERT_TRUE(orb::JsonUtil::decodeJson(jsonString, &root));

    // WHEN: getIntegerArray is called on the 'key' field
    auto result = orb::JsonUtil::getIntegerArray(root, "key");

    // THEN: the result contains the integer values from 'key'
    ASSERT_EQ(result.size(), 4u);
    EXPECT_EQ(result[0], 458);
    EXPECT_EQ(result[1], 65535);
    EXPECT_EQ(result[2], -1);
    EXPECT_EQ(result[3], 0);
}

TEST_F(JsonUtilTest, TestConvertJsonToString_SimpleObject) {
    // GIVEN: a simple JSON object
    Json::Value jsonObject;
    jsonObject["stringField"] = "test_value";
    jsonObject["intField"] = 42;
    jsonObject["boolField"] = true;

    // WHEN: convertJsonToString is called
    std::string result = orb::JsonUtil::convertJsonToString(jsonObject);

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
}

TEST_F(JsonUtilTest, TestConvertJsonToString_ComplexObject) {
    // GIVEN: a complex JSON object with nested structures
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
    std::string result = orb::JsonUtil::convertJsonToString(jsonObject);

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

TEST_F(JsonUtilTest, TestConvertJsonToString_EmptyObject) {
    // GIVEN: an empty JSON object
    Json::Value jsonObject(Json::objectValue);

    // WHEN: convertJsonToString is called
    std::string result = orb::JsonUtil::convertJsonToString(jsonObject);

    // THEN: a valid JSON string is returned
    EXPECT_FALSE(result.empty());
    EXPECT_EQ(result, "{}");
}

TEST_F(JsonUtilTest, TestConvertJsonToString_Array) {
    // GIVEN: a JSON array
    Json::Value jsonArray(Json::arrayValue);
    jsonArray.append("item1");
    jsonArray.append("item2");
    jsonArray.append(42);

    // WHEN: convertJsonToString is called
    std::string result = orb::JsonUtil::convertJsonToString(jsonArray);

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
    EXPECT_TRUE(parsedJson.isArray());
    EXPECT_EQ(parsedJson.size(), (unsigned int)3);
    EXPECT_EQ(parsedJson[0].asString(), "item1");
    EXPECT_EQ(parsedJson[1].asString(), "item2");
    EXPECT_EQ(parsedJson[2].asInt(), 42);
}

TEST_F(JsonUtilTest, TestConvertJsonToString_StringValue) {
    // GIVEN: a JSON string value
    Json::Value jsonString = "test_string";

    // WHEN: convertJsonToString is called
    std::string result = orb::JsonUtil::convertJsonToString(jsonString);

    // THEN: a valid JSON string is returned
    EXPECT_FALSE(result.empty());
    EXPECT_EQ(result, "\"test_string\"");
}

TEST_F(JsonUtilTest, TestConvertJsonToString_NumberValue) {
    // GIVEN: a JSON number value
    Json::Value jsonNumber = 123.45;

    // WHEN: convertJsonToString is called
    std::string result = orb::JsonUtil::convertJsonToString(jsonNumber);

    // THEN: a valid JSON string is returned
    EXPECT_FALSE(result.empty());
    EXPECT_EQ(result, "123.45");
}

TEST_F(JsonUtilTest, TestConvertJsonToString_BooleanValue) {
    // GIVEN: a JSON boolean value
    Json::Value jsonBool = true;

    // WHEN: convertJsonToString is called
    std::string result = orb::JsonUtil::convertJsonToString(jsonBool);

    // THEN: a valid JSON string is returned
    EXPECT_FALSE(result.empty());
    EXPECT_EQ(result, "true");
}

TEST_F(JsonUtilTest, TestConvertJsonToString_NullValue) {
    // GIVEN: a JSON null value
    Json::Value jsonNull = Json::Value(Json::nullValue);

    // WHEN: convertJsonToString is called
    std::string result = orb::JsonUtil::convertJsonToString(jsonNull);

    // THEN: a valid JSON string is returned
    EXPECT_FALSE(result.empty());
    EXPECT_EQ(result, "null");
}
