#ifndef OP_APP_ACQUISITION_TEST_INTERFACE_H
#define OP_APP_ACQUISITION_TEST_INTERFACE_H

#include "OpAppAcquisition.h"
#include <memory>
#include <string>

/**
 * @brief Test interface for OpAppAcquisition that provides controlled access
 * to internal functionality for testing purposes while maintaining encapsulation.
 *
 * This interface should only be used in test code.
 */
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
     * @return The result of the DNS lookup
     */
    std::string doDnsSrvLookup();

private:
    explicit OpAppAcquisitionTestInterface(
        const std::string& opapp_fqdn,
        bool is_network_available);

    std::unique_ptr<OpAppAcquisition> m_acquisition;
};

#endif /* OP_APP_ACQUISITION_TEST_INTERFACE_H */

