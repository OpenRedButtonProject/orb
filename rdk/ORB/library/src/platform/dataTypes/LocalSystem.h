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
 * @brief orb::LocalSystem
 *
 * Representation of the local system.
 * (See OIPF DAE spec section 7.3.3 an in particular 7.3.3.2)
 */
class LocalSystem
{
public:

   /**
    * Constructor.
    *
    * @param vendorName      String identifying the vendor name of the device
    * @param modelName       String identifying the model name of the device
    * @param softwareVersion String identifying the version number of the platform firmware
    * @param hardwareVersion String identifying the version number of the platform hardware
    */
   LocalSystem(
      std::string vendorName,
      std::string modelName,
      std::string softwareVersion,
      std::string hardwareVersion
      )
      : m_vendorName(vendorName)
      , m_modelName(modelName)
      , m_softwareVersion(softwareVersion)
      , m_hardwareVersion(hardwareVersion)
   {
   }

   /**
    * Destructor.
    */
   ~LocalSystem()
   {
   }

   std::string GetVendorName() const
   {
      return m_vendorName;
   }

   std::string GetModelName() const
   {
      return m_modelName;
   }

   std::string GetSoftwareVersion() const
   {
      return m_softwareVersion;
   }

   std::string GetHardwareVersion() const
   {
      return m_hardwareVersion;
   }

private:

   std::string m_vendorName;
   std::string m_modelName;
   std::string m_softwareVersion;
   std::string m_hardwareVersion;
}; // class LocalSystem
} // namespace orb