#ifndef ORB_DNS_SRV_RESOLVER_H
#define ORB_DNS_SRV_RESOLVER_H

#include "SrvRecord.h"
#include <vector>
#include <string>
#include <cstdint>

namespace orb {

/**
 * @brief DNS SRV record resolver using raw UDP sockets.
 *
 * Performs DNS SRV lookups (RFC 2782) without external dependencies.
 */
class DnsSrvResolver {
public:
    /**
     * @brief Constructor.
     * @param dnsServer DNS server IP address (default: "8.8.8.8")
     * @param timeoutMs Query timeout in milliseconds (default: 5000)
     */
    explicit DnsSrvResolver(const std::string& dnsServer = "8.8.8.8",
                            int timeoutMs = 5000);
    ~DnsSrvResolver() = default;

    // Prevent copying
    DnsSrvResolver(const DnsSrvResolver&) = delete;
    DnsSrvResolver& operator=(const DnsSrvResolver&) = delete;

    /**
     * @brief Query SRV records for a service name.
     *
     * @param serviceName Full SRV service name (e.g., "_hbbtv-ait._tcp.example.com")
     * @return Vector of SRV records, empty on failure
     */
    std::vector<SrvRecord> Query(const std::string& serviceName);

    /**
     * @brief Set the DNS server address.
     * @param dnsServer DNS server IP address
     */
    void SetDnsServer(const std::string& dnsServer) { m_dnsServer = dnsServer; }

    /**
     * @brief Set the query timeout.
     * @param timeoutMs Timeout in milliseconds
     */
    void SetTimeout(int timeoutMs) { m_timeoutMs = timeoutMs; }

    // Test interface access
    friend class DnsSrvResolverTestInterface;

private:
    /**
     * @brief Build a DNS query packet for SRV record lookup.
     * @param name Domain name to query
     * @param transactionId Transaction ID for the query
     * @return Query packet bytes, empty on error
     */
    std::vector<uint8_t> BuildDnsQuery(const std::string& name, uint16_t transactionId);

    /**
     * @brief Parse a DNS response and extract SRV records.
     * @param response Response packet bytes
     * @param length Response length
     * @return Vector of SRV records
     */
    std::vector<SrvRecord> ParseDnsResponse(const uint8_t* response, size_t length);

    /**
     * @brief Parse a domain name from DNS wire format.
     * @param response Response packet bytes
     * @param responseLen Response length
     * @param offset Current offset (updated on return)
     * @return Parsed domain name
     */
    std::string ParseDomainName(const uint8_t* response, size_t responseLen,
                                size_t& offset);

    std::string m_dnsServer;
    int m_timeoutMs;
};

} // namespace orb

#endif // ORB_DNS_SRV_RESOLVER_H

