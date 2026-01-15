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
 *
 * Hash Calculator Interface
 */
#ifndef I_HASH_CALCULATOR_H
#define I_HASH_CALCULATOR_H

#include <string>
#include <filesystem>

namespace orb
{

/**
 * @brief Interface for hash calculation.
 *
 * Allows mock implementations for testing.
 */
class IHashCalculator {
public:
    virtual ~IHashCalculator() = default;

    /**
     * Calculate SHA256 hash of a file.
     *
     * @param filePath Path to the file to hash
     * @return Hex-encoded SHA256 hash string
     */
    virtual std::string calculateSHA256Hash(const std::filesystem::path& filePath) const = 0;
};

} // namespace orb

#endif /* I_HASH_CALCULATOR_H */

