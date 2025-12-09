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
#include "JsonImpl.h"
#include "JsonUtil.h"

namespace orb
{
JsonImpl::JsonImpl() {}

JsonImpl::JsonImpl(std::string jsonString)
{
    if (!jsonString.empty()) {
        parse(jsonString);
    }
}

JsonImpl::JsonImpl(Json::Value json)
{
    mJson = json;
}


bool JsonImpl::parse(std::string jsonString)
{

    return JsonUtil::decodeJson(jsonString, &mJson);
}

bool JsonImpl::hasParam(const std::string &param, const JsonType& type) const
{
    return JsonUtil::HasParam(mJson, param, convertToJsonValueType(type));
}

std::string JsonImpl::toString() const
{
    return JsonUtil::convertJsonToString(mJson);
}

int JsonImpl::getInteger(const std::string& key) const
{
    return JsonUtil::getIntegerValue(mJson, key);
}

bool JsonImpl::getBool(const std::string& key) const
{
    return JsonUtil::getBoolValue(mJson, key);
}

std::string JsonImpl::getString(const std::string& key) const
{
    return JsonUtil::getStringValue(mJson, key);
}

std::unique_ptr<IJson> JsonImpl::getObject(const std::string& key) const
{
    return std::make_unique<JsonImpl>(mJson[key]);
}

Json::ValueType JsonImpl::convertToJsonValueType(const JsonType& type) const
{
    switch (type) {
        case JsonType::JSON_TYPE_STRING:
            return Json::stringValue;
        case JsonType::JSON_TYPE_INTEGER:
            return Json::intValue;
        case JsonType::JSON_TYPE_BOOLEAN:
            return Json::booleanValue;
        case JsonType::JSON_TYPE_ARRAY:
            return Json::arrayValue;
        case JsonType::JSON_TYPE_OBJECT:
            return Json::objectValue;
    }
    return Json::nullValue;
}

void JsonImpl::setInteger(const std::string& key, const int value, const std::string& subKey)
{
    setValue(key, Json::Value(value), subKey);
}

void JsonImpl::setBool(const std::string& key, const bool value, const std::string& subKey)
{
    setValue(key, Json::Value(value), subKey);
}

void JsonImpl::setString(const std::string& key, const std::string& value, const std::string& subKey)
{
    setValue(key, Json::Value(value), subKey);
}

void JsonImpl::setValue(const std::string& key, Json::Value value, const std::string& subKey)
{
    if (!subKey.empty()) {
        mJson[key][subKey] = value;
    }
    else {
        mJson[key] = value;
    }
}

void JsonImpl::setArray(const std::string& key, const std::vector<uint16_t>& array)
{
    setJsonArray(key, array);
}

void JsonImpl::setArray(const std::string& key, const std::vector<int>& array)
{
    setJsonArray(key, array);
}

    //  create a template function to set the array value
template<typename T>
void JsonImpl::setJsonArray(const std::string& key, const std::vector<T>& array)
{
    Json::Value jsonArray(Json::arrayValue);
    for (const auto& item : array)
    {
        jsonArray.append(item);
    }
    mJson[key] = jsonArray;
}

std::vector<uint16_t> JsonImpl::getUint16Array(const std::string& key) const
{
    std::vector<uint16_t> result;
    for (const auto& item : JsonUtil::getIntegerArray(mJson, key)) {
        result.push_back(static_cast<uint16_t>(item));
    }
    return result;
}

/**
 * The implementation of static method createJson of JsonFactory class
 */
std::unique_ptr<IJson> IJson::create(const std::string& jsonString)
{
    return std::make_unique<orb::JsonImpl>(jsonString);
}

}
