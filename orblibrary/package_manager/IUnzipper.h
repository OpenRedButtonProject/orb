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

namespace orb
{

/**
 * @brief Interface for ZIP archive extraction.
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
};

} // namespace orb

#endif /* I_UNZIPPER_H */

