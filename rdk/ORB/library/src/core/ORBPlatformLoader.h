/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

#pragma once

#include <memory>
#include "ORBPlatform.h"

namespace orb {
/**
 * @brief orb::ORBPlatformLoader
 *
 * The ORB platform loader dynamically loads the ORB platform implementation
 * shared library.
 */
class ORBPlatformLoader {
public:

   /**
    * Constructor.
    */
   ORBPlatformLoader();

   /**
    * Destructor.
    */
   ~ORBPlatformLoader();

   /**
    * Load the ORB implementation library.
    *
    * @return A pointer to the resulting ORBPlatform object
    */
   ORBPlatform* Load();

   /**
    * Unload the ORB platform implementation library.
    *
    * @param orbPlatform Pointer to the ORB platform object
    *
    * @return true in success, false otherwise
    */
   bool Unload(ORBPlatform *orbPlatform);

private:

   void *m_lib;
}; // class ORBPlatformLoader
} // namespace orb
