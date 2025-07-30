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

}; // class JsonUtil

} // namespace orb

#endif // JSON_UTIL_H