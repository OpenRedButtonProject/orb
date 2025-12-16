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

#include "OpAppAcquisition.h"
#include "DnsSrvResolver.h"
#include "HttpDownloader.h"
#include "SrvRecord.h"
#include "log.h"

#include <algorithm>
#include <random>

namespace orb
{
const unsigned int DEFAULT_TIMEOUT_MS = 10000;

OpAppAcquisition::OpAppAcquisition(const std::string& userAgent)
    : m_downloader(std::make_unique<HttpDownloader>(DEFAULT_TIMEOUT_MS, userAgent))
{
    // Set appropriate Accept header for AIT
    m_downloader->SetAcceptHeader("application/vnd.dvb.ait+xml, application/xml, text/xml");
}

OpAppAcquisition::~OpAppAcquisition() = default;

AcquisitionResult OpAppAcquisition::Fetch(const std::string& fqdn, bool networkAvailable,
                                          const std::string& userAgent)
{
    OpAppAcquisition acquisition(userAgent);
    return acquisition.FetchAitXml(fqdn, networkAvailable);
}

AcquisitionResult OpAppAcquisition::FetchAitXml(const std::string& fqdn, bool networkAvailable)
{
    /* TS 103 606 V1.2.1 (2024-03) Section 6.1.5.1 XML AIT Acquisition
     * "If the terminal discovers the location of an XML AIT using DNS SRV as
     * defined in clause 6.1.4, the terminal shall perform a HTTP GET request
     * based on the priority and weighting of the returned SRV records..."
     */
    if (!networkAvailable) {
        LOG(ERROR) << "Network is not available";
        return AcquisitionResult::Failure("Network is not available");
    }

    if (!validateFqdn(fqdn)) {
        LOG(ERROR) << "Invalid FQDN: " << fqdn;
        return AcquisitionResult::Failure("Invalid FQDN: " + fqdn);
    }

    auto records = doDnsSrvLookup(fqdn);
    if (records.empty()) {
        return AcquisitionResult::Failure("No SRV records found for FQDN: " + fqdn);
    }

    while (!records.empty()) {
        const auto nextSrvRecord = popNextSrvRecord(records);
        if (nextSrvRecord.target.empty()) {
            LOG(ERROR) << "Failed to select SRV record";
            continue;
        }

        // Perform HTTPS GET request to the next SRV record
        // TS 103 606 specifies the path for XML AIT
        LOG(INFO) << "Attempting to retrieve AIT from: " << nextSrvRecord.target
                  << ":" << nextSrvRecord.port;

        auto downloadedObject = m_downloader->Download(
            nextSrvRecord.target, nextSrvRecord.port,
            "/opapp.aitx", true /* use HTTPS */);

        if (downloadedObject && downloadedObject->IsSuccess()) {
            // Optionally validate content type
            std::string contentType = downloadedObject->GetContentType();
            if (contentType.find("xml") != std::string::npos ||
                contentType.find("application/vnd.dvb.ait") != std::string::npos) {
                LOG(INFO) << "Successfully retrieved AIT XML";
            } else {
                LOG(WARNING) << "Unexpected content type: " << contentType;
            }
            // Return success - content may be valid even with unexpected content type
            return AcquisitionResult::Success(
                downloadedObject->GetContent(),
                downloadedObject->GetStatusCode());
        }

        LOG(WARNING) << "Failed to retrieve AIT from " << nextSrvRecord.target
                     << ", trying next SRV record...";
    }

    LOG(ERROR) << "Failed to retrieve AIT from any SRV record";
    return AcquisitionResult::Failure("Failed to retrieve AIT from any SRV record");
}

bool OpAppAcquisition::validateFqdn(const std::string& fqdn)
{
    if (fqdn.empty()) {
        return false;
    }
    if (fqdn.find(".") == std::string::npos) {
        return false;
    }
    return true;
}

std::vector<SrvRecord> OpAppAcquisition::doDnsSrvLookup(const std::string& fqdn)
{
    // Section 6.1.4 of TS 103 606 V1.2.1 (2024-03)
    const std::string serviceName = "_hbbtv-ait._tcp." + fqdn;

    LOG(INFO) << "Performing DNS SRV lookup for: " << serviceName;

    DnsSrvResolver resolver;
    std::vector<SrvRecord> records = resolver.Query(serviceName);

    if (records.empty()) {
        LOG(ERROR) << "No SRV records found for: " << serviceName;
    }

    return records;
}

SrvRecord OpAppAcquisition::popNextSrvRecord(std::vector<SrvRecord>& records)
{
    if (records.empty()) {
        return SrvRecord();
    }
    const SrvRecord record = selectBestSrvRecord(records);
    // Remove the selected record from the input records vector
    records.erase(std::find_if(records.begin(), records.end(),
                                [&record](const SrvRecord& r) {
                                    return r.target == record.target && r.port == record.port;
                                }));
    return record;
}

SrvRecord OpAppAcquisition::selectBestSrvRecord(const std::vector<SrvRecord>& records)
{
    if (records.empty()) {
        return SrvRecord();
    }

    // Sort by priority (lower is better)
    std::vector<SrvRecord> sorted = records;
    std::sort(sorted.begin(), sorted.end(),
              [](const SrvRecord& a, const SrvRecord& b) {
                  return a.priority < b.priority;
              });

    // Get all records with the best (lowest) priority
    uint16_t bestPriority = sorted[0].priority;
    std::vector<SrvRecord> candidates;
    for (const auto& record : sorted) {
        if (record.priority == bestPriority) {
            candidates.push_back(record);
        }
    }

    // If only one candidate, return it
    if (candidates.size() == 1) {
        return candidates[0];
    }

    // Weighted random selection among candidates (RFC 2782)
    uint32_t totalWeight = 0;
    for (const auto& record : candidates) {
        totalWeight += record.weight;
    }

    if (totalWeight == 0) {
        // All weights are 0, select randomly
        std::random_device rd;
        std::uniform_int_distribution<size_t> dist(0, candidates.size() - 1);
        return candidates[dist(rd)];
    }

    std::random_device rd;
    std::uniform_int_distribution<uint32_t> dist(0, totalWeight - 1);
    uint32_t randomValue = dist(rd);

    uint32_t cumulativeWeight = 0;
    for (const auto& record : candidates) {
        cumulativeWeight += record.weight;
        if (randomValue < cumulativeWeight) {
            return record;
        }
    }

    // Fallback (shouldn't reach here)
    return candidates[0];
}

} // namespace orb
