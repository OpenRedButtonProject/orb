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
 * ZIP Archive Extractor Implementation
 */

#include "Unzipper.h"
#include <system_error>

#ifdef IS_CHROMIUM
// Chromium's zip library
#include "third_party/zlib/google/zip.h"
#include "third_party/zlib/google/zip_reader.h"
#include "base/files/file_path.h"
#endif

namespace orb
{

bool Unzipper::unzip(
    const std::filesystem::path& zipFile,
    const std::filesystem::path& destDir,
    std::string& outError) const
{
#ifdef IS_CHROMIUM
    // Convert std::filesystem::path to base::FilePath for Chromium API
    base::FilePath zip_file_path(zipFile.string());
    base::FilePath dest_dir_path(destDir.string());

    // Ensure destination directory exists
    std::error_code ec;
    if (!std::filesystem::exists(destDir)) {
        if (!std::filesystem::create_directories(destDir, ec)) {
            outError = "Failed to create output directory: " + ec.message();
            return false;
        }
    }

    // Use Chromium's zip::Unzip function
    if (!zip::Unzip(zip_file_path, dest_dir_path)) {
        outError = "Failed to unzip file: " + zipFile.string();
        return false;
    }

    return true;
#else
    // Non-Chromium implementation placeholder
    // TODO: Implement using minizip or other zip library for standalone builds
    (void)zipFile;
    (void)destDir;
    outError = "Unzip not implemented for non-Chromium builds";
    return false;
#endif
}

bool Unzipper::getTotalUncompressedSize(
    const std::filesystem::path& zipFile,
    size_t& outSize,
    std::string& outError) const
{
#ifdef IS_CHROMIUM
    base::FilePath zip_file_path(zipFile.string());

    zip::ZipReader reader;
    if (!reader.Open(zip_file_path)) {
        outError = "Failed to open ZIP file: " + zipFile.string();
        return false;
    }

    size_t totalSize = 0;
    while (const zip::ZipReader::Entry* entry = reader.Next()) {
        // original_size is 0 for directories, so we can sum all entries
        totalSize += entry->original_size;
    }

    outSize = totalSize;
    return true;
#else
    // Non-Chromium implementation placeholder
    (void)zipFile;
    (void)outSize;
    outError = "getTotalUncompressedSize not implemented for non-Chromium builds";
    return false;
#endif
}

bool Unzipper::readFileFromZip(
    const std::filesystem::path& zipFile,
    const std::string& filePathInZip,
    std::vector<uint8_t>& outContent,
    std::string& outError) const
{
#ifdef IS_CHROMIUM
    base::FilePath zip_file_path(zipFile.string());

    zip::ZipReader reader;
    if (!reader.Open(zip_file_path)) {
        outError = "Failed to open ZIP file: " + zipFile.string();
        return false;
    }

    // Search for the requested file
    while (const zip::ZipReader::Entry* entry = reader.Next()) {
        if (entry->path.AsUTF8Unsafe() == filePathInZip) {
            // Found the file - extract its contents
            std::string content;
            if (!reader.ExtractCurrentEntryToString(&content)) {
                outError = "Failed to extract file from ZIP: " + filePathInZip;
                return false;
            }

            outContent.assign(content.begin(), content.end());
            return true;
        }
    }

    outError = "File not found in ZIP: " + filePathInZip;
    return false;
#else
    // Non-Chromium implementation placeholder
    (void)zipFile;
    (void)filePathInZip;
    (void)outContent;
    outError = "readFileFromZip not implemented for non-Chromium builds";
    return false;
#endif
}

} // namespace orb

