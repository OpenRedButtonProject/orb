/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

#include "ParentalRating.h"

namespace orb {
/**
 * Constructor.
 *
 * @param name   The age range in string format: "4-18"
 * @param scheme The scheme - "dvb-si" always
 * @param region Region might not be needed
 * @param value  The numeric value of name
 * @param labels Label flags might not be needed
 */
ParentalRating::ParentalRating(
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

ParentalRating::~ParentalRating()
{
}

std::string ParentalRating::GetName() const
{
   return m_name;
}

std::string ParentalRating::GetScheme() const
{
   return m_scheme;
}

std::string ParentalRating::GetRegion() const
{
   return m_region;
}

int ParentalRating::GetValue() const
{
   return m_value;
}

int ParentalRating::GetLabels() const
{
   return m_labels;
}

JsonObject ParentalRating::ToJsonObject() const
{
   JsonObject json_parentalRating;
   json_parentalRating.Set("name", GetName());
   json_parentalRating.Set("scheme", GetScheme());
   json_parentalRating.Set("region", GetRegion());
   json_parentalRating.Set("value", GetValue());
   json_parentalRating.Set("labels", GetLabels());
   return json_parentalRating;
}
} // namespace orb
