#ifndef OP_APP_PACKAGE_MANAGER_TEST_INTERFACE_H
#define OP_APP_PACKAGE_MANAGER_TEST_INTERFACE_H

#include "OpAppPackageManager.h"
#include <memory>
#include <string>

namespace orb
{

// Forward declarations
class IAitFetcher;
class IXmlParser;

/**
 * @brief Test interface for OpAppPackageManager that provides controlled access
 * to internal functionality for testing purposes while maintaining encapsulation.
 *
 * This interface should only be used in test code and provides a clean API
 * for testing the package manager's functionality without exposing private
 * implementation details.
 */
class OpAppPackageManagerTestInterface
{
public:
    /**
     * @brief Creates a test interface for the package manager
     * @param configuration The configuration for the package manager
     * @return A test interface instance
     */
    static std::unique_ptr<OpAppPackageManagerTestInterface> create(
        const OpAppPackageManager::Configuration& configuration);

    /**
     * @brief Creates a test interface with custom dependencies for testing
     * @param configuration The configuration for the package manager
     * @param hashCalculator Custom hash calculator for testing
     * @param decryptor Custom decryptor for testing
     * @return A test interface instance
     */
    static std::unique_ptr<OpAppPackageManagerTestInterface> create(
        const OpAppPackageManager::Configuration& configuration,
        std::unique_ptr<IHashCalculator> hashCalculator,
        std::unique_ptr<IDecryptor> decryptor);

    /**
     * @brief Creates a test interface with all custom dependencies for testing
     * @param configuration The configuration for the package manager
     * @param hashCalculator Custom hash calculator for testing
     * @param decryptor Custom decryptor for testing
     * @param aitFetcher Custom AIT fetcher for testing
     * @return A test interface instance
     */
    static std::unique_ptr<OpAppPackageManagerTestInterface> create(
        const OpAppPackageManager::Configuration& configuration,
        std::unique_ptr<IHashCalculator> hashCalculator,
        std::unique_ptr<IDecryptor> decryptor,
        std::unique_ptr<IAitFetcher> aitFetcher);

    /**
     * @brief Creates a test interface with all custom dependencies including XML parser
     * @param configuration The configuration for the package manager
     * @param hashCalculator Custom hash calculator for testing
     * @param decryptor Custom decryptor for testing
     * @param aitFetcher Custom AIT fetcher for testing
     * @param xmlParser Custom XML parser for testing
     * @return A test interface instance
     */
    static std::unique_ptr<OpAppPackageManagerTestInterface> create(
        const OpAppPackageManager::Configuration& configuration,
        std::unique_ptr<IHashCalculator> hashCalculator,
        std::unique_ptr<IDecryptor> decryptor,
        std::unique_ptr<IAitFetcher> aitFetcher,
        std::unique_ptr<IXmlParser> xmlParser);

    /**
     * @brief Destructor
     */
    ~OpAppPackageManagerTestInterface();

    // Prevent copying and moving
    OpAppPackageManagerTestInterface(const OpAppPackageManagerTestInterface&) = delete;
    OpAppPackageManagerTestInterface& operator=(const OpAppPackageManagerTestInterface&) = delete;
    OpAppPackageManagerTestInterface(OpAppPackageManagerTestInterface&&) = delete;
    OpAppPackageManagerTestInterface& operator=(OpAppPackageManagerTestInterface&&) = delete;

    // Public API methods (same as OpAppPackageManager)
    void start();
    bool isRunning() const;
    bool isUpdating() const;
    bool isPackageInstalled(const std::string& packagePath);
    void checkForUpdates();
    std::string calculateFileSHA256Hash(const std::string& filePath) const;
    PackageOperationResult getPackageFiles();
    std::string getLastErrorMessage() const;
    void clearLastError();

    // Test-specific methods that provide controlled access to internal functionality
    /**
     * @brief Sets a candidate package file for testing
     * @param packageFile Path to the package file
     */
    void setCandidatePackageFile(const std::string& packageFile);

    /**
     * @brief Performs package file check (internal method exposed for testing)
     * @return Package status
     */
    OpAppPackageManager::PackageStatus doPackageFileCheck();

    /**
     * @brief Attempts to install the package (internal method exposed for testing)
     * @return Package status
     */
    OpAppPackageManager::PackageStatus tryPackageInstall();

    /**
     * @brief Decrypts a package file (internal method exposed for testing)
     * @param filePath Path to the file to decrypt
     * @return Operation result
     */
    PackageOperationResult decryptPackageFile(const std::string& filePath) const;

    /**
     * @brief Verifies a package file (internal method exposed for testing)
     * @param filePath Path to the file to verify
     * @return Operation result
     */
    PackageOperationResult verifyPackageFile(const std::string& filePath) const;

    /**
     * @brief Unzips a package file (internal method exposed for testing)
     * @param filePath Path to the file to unzip
     * @return Success status
     */
    bool unzipPackageFile(const std::string& filePath) const;

    /**
     * @brief Performs remote package check (internal method exposed for testing)
     * @return Package status
     */
    OpAppPackageManager::PackageStatus doRemotePackageCheck();

    /**
     * @brief Parses AIT files (internal method exposed for testing)
     * @param aitFiles Vector of paths to AIT XML files
     * @param packages Vector of discovered PackageInfo
     * @return PackageOperationResult with success status and any error messages.
     */
    PackageOperationResult parseAitFiles(const std::vector<std::string>& aitFiles, std::vector<PackageInfo>& packages);

    /**
     * @brief Gets installed package info (internal method exposed for testing)
     * @param orgId Organization ID
     * @param appId Application ID
     * @param outPackage Output PackageInfo with installation details if found
     * @return true if an installed package was found, false otherwise
     */
    bool getInstalledPackage(uint32_t orgId, uint16_t appId, PackageInfo& outPackage) const;

    /**
     * @brief Gets the underlying package manager instance
     * @return Reference to the package manager
     */
    OpAppPackageManager& getPackageManager() { return *m_PackageManager; }

    /**
     * @brief Gets the underlying package manager instance (const)
     * @return Const reference to the package manager
     */
    const OpAppPackageManager& getPackageManager() const { return *m_PackageManager; }

private:
    /**
     * @brief Private constructor - use create() methods instead
     * @param packageManager The package manager instance (takes ownership)
     */
    explicit OpAppPackageManagerTestInterface(std::unique_ptr<OpAppPackageManager> packageManager);

    std::unique_ptr<OpAppPackageManager> m_PackageManager;
};

} // namespace orb

#endif /* OP_APP_PACKAGE_MANAGER_TEST_INTERFACE_H */
