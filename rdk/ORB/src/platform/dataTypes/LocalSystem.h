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
 * @brief orb::LocalSystem
 *
 * Representation of the local system.
 */
class LocalSystem {

public:

  LocalSystem(
    bool valid,
    std::string vendorName,
    std::string modelName,
    std::string softwareVersion,
    std::string hardwareVersion
  );

  ~LocalSystem();

  bool IsValid() const;
  std::string GetVendorName() const;
  std::string GetModelName() const;
  std::string GetSoftwareVersion() const;
  std::string GetHardwareVersion() const;

  JsonObject ToJsonObject() const;

private:

  bool m_valid;
  std::string m_vendorName;
  std::string m_modelName;
  std::string m_softwareVersion;
  std::string m_hardwareVersion;  

}; // class LocalSystem

} // namespace orb
