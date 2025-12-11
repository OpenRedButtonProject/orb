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
#include "HttpDownloader.h"
#include "log.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <algorithm>
#include <random>

namespace orb
{

// DNS constants
namespace {
    constexpr uint16_t DNS_PORT = 53;
    constexpr uint16_t DNS_TYPE_SRV = 33;
    constexpr uint16_t DNS_CLASS_IN = 1;
    constexpr size_t DNS_HEADER_SIZE = 12;
    constexpr size_t DNS_MAX_RESPONSE_SIZE = 512;
    constexpr uint8_t DNS_COMPRESSION_MASK = 0xC0;
}

OpAppAcquisition::OpAppAcquisition(const std::string &opapp_fqdn, bool is_network_available)
    : m_opapp_fqdn(opapp_fqdn)
    , m_is_network_available(is_network_available)
{
}

OpAppAcquisition::~OpAppAcquisition()
{
}

bool OpAppAcquisition::validateFqdn(const std::string &fqdn)
{
    if (fqdn.empty()) {
        return false;
    }
    if (fqdn.find(".") == std::string::npos) {
        return false;
    }
    return true;
}

std::string OpAppAcquisition::retrieveOpAppAitXml()
{
    /* TS 103 606 V1.2.1 (2024-03) Section 6.1.5.1 XML AIT Acquisition
     * "If the terminal discovers the location of an XML AIT using DNS SRV as
     * defined in clause 6.1.4, the terminal shall perform a HTTP GET request
     * based on the priority and weighting of the returned SRV records..."
     */
    if (!m_is_network_available) {
        LOG(ERROR) << "Network is not available";
        return "";
    }

    auto records = doDnsSrvLookup();

    // Create HTTP downloader with appropriate Accept header for AIT
    HttpDownloader downloader;
    downloader.SetAcceptHeader("application/vnd.dvb.ait+xml, application/xml, text/xml");

    while (!records.empty()) {
        const auto nextSrvRecord = popNextSrvRecord(records);
        if (nextSrvRecord.target.empty()) {
            LOG(ERROR) << "Failed to select SRV record";
            continue;
        }

        // Perform HTTP GET request to the next SRV record
        // TS 103 606 specifies the path should be /.well-known/hbbtv for XML AIT
        LOG(INFO) << "Attempting to retrieve AIT from: " << nextSrvRecord.target
                  << ":" << nextSrvRecord.port;

        auto result = downloader.Download(nextSrvRecord.target, nextSrvRecord.port,
                                          "/.well-known/hbbtv");

        if (result && result->IsSuccess()) {
            // Optionally validate content type
            std::string contentType = result->GetContentType();
            if (contentType.find("xml") != std::string::npos ||
                contentType.find("application/vnd.dvb.ait") != std::string::npos) {
                LOG(INFO) << "Successfully retrieved AIT XML";
                return result->GetContent();
            }
            LOG(WARNING) << "Unexpected content type: " << contentType;
            // Still return content as it may be valid
            return result->GetContent();
        }

        LOG(WARNING) << "Failed to retrieve AIT from " << nextSrvRecord.target
                     << ", trying next SRV record...";
    }

    LOG(ERROR) << "Failed to retrieve AIT from any SRV record";
    return "";
}

std::vector<uint8_t> OpAppAcquisition::buildDnsQuery(const std::string& name, uint16_t transactionId)
{
    std::vector<uint8_t> query;

    // DNS Header (12 bytes)
    // Transaction ID (2 bytes)
    query.push_back((transactionId >> 8) & 0xFF);
    query.push_back(transactionId & 0xFF);

    // Flags: Standard query, recursion desired (0x0100)
    query.push_back(0x01);
    query.push_back(0x00);

    // QDCOUNT: 1 question
    query.push_back(0x00);
    query.push_back(0x01);

    // ANCOUNT: 0 answers
    query.push_back(0x00);
    query.push_back(0x00);

    // NSCOUNT: 0 authority records
    query.push_back(0x00);
    query.push_back(0x00);

    // ARCOUNT: 0 additional records
    query.push_back(0x00);
    query.push_back(0x00);

    // Question section - encode domain name
    size_t pos = 0;
    while (pos < name.length()) {
        size_t dotPos = name.find('.', pos);
        if (dotPos == std::string::npos) {
            dotPos = name.length();
        }

        size_t labelLen = dotPos - pos;
        if (labelLen > 63) {
            LOG(ERROR) << "DNS label too long: " << labelLen;
            return {};
        }

        query.push_back(static_cast<uint8_t>(labelLen));
        for (size_t i = pos; i < dotPos; i++) {
            query.push_back(static_cast<uint8_t>(name[i]));
        }
        pos = dotPos + 1;
    }

    // Null terminator for domain name
    query.push_back(0x00);

    // QTYPE: SRV (33)
    query.push_back((DNS_TYPE_SRV >> 8) & 0xFF);
    query.push_back(DNS_TYPE_SRV & 0xFF);

    // QCLASS: IN (1)
    query.push_back((DNS_CLASS_IN >> 8) & 0xFF);
    query.push_back(DNS_CLASS_IN & 0xFF);

    return query;
}

std::string OpAppAcquisition::parseDomainName(const uint8_t* response, size_t responseLen,
                                               size_t& offset)
{
    std::string name;
    bool jumped = false;
    size_t originalOffset = offset;
    int maxJumps = 10; // Prevent infinite loops from malformed packets

    while (offset < responseLen && maxJumps > 0) {
        uint8_t len = response[offset];

        if (len == 0) {
            // End of name
            if (!jumped) {
                offset++;
            }
            break;
        }

        if ((len & DNS_COMPRESSION_MASK) == DNS_COMPRESSION_MASK) {
            // Compression pointer
            if (offset + 1 >= responseLen) {
                LOG(ERROR) << "DNS compression pointer truncated";
                return "";
            }

            uint16_t pointer = ((len & 0x3F) << 8) | response[offset + 1];
            if (!jumped) {
                originalOffset = offset + 2;
            }
            offset = pointer;
            jumped = true;
            maxJumps--;
            continue;
        }

        // Regular label
        offset++;
        if (offset + len > responseLen) {
            LOG(ERROR) << "DNS label extends beyond response";
            return "";
        }

        if (!name.empty()) {
            name += ".";
        }
        name.append(reinterpret_cast<const char*>(response + offset), len);
        offset += len;
    }

    if (jumped) {
        offset = originalOffset;
    }

    return name;
}

std::vector<SrvRecord> OpAppAcquisition::parseDnsResponse(const uint8_t* response, size_t length)
{
    std::vector<SrvRecord> records;

    if (length < DNS_HEADER_SIZE) {
        LOG(ERROR) << "DNS response too short: " << length;
        return records;
    }

    // Check response flags
    uint16_t flags = (response[2] << 8) | response[3];
    uint8_t rcode = flags & 0x0F;
    if (rcode != 0) {
        LOG(ERROR) << "DNS query failed with rcode: " << static_cast<int>(rcode);
        return records;
    }

    // Get counts from header
    uint16_t qdcount = (response[4] << 8) | response[5];
    uint16_t ancount = (response[6] << 8) | response[7];

    if (ancount == 0) {
        LOG(INFO) << "No SRV records found";
        return records;
    }

    // Skip header
    size_t offset = DNS_HEADER_SIZE;

    // Skip question section
    for (uint16_t i = 0; i < qdcount && offset < length; i++) {
        // Skip QNAME
        while (offset < length && response[offset] != 0) {
            if ((response[offset] & DNS_COMPRESSION_MASK) == DNS_COMPRESSION_MASK) {
                offset += 2;
                break;
            }
            offset += response[offset] + 1;
        }
        if (offset < length && response[offset] == 0) {
            offset++; // Skip null terminator
        }
        offset += 4; // Skip QTYPE and QCLASS
    }

    // Parse answer section
    for (uint16_t i = 0; i < ancount && offset < length; i++) {
        // Skip NAME
        parseDomainName(response, length, offset);

        if (offset + 10 > length) {
            LOG(ERROR) << "Answer record truncated";
            break;
        }

        uint16_t type = (response[offset] << 8) | response[offset + 1];
        offset += 2;

        uint16_t cls = (response[offset] << 8) | response[offset + 1];
        offset += 2;

        // TTL (4 bytes) - skip
        offset += 4;

        uint16_t rdlength = (response[offset] << 8) | response[offset + 1];
        offset += 2;

        if (offset + rdlength > length) {
            LOG(ERROR) << "RDATA extends beyond response";
            break;
        }

        if (type == DNS_TYPE_SRV && cls == DNS_CLASS_IN && rdlength >= 6) {
            SrvRecord record;
            record.priority = (response[offset] << 8) | response[offset + 1];
            record.weight = (response[offset + 2] << 8) | response[offset + 3];
            record.port = (response[offset + 4] << 8) | response[offset + 5];

            size_t targetOffset = offset + 6;
            record.target = parseDomainName(response, length, targetOffset);

            if (!record.target.empty()) {
                records.push_back(record);
                LOG(INFO) << "Found SRV record: priority=" << record.priority
                          << " weight=" << record.weight
                          << " port=" << record.port
                          << " target=" << record.target;
            }
        }

        offset += rdlength;
    }

    return records;
}

std::vector<SrvRecord> OpAppAcquisition::querySrvRecords(
    const std::string& serviceName,
    const std::string& dnsServer,
    int timeoutMs)
{
    std::vector<SrvRecord> records;

    // Create UDP socket
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        LOG(ERROR) << "Failed to create UDP socket: " << strerror(errno);
        return records;
    }

    // Set socket timeout
    struct timeval tv;
    tv.tv_sec = timeoutMs / 1000;
    tv.tv_usec = (timeoutMs % 1000) * 1000;
    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        LOG(ERROR) << "Failed to set socket timeout: " << strerror(errno);
        close(sock);
        return records;
    }

    // Setup DNS server address
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(DNS_PORT);
    if (inet_pton(AF_INET, dnsServer.c_str(), &serverAddr.sin_addr) != 1) {
        LOG(ERROR) << "Invalid DNS server address: " << dnsServer;
        close(sock);
        return records;
    }

    // Generate random transaction ID
    std::random_device rd;
    uint16_t transactionId = static_cast<uint16_t>(rd());

    // Build DNS query
    std::vector<uint8_t> query = buildDnsQuery(serviceName, transactionId);
    if (query.empty()) {
        close(sock);
        return records;
    }

    // Send query
    ssize_t sent = sendto(sock, query.data(), query.size(), 0,
                          reinterpret_cast<struct sockaddr*>(&serverAddr),
                          sizeof(serverAddr));
    if (sent < 0) {
        LOG(ERROR) << "Failed to send DNS query: " << strerror(errno);
        close(sock);
        return records;
    }

    // Receive response
    uint8_t response[DNS_MAX_RESPONSE_SIZE];
    struct sockaddr_in fromAddr;
    socklen_t fromLen = sizeof(fromAddr);

    ssize_t received = recvfrom(sock, response, sizeof(response), 0,
                                 reinterpret_cast<struct sockaddr*>(&fromAddr),
                                 &fromLen);
    close(sock);

    if (received < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            LOG(ERROR) << "DNS query timed out";
        } else {
            LOG(ERROR) << "Failed to receive DNS response: " << strerror(errno);
        }
        return records;
    }

    // Verify transaction ID
    if (received >= 2) {
        uint16_t responseId = (response[0] << 8) | response[1];
        if (responseId != transactionId) {
            LOG(ERROR) << "DNS transaction ID mismatch";
            return records;
        }
    }

    // Parse response
    records = parseDnsResponse(response, static_cast<size_t>(received));

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

    // Weighted random selection among candidates
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

std::vector<SrvRecord> OpAppAcquisition::doDnsSrvLookup()
{
    if (!m_is_network_available) {
        LOG(ERROR) << "Network is not available";
        return std::vector<SrvRecord>();
    }

    if (!validateFqdn(m_opapp_fqdn)) {
        LOG(ERROR) << "Invalid FQDN: " << m_opapp_fqdn;
        return std::vector<SrvRecord>();
    }

    // Section 6.1.4 of TS 103 606 V1.2.1 (2024-03)
    const std::string serviceName = "_hbbtv-ait._tcp." + m_opapp_fqdn;

    LOG(INFO) << "Performing DNS SRV lookup for: " << serviceName;

    std::vector<SrvRecord> records = querySrvRecords(serviceName);
    if (records.empty()) {
        LOG(ERROR) << "No SRV records found for: " << serviceName;
        return std::vector<SrvRecord>();
    }

    return records;
}

} // namespace orb
