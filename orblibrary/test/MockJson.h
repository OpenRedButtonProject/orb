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
#ifndef MOCKJSON_H
#define MOCKJSON_H

#include "IJson.h"
#include <gmock/gmock.h>
#include <memory>
#include <unordered_map>
#include <vector>

namespace orb {

/**
 * Mock implementation of IJson interface for testing
 */
class MockJson : public IJson {
public:
    MOCK_METHOD(bool, parse, (std::string jsonString), (override));
    MOCK_METHOD(bool, hasParam, (const std::string& param, const JsonType& type), (const, override));
    MOCK_METHOD(std::string, toString, (), (const, override));
    MOCK_METHOD(int, getInteger, (const std::string& key), (const, override));
    MOCK_METHOD(bool, getBool, (const std::string& key), (const, override));
    MOCK_METHOD(std::string, getString, (const std::string& key), (const, override));
    MOCK_METHOD(std::unique_ptr<IJson>, getObject, (const std::string& key), (const, override));
    MOCK_METHOD(void, setInteger, (const std::string& key, const int value, const std::string& subKey), (override));
    MOCK_METHOD(void, setBool, (const std::string& key, const bool value, const std::string& subKey), (override));
    MOCK_METHOD(void, setString, (const std::string& key, const std::string& value, const std::string& subKey), (override));
    MOCK_METHOD(void, setArray, (const std::string& key, const std::vector<uint16_t>& array), (override));
    MOCK_METHOD(void, setArray, (const std::string& key, const std::vector<int>& array), (override));
    MOCK_METHOD(std::vector<uint16_t>, getUint16Array, (const std::string& key), (const, override));
};


} // namespace orb

#endif // MOCKJSON_H
