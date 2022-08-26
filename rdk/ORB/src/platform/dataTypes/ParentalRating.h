/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

#pragma once

#include <core/core.h>

using namespace WPEFramework::Core::JSON;

namespace orb {
/**
 * @brief orb::ParentalRating
 *
 * Representation of the parental rating.
 */
class ParentalRating {
public:

   ParentalRating(
      std::string name,
      std::string scheme,
      std::string region,
      int value,
      int labels
      );

   ~ParentalRating();

   std::string GetName() const;
   std::string GetScheme() const;
   std::string GetRegion() const;
   int GetValue() const;
   int GetLabels() const;

   JsonObject ToJsonObject() const;

private:

   std::string m_name;
   std::string m_scheme;
   std::string m_region;
   int m_value;
   int m_labels;
}; // class ParentalRating
} // namespace orb
