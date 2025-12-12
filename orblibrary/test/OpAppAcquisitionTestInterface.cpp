#include "OpAppAcquisitionTestInterface.h"

namespace orb
{

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

std::vector<SrvRecord> OpAppAcquisitionTestInterface::doDnsSrvLookup()
{
    return m_acquisition->doDnsSrvLookup();
}

SrvRecord OpAppAcquisitionTestInterface::selectBestSrvRecord(
    const std::vector<SrvRecord>& records)
{
    return m_acquisition->selectBestSrvRecord(records);
}

SrvRecord OpAppAcquisitionTestInterface::popNextSrvRecord(
    std::vector<SrvRecord>& records)
{
    return m_acquisition->popNextSrvRecord(records);
}

std::string OpAppAcquisitionTestInterface::retrieveOpAppAitXml()
{
    return m_acquisition->retrieveOpAppAitXml();
}

} // namespace orb
