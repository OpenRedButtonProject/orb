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

#include "StringUtil.h"
#include <cstring>

namespace orb
{

bool StringUtil::ResolveMethod(std::string input, std::string& component, std::string& method)
{
    std::vector<std::string> tokens;
    for (auto i = strtok(&input[0], "."); i != NULL; i = strtok(NULL, "."))
    {
        tokens.push_back(i);
    }
    if (tokens.size() != 2)
    {
        return false;
    }

    component = tokens[0];
    method = tokens[1];

    return true;
}

} // namespace orb