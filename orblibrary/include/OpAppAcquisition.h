#ifndef OP_APP_ACQUISITION_H
#define OP_APP_ACQUISITION_H

#include <string>

class OpAppAcquisition {
public:
    OpAppAcquisition(const std::string &opapp_fqdn, bool is_network_available);
    ~OpAppAcquisition();

    friend class OpAppAcquisitionTestInterface;

private:
    /* Perform a DNS SRV lookup for the OpApp as defined in TS 103 606 V1.2.1 (2024-03)
    * Section 6.1.4 and returns the URL of the
    *
    */
    std::string doDnsSrvLookup();

    /* Minimum validation of the FQDN - not empty and contains a dot */
    bool validateFqdn(const std::string &fqdn);

    std::string m_opapp_fqdn;
    bool m_is_network_available;
};

#endif // OP_APP_ACQUISITION_H
