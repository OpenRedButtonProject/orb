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
#ifndef OP_APP_ACQUISITION_H
#define OP_APP_ACQUISITION_H

#include <string>
#include <vector>
#include <cstdint>

namespace orb
{

/**
 * @brief Represents a DNS SRV record
 */
struct SrvRecord {
    uint16_t priority;
    uint16_t weight;
    uint16_t port;
    std::string target;

    SrvRecord() : priority(0), weight(0), port(0) {}
    SrvRecord(uint16_t p, uint16_t w, uint16_t pt, const std::string& t)
        : priority(p), weight(w), port(pt), target(t) {}
};

class OpAppAcquisition {
public:
    OpAppAcquisition(const std::string &opapp_fqdn, bool is_network_available);
    ~OpAppAcquisition();

    friend class OpAppAcquisitionTestInterface;

    // Uses doDnsSrvLookup() to retrieve the AIT service URL and then retrieves the AIT XML file from the URL.
    //
    // @return The AIT XML file contents, or empty string on failure
    std::string retrieveOpAppAitXml();

private:
    /**
     * Perform a DNS SRV lookup for the OpApp as defined in TS 103 606 V1.2.1 (2024-03)
     * Section 6.1.4 and returns the SRV records for the AIT service.
     *
     * @return Vector of SRV records, empty on failure
     */
    std::vector<SrvRecord> doDnsSrvLookup();

    /**
     * Query DNS SRV records for a given service name.
     *
     * @param serviceName The full SRV service name (e.g., "_hbbtv-ait._tcp.example.com")
     * @param dnsServer The DNS server IP address (default: "8.8.8.8")
     * @param timeoutMs Timeout in milliseconds (default: 5000)
     * @return Vector of SRV records, empty on failure
     */
    std::vector<SrvRecord> querySrvRecords(
        const std::string& serviceName,
        const std::string& dnsServer = "8.8.8.8",
        int timeoutMs = 5000);

    /**
     * Pops the next SRV record from a list of SRV records based on priority and weight.
     * Removes the returned SRV record from the input records vector.
     *
     * @param records The SRV records to get the next from
     * @return The next SRV record, or empty record if none available
     */
    SrvRecord popNextSrvRecord(std::vector<SrvRecord>& records);

    /**
     * Select the best SRV record based on priority and weight.
     * Lower priority values are preferred. Among equal priorities,
     * records are selected based on weight.
     *
     * @param records The SRV records to select from
     * @return The selected SRV record, or empty record if none available
     */
    SrvRecord selectBestSrvRecord(const std::vector<SrvRecord>& records);

    /* Minimum validation of the FQDN - not empty and contains a dot */
    bool validateFqdn(const std::string &fqdn);

    /* Build a DNS query packet for SRV record lookup */
    std::vector<uint8_t> buildDnsQuery(const std::string& name, uint16_t transactionId);

    /* Parse DNS response and extract SRV records */
    std::vector<SrvRecord> parseDnsResponse(const uint8_t* response, size_t length);

    /* Parse a domain name from DNS wire format */
    std::string parseDomainName(const uint8_t* response, size_t responseLen,
                                 size_t& offset);

    /* Perform an HTTP GET request to the given URL and port */
    std::string performHttpGet(const std::string& url, uint16_t port);

    std::string m_opapp_fqdn;
    bool m_is_network_available;
};

} // namespace orb

#endif // OP_APP_ACQUISITION_H
