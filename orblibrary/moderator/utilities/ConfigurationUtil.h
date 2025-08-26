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
#ifndef CONFIGURATION_UTIL_H
#define CONFIGURATION_UTIL_H

#include <string>
#include <json/json.h>
#include "OrbConstants.h"

namespace orb
{

/**
 * Configuration utility class providing helper functions for configuration operations.
 * This class contains static utility methods for working with configuration data
 * such as capabilities, audio profiles, video profiles, and display formats.
 */
class ConfigurationUtil
{
public:
    /**
     * Default constructor.
     */
    ConfigurationUtil() = default;

    /**
     * Virtual destructor.
     */
    virtual ~ConfigurationUtil() = default;

    /**
     * Generates a request string for a given method and application type.
     *
     * @param method The method to generate a request for.
     * @param apptype The application type.
     * @return A string representing the request.
     */
    static std::string generateRequest(std::string method, ApplicationType apptype);

}; // class ConfigurationUtil

} // namespace orb

#endif // CONFIGURATION_UTIL_H