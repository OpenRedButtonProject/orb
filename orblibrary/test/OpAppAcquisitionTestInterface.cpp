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

std::vector<uint8_t> OpAppAcquisitionTestInterface::buildDnsQuery(
    const std::string& name, uint16_t transactionId)
{
    return m_acquisition->buildDnsQuery(name, transactionId);
}

std::vector<SrvRecord> OpAppAcquisitionTestInterface::parseDnsResponse(
    const uint8_t* response, size_t length)
{
    return m_acquisition->parseDnsResponse(response, length);
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

std::vector<SrvRecord> OpAppAcquisitionTestInterface::querySrvRecords(
    const std::string& serviceName,
    const std::string& dnsServer,
    int timeoutMs)
{
    return m_acquisition->querySrvRecords(serviceName, dnsServer, timeoutMs);
}

std::string OpAppAcquisitionTestInterface::retrieveOpAppAitXml()
{
    return m_acquisition->retrieveOpAppAitXml();
}

std::string OpAppAcquisitionTestInterface::performHttpGet(
    const std::string& url, uint16_t port)
{
    return m_acquisition->performHttpGet(url, port);
}

} // namespace orb
