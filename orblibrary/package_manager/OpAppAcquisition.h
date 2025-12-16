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
 * OpAppAcquisition - Internal implementation for AIT XML acquisition
 * This is an implementation detail of OpAppPackageManager and should not
 * be used directly by external code.
 */

#ifndef OP_APP_ACQUISITION_H
#define OP_APP_ACQUISITION_H

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
 * @brief Result of an AIT acquisition attempt.
 */
struct AcquisitionResult {
    bool success;
    std::string content;
    std::string errorMessage;
    int statusCode;  // HTTP status code if available, -1 otherwise

    AcquisitionResult()
        : success(false), statusCode(-1) {}

    AcquisitionResult(bool s, const std::string& c, const std::string& err = "", int code = -1)
        : success(s), content(c), errorMessage(err), statusCode(code) {}

    static AcquisitionResult Success(const std::string& content, int statusCode = 200) {
        return AcquisitionResult(true, content, "", statusCode);
    }

    static AcquisitionResult Failure(const std::string& errorMessage) {
        return AcquisitionResult(false, "", errorMessage, -1);
    }
};

/**
 * @brief Interface for AIT acquisition - allows mocking in tests.
 */
class IOpAppAcquisition {
public:
    virtual ~IOpAppAcquisition() = default;

    /**
     * @brief Fetch the AIT XML for a given FQDN.
     *
     * @param fqdn The fully qualified domain name of the OpApp
     * @param networkAvailable Whether network is currently available
     * @return AcquisitionResult containing success status and content/error
     */
    virtual AcquisitionResult FetchAitXml(const std::string& fqdn, bool networkAvailable) = 0;
};

/**
 * @brief Default implementation of AIT acquisition using DNS SRV lookup and HTTPS.
 *
 * Implements the OpApp discovery process defined in TS 103 606 V1.2.1 (2024-03):
 * - Section 6.1.4: DNS SRV lookup for _hbbtv-ait._tcp.<fqdn>
 * - Section 6.1.5.1: XML AIT Acquisition via HTTPS
 */
class OpAppAcquisition : public IOpAppAcquisition {
public:
    /**
     * @brief Constructor.
     * @param userAgent HTTP User-Agent header value (TS 103 606 Section 6.1.5.1)
     */
    explicit OpAppAcquisition(const std::string& userAgent = "");
    ~OpAppAcquisition() override;

    // Prevent copying
    OpAppAcquisition(const OpAppAcquisition&) = delete;
    OpAppAcquisition& operator=(const OpAppAcquisition&) = delete;

    /**
     * @brief Fetch the AIT XML for a given FQDN.
     *
     * Performs DNS SRV lookup followed by HTTPS GET to retrieve the AIT XML.
     * Tries multiple SRV records in priority/weight order until success.
     *
     * @param fqdn The fully qualified domain name of the OpApp
     * @param networkAvailable Whether network is currently available
     * @return AcquisitionResult containing success status and content/error
     */
    AcquisitionResult FetchAitXml(const std::string& fqdn, bool networkAvailable) override;

    /**
     * @brief Static convenience method for one-shot AIT fetching.
     *
     * Creates a temporary OpAppAcquisition instance and fetches AIT XML.
     *
     * @param fqdn The fully qualified domain name of the OpApp
     * @param networkAvailable Whether network is currently available
     * @param userAgent HTTP User-Agent header value
     * @return AcquisitionResult containing success status and content/error
     */
    static AcquisitionResult Fetch(const std::string& fqdn, bool networkAvailable,
                                   const std::string& userAgent = "");

    // Friend class for testing private methods
    friend class OpAppAcquisitionTestInterface;

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

    std::unique_ptr<HttpDownloader> m_downloader;
};

} // namespace orb

#endif // OP_APP_ACQUISITION_H

