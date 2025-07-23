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

} // namespace orb