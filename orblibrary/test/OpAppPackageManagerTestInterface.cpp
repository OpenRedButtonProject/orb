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

bool OpAppPackageManagerTestInterface::isPackageInstalled(const std::filesystem::path& packagePath)
{
    return m_PackageManager->isPackageInstalled(packagePath);
}

void OpAppPackageManagerTestInterface::checkForUpdates()
{
    m_PackageManager->checkForUpdates();
}

std::string OpAppPackageManagerTestInterface::calculateFileSHA256Hash(const std::filesystem::path& filePath) const
{
    return m_PackageManager->calculateFileSHA256Hash(filePath);
}

int OpAppPackageManagerTestInterface::searchLocalPackageFiles(std::vector<std::filesystem::path>& outPackageFiles)
{
    return m_PackageManager->searchLocalPackageFiles(outPackageFiles);
}

std::string OpAppPackageManagerTestInterface::getLastErrorMessage() const
{
    return m_PackageManager->getLastErrorMessage();
}

void OpAppPackageManagerTestInterface::clearLastError()
{
    m_PackageManager->clearLastError();
}

void OpAppPackageManagerTestInterface::setCandidatePackageFile(const std::filesystem::path& packageFile)
{
    // Access private member directly since we're a friend class
    m_PackageManager->m_CandidatePackageFile = packageFile;
}

OpAppPackageManager::PackageStatus OpAppPackageManagerTestInterface::doLocalPackageCheck()
{
    // Access private method directly since we're a friend class
    return m_PackageManager->doLocalPackageCheck();
}

OpAppPackageManager::PackageStatus OpAppPackageManagerTestInterface::installFromPackageFile()
{
    // Access private method directly since we're a friend class
    return m_PackageManager->installFromPackageFile();
}

bool OpAppPackageManagerTestInterface::decryptPackageFile(const std::filesystem::path& filePath, std::filesystem::path& outFile)
{
    // Access private method directly since we're a friend class
    return m_PackageManager->decryptPackageFile(filePath, outFile);
}

bool OpAppPackageManagerTestInterface::verifyZipPackage(const std::filesystem::path& filePath)
{
    // Access private method directly since we're a friend class
    return m_PackageManager->verifyZipPackage(filePath);
}

bool OpAppPackageManagerTestInterface::unzipPackageFile(const std::filesystem::path& inFile, std::filesystem::path& outPath)
{
    // Access private method directly since we're a friend class
    return m_PackageManager->unzipPackageFile(inFile, outPath);
}

OpAppPackageManager::PackageStatus OpAppPackageManagerTestInterface::doRemotePackageCheck()
{
    // Access private method directly since we're a friend class
    return m_PackageManager->doRemotePackageCheck();
}

bool OpAppPackageManagerTestInterface::parseAitFiles(const std::vector<std::filesystem::path>& aitFiles, std::vector<PackageInfo>& packages)
{
    // Access private method directly since we're a friend class
    return m_PackageManager->parseAitFiles(aitFiles, packages);
}

bool OpAppPackageManagerTestInterface::isPackageInstalled(uint32_t orgId, uint16_t appId, PackageInfo& outPackage) const
{
    // Access private method directly since we're a friend class
    return m_PackageManager->isPackageInstalled(orgId, appId, outPackage);
}

bool OpAppPackageManagerTestInterface::movePackageFileToInstallationDirectory(const std::filesystem::path& packageFilePath)
{
    // Access private method directly since we're a friend class
    return m_PackageManager->movePackageFileToInstallationDirectory(packageFilePath);
}

bool OpAppPackageManagerTestInterface::verifyUnzippedPackage(const std::filesystem::path& filePath)
{
    // Access private method directly since we're a friend class
    return m_PackageManager->verifyUnzippedPackage(filePath);
}

bool OpAppPackageManagerTestInterface::installToPersistentStorage(const std::filesystem::path& filePath)
{
    // Access private method directly since we're a friend class
    return m_PackageManager->installToPersistentStorage(filePath);
}

std::filesystem::path OpAppPackageManagerTestInterface::getCandidatePackageFile() const
{
    // Access private member directly since we're a friend class
    return m_PackageManager->m_CandidatePackageFile;
}

} // namespace orb
