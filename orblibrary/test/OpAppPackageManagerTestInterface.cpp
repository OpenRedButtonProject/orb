#include "OpAppPackageManagerTestInterface.h"
#include "OpAppPackageManager.h"
#include "AitFetcher.h"  // For IAitFetcher
#include "xml_parser.h"  // For IXmlParser

namespace orb
{

std::unique_ptr<OpAppPackageManagerTestInterface> OpAppPackageManagerTestInterface::create(
    const OpAppPackageManager::Configuration& configuration)
{
    auto packageManager = std::make_unique<OpAppPackageManager>(configuration);
    return std::unique_ptr<OpAppPackageManagerTestInterface>(new OpAppPackageManagerTestInterface(std::move(packageManager)));
}

std::unique_ptr<OpAppPackageManagerTestInterface> OpAppPackageManagerTestInterface::create(
    const OpAppPackageManager::Configuration& configuration,
    std::unique_ptr<IHashCalculator> hashCalculator,
    std::unique_ptr<IDecryptor> decryptor)
{
    auto packageManager = std::make_unique<OpAppPackageManager>(
        configuration, std::move(hashCalculator), std::move(decryptor));
    return std::unique_ptr<OpAppPackageManagerTestInterface>(new OpAppPackageManagerTestInterface(std::move(packageManager)));
}

std::unique_ptr<OpAppPackageManagerTestInterface> OpAppPackageManagerTestInterface::create(
    const OpAppPackageManager::Configuration& configuration,
    std::unique_ptr<IHashCalculator> hashCalculator,
    std::unique_ptr<IDecryptor> decryptor,
    std::unique_ptr<IAitFetcher> aitFetcher)
{
    auto packageManager = std::make_unique<OpAppPackageManager>(
        configuration, std::move(hashCalculator), std::move(decryptor), std::move(aitFetcher));
    return std::unique_ptr<OpAppPackageManagerTestInterface>(new OpAppPackageManagerTestInterface(std::move(packageManager)));
}

std::unique_ptr<OpAppPackageManagerTestInterface> OpAppPackageManagerTestInterface::create(
    const OpAppPackageManager::Configuration& configuration,
    std::unique_ptr<IHashCalculator> hashCalculator,
    std::unique_ptr<IDecryptor> decryptor,
    std::unique_ptr<IAitFetcher> aitFetcher,
    std::unique_ptr<IXmlParser> xmlParser)
{
    auto packageManager = std::make_unique<OpAppPackageManager>(
        configuration, std::move(hashCalculator), std::move(decryptor),
        std::move(aitFetcher), std::move(xmlParser));
    return std::unique_ptr<OpAppPackageManagerTestInterface>(new OpAppPackageManagerTestInterface(std::move(packageManager)));
}

OpAppPackageManagerTestInterface::OpAppPackageManagerTestInterface(std::unique_ptr<OpAppPackageManager> packageManager)
    : m_PackageManager(std::move(packageManager))
{
}

OpAppPackageManagerTestInterface::~OpAppPackageManagerTestInterface() = default;

void OpAppPackageManagerTestInterface::start()
{
    m_PackageManager->start();
}

bool OpAppPackageManagerTestInterface::isRunning() const
{
    return m_PackageManager->isRunning();
}

bool OpAppPackageManagerTestInterface::isUpdating() const
{
    return m_PackageManager->isUpdating();
}

bool OpAppPackageManagerTestInterface::isPackageInstalled(const std::string& packagePath)
{
    return m_PackageManager->isPackageInstalled(packagePath);
}

void OpAppPackageManagerTestInterface::checkForUpdates()
{
    m_PackageManager->checkForUpdates();
}

std::string OpAppPackageManagerTestInterface::calculateFileSHA256Hash(const std::string& filePath) const
{
    return m_PackageManager->calculateFileSHA256Hash(filePath);
}

PackageOperationResult OpAppPackageManagerTestInterface::getPackageFiles()
{
    return m_PackageManager->getPackageFiles();
}

std::string OpAppPackageManagerTestInterface::getLastErrorMessage() const
{
    return m_PackageManager->getLastErrorMessage();
}

void OpAppPackageManagerTestInterface::clearLastError()
{
    m_PackageManager->clearLastError();
}

void OpAppPackageManagerTestInterface::setCandidatePackageFile(const std::string& packageFile)
{
    // Access private member directly since we're a friend class
    m_PackageManager->m_CandidatePackageFile = packageFile;
}

OpAppPackageManager::PackageStatus OpAppPackageManagerTestInterface::doPackageFileCheck()
{
    // Access private method directly since we're a friend class
    return m_PackageManager->doPackageFileCheck();
}

OpAppPackageManager::PackageStatus OpAppPackageManagerTestInterface::tryPackageInstall()
{
    // Access private method directly since we're a friend class
    return m_PackageManager->tryPackageInstall();
}

PackageOperationResult OpAppPackageManagerTestInterface::decryptPackageFile(const std::string& filePath) const
{
    // Access private method directly since we're a friend class
    return m_PackageManager->decryptPackageFile(filePath);
}

PackageOperationResult OpAppPackageManagerTestInterface::verifyPackageFile(const std::string& filePath) const
{
    // Access private method directly since we're a friend class
    return m_PackageManager->verifyPackageFile(filePath);
}

bool OpAppPackageManagerTestInterface::unzipPackageFile(const std::string& filePath) const
{
    // Access private method directly since we're a friend class
    return m_PackageManager->unzipPackageFile(filePath);
}

OpAppPackageManager::PackageStatus OpAppPackageManagerTestInterface::doRemotePackageCheck()
{
    // Access private method directly since we're a friend class
    return m_PackageManager->doRemotePackageCheck();
}

PackageOperationResult OpAppPackageManagerTestInterface::parseAitFiles(const std::vector<std::string>& aitFiles, std::vector<PackageInfo>& packages)
{
    // Access private method directly since we're a friend class
    return m_PackageManager->parseAitFiles(aitFiles, packages);
}

bool OpAppPackageManagerTestInterface::getInstalledPackage(uint32_t orgId, uint16_t appId, PackageInfo& outPackage) const
{
    // Access private method directly since we're a friend class
    return m_PackageManager->getInstalledPackage(orgId, appId, outPackage);
}

} // namespace orb
