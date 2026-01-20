#ifndef OP_APP_PACKAGE_MANAGER_TEST_INTERFACE_H
#define OP_APP_PACKAGE_MANAGER_TEST_INTERFACE_H

#include "OpAppPackageManager.h"
#include <memory>
#include <string>
#include <filesystem>
#include <vector>

namespace orb
{

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
     * @param deps Optional dependencies for testing (default creates production implementations)
     * @return A test interface instance
     */
    static std::unique_ptr<OpAppPackageManagerTestInterface> create(
        const OpAppPackageManager::Configuration& configuration,
        OpAppPackageManager::Dependencies deps = {});

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
    void checkForUpdates(bool isFirstInstall);
    std::string calculateFileSHA256Hash(const std::filesystem::path& filePath) const;
    int searchLocalPackageFiles(std::vector<std::filesystem::path>& outPackageFiles);
    std::string getLastErrorMessage() const;
    void clearLastError();

    // Test-specific methods that provide controlled access to internal functionality
    /**
     * @brief Sets a candidate package file for testing
     * @param packageFile Path to the package file
     */
    void setCandidatePackageFile(const std::filesystem::path& packageFile);

    /**
     * @brief Performs package file check (internal method exposed for testing)
     * @return Package status
     */
    OpAppPackageManager::PackageStatus doLocalPackageCheck();

    /**
     * @brief Performs installation from package file (internal method exposed for testing)
     * @return PackageStatus::Installed on success, or specific failure status
     */
    OpAppPackageManager::PackageStatus installFromPackageFile();

    /**
     * @brief Decrypts a package file (internal method exposed for testing)
     * @param filePath Path to the file to decrypt
     * @param outFiles Output vector of decrypted file paths
     * @return true if decryption succeeded, false otherwise
     */
    bool decryptPackageFile(const std::filesystem::path& filePath, std::filesystem::path& outFile);

    /**
     * @brief Verifies a package file (internal method exposed for testing)
     * @param filePath Path to the file to verify
     * @return true if verification succeeded, false otherwise
     */
    bool verifyZipPackage(const std::filesystem::path& filePath);

    /**
     * @brief Unzips a package file (internal method exposed for testing)
     *
     * Also validates unzipped size against m_MaxUnzippedPackageSize.
     *
     * @param inFile Path to the ZIP package file to unzip
     * @param outPath Destination directory for extracted contents
     * @return Success status
     */
    bool unzipPackageFile(const std::filesystem::path& inFile, const std::filesystem::path& outPath);

    /**
     * @brief Performs remote package check (internal method exposed for testing)
     * @return Package status
     */
    OpAppPackageManager::PackageStatus doRemotePackageCheck();

    /**
     * @brief Parses AIT files (internal method exposed for testing)
     * @param aitFiles Vector of paths to AIT XML files
     * @param packages Vector of discovered PackageInfo
     * @return true if at least one valid OpApp descriptor was found, false otherwise.
     */
    bool parseAitFiles(const std::vector<std::filesystem::path>& aitFiles, std::vector<PackageInfo>& packages);

    /**
     * @brief Moves package file to installation directory (internal method exposed for testing)
     * @param packageFilePath Path to the package file to move
     * @return true if successful, false otherwise
     */
    bool movePackageFileToInstallationDirectory(const std::filesystem::path& packageFilePath);

    /**
     * @brief Downloads package file (internal method exposed for testing)
     * @param packageInfo The package information containing the download URL
     * @return true if download succeeded, false otherwise
     */
    bool downloadPackageFile(const PackageInfo& packageInfo);

    /**
     * @brief Copies package to persistent storage (internal method exposed for testing)
     * @param filePath Path to the package file
     * @return true if copy succeeded, false otherwise
     */
    bool installToPersistentStorage(const std::filesystem::path& filePath);

    /**
     * @brief Saves installation receipt (internal method exposed for testing)
     * @param pkg The package information to save
     * @return true if save succeeded, false otherwise
     */
    bool saveInstallReceipt(const PackageInfo& pkg);

    /**
     * @brief Loads installation receipt (internal method exposed for testing)
     * @param outPackage Output PackageInfo populated with installed package details
     * @return true if load succeeded, false otherwise
     */
    bool loadInstallReceipt(PackageInfo& outPackage) const;

    /**
     * @brief Sets the candidate package info for testing
     * @param pkg The package information to set as candidate
     */
    void setCandidatePackage(const PackageInfo& pkg);

    /**
     * @brief Sets the candidate package info for testing (convenience overload)
     * @param orgId Organization ID
     * @param appId Application ID
     * @param name Application name
     */
    void setCandidatePackage(uint32_t orgId, uint16_t appId, const std::string& name);

    /**
     * @brief Sets the candidate package hash for testing
     * @param hash The hash to set
     */
    void setCandidatePackageHash(const std::string& hash);

    /**
     * @brief Gets the current candidate package file path
     * @return The candidate package file path
     */
    std::filesystem::path getCandidatePackageFile() const;

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
