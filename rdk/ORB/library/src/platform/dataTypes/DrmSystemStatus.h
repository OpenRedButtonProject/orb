/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

#pragma once

#include <string>
#include <vector>

namespace orb
{
/**
 * Representation of a DRM system status.
 */
class DrmSystemStatus
{
public:

   /**
    * Enumerates the DrmSystemStatus::m_status values
    */
   enum State
   {
      DRM_STATE_READY       = 0x00, // (decimal 0) fully initialised and ready
      DRM_STATE_UNKNOWN     = 0x01, // (decimal 1) no longer available
      DRM_STATE_IITIALISING = 0x02, // (decimal 2) initialising and not ready to communicate
      DRM_STATE_ERROR       = 0x03  // (decimal 3) in error state
   };

public:

   /**
    * Default constructor.
    */
   DrmSystemStatus()
      : m_drmSystem("")
      , m_status(State::DRM_STATE_UNKNOWN)
      , m_protectionGateways("")
      , m_supportedFormats("")
   {
   }

   /**
    * Constructor.
    *
    * @param drmSystem          ID of the DRM system
    * @param drmSystemIds       List of the DRM System IDs handled by the DRM System
    * @param status             Status of the indicated DRM system
    * @param protectionGateways Space-separated list of zero or more CSP Gateway types that are
    *                           capable of supporting the DRM system
    * @param supportedFormats   Space separated list of zero or more supported file and/or
    *                           container formats by the DRM system
    */
   DrmSystemStatus(
      std::string drmSystem,
      std::vector<std::string> drmSystemIds,
      State status,
      std::string protectionGateways,
      std::string supportedFormats
      )
      : m_drmSystem(drmSystem)
      , m_drmSystemIds(drmSystemIds)
      , m_status(status)
      , m_protectionGateways(protectionGateways)
      , m_supportedFormats(supportedFormats)
   {
   }

   /**
    * Destructor.
    */
   ~DrmSystemStatus()
   {
      m_drmSystemIds.clear();
   }

public:

   void SetDrmSystem(std::string drmSystem)
   {
      m_drmSystem = drmSystem;
   }

   void SetDrmSystemIds(std::vector<std::string> drmSystemIds)
   {
      m_drmSystemIds = drmSystemIds;
   }

   void SetStatus(State status)
   {
      m_status = status;
   }

   void SetProtectionGateways(std::string protectionGateways)
   {
      m_protectionGateways = protectionGateways;
   }

   void SetSupportedFormats(std::string supportedFormats)
   {
      m_supportedFormats = supportedFormats;
   }

   std::string GetDrmSystem() const
   {
      return m_drmSystem;
   }

   std::vector<std::string> GetDrmSystemIds() const
   {
      return m_drmSystemIds;
   }

   State GetStatus() const
   {
      return m_status;
   }

   std::string GetProtectionGateways() const
   {
      return m_protectionGateways;
   }

   std::string GetSupportedFormats() const
   {
      return m_supportedFormats;
   }

private:

   // member variables
   std::string m_drmSystem;
   std::vector<std::string> m_drmSystemIds;
   State m_status;
   std::string m_protectionGateways;
   std::string m_supportedFormats;
}; // class DrmSystemStatus
} // namespace orb