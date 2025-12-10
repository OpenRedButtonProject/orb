#include "OpAppAcquisition.h"
#include "log.h"

OpAppAcquisition::OpAppAcquisition(const std::string &opapp_fqdn, bool is_network_available)
    : m_opapp_fqdn(opapp_fqdn)
    , m_is_network_available(is_network_available)
{
}

OpAppAcquisition::~OpAppAcquisition()
{
}

bool OpAppAcquisition::validateFqdn(const std::string &fqdn)
{
    if (fqdn.empty()) {
        return false;
    }
    if (fqdn.find(".") == std::string::npos) {
        return false;
    }
    return true;
}

std::string OpAppAcquisition::doDnsSrvLookup()
{
    if (!m_is_network_available) {
        LOG(ERROR) << "Network is not available";
        return "";
    }

    if (!validateFqdn(m_opapp_fqdn)) {
        LOG(ERROR) << "Invalid FQDN: " << m_opapp_fqdn;
        return "";
    }

    // Section 6.1.4 of TS 103 606 V1.2.1 (2024-03)
    const std::string prefix = "_hbbtv-ait._tcp.";
    return m_opapp_fqdn;
}
