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

#include "JsonUtil.h"
#include "log.h"

namespace orb
{

    constexpr int OPTIONAL_INT_NOT_SET = -999999;
    const std::string OPTIONAL_STR_NOT_SET = "";

    bool JsonUtil::decodeJson(std::string jsonString, Json::Value *jsonval)
    {
        Json::CharReaderBuilder reader;
        std::string errors;
        std::istringstream responseStream(jsonString);
        bool parseSuccess = Json::parseFromStream(reader, responseStream, jsonval, &errors);
        if (!parseSuccess) {
            LOGE("Json parsing failed: " << errors);
            return false;
        }
        return true;
    }

    bool JsonUtil::HasParam(const Json::Value &json, const std::string &param, const Json::ValueType& type)
    {
        return json.isMember(param) && json[param].type() == type;
    }

    bool JsonUtil::HasJsonParam(const Json::Value &json, const std::string &param)
    {
        return json.isMember(param) && json[param].isObject();
    }

    std::string JsonUtil::convertJsonToString(const Json::Value& json)
    {
        Json::StreamWriterBuilder writer;
        return Json::writeString(writer, json);
    }

    std::string JsonUtil::getStringValue(const Json::Value &json, const std::string &key)
    {
        if (HasParam(json, key, Json::stringValue)) {
            return json[key].asString();
        }
        return OPTIONAL_STR_NOT_SET;
    }

    int JsonUtil::getIntegerValue(const Json::Value &json, const std::string &key)
    {
        if (HasParam(json, key, Json::intValue)) {
            return json[key].asInt();
        }
        return OPTIONAL_INT_NOT_SET;
    }

    bool JsonUtil::getBoolValue(const Json::Value &json, const std::string &key)
    {
        if (HasParam(json, key, Json::booleanValue)) {
            return json[key].asBool();
        }
        return false;
    }

    std::vector<int> JsonUtil::getIntegerArray(const Json::Value& json, const std::string& key)
    {
        if (!json.isMember(key))
        {
            LOGE("Key '" << key << "' not found in JSON object");
            return {};
        }

        const Json::Value& value = json[key];
        if (!value.isArray())
        {
            LOGE("Value for key '" << key << "' is not an array");
            return {};
        }

        std::vector<int> result;
        for (const auto& element : value)
        {
            if (!element.isInt() && !element.isUInt())
            {
                LOGE("Array element is not an integer in key '" << key << "'. Skipping!");
                continue;
            }

            result.push_back(element.asInt());
        }

        return result;
    }

    Json::Value JsonUtil::GetMethodsInJsonArray(const std::unordered_set<std::string>& set)
    {
        Json::Value value;
        int index = 0;
        // Loop through the unordered_set 'set' and copy each string item into 'value' as a Json::arrayValue
        for (const std::string& item : set)
        {
            // Add each 'item' as a Json::Value to 'value' at the current 'index'
            value[index++] = Json::Value(item);
        }
        return value;
    }

    bool JsonUtil::IsMethodInJsonArray(const Json::Value& array, const std::string& method)
    {
        if (array.type() == Json::arrayValue)
        {
            for (const auto& element : array)
            {
                if (element.asString() == method)
                {
                    return true;
                }
            }
        }
        return false;
    }

    bool JsonUtil::IsMethodInSet(const std::unordered_set<std::string> &set, const std::string& method)
    {
        return set.find(method) != set.end();
    }

    void JsonUtil::AddArrayToJson(Json::Value &json, const std::string &key, const std::vector<int> &array)
    {
       Json::Value jsonArray(Json::arrayValue);
       for (const auto& item : array)
       {
           jsonArray.append(item);
       }
       json[key] = jsonArray;
   }

   std::string JsonUtil::AddPropertyToParams(const std::string &jsonString, const std::string &key, int value)
   {
       Json::Value jsonval;
       if (decodeJson(jsonString, &jsonval)) {
           jsonval["params"][key] = value;
           return convertJsonToString(jsonval);
       }
       return "";
   }
} // namespace orb
