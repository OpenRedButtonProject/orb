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
#ifndef JSONIMPL_H
#define JSONIMPL_H

#include "IJson.h"
#include <json/json.h>

namespace orb
{

/**
 * Json implementation class which wraps the Json::Value object.
 * The methods implemented here are based static methods of the JsonUtil class.
 */
class JsonImpl : public IJson
{
public:
    JsonImpl();
    JsonImpl(std::string jsonString);
    JsonImpl(Json::Value json);
    JsonImpl(std::vector<uint16_t> array);
    JsonImpl(std::vector<int> array);
    virtual ~JsonImpl() {}
    bool parse(std::string jsonString) override;
    bool hasParam(const std::string &param, const JsonType& type = JsonType::JSON_TYPE_OBJECT) const override;
    std::string toString() const override;
    int getInteger(const std::string& key) const override;
    bool getBool(const std::string& key) const override;
    std::string getString(const std::string& key) const override;
    std::unique_ptr<IJson> getObject(const std::string& key) const override;
    void setInteger(const std::string& key, const int value, const std::string& subKey) override;
    void setBool(const std::string& key, const bool value, const std::string& subKey) override;
    void setString(const std::string& key, const std::string& value, const std::string& subKey) override;
    void setArray(const std::string& key, const std::vector<uint16_t>& array) override;
    void setArray(const std::string& key, const std::vector<int>& array) override;
    std::vector<uint16_t> getUint16Array(const std::string& key) const override;

private:
    Json::ValueType convertToJsonValueType(const JsonType& type) const;
    void setValue(const std::string& key, Json::Value value, const std::string& subKey);
    template<typename T>
    void setJsonArray(const std::string& key, const std::vector<T>& array);

private:
    Json::Value mJson;
};
} // namespace orb

#endif // JSONIMPL_H