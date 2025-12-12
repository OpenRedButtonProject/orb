#include "OpAppAcquisitionTestInterface.h"

namespace orb
{

OpAppAcquisitionTestInterface::OpAppAcquisitionTestInterface(const std::string& userAgent)
    : m_acquisition(std::make_unique<OpAppAcquisition>(userAgent))
{
}

OpAppAcquisitionTestInterface::~OpAppAcquisitionTestInterface() = default;

std::unique_ptr<OpAppAcquisitionTestInterface> OpAppAcquisitionTestInterface::create(
    const std::string& userAgent)
{
    return std::unique_ptr<OpAppAcquisitionTestInterface>(
        new OpAppAcquisitionTestInterface(userAgent));
}

bool OpAppAcquisitionTestInterface::validateFqdn(const std::string& fqdn)
{
    return m_acquisition->validateFqdn(fqdn);
}

std::vector<SrvRecord> OpAppAcquisitionTestInterface::doDnsSrvLookup(const std::string& fqdn)
{
    return m_acquisition->doDnsSrvLookup(fqdn);
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

AcquisitionResult OpAppAcquisitionTestInterface::FetchAitXml(
    const std::string& fqdn, bool networkAvailable)
{
    return m_acquisition->FetchAitXml(fqdn, networkAvailable);
}

AcquisitionResult OpAppAcquisitionTestInterface::StaticFetch(
    const std::string& fqdn, bool networkAvailable)
{
    return OpAppAcquisition::Fetch(fqdn, networkAvailable);
}

} // namespace orb
