#ifndef OP_APP_ACQUISITION_TEST_INTERFACE_H
#define OP_APP_ACQUISITION_TEST_INTERFACE_H

#include "OpAppAcquisition.h"
#include <memory>
#include <string>
#include <vector>
#include <cstdint>

/**
 * @brief Test interface for OpAppAcquisition that provides controlled access
 * to internal functionality for testing purposes while maintaining encapsulation.
 *
 * This interface should only be used in test code.
 */
namespace orb
{

class OpAppAcquisitionTestInterface
{
public:
    /**
     * @brief Creates a test interface for OpAppAcquisition
     * @param opapp_fqdn The fully qualified domain name of the OpApp
     * @param is_network_available Whether network is available
     * @return A test interface instance
     */
    static std::unique_ptr<OpAppAcquisitionTestInterface> create(
        const std::string& opapp_fqdn,
        bool is_network_available);

    /**
     * @brief Destructor
     */
    ~OpAppAcquisitionTestInterface();

    // Prevent copying and moving
    OpAppAcquisitionTestInterface(const OpAppAcquisitionTestInterface&) = delete;
    OpAppAcquisitionTestInterface& operator=(const OpAppAcquisitionTestInterface&) = delete;
    OpAppAcquisitionTestInterface(OpAppAcquisitionTestInterface&&) = delete;
    OpAppAcquisitionTestInterface& operator=(OpAppAcquisitionTestInterface&&) = delete;

    /**
     * @brief Validates an FQDN string
     * @param fqdn The FQDN to validate
     * @return true if valid, false otherwise
     */
    bool validateFqdn(const std::string& fqdn);

    /**
     * @brief Performs DNS SRV lookup
     * @return The result of the DNS lookup (target:port) or empty string
     */
    std::string doDnsSrvLookup();

    /**
     * @brief Builds a DNS query packet for testing
     * @param name The domain name to query
     * @param transactionId The transaction ID for the query
     * @return The DNS query packet bytes
     */
    std::vector<uint8_t> buildDnsQuery(const std::string& name, uint16_t transactionId);

    /**
     * @brief Parses a DNS response for testing
     * @param response The DNS response bytes
     * @param length The length of the response
     * @return Vector of SRV records parsed from the response
     */
    std::vector<SrvRecord> parseDnsResponse(const uint8_t* response, size_t length);

    /**
     * @brief Selects the best SRV record based on priority/weight
     * @param records The SRV records to select from
     * @return The selected SRV record
     */
    SrvRecord selectBestSrvRecord(const std::vector<SrvRecord>& records);

    /**
     * @brief Queries SRV records from a DNS server
     * @param serviceName The full service name to query
     * @param dnsServer The DNS server IP address
     * @param timeoutMs Timeout in milliseconds
     * @return Vector of SRV records
     */
    std::vector<SrvRecord> querySrvRecords(
        const std::string& serviceName,
        const std::string& dnsServer = "8.8.8.8",
        int timeoutMs = 5000);

private:
    explicit OpAppAcquisitionTestInterface(
        const std::string& opapp_fqdn,
        bool is_network_available);

    std::unique_ptr<OpAppAcquisition> m_acquisition;
};

} // namespace orb

#endif /* OP_APP_ACQUISITION_TEST_INTERFACE_H */
