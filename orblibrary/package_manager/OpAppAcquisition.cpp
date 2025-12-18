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
#include <filesystem>
#include <fstream>
#include <random>
#include <regex>

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
                                          const std::string& outputDirectory,
                                          const std::string& userAgent)
{
    OpAppAcquisition acquisition(userAgent);
    return acquisition.FetchAitXmls(fqdn, networkAvailable, outputDirectory);
}

AcquisitionResult OpAppAcquisition::FetchAitXmls(const std::string& fqdn,
                                                           bool networkAvailable,
                                                           const std::string& outputDirectory)
{
    /* TS 103 606 V1.2.1 (2024-03) Section 6.1.5.1 XML AIT Acquisition
     * "The result of the process is a number of (XML) AITs..."
     * This method fetches AITs from ALL reachable SRV record targets.
     */
    if (!networkAvailable) {
        LOG(ERROR) << "Network is not available";
        return AcquisitionResult("Network is not available");
    }

    if (!validateFqdn(fqdn)) {
        LOG(ERROR) << "Invalid FQDN: " << fqdn;
        return AcquisitionResult("Invalid FQDN: " + fqdn);
    }

    if (outputDirectory.empty()) {
        LOG(ERROR) << "Output directory not specified";
        return AcquisitionResult("Output directory not specified");
    }

    // Ensure output directory exists
    std::error_code ec;
    if (!std::filesystem::exists(outputDirectory)) {
        if (!std::filesystem::create_directories(outputDirectory, ec)) {
            LOG(ERROR) << "Failed to create output directory: " << outputDirectory
                       << ", error: " << ec.message();
            return AcquisitionResult(
                "Failed to create output directory: " + outputDirectory);
        }
    }

    auto records = doDnsSrvLookup(fqdn);
    if (records.empty()) {
        return AcquisitionResult("No SRV records found for FQDN: " + fqdn);
    }

    std::vector<std::string> acquiredFiles;
    std::vector<std::string> errors;
    int fileIndex = 0;

    // Process ALL records, don't stop on first success
    while (!records.empty()) {
        const auto nextSrvRecord = popNextSrvRecord(records);
        if (nextSrvRecord.target.empty()) {
            LOG(ERROR) << "Failed to select SRV record";
            continue;
        }

        LOG(INFO) << "Attempting to retrieve AIT from: " << nextSrvRecord.target
                  << ":" << nextSrvRecord.port;

        auto downloadedObject = m_downloader->Download(
            nextSrvRecord.target, nextSrvRecord.port,
            "/opapp.aitx", true /* use HTTPS */);

        if (downloadedObject && downloadedObject->IsSuccess()) {
            // Validate content type - See TS 102796 Section 7.3.2.4
            std::string contentType = downloadedObject->GetContentType();
            if (contentType.find("xml") != std::string::npos ||
                contentType.find("application/vnd.dvb.ait") != std::string::npos) {
                LOG(INFO) << "Successfully retrieved AIT XML from " << nextSrvRecord.target;

                // Generate unique filename and write to disk
                std::string filename = generateAitFilename(fileIndex++, nextSrvRecord.target);
                std::string filePath = outputDirectory + "/" + filename;

                if (writeAitToFile(downloadedObject->GetContent(), filePath)) {
                    acquiredFiles.push_back(filePath);
                    LOG(INFO) << "AIT written to: " << filePath;
                } else {
                    std::string error = "Failed to write AIT from " + nextSrvRecord.target +
                                        " to " + filePath;
                    LOG(ERROR) << error;
                    errors.push_back(error);
                }
            } else {
                // Ignore unexpected content type
                LOG(WARNING) << "Unexpected content type from " << nextSrvRecord.target
                             << ": " << contentType;
            }

        } else {
            std::string error = "Failed to download AIT from " + nextSrvRecord.target +
                                ":" + std::to_string(nextSrvRecord.port);
            LOG(WARNING) << error << ", trying next SRV record...";
            errors.push_back(error);
        }
    }

    if (acquiredFiles.empty()) {
        LOG(ERROR) << "Failed to retrieve AIT from any SRV record";
        return AcquisitionResult("Failed to retrieve AIT from any SRV record");
    }

    LOG(INFO) << "Successfully acquired " << acquiredFiles.size() << " AIT file(s)";
    return AcquisitionResult(acquiredFiles, errors);
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

std::string OpAppAcquisition::generateAitFilename(int index, const std::string& target)
{
    // Sanitize target hostname for use in filename
    // Replace characters that are invalid in filenames with underscores
    std::string sanitized = target;
    std::regex invalidChars("[^a-zA-Z0-9._-]");
    sanitized = std::regex_replace(sanitized, invalidChars, "_");

    // Format: ait_<index>_<sanitized_target>.xml
    return "ait_" + std::to_string(index) + "_" + sanitized + ".xml";
}

bool OpAppAcquisition::writeAitToFile(const std::string& content, const std::string& filePath)
{
    // Write to a temporary file first, then rename for atomic operation
    std::string tempPath = filePath + ".tmp";
    std::error_code ec;

    std::ofstream outFile(tempPath, std::ios::binary | std::ios::trunc);
    if (!outFile.is_open()) {
        LOG(ERROR) << "Failed to open temp file for writing: " << tempPath;
        return false;
    }

    outFile.write(content.c_str(), content.size());
    outFile.close();

    if (outFile.fail()) {
        LOG(ERROR) << "Failed to write content to temp file: " << tempPath;
        std::filesystem::remove(tempPath, ec);
        return false;
    }

    // Atomic rename
    std::filesystem::rename(tempPath, filePath, ec);
    if (ec) {
        LOG(ERROR) << "Failed to rename temp file to " << filePath
                   << ": " << ec.message();
        std::filesystem::remove(tempPath, ec);
        return false;
    }

    return true;
}

} // namespace orb
