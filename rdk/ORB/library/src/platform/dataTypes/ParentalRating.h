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

#pragma once

#include <string>

namespace orb
{
/**
 * @brief orb::ParentalRating
 *
 * Representation of the parental rating.
 * (See OIPF DAE spec section 7.9.4 and in particular 7.9.4.1)
 */
class ParentalRating {
public:

    /**
     * Constructor.
     *
     * @param name   The string representation of the parental rating value for the respective
     *               rating scheme denoted by property scheme
     * @param scheme Unique name identifying the parental rating guidance scheme to which this
     *               parental rating value refers
     * @param region The region to which the parental rating value applies as an alpha-2 region code
     *               as defined in ISO 3166-1
     * @param value  The parental rating value represented as an index into the set of values
     *               defined as part of the ParentalRatingScheme identified through
     *               property "scheme"
     * @param labels The labels property represents a set of parental advisory flags that may
     *               provide additional information about the rating
     */
    ParentalRating(
        std::string name,
        std::string scheme,
        std::string region,
        int value,
        int labels
        )
        : m_name(name)
        , m_scheme(scheme)
        , m_region(region)
        , m_value(value)
        , m_labels(labels)
    {
    }

    ~ParentalRating()
    {
    }

    std::string GetName() const
    {
        return m_name;
    }

    std::string GetScheme() const
    {
        return m_scheme;
    }

    std::string GetRegion() const
    {
        return m_region;
    }

    int GetValue() const
    {
        return m_value;
    }

    int GetLabels() const
    {
        return m_labels;
    }

private:

    std::string m_name;
    std::string m_scheme;
    std::string m_region;
    int m_value;
    int m_labels;
}; // class ParentalRating
} // namespace orb