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

#ifndef STRING_UTIL_H
#define STRING_UTIL_H

#include <string>
#include <vector>

namespace orb
{

/**
 * String utility class providing helper functions for string operations.
 * This class contains static utility methods for working with string data.
 */
class StringUtil
{
public:
    /**
     * Default constructor.
     */
    StringUtil() = default;

    /**
     * Virtual destructor.
     */
    virtual ~StringUtil() = default;

    /**
     * Resolves the component and method from the specified input, which has the following form:
     *
     * <component>.<method>
     *
     * @param input  (in)  The input string
     * @param component (out) Holds the resolved component in success
     * @param method (out) Holds the resolved method in success
     *
     * @return true in success, otherwise false
     */
    static bool ResolveMethod(std::string input, std::string& component, std::string& method);

}; // class StringUtil

} // namespace orb

#endif // STRING_UTIL_H