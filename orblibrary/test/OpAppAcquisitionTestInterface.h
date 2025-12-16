#ifndef OP_APP_ACQUISITION_TEST_INTERFACE_H
#define OP_APP_ACQUISITION_TEST_INTERFACE_H

#include "OpAppAcquisition.h"
#include "SrvRecord.h"
#include <memory>
#include <string>
#include <vector>

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
     * @param userAgent HTTP User-Agent header value (default: empty)
     * @return A test interface instance
     */
    static std::unique_ptr<OpAppAcquisitionTestInterface> create(const std::string& userAgent = "");

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
     * @param fqdn The FQDN to query
     * @return Vector of SRV records, empty on failure
     */
    std::vector<SrvRecord> doDnsSrvLookup(const std::string& fqdn);

    /**
     * @brief Selects the best SRV record based on priority/weight
     * @param records The SRV records to select from
     * @return The selected SRV record
     */
    SrvRecord selectBestSrvRecord(const std::vector<SrvRecord>& records);

    /**
     * @brief Pops the next SRV record and removes it from the list
     * @param records The SRV records to get from (modified in place)
     * @return The next SRV record based on priority/weight
     */
    SrvRecord popNextSrvRecord(std::vector<SrvRecord>& records);

    /**
     * @brief Fetches AIT XML using the simplified interface
     * @param fqdn The FQDN to query
     * @param networkAvailable Whether network is available
     * @return AcquisitionResult with success status and content/error
     */
    AcquisitionResult FetchAitXml(const std::string& fqdn, bool networkAvailable);

    /**
     * @brief Static fetch method test (convenience wrapper)
     * @param fqdn The FQDN to query
     * @param networkAvailable Whether network is available
     * @return AcquisitionResult with success status and content/error
     */
    static AcquisitionResult StaticFetch(const std::string& fqdn, bool networkAvailable);

private:
    explicit OpAppAcquisitionTestInterface(const std::string& userAgent);

    std::unique_ptr<OpAppAcquisition> m_acquisition;
};

} // namespace orb

#endif /* OP_APP_ACQUISITION_TEST_INTERFACE_H */
