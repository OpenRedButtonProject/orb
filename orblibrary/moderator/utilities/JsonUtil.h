/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef JSON_UTIL_H
#define JSON_UTIL_H

#include <json/json.h>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace orb
{

/**
 * JSON utility class providing helper functions for JSON operations.
 * This class contains static utility methods for working with JSON data.
 */
class JsonUtil
{
public:
    /**
     * Default constructor.
     */
    JsonUtil() = default;

    /**
     * Virtual destructor.
     */
    virtual ~JsonUtil() = default;

    /**
     * Decodes a JSON string into a Json::Value object.
     *
     * @param jsonString The JSON string to decode.
     * @param jsonval Pointer to the Json::Value object where the decoded JSON will be stored.
     * @return 'true' if the JSON string was successfully decoded, 'false' otherwise.
     */
    static bool decodeJson(std::string jsonString, Json::Value *jsonval);

    /**
     * Check if a JSON object has a specified parameter with a certain data type.
     *
     * @param json The JSON object to check for the presence of the parameter.
     * @param param The name of the parameter to search for within the JSON object.
     * @param type The expected data type of the parameter.
     * @return 'true' if the parameter 'param' exists within the JSON object
     *          and has the specified data type, 'false' otherwise.
     */
    static bool HasParam(const Json::Value &json, const std::string &param, const Json::ValueType& type);

    /**
     * Check if a JSON object has a specified parameter with a json data type.
     *
     * @param json The JSON object to check for the presence of the parameter.
     * @param param The name of the parameter to search for within the JSON object.
     * @return 'true' if the parameter 'param' exists within the JSON object
     *          and has the json data type, 'false' otherwise.
     */
    static bool HasJsonParam(const Json::Value &json, const std::string &param);

    /**
     * Converts a JSON object to a string.
     *
     * @param json The JSON object to convert.
     * @return A string representation of the JSON object.
     */
    static std::string convertJsonToString(const Json::Value& json);

    /**
     * Gets an integer value from a JSON object by key.
     *
     * @param json The JSON object to extract the integer from.
     * @param key The key of the integer value in the JSON object.
     * @return The integer value if the key exists and the value is an integer,
     *         0 otherwise.
     */
    static int getIntegerValue(const Json::Value& json, const std::string& key);

    /**
     * Gets a boolean value from a JSON object by key.
     *
     * @param json The JSON object to extract the boolean from.
     * @param key The key of the boolean value in the JSON object.
     * @return The boolean value if the key exists and the value is a boolean,
     *         false otherwise.
     */
    static bool getBoolValue(const Json::Value& json, const std::string& key);

    /**
     * Gets a string value from a JSON object by key.
     *
     * @param json The JSON object to extract the string from.
     * @param key The key of the string value in the JSON object.
     * @return The string value if the key exists and the value is a string,
     *         empty string otherwise.
     */
    static std::string getStringValue(const Json::Value& json, const std::string& key);

    /**
     * Gets an array of unsigned 16-bit integers from a JSON object by key.
     *
     * @param json The JSON object to extract the integer array from.
     * @param key The key of the integer array in the JSON object.
     * @return A vector of uint16_t if the key exists and the value is an array of strings
     *         that can be converted to unsigned 16-bit integers, empty vector otherwise.
     */
    static std::vector<uint16_t> getIntegerArray(const Json::Value& json, const std::string& key);

    /**
     * Get a JSON array containing the methods in a set.
     *
     * @param set The set of methods to convert to a JSON array.
     * @return A JSON array containing the methods in the set.
     */
    static Json::Value GetMethodsInJsonArray(const std::unordered_set<std::string>& set);

    /**
     * Check if a method is present in a set of methods.
     *
     * @param set The set of methods to check for the presence of the method.
     * @param method The method to search for within the set.
     * @return 'true' if the method is present in the set, 'false' otherwise.
     */
    static bool IsMethodInSet(const std::unordered_set<std::string> &set, const std::string& method);

    /**
     * Check if a method is present in a JSON array.
     *
     * @param array The JSON array to check for the presence of the method.
     * @param method The method to search for within the JSON array.
     * @return 'true' if the method is present in the JSON array, 'false' otherwise.
     */
    static bool IsMethodInJsonArray(const Json::Value& array, const std::string& method);

    /**
     * Add an array to a JSON object.
     *
     * @param json The JSON object to add the array to.
     * @param key The key to use for the array in the JSON object.
     * @param array The array to add to the JSON object.
     */
    static void AddArrayToJson(Json::Value &json, const std::string &key, const std::vector<int> &array);

    /**
     * Add a property to the params of a JSON object.
     *
     * @param jsonString The JSON string to add the property to.
     * @param key The key to use for the property in the JSON object.
     * @param value The integer value for the property in the JSON object.
     * @return The JSON string with the property added to the params.
     */
    static std::string AddPropertyToParams(const std::string &jsonString, const std::string &key, int value);


}; // class JsonUtil

} // namespace orb

#endif // JSON_UTIL_H