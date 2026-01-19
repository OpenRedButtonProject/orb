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
 * ZIP Archive Extractor
 *
 * Implements ZIP extraction for operator application packages as per:
 * - ETSI TS 103 606 Section 11.3.4 (Application package structure)
 */
#ifndef UNZIPPER_H
#define UNZIPPER_H

#include "IUnzipper.h"
#include <filesystem>
#include <string>
#include <vector>
#include <cstdint>

namespace orb
{

/**
 * @brief ZIP Archive Extractor and Inspector
 *
 * Extracts and inspects ZIP archives containing operator application packages.
 * Supports reading ZIP metadata and individual files without full extraction,
 * enabling pre-extraction validation per TS 103 606 Section 6.1.8.
 *
 * Implementation notes:
 * - For Chromium builds (IS_CHROMIUM defined): Uses Chromium's zip library
 *   from third_party/zlib/google/zip.h and zip_reader.h
 * - For non-Chromium builds: Not yet implemented (placeholder)
 */
class Unzipper : public IUnzipper {
public:
    /**
     * @brief Construct an Unzipper.
     */
    Unzipper() = default;

    /**
     * @brief Extract a ZIP archive to a destination directory.
     *
     * @param zipFile Path to the ZIP archive file
     * @param destDir Destination directory for extracted contents
     * @param outError Error message if extraction fails
     * @return true on success, false on failure
     */
    bool unzip(
        const std::filesystem::path& zipFile,
        const std::filesystem::path& destDir,
        std::string& outError) const override;

    /**
     * @brief Get the total uncompressed size of all files in a ZIP archive.
     *
     * @param zipFile Path to the ZIP archive file
     * @param outSize Output total uncompressed size in bytes
     * @param outError Error message if operation fails
     * @return true on success, false on failure
     */
    bool getTotalUncompressedSize(
        const std::filesystem::path& zipFile,
        size_t& outSize,
        std::string& outError) const override;

    /**
     * @brief Read a single file from a ZIP archive without full extraction.
     *
     * @param zipFile Path to the ZIP archive file
     * @param filePathInZip Path of the file within the ZIP
     * @param outContent Output buffer for the file contents
     * @param outError Error message if operation fails
     * @return true on success, false on failure
     */
    bool readFileFromZip(
        const std::filesystem::path& zipFile,
        const std::string& filePathInZip,
        std::vector<uint8_t>& outContent,
        std::string& outError) const override;
};

} // namespace orb

#endif /* UNZIPPER_H */

