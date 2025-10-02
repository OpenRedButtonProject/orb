/**
 * ORB Software. Copyright (c) 2025 Ocean Blue Software Limited
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
#ifndef IJSON_H
#define IJSON_H

#include <string>
#include <unordered_set>
#include <vector>
#include <cstdint>

namespace orb
{

/**
 * Interface for a Json class.
 */
class IJson
{

public:
    enum JsonType
    {
        JSON_TYPE_STRING,
        JSON_TYPE_INTEGER,
        JSON_TYPE_BOOLEAN,
        JSON_TYPE_ARRAY,
        JSON_TYPE_OBJECT
    };

    virtual ~IJson() = default;

    /**
     * Parses a JSON string into an Json object.
     *
     * @param jsonString The JSON string to parse.
     * @return 'true' if the JSON string was successfully parsed, 'false' otherwise.
     */
    virtual bool parse(std::string jsonString) = 0;

    /**
     * Check if a Json object has a specified parameter with a certain JsonType.
     *
     * @param param The name of the parameter to search for within the Json object.
     * @param type The expected data type of the parameter.
     * @return 'true' if the parameter 'param' exists within the Json object
     *          and has the specified JsonType, 'false' otherwise.
     */
    virtual bool hasParam(const std::string &param, const JsonType& type = JSON_TYPE_OBJECT) const = 0;

    /**
     * Converts current Json object to a string.
     *
     * @return A string representation of the Json object.
     */
    virtual std::string toString() const = 0;

    /**
     * Gets an integer value from a Json object by key.
     *
     * @param key The key of the integer value in the Json object.
     * @return The integer value if the key exists and the value is an integer,
     *         0 otherwise.
     */
    virtual int getInteger(const std::string& key) const = 0;

    /**
     * Gets a boolean value from a Json object by key.
     *
     * @param key The key of the boolean value in the Json object.
     * @return The boolean value if the key exists and the value is a boolean,
     *         false otherwise.
     */
    virtual bool getBool(const std::string& key) const = 0;

    /**
     * Gets a string value from a Json object by key.
     *
     * @param key The key of the string value in the Json object.
     * @return The string value if the key exists and the value is a string,
     *         empty string otherwise.
     */
    virtual std::string getString(const std::string& key) const = 0;

    /**
     * Gets an object value from a Json object by key.
     *
     * @param key The key of the object value in the Json object.
     * @return The object value if the key exists and the value is an object,
     *         nullptr otherwise.
     */
    virtual std::unique_ptr<IJson> getObject(const std::string& key) const = 0;

    /**
     * Sets an integer value in a Json object by key.
     *
     * @param key The key of the integer value in the Json object.
     * @param value The integer value to set.
     * @param subKey The sub-key of the integer value in the Json object.
     */
    virtual void setInteger(const std::string& key, const int value, const std::string& subKey = {}) = 0;

    /**
     * Sets a boolean value in a Json object by key.
     *
     * @param key The key of the boolean value in the Json object.
     * @param value The boolean value to set.
     * @param subKey The sub-key of the boolean value in the Json object.
     */
    virtual void setBool(const std::string& key, const bool value, const std::string& subKey = {}) = 0;

    /**
     * Sets a string value in a Json object by key.
     *
     * @param key The key of the string value in the Json object.
     * @param value The string value to set.
     * @param subKey The sub-key of the string value in the Json object.
     */
     virtual void setString(const std::string& key, const std::string& value, const std::string& subKey = {}) = 0;

    /**
     * Sets an array with uint16_t values in a Json object by key.
     *
     * @param key The key of the array in the Json object.
     * @param array The array of uint16_t to set.
     * @param subKey The sub-key of the array value in the Json object.
     */
    virtual void setArray(const std::string& key, const std::vector<uint16_t>& array) = 0;

    /**
     * Sets an integer array in a Json object by key.
     *
     * @param key The key of the array in the Json object.
     * @param array The array of integers to set.
     * @param subKey The sub-key of the array value in the Json object.
     */
    virtual void setArray(const std::string& key, const std::vector<int>& array) = 0;

    /**
     * Gets an array of unsigned 16-bit integers from a Json object by key.
     */
    virtual std::vector<uint16_t> getUint16Array(const std::string& key) const = 0;
};

/**
 * Json Factory class.
 * This class is responsible for creating a Json object.
 */
class JsonFactory
{
private:
    /**
     * Private Constructor
     */
    JsonFactory() = default;
    /**
     * Destructor
     */
    ~JsonFactory() = default;

public:
    /**
     * Create a Json object.
     *
     * @param jsonString The JSON string to create the instance from, default is empty string
     * @return A unique pointer to the created instance
     */
    static std::unique_ptr<IJson> createJson(const std::string& jsonString = {});
};

} // namespace orb

#endif // IJSON_H