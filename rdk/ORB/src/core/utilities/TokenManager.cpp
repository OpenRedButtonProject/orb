/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

#include "TokenManager.h"
#include "URI.h"
#include "Base64.h"
#include "SHA256.h"
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
JsonObject CreateTokenFromPayload(std::string key, JsonObject payload)
{
    JsonObject token;
    std::string json;
    std::string signature;
    payload.ToString(json);
    signature = GetHash(key, json);
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
JsonObject GetPayloadFromToken(std::string key, JsonObject token)
{
    JsonObject payload;
    std::string json;
    std::string claimedSignature;
    std::string signature;
    payload = token["payload"].Object();
    payload.ToString(json);
    claimedSignature = token["signature"].String();
    signature = GetHash(key, json);
    if (!signature.empty() && signature == claimedSignature)
    {
        payload.FromString(json);
    }
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
    fprintf(stderr, "[TokenManager::TokenManager]\n");
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
JsonObject TokenManager::CreateToken(int appId, std::string uri)
{
    JsonObject payload;
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
JsonObject TokenManager::GetTokenPayload(JsonObject token)
{
    JsonObject payload = GetPayloadFromToken(m_tokenSecretKey, token);
    return payload;
}
} // namespace orb
