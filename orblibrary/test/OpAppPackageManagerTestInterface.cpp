#include "OpAppPackageManagerTestInterface.h"
#include "third_party/orb/orblibrary/include/OpAppPackageManager.h"

std::unique_ptr<OpAppPackageManagerTestInterface> OpAppPackageManagerTestInterface::create(
    const OpAppPackageManager::Configuration& configuration)
{
    OpAppPackageManager& packageManager = OpAppPackageManager::getInstance(configuration);
    return std::unique_ptr<OpAppPackageManagerTestInterface>(new OpAppPackageManagerTestInterface(packageManager));
}

std::unique_ptr<OpAppPackageManagerTestInterface> OpAppPackageManagerTestInterface::create(
    const OpAppPackageManager::Configuration& configuration,
    std::unique_ptr<IHashCalculator> hashCalculator,
    std::unique_ptr<IDecryptor> decryptor)
{
    OpAppPackageManager& packageManager = OpAppPackageManager::getInstance(configuration, std::move(hashCalculator), std::move(decryptor));
    return std::unique_ptr<OpAppPackageManagerTestInterface>(new OpAppPackageManagerTestInterface(packageManager));
}

OpAppPackageManagerTestInterface::OpAppPackageManagerTestInterface(OpAppPackageManager& packageManager)
    : m_PackageManager(&packageManager)
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
