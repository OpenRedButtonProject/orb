#include "AitFetcherTestInterface.h"

namespace orb
{

AitFetcherTestInterface::AitFetcherTestInterface(const std::string& userAgent)
    : m_fetcher(std::make_unique<AitFetcher>(userAgent))
{
}

AitFetcherTestInterface::~AitFetcherTestInterface() = default;

std::unique_ptr<AitFetcherTestInterface> AitFetcherTestInterface::create(
    const std::string& userAgent)
{
    return std::unique_ptr<AitFetcherTestInterface>(
        new AitFetcherTestInterface(userAgent));
}

bool AitFetcherTestInterface::validateFqdn(const std::string& fqdn)
{
    return m_fetcher->validateFqdn(fqdn);
}

std::vector<SrvRecord> AitFetcherTestInterface::doDnsSrvLookup(const std::string& fqdn)
{
    return m_fetcher->doDnsSrvLookup(fqdn);
}

SrvRecord AitFetcherTestInterface::selectBestSrvRecord(
    const std::vector<SrvRecord>& records)
{
    return m_fetcher->selectBestSrvRecord(records);
}

SrvRecord AitFetcherTestInterface::popNextSrvRecord(
    std::vector<SrvRecord>& records)
{
    return m_fetcher->popNextSrvRecord(records);
}

AitFetchResult AitFetcherTestInterface::FetchAitXmls(
    const std::string& fqdn, bool networkAvailable, const std::string& outputDirectory)
{
    return m_fetcher->FetchAitXmls(fqdn, networkAvailable, outputDirectory);
}

AitFetchResult AitFetcherTestInterface::StaticFetch(
    const std::string& fqdn, bool networkAvailable, const std::string& outputDirectory)
{
    return AitFetcher::Fetch(fqdn, networkAvailable, outputDirectory);
}

std::string AitFetcherTestInterface::generateAitFilename(int index, const std::string& target)
{
    return m_fetcher->generateAitFilename(index, target);
}

bool AitFetcherTestInterface::writeAitToFile(const std::string& content,
                                              const std::string& filePath)
{
    return m_fetcher->writeAitToFile(content, filePath);
}

} // namespace orb

