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
#include "log.hpp"

namespace orb
{


bool JsonUtil::decodeJson(std::string jsonString, Json::Value *jsonval)
{
    Json::CharReaderBuilder builder;
    const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
    int rlen = static_cast<int>(jsonString.length());
    std::string err;

    if (!reader->parse(jsonString.c_str(), jsonString.c_str() + rlen, jsonval, &err))
    {
        LOGE("Json parsing failed: " << err);
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

int JsonUtil::getIntegerValue(const Json::Value& json, const std::string& key)
{
    if (!json.isMember(key))
    {
        LOGE("Key '" << key << "' not found in JSON object");
        return 0;
    }

    const Json::Value& value = json[key];
    if (!value.isInt())
    {
        LOGE("Value for key '" << key << "' is not an integer");
        return 0;
    }

    return value.asInt();
}

bool JsonUtil::getBoolValue(const Json::Value& json, const std::string& key)
{
    if (!json.isMember(key))
    {
        LOGE("Key '" << key << "' not found in JSON object");
        return false;
    }

    const Json::Value& value = json[key];
    if (!value.isBool())
    {
        LOGE("Value for key '" << key << "' is not a boolean");
        return false;
    }

    return value.asBool();
}

std::string JsonUtil::getStringValue(const Json::Value& json, const std::string& key)
{
    if (!json.isMember(key))
    {
        LOGE("Key '" << key << "' not found in JSON object");
        return "";
    }

    const Json::Value& value = json[key];
    if (!value.isString())
    {
        LOGE("Value for key '" << key << "' is not a string");
        return "";
    }

    return value.asString();
}

std::vector<uint16_t> JsonUtil::getIntegerArray(const Json::Value& json, const std::string& key)
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

    std::vector<uint16_t> result;
    for (const auto& element : value)
    {
        if (!element.isString())
        {
            LOGE("Array element is not a string in key '" << key << "'");
            return {};
        }
        
        const std::string& strValue = element.asString();
        if (strValue.empty())
        {
            LOGE("Array element is empty string in key '" << key << "'");
            return {};
        }
        
        // Check if string contains only digits (no negative numbers for uint16_t)
        bool isValid = true;
        for (size_t i = 0; i < strValue.length(); ++i)
        {
            if (strValue[i] < '0' || strValue[i] > '9')
            {
                isValid = false;
                break;
            }
        }
        
        if (!isValid)
        {
            LOGE("Array element '" << strValue << "' cannot be converted to uint16_t in key '" << key << "'");
            return {};
        }
        
        // Manual conversion to avoid exceptions
        uint16_t uintValue = 0;
        for (size_t i = 0; i < strValue.length(); ++i)
        {
            uintValue = uintValue * 10 + (strValue[i] - '0');
            
            // Check for overflow (uint16_t max value is 65535)
            if (uintValue > 65535)
            {
                LOGE("Array element '" << strValue << "' is too large for uint16_t in key '" << key << "'");
                return {};
            }
        }
        
        result.push_back(uintValue);
    }

    return result;
}

} // namespace orb