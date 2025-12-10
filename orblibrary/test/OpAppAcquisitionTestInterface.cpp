#include "OpAppAcquisitionTestInterface.h"

OpAppAcquisitionTestInterface::OpAppAcquisitionTestInterface(
    const std::string& opapp_fqdn,
    bool is_network_available)
    : m_acquisition(std::make_unique<OpAppAcquisition>(opapp_fqdn, is_network_available))
{
}

OpAppAcquisitionTestInterface::~OpAppAcquisitionTestInterface() = default;

std::unique_ptr<OpAppAcquisitionTestInterface> OpAppAcquisitionTestInterface::create(
    const std::string& opapp_fqdn,
    bool is_network_available)
{
    return std::unique_ptr<OpAppAcquisitionTestInterface>(
        new OpAppAcquisitionTestInterface(opapp_fqdn, is_network_available));
}

bool OpAppAcquisitionTestInterface::validateFqdn(const std::string& fqdn)
{
    return m_acquisition->validateFqdn(fqdn);
}

std::string OpAppAcquisitionTestInterface::doDnsSrvLookup()
{
    return m_acquisition->doDnsSrvLookup();
}

