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

namespace orb
{

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

} // namespace orb