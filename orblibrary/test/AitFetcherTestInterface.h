#ifndef AIT_FETCHER_TEST_INTERFACE_H
#define AIT_FETCHER_TEST_INTERFACE_H

#include "AitFetcher.h"
#include "SrvRecord.h"
#include <memory>
#include <string>
#include <vector>

/**
 * @brief Test interface for AitFetcher that provides controlled access
 * to internal functionality for testing purposes while maintaining encapsulation.
 *
 * This interface should only be used in test code.
 */
namespace orb
{

class AitFetcherTestInterface
{
public:
    /**
     * @brief Creates a test interface for AitFetcher
     * @param userAgent HTTP User-Agent header value (default: empty)
     * @return A test interface instance
     */
    static std::unique_ptr<AitFetcherTestInterface> create(const std::string& userAgent = "");

    /**
     * @brief Destructor
     */
    ~AitFetcherTestInterface();

    // Prevent copying and moving
    AitFetcherTestInterface(const AitFetcherTestInterface&) = delete;
    AitFetcherTestInterface& operator=(const AitFetcherTestInterface&) = delete;
    AitFetcherTestInterface(AitFetcherTestInterface&&) = delete;
    AitFetcherTestInterface& operator=(AitFetcherTestInterface&&) = delete;

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
     * @brief Fetches ALL AIT XMLs and writes them to files
     * @param fqdn The FQDN to query
     * @param networkAvailable Whether network is available
     * @param outputDirectory Directory where AIT files will be written
     * @return AitFetchResult with file paths and status
     */
    AitFetchResult FetchAitXmls(const std::string& fqdn, bool networkAvailable,
                                const std::string& outputDirectory);

    /**
     * @brief Static fetch method test (convenience wrapper)
     * @param fqdn The FQDN to query
     * @param networkAvailable Whether network is available
     * @param outputDirectory Directory where AIT files will be written
     * @return AitFetchResult with file paths and status
     */
    static AitFetchResult StaticFetch(const std::string& fqdn, bool networkAvailable,
                                      const std::string& outputDirectory);

    /**
     * @brief Test helper to generate an AIT filename
     * @param index The index of the AIT file
     * @param target The target server hostname
     * @return A sanitized filename
     */
    std::string generateAitFilename(int index, const std::string& target);

    /**
     * @brief Test helper to write AIT content to a file
     * @param content The AIT XML content
     * @param filePath The destination file path
     * @return true on success
     */
    bool writeAitToFile(const std::string& content, const std::string& filePath);

private:
    explicit AitFetcherTestInterface(const std::string& userAgent);

    std::unique_ptr<AitFetcher> m_fetcher;
};

} // namespace orb

#endif /* AIT_FETCHER_TEST_INTERFACE_H */

