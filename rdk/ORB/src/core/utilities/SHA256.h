/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

#pragma once

#include <string>

namespace orb {
/**
 * @brief orb::SHA256
 *
 * Implementation of the SHA256 encryption algorithm.
 */
class SHA256 {
public:

   /**
    * @brief SHA256::Encrypt
    *
    * Encrypt the specified data using the SHA256 algorithm.
    *
    * @param data The data to be encrypted
    *
    * @return The encrypted data
    */
   static std::string Encrypt(std::string data);
}; // class SHA256
} // namespace orb
