/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

#pragma once

#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace orb {
/**
 * @brief orb::TokenManager
 *
 * Implementation of a token manager that is used to create and process
 * JSON tokens that are intended to be used by the WPE bridge when issuing
 * requests to the ORB plugin.
 */
class TokenManager {
public:

   /**
    * Constructor.
    */
   TokenManager();

   /**
    * Destructor.
    */
   ~TokenManager();

   /**
    * @brief TokenManager::CreateToken
    *
    * Create a JSON token containing the given application ID and URI.
    *
    * @param appId The application ID to be included in the JSON token
    * @param uri   The application URI to be included in the JSON token
    *
    * @return The JSON token
    */
   json CreateToken(int appId, std::string uri);

   /**
    * @brief TokenManager::GetTokenPayload
    *
    * Get the payload from the specified JSON token.
    *
    * @param token The JSON token
    *
    * @return A JSON object containing the payload
    */
   json GetTokenPayload(json token);

private:

   std::string m_tokenSecretKey;
}; // class TokenManager
} // namespace orb
