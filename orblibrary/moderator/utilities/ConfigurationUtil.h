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
#ifndef CONFIGURATION_UTIL_H
#define CONFIGURATION_UTIL_H

#include <string>
#include <vector>
#include <memory>
#include <json/json.h>
#include "Capabilities.h"
#include "OrbConstants.h"

namespace orb
{

/**
 * Configuration utility class providing helper functions for configuration operations.
 * This class contains static utility methods for working with configuration data
 * such as capabilities, audio profiles, video profiles, and display formats.
 */
class ConfigurationUtil
{
public:
    /**
     * Default constructor.
     */
    ConfigurationUtil() = default;

    /**
     * Virtual destructor.
     */
    virtual ~ConfigurationUtil() = default;

    /**
     * Creates a default Capabilities object with empty values.
     *
     * @return A shared pointer to a new Capabilities object.
     */
    static std::shared_ptr<Capabilities> createDefaultCapabilities(ApplicationType apptype);

    /**
    * Creates a default vector of AudioProfile objects.
    *
    * @return A vector of AudioProfile objects.
    */
    static std::vector<AudioProfile> createDefaultAudioProfiles();

    /**
     * Creates an AudioProfile object with the given parameters.
     *
     * @param name The name of the audio profile.
     * @param type The type of the audio profile.
     * @param transport The transport of the audio profile.
     * @param syncTl The syncTl of the audio profile.
     * @param drmSystemId The drmSystemId of the audio profile.
     * @return An AudioProfile object.
     */
    static AudioProfile createAudioProfile(
        std::string name,
        std::string type,
        std::string transport,
        std::string syncTl,
        std::string drmSystemId);

    /**
     * Creates a default vector of VideoProfile objects.
     *
     * @return A vector of VideoProfile objects.
     */
    static std::vector<VideoProfile> createDefaultVideoProfiles();

    /**
     * Creates a VideoProfile object with the given parameters.
     *
     * @param name The name of the video profile.
     * @param type The type of the video profile.
     * @param transport The transport of the video profile.
     * @param syncTl The syncTl of the video profile.
     * @param drmSystemId The drmSystemId of the video profile.
     * @param hdr The hdr of the video profile.
     * @return A VideoProfile object.
     */
    static VideoProfile createVideoProfile(
        std::string name,
        std::string type,
        std::string transport,
        std::string syncTl,
        std::string drmSystemId,
        std::string hdr);

    /**
     * Creates a default VideoDisplayFormat object with zero values.
     *
     * @return A new VideoDisplayFormat object.
     */
    static VideoDisplayFormat createDefaultVideoDisplayFormat();

    /**
     * Converts a Capabilities object to a JSON string.
     *
     * @param capabilities The Capabilities object to convert.
     * @return A JSON object representation of the Capabilities object.
     */
    static Json::Value capabilitiesToJson(const Capabilities& capabilities);

    /**
     * Converts a vector of AudioProfile objects to a JSON string.
     *
     * @param audioProfiles The vector of AudioProfile objects to convert.
     * @return A JSON object representation of the AudioProfile objects.
     */
    static Json::Value audioProfilesToJson(const std::vector<AudioProfile>& audioProfiles);

    /**
     * Converts an AudioProfile object to a JSON string.
     *
     * @param audioProfile The AudioProfile object to convert.
     * @return A JSON object representation of the AudioProfile object.
     */
    static Json::Value audioProfileToJson(const AudioProfile& audioProfile);

    /**
     * Converts a vector of VideoProfile objects to a JSON string.
     *
     * @param videoProfiles The vector of VideoProfile objects to convert.
     * @return A JSON object representation of the VideoProfile objects.
     */
    static Json::Value videoProfilesToJson(const std::vector<VideoProfile>& videoProfiles);

    /**
     * Returns the JSON RPC server URL.
     *
     * @param port The port number to use for the JSON RPC server.
     * @return The JSON RPC server URL.
     */
    static std::string getJsonRpcServerUrl(int port);

    /**
     * Returns the JSON RPC server endpoint.
     *
     * @return The JSON RPC server endpoint.
     */
    static std::string getJsonRpcServerEndpoint();

    /**
     * @param apptype The application type.
     * @return The JSON RPC server port.
     */
    static int getJsonRpcServerPort(ApplicationType apptype);

    /**
     * Generates a random number string.
     *
     * @return A random number string.
     */
    static std::string generateRandomNumberStr();

}; // class ConfigurationUtil

} // namespace orb

#endif // CONFIGURATION_UTIL_H