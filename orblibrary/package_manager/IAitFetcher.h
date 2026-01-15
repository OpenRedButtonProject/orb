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
 * AIT Fetcher Interface
 */
#ifndef I_AIT_FETCHER_H
#define I_AIT_FETCHER_H

#include <string>
#include <vector>

namespace orb
{

/**
 * @brief Result of an AIT fetch attempt.
 *
 * AITs are written to files to avoid heap pressure with large/many files.
 * Per TS 103 606: "The result of the process is a number of (XML) AITs..."
 */
struct AitFetchResult {
    bool success;                        // True if at least one AIT was acquired
    std::vector<std::string> aitFiles;   // Paths to acquired AIT XML files
    std::vector<std::string> errors;     // Non-fatal errors encountered
    std::string fatalError;              // Fatal error (empty if success)

    // Default constructor - failure state
    AitFetchResult()
        : success(false) {}

    // Failure constructor - with fatal error message
    explicit AitFetchResult(const std::string& fatalError)
        : success(false), fatalError(fatalError) {}

    // Success constructor - with acquired files and any non-fatal errors
    AitFetchResult(const std::vector<std::string>& aitFiles,
                   const std::vector<std::string>& errors)
        : success(!aitFiles.empty()), aitFiles(aitFiles), errors(errors) {}
};

/**
 * @brief Interface for AIT fetching - allows mocking in tests.
 */
class IAitFetcher {
public:
    virtual ~IAitFetcher() = default;

    /**
     * @brief Fetch ALL AIT XMLs for a given FQDN, writing each to a file.
     *
     * Iterates through all SRV records and downloads AIT from each reachable target.
     * AITs are written to individual files in the specified output directory.
     *
     * @param fqdn The fully qualified domain name of the OpApp
     * @param networkAvailable Whether network is currently available
     * @param outputDirectory Directory where AIT files will be written
     * @return AitFetchResult containing file paths and status
     */
    virtual AitFetchResult FetchAitXmls(
        const std::string& fqdn,
        bool networkAvailable,
        const std::string& outputDirectory) = 0;
};

} // namespace orb

#endif // I_AIT_FETCHER_H

