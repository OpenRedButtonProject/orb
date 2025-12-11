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

} // namespace orb
