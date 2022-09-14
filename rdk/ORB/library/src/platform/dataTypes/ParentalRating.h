/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
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