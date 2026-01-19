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
 * ZIP Archive Extractor Interface
 */
#ifndef I_UNZIPPER_H
#define I_UNZIPPER_H

#include <string>
#include <filesystem>
#include <vector>
#include <cstdint>

namespace orb
{

/**
 * @brief Interface for ZIP archive extraction and inspection.
 *
 * Provides methods for extracting ZIP archives and inspecting their contents
 * without full extraction, as required for package verification per
 * TS 103 606 Section 6.1.8.
 *
 * Allows mock implementations for testing.
 */
class IUnzipper {
public:
    virtual ~IUnzipper() = default;

    /**
     * Extract a ZIP archive to a destination directory.
     *
     * @param zipFile Path to the ZIP archive file
     * @param destDir Destination directory for extracted contents
     * @param outError Output error message if extraction fails
     * @return true if extraction succeeded, false otherwise
     */
    virtual bool unzip(
        const std::filesystem::path& zipFile,
        const std::filesystem::path& destDir,
        std::string& outError) const = 0;

    /**
     * Get the total uncompressed size of all files in a ZIP archive.
     *
     * Reads the size from ZIP metadata without extracting files.
     * Used for pre-extraction size validation per TS 103 606 Section 6.1.8.
     *
     * @param zipFile Path to the ZIP archive file
     * @param outSize Output total uncompressed size in bytes
     * @param outError Output error message if operation fails
     * @return true if size was retrieved successfully, false otherwise
     */
    virtual bool getTotalUncompressedSize(
        const std::filesystem::path& zipFile,
        size_t& outSize,
        std::string& outError) const = 0;

    /**
     * Read a single file from a ZIP archive without full extraction.
     *
     * Used to read files like opapp.aitx for verification without
     * extracting the entire package.
     *
     * @param zipFile Path to the ZIP archive file
     * @param filePathInZip Path of the file within the ZIP (e.g., "opapp.aitx")
     * @param outContent Output buffer for the file contents
     * @param outError Output error message if operation fails
     * @return true if file was read successfully, false otherwise
     */
    virtual bool readFileFromZip(
        const std::filesystem::path& zipFile,
        const std::string& filePathInZip,
        std::vector<uint8_t>& outContent,
        std::string& outError) const = 0;
};

} // namespace orb

#endif /* I_UNZIPPER_H */

