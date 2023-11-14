/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "TokenManager.h"
#include "URI.h"
#include "Base64.h"
#include "SHA256.h"
#include "ORBLogging.h"
#include <uuid/uuid.h>

using namespace orb;

/**
 * @brief GenerateUUID
 *
 * Generate and return a random UUID as string.
 *
 * @return a random UUID as string
 */
static
std::string GenerateUUID(void)
{
    char *generated = (char *) malloc(37 * sizeof(char));
    uuid_t uuid;
    uuid_generate_random(uuid);
    uuid_unparse(uuid, generated);
    std::string result(generated);
    free(generated);
    return result;
}

/**
 * @brief GetHash
 *
 * Resolve and return the SHA256 hash of the specified message and key.
 *
 * @param key     The key to be used as input
 * @param message The message to be hashed
 *
 * @return The SHA256 hash
 */
static
std::string GetHash(std::string key, std::string message)
{
    std::string encryptedMessage = SHA256::Encrypt(message + key);
    return Base64::Encode(encryptedMessage);
}

/**
 * @brief CreateTokenFromPayload
 *
 * Create a new JSON token using the specified key and payload.
 *
 * @param key     The key to be used for creating the JSON token
 * @param payload The payload to be included in the JSON token
 *
 * @return The resulting JSON token
 */
static
json CreateTokenFromPayload(std::string key, json payload)
{
    json token;
    std::string signature;
    signature = GetHash(key, payload.dump());
    if (signature.empty())
    {
        return token;
    }
    token["payload"] = payload;
    token["signature"] = signature;
    return token;
}

/**
 * @brief GetTokenPayload
 *
 * Get the payload from the specified JSON token.
 *
 * @param key   The key to be used for assessing the JSON token's claimed signature
 * @param token The JSON token
 *
 * @return A JSON object containing the payload
 */
static
json GetPayloadFromToken(std::string key, json token)
{
    json payload;
    std::string claimedSignature;
    std::string signature;
    if (token["payload"].is_null() || token["payload"].empty() ||
        token["signature"].is_null() || token["signature"].empty())
    {
        return "{}"_json;
    }
    payload = token["payload"];
    claimedSignature = token["signature"];
    signature = GetHash(key, payload.dump());
    ORB_LOG("signature=%s", signature.c_str());
    if (!signature.empty() && signature == claimedSignature)
    {
        return payload;
    }
    payload = "{}"_json;
    return payload;
}

/**
 * @brief GetOrigin
 *
 * Get the origin of the specified URI.
 *
 * @param uri The URI
 *
 * @return The origin of the specified URI
 */
static
std::string GetOrigin(std::string uri)
{
    std::shared_ptr<URI> theUri = URI::Parse(uri);
    std::string protocol = theUri->GetProtocol();
    if (!(protocol == "http") && !(protocol == "https") && !(protocol == "dvb"))
    {
        return "uuid-" + GenerateUUID();
    }
    std::string port = theUri->GetPort();
    return protocol + "://" + theUri->GetHost() + ((port == "-1" || port.empty()) ? "" : ":" +
                                                   port);
}

namespace orb {
/**
 * Constructor.
 */
TokenManager::TokenManager()
{
    ORB_LOG_NO_ARGS();
    m_tokenSecretKey = GenerateUUID();
}

/**
 * Destructor.
 */
TokenManager::~TokenManager()
{
}

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
json TokenManager::CreateToken(int appId, std::string uri)
{
    json payload;
    payload["appId"] = appId;
    payload["uri"] = uri;
    payload["origin"] = GetOrigin(uri);
    return CreateTokenFromPayload(m_tokenSecretKey, payload);
}

/**
 * @brief TokenManager::GetTokenPayload
 *
 * Get the payload from the specified JSON token.
 *
 * @param token The JSON token
 *
 * @return A JSON object containing the payload
 */
json TokenManager::GetTokenPayload(json token)
{
    json payload = "{}"_json;
    if (!token.is_null() && !token.empty())
    {
        payload = GetPayloadFromToken(m_tokenSecretKey, token);
    }
    return payload;
}
} // namespace orb
