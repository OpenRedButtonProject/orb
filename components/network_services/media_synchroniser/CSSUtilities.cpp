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
 *
 * NOTICE: This file has been created by Ocean Blue Software and is based on
 * the original work (https://github.com/bbc/pydvbcss) of the British
 * Broadcasting Corporation, as part of a translation of that work from a
 * Python library/tool to a native service. The following is the copyright
 * notice of the original work:
 *
 * Copyright 2015 British Broadcasting Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <memory>
#include "CSSUtilities.h"
#include "log.h"
#include <regex>
#include <iomanip>
#include <sstream>

namespace NetworkServices {
namespace CSSUtilities {
bool unpack(const std::string &msg, Json::Value &msgToJson)
{
#if JSONCPP_VERSION_HEXA > 0x01080200
    Json::CharReaderBuilder builder = {};
    auto reader = std::unique_ptr<Json::CharReader>(builder.newCharReader());

    std::string errors{};
    const auto is_parsed = reader->parse(msg.c_str(),
        msg.c_str() + msg.length(),
        &msgToJson,
        &errors);
#else
    Json::Reader reader;

    const auto is_parsed = reader.parse(msg.c_str(),
        msg.c_str() + msg.length(),
        msgToJson,
        false /* Discard comments */);
#endif
    if (!is_parsed)
    {
#if JSONCPP_VERSION_HEXA > 0x01080200
        LOG(LOG_ERROR, "CSSUtilities::unpack ERROR: Could not parse! %s\n", errors.c_str());
#else
        LOG(LOG_ERROR, "CSSUtilities::unpack ERROR: Could not parse!\n");
#endif
        return false;
    }

    return true;
}

bool isWallclockTimeValid(const std::string &wct)
{
    return regex_match(wct, std::regex("^(\\+|-)inf$|^[0-9]+$"));
}

std::string getDvbUrlIdFromInt(int value)
{
    std::stringstream stream;
    stream << std::setfill('0') <<
        std::setw(4) <<
        std::hex <<
        value;
    return stream.str();
}
}
}
