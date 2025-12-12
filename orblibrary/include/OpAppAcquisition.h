#ifndef OP_APP_ACQUISITION_H
#define OP_APP_ACQUISITION_H

#include <memory>
#include <string>
#include <vector>

namespace orb
{
struct SrvRecord;
class HttpDownloader;
class DownloadedObject;  // Now a standalone class, can be forward declared

class OpAppAcquisition {
public:
    OpAppAcquisition(const std::string &opapp_fqdn, bool is_network_available);
    ~OpAppAcquisition();

    friend class OpAppAcquisitionTestInterface;

    /**
     * Uses doDnsSrvLookup() to retrieve the AIT service URL and then retrieves
     * the AIT XML file from the URL.
     * Downloaded content is stored in m_downloadedObject.
     *
     * @return 0 on success, -1 on failure
     */
    int retrieveOpAppAitXml();

    /**
     * Get the downloaded content.
     *
     * @return The downloaded content, or empty string if none available
     */
    std::string getDownloadedContent() const;

private:
    /**
     * Perform a DNS SRV lookup for the OpApp as defined in TS 103 606 V1.2.1 (2024-03)
     * Section 6.1.4 and returns the SRV records for the AIT service.
     *
     * @return Vector of SRV records, empty on failure
     */
    std::vector<SrvRecord> doDnsSrvLookup();

    /**
     * Pops the next SRV record from a list of SRV records based on priority and weight.
     * Removes the returned SRV record from the input records vector.
     *
     * @param records The SRV records to get the next from
     * @return The next SRV record, or empty record if none available
     */
    SrvRecord popNextSrvRecord(std::vector<SrvRecord>& records);

    /**
     * Select the best SRV record based on priority and weight.
     * Lower priority values are preferred. Among equal priorities,
     * records are selected based on weight (RFC 2782).
     *
     * @param records The SRV records to select from
     * @return The selected SRV record, or empty record if none available
     */
    SrvRecord selectBestSrvRecord(const std::vector<SrvRecord>& records);

    /* Minimum validation of the FQDN - not empty and contains a dot */
    bool validateFqdn(const std::string &fqdn);

    std::string m_opapp_fqdn;
    bool m_is_network_available;

    std::unique_ptr<HttpDownloader> m_downloader;
    std::shared_ptr<DownloadedObject> m_downloadedObject;
};

} // namespace orb

#endif // OP_APP_ACQUISITION_H
