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
 * AitFetcher - Fetches AIT XML files via DNS SRV lookup and HTTPS
 * This is an implementation detail of OpAppPackageManager and should not
 * be used directly by external code.
 */

#ifndef AIT_FETCHER_H
#define AIT_FETCHER_H

#include <memory>
#include <string>
#include <vector>

namespace orb
{

// Forward declarations
struct SrvRecord;
class HttpDownloader;
class DownloadedObject;

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

/**
 * @brief Default implementation of AIT fetching using DNS SRV lookup and HTTPS.
 *
 * Implements the OpApp discovery process defined in TS 103 606 V1.2.1 (2024-03):
 * - Section 6.1.4: DNS SRV lookup for _hbbtv-ait._tcp.<fqdn>
 * - Section 6.1.5.1: XML AIT Acquisition via HTTPS
 */
class AitFetcher : public IAitFetcher {
public:
    /**
     * @brief Constructor.
     * @param userAgent HTTP User-Agent header value (TS 103 606 Section 6.1.5.1)
     */
    explicit AitFetcher(const std::string& userAgent = "");
    ~AitFetcher() override;

    // Prevent copying
    AitFetcher(const AitFetcher&) = delete;
    AitFetcher& operator=(const AitFetcher&) = delete;

    /**
     * @brief Fetch ALL AIT XMLs for a given FQDN, writing each to a file.
     *
     * Performs DNS SRV lookup followed by HTTPS GET to retrieve AIT XML from
     * ALL reachable SRV record targets. Each AIT is written to an individual
     * file in the specified output directory.
     *
     * @param fqdn The fully qualified domain name of the OpApp
     * @param networkAvailable Whether network is currently available
     * @param outputDirectory Directory where AIT files will be written
     * @return AitFetchResult containing file paths and status
     */
    AitFetchResult FetchAitXmls(
        const std::string& fqdn,
        bool networkAvailable,
        const std::string& outputDirectory) override;

    /**
     * @brief Static convenience method for fetching AITs.
     *
     * Creates a temporary AitFetcher instance and fetches all AIT XMLs.
     *
     * @param fqdn The fully qualified domain name of the OpApp
     * @param networkAvailable Whether network is currently available
     * @param outputDirectory Directory where AIT files will be written
     * @param userAgent HTTP User-Agent header value
     * @return AitFetchResult containing file paths and status
     */
    static AitFetchResult Fetch(const std::string& fqdn, bool networkAvailable,
                                const std::string& outputDirectory,
                                const std::string& userAgent = "");

    // Friend class for testing private methods
    friend class AitFetcherTestInterface;

private:
    /**
     * @brief Perform a DNS SRV lookup for the OpApp.
     * @param fqdn The FQDN to query
     * @return Vector of SRV records, empty on failure
     */
    std::vector<SrvRecord> doDnsSrvLookup(const std::string& fqdn);

    /**
     * @brief Pops the next SRV record based on priority and weight.
     * @param records The SRV records (modified in place)
     * @return The next SRV record, or empty record if none available
     */
    SrvRecord popNextSrvRecord(std::vector<SrvRecord>& records);

    /**
     * @brief Select the best SRV record based on priority and weight (RFC 2782).
     * @param records The SRV records to select from
     * @return The selected SRV record
     */
    SrvRecord selectBestSrvRecord(const std::vector<SrvRecord>& records);

    /**
     * @brief Validate FQDN format (not empty, contains dot).
     * @param fqdn The FQDN to validate
     * @return true if valid
     */
    bool validateFqdn(const std::string& fqdn);

    /**
     * @brief Generate a unique filename for an AIT file.
     * @param index The index of the AIT file
     * @param target The target server hostname
     * @return A sanitized filename
     */
    std::string generateAitFilename(int index, const std::string& target);

    /**
     * @brief Write AIT content to file atomically.
     * @param content The AIT XML content
     * @param filePath The destination file path
     * @return true on success
     */
    bool writeAitToFile(const std::string& content, const std::string& filePath);

    std::unique_ptr<HttpDownloader> m_downloader;
};

} // namespace orb

#endif // AIT_FETCHER_H

