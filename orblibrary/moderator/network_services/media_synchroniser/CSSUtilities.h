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

#ifndef WIP_DVBCSS_HBBTV_CSSUTILITIES_H
#define WIP_DVBCSS_HBBTV_CSSUTILITIES_H

#include <json/json.h>
#include <unordered_map>
#include <string>

namespace NetworkServices {
namespace CSSUtilities {
bool unpack(const std::string &msg, Json::Value &msgToJson);
bool isWallclockTimeValid(const std::string &wct);
std::string getDvbUrlIdFromInt(int value);

namespace CIIMessageProperties {
const std::string keys[10] = {
    "protocolVersion",
    "mrsUrl",
    "contentId",
    "contentIdStatus",
    "presentationStatus",
    "wcUrl",
    "tsUrl",
    "teUrl",
    "private",
    "timelines"
};
const std::string presentationStatus[3] = {
    "okay",
    "transitioning",
    "fault"
};
const std::string contentIdStatus[2] = {
    "partial",
    "final"
};
const std::string protocolVersion = "1.1";
}
}
};


#endif //WIP_DVBCSS_HBBTV_CSSUTILITIES_H
