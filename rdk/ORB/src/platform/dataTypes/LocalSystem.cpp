/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

#include "LocalSystem.h"

namespace orb {
/**
 * Constructor.
 *
 * @param valid
 * @param vendorName
 * @param modelName
 * @param softwareVersion
 * @param hardwareVersion
 */
LocalSystem::LocalSystem(
   bool valid,
   std::string vendorName,
   std::string modelName,
   std::string softwareVersion,
   std::string hardwareVersion
   )
   : m_valid(valid)
   , m_vendorName(vendorName)
   , m_modelName(modelName)
   , m_softwareVersion(softwareVersion)
   , m_hardwareVersion(hardwareVersion)
{
}

/**
 * Destructor.
 */
LocalSystem::~LocalSystem()
{
}

bool LocalSystem::IsValid() const
{
   return m_valid;
}

std::string LocalSystem::GetVendorName() const
{
   return m_vendorName;
}

std::string LocalSystem::GetModelName() const
{
   return m_modelName;
}

std::string LocalSystem::GetSoftwareVersion() const
{
   return m_softwareVersion;
}

std::string LocalSystem::GetHardwareVersion() const
{
   return m_hardwareVersion;
}

JsonObject LocalSystem::ToJsonObject() const
{
   JsonObject json_LocalSystem;
   json_LocalSystem.Set("valid", IsValid());
   json_LocalSystem.Set("vendorName", GetVendorName());
   json_LocalSystem.Set("modelName", GetModelName());
   json_LocalSystem.Set("softwareVersion", GetSoftwareVersion());
   json_LocalSystem.Set("hardwareVersion", GetHardwareVersion());
   return json_LocalSystem;
}
} // namespace orb
