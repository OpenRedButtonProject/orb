/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef OP_APP_PACKAGE_MANAGER_H
#define OP_APP_PACKAGE_MANAGER_H

#include <string>
#include <thread>
#include <atomic>
#include <memory>
#include <vector>
#include <mutex>
#include <functional>
#include <filesystem>

#include "ait.h"

namespace orb
{

// Forward declarations
class IAitFetcher;
class IXmlParser;

/**
 * @brief Package information - represents both discovered and installed packages.
 *
 * Used for:
 * - Packages discovered from AIT (remote)
 * - Currently installed package (local)
 */
struct PackageInfo {
    // Identity (from AIT applicationIdentifier)
    uint32_t orgId = 0;
    uint16_t appId = 0;

    // Version info
    uint32_t xmlVersion = 0;

    // Location info (from AIT)
    std::string baseUrl;      // Transport URL base
    std::string location;     // Application location (e.g., "index.html")
    std::string name;         // Application name

    // Installation state (only set for installed packages)
    bool isInstalled = false;
    std::filesystem::path installPath;  // Local path where package is installed
    std::string packageHash;  // SHA256 hash of the installed package
    std::string installedAt;  // ISO timestamp of installation

    // Comparison helpers
    bool isSameApp(const PackageInfo& other) const {
        return orgId == other.orgId && appId == other.appId;
    }

    bool isNewerThan(const PackageInfo& other) const {
        return xmlVersion > other.xmlVersion;
    }

    // Construct full app URL
    std::string getAppUrl() const {
        if (baseUrl.empty()) return "";
        std::string url = baseUrl;
        if (!url.empty() && url.back() != '/' && !location.empty() && location.front() != '/') {
            url += '/';
        }
        return url + location;
    }
};

// Type alias for backwards compatibility during transition
using AitAppDescriptor = PackageInfo;

// Hash calculation interface for testing
class IHashCalculator {
public:
  virtual ~IHashCalculator() = default;
  virtual std::string calculateSHA256Hash(const std::filesystem::path& filePath) const = 0;
};

class IDecryptor {
public:
  virtual ~IDecryptor() = default;
  /**
   * Decrypts a package file and returns the output files.
   * @param filePath Path to the encrypted package file
   * @param outFile Output decrypted file path
   * @param outError Output error message if decryption fails
   * @return true if decryption succeeded, false otherwise
   */
  virtual bool decrypt(
    const std::filesystem::path& filePath,
    std::filesystem::path& outFile,
    std::string& outError) const = 0;
};

class OpAppPackageManager
{
public:

  // See TS 103 606 V1.2.1 (2024-03) A.2.2.1
  // Note "There is no event for a successful update as the operator application will be restarted at that point."
  enum class OpAppUpdateStatus {
    NONE,
    SOFTWARE_DISCOVERING,
    SOFTWARE_DISCOVERY_FAILED,
    SOFTWARE_CURRENT,
    SOFTWARE_DOWNLOADING,
    SOFTWARE_DOWNLOAD_FAILED,
    SOFTWARE_DOWNLOADED,
    SOFTWARE_UNPACKING,
    SOFTWARE_INSTALLATION_FAILED,
    INVALID_STATE
  };

  enum class PackageStatus {
    None,
    NoUpdateAvailable,
    DiscoveryFailed,
    Installed,
    UpdateAvailable,
    UpdateFailed,
    UnzipFailed,
    DecryptionFailed,
    VerificationFailed,
    ConfigurationError
  };

  // Callback function types for update completion
  using UpdateSuccessCallback = std::function<void(const std::filesystem::path& packagePath)>;
  using UpdateFailureCallback = std::function<void(PackageStatus status, const std::string& errorMessage)>;

  struct Configuration {

      /* Fully Qualified Domain Name (Section 6.1.4 of TS 103 606 V1.2.1) */
      std::string m_OpAppFqdn;

      // For local package checking, the following three fields must be set:

      /* Location of installable OpApp (.cms) package files (e.g. /mnt/sdcard/orb/packages).
       * If empty, does a remote check for updates. */
      std::filesystem::path m_PackageLocation;

      /* File path to the hash of the installed OpApp package.
       * If empty, does not check for package hash.
       * FREE-315 Used for local package checking, may be useful for remote package checking.*/
      std::filesystem::path m_PackageHashFilePath;

      std::filesystem::path m_PrivateKeyFilePath;
      std::filesystem::path m_PublicKeyFilePath;
      std::filesystem::path m_CertificateFilePath;

      std::filesystem::path m_DestinationDirectory; /* Directory where the package is decrypted, unzipped and verified */
      std::filesystem::path m_OpAppInstallDirectory; /* Directory where the OpApp is installed */
      UpdateSuccessCallback m_OnUpdateSuccess; /* Callback called when update completes successfully */
      UpdateFailureCallback m_OnUpdateFailure; /* Callback called when update fails */

      /* HTTP User-Agent header for AIT requests (TS 103 606 V1.2.1 Section 6.1.5.1)
       * Format per ETSI TS 102 796 Section 7.3.2.4 (HbbTV User-Agent string)
       * Example: "HbbTV/1.6.1 (+DRM;+PVR;+RTSP;+OMID) vendor/1.0" */
      std::string m_UserAgent = "HbbTV/1.6.1 (+DRM;+PVR;+RTSP;+OMID) orb/1.0";

      /* Directory where acquired AIT XML files are stored.
       * If empty, uses a subdirectory "ait_cache" of m_DestinationDirectory. */
      std::filesystem::path m_AitOutputDirectory;
  };

  // Constructors
  explicit OpAppPackageManager(const Configuration& configuration);

  // Constructor with custom hash calculator (for testing)
  OpAppPackageManager(
    const Configuration& configuration,
    std::unique_ptr<IHashCalculator> hashCalculator);

  // Constructor with custom hash calculator and decryptor (for testing)
  OpAppPackageManager(
    const Configuration& configuration,
    std::unique_ptr<IHashCalculator> hashCalculator,
    std::unique_ptr<IDecryptor> decryptor);

  // Constructor with all dependencies (for testing)
  OpAppPackageManager(
    const Configuration& configuration,
    std::unique_ptr<IHashCalculator> hashCalculator,
    std::unique_ptr<IDecryptor> decryptor,
    std::unique_ptr<IAitFetcher> aitFetcher);

  // Constructor with all dependencies including XML parser (for testing)
  OpAppPackageManager(
    const Configuration& configuration,
    std::unique_ptr<IHashCalculator> hashCalculator,
    std::unique_ptr<IDecryptor> decryptor,
    std::unique_ptr<IAitFetcher> aitFetcher,
    std::unique_ptr<IXmlParser> xmlParser);

  ~OpAppPackageManager();

  // Prevent copying and moving
  OpAppPackageManager(const OpAppPackageManager&) = delete;
  OpAppPackageManager& operator=(const OpAppPackageManager&) = delete;
  OpAppPackageManager(OpAppPackageManager&&) = delete;
  OpAppPackageManager& operator=(OpAppPackageManager&&) = delete;

  void start();
  void stop();
  bool isRunning() const;

  /**
   * isPackageInstalled()
   *
   * Checks if a package for a given org/app ID is installed.
   * If installed, returns true and populates outPackage with the package details.
   *
   * @param orgId Organization ID
   * @param appId Application ID
   * @param outPackage Output PackageInfo with installation details if found
   * @return true if an installed package was found, false otherwise
   */
  bool isPackageInstalled(uint32_t orgId, uint16_t appId, PackageInfo& outPackage) const;

  /**
   * isPackageInstalled(const std::filesystem::path& packagePath)
   *
   * Checks if the package at the given path is installed by comparing hashes.
   *
   * Returns:
   *  true if the package is installed.
   *  false if the package is not installed.
   */
  bool isPackageInstalled(const std::filesystem::path& packagePath);

  /**
   * checkForUpdates()
   *
   * Main entry point for checking for updates and installing the package if an update is available.
   * Calls tryLocalUpdate() or tryRemoteUpdate() as appropriate.
   *
   * Flow:
   * checkForUpdates()
    │
    ├─► tryLocalUpdate()     ─── Check for local package file
    │       │                    (in m_PackageLocation directory)
    │       │
    │       ├─► doLocalPackageCheck()    ─ Compare hash with installed version
    │       ├─► movePackageFileToInstallationDirectory()
    │       └─► installFromPackageFile() ─ Decrypt, verify, unzip, install
    │
    │   If no local update found:
    │
    └─► tryRemoteUpdate()    ─── Fetch AIT from remote server
            │                    (using m_OpAppFqdn)
            │
            ├─► doRemotePackageCheck()   ─ Fetch AITs, parse for OpApp info
            ├─► downloadPackageFile()    ─ Download the package
            └─► installFromPackageFile() ─ Decrypt, verify, unzip, install
   */
  void checkForUpdates();

  /**
   * setOpAppUpdateStatus(OpAppUpdateStatus status)
   *
   * Sets the update status.
   *
   * @param status The update status to set
   */
  void  setOpAppUpdateStatus(OpAppUpdateStatus status);
  OpAppUpdateStatus getOpAppUpdateStatus() const;

  /* Returns the URL of the currently installed OpApp, otherwise empty string */
  std::string getOpAppUrl() const;

  // Public method for calculating SHA256 hash (useful for testing and external use)
  std::string calculateFileSHA256Hash(const std::filesystem::path& filePath) const;

  /**
   * Search the local package location 'Configuration::m_PackageLocation' for package files.
   * @param outPackageFiles Output vector of found package file paths
   * @return Number of package files found (0 or more), or -1 on error.
   *         On error (e.g., multiple files found), sets m_LastErrorMessage.
   */
  int searchLocalPackageFiles(std::vector<std::filesystem::path>& outPackageFiles);

  // Error handling
  std::string getLastErrorMessage() const { return m_LastErrorMessage; }
  void clearLastError() { m_LastErrorMessage.clear(); }

  // Make the test interface a friend class
  friend class OpAppPackageManagerTestInterface;

private:
  /*
  * tryLocalUpdate()
  *
  * Checks for a local package file and compares hash to installed package hash.
  *
  * Returns:
  *  true if a package is installed successfully.
  *  false if no package is found or the package is not installed.
  */
  bool tryLocalUpdate();

  /**
   * tryRemoteUpdate()
   *
   * Attempts to update the OpApp from a remote source. See TS 103 606 Section 6.1
   *
   * Returns:
   *  true if a package is installed successfully.
   *  false if no package is found or the package is not installed.
   */
  bool tryRemoteUpdate();

  /**
   * doLocalPackageCheck()
   *
   * Checks for the existence of a *single* OpApp package file, ending with the package suffix
   * in the directory set by m_PackageLocation, and checks its SHA256 hash against any existing
   * hash found at m_PackageHashFilePath.
   *
   * If the package file is found, it is saved to m_CandidatePackageFile.
   *
   * Returns:
   *  PackageStatus::DiscoveryFailed if no package file is found.
   *  PackageStatus::Installed if the package file exists and the hash is the same.
   *  PackageStatus::UpdateAvailable if the package file exists and the hash is different.
   *  PackageStatus::ConfigurationError if multiple package files are found.
   *  PackageStatus::ConfigurationError for any other error.
   */
  PackageStatus doLocalPackageCheck();

  /**
   * doRemotePackageCheck()
   *
   * Checks for a remote package using AIT acquisition.
   * See TS 103 606 Section 6.1.5
   *
   * Returns:
   *  PackageStatus::DiscoveryFailed if no AIT files could be found.
   *  PackageStatus::UpdateAvailable if an XML AIT with new version of an OpApp is found.
   *  PackageStatus::Installed if the package is already installed.
   *  PackageStatus::ConfigurationError if FQDN is not set or the AIT
   *  files cannot be parsed, fetched or saved.
   */
  PackageStatus doRemotePackageCheck();

  /**
   * downloadPackageFile(const PackageInfo& packageInfo)
   *
   * Downloads the package file from the remote source based on packageInfo.
   * See TS 103 606 Section 6.1.7.
   * If successful, sets m_CandidatePackageFile to the downloaded package file path.
   *
   * @param packageInfo The package information to download
   * @return true if the package file is downloaded successfully, false otherwise.
   *         On error, sets m_LastErrorMessage.
   */
  bool downloadPackageFile(const PackageInfo& packageInfo);

  /**
   * installFromPackageFile()
   *
   * Performs the common installation flow: decrypt, verify, unpack, verify unzipped, copy.
   * Assumes m_CandidatePackageFile is set to the package file location.
   *
   * @return PackageStatus::Installed on success, or specific failure status
   */
  PackageStatus installFromPackageFile();

  /**
   * decryptPackageFile()
   *
   * Decrypts the package file. See TS 103 606 Section 6.1.8.
   *
   * @param filePath Path to the encrypted package file
   * @param outFiles Output vector of decrypted file paths
   * @return true if the package file is decrypted successfully, false otherwise.
   *         On error, sets m_LastErrorMessage.
   */
  bool decryptPackageFile(const std::filesystem::path& filePath, std::filesystem::path& outFile);

  /**
   * verifyZipPackage()
   *
   * Verifies the package file. See TS 103 606 Section 6.1.8.
   *
   * @param filePath Path to the package file to verify
   * @return true if the package is compatible and a new version of OpApp, false otherwise.
   *         On error, sets m_LastErrorMessage.
   */
  bool verifyZipPackage(const std::filesystem::path& filePath);

  /**
   * unzipPackageFile()
   *
   * Unzips the package file found in inFile. See TS 103 606 Section 6.1.8.
   *
   * Returns:
   *  true if the package is unzipped successfully.
   *  false if the package cannot be unzipped.
   */
  bool unzipPackageFile(const std::filesystem::path& inFile, const std::filesystem::path& outPath);

  /**
   * verifyUnzippedPackage()
   *
   * Verifies the unzipped package file as per TS 103 606 Section 11.3.4.5.
   *
   * @param filePath Path to the unzipped package file
   * @return true if the package is verified successfully, false otherwise.
   *         On error, sets m_LastErrorMessage.
   */
  bool verifyUnzippedPackage(const std::filesystem::path& filePath);

  /**
   * installToPersistentStorage()
   *
   * Installs the package file to persistent storage.
   * Creates the directory structure m_Configuration.m_OpAppInstallDirectory/appId/orgId
   * if it does not exist or deletes the directory structure if the package is being updated.
   *
   * @param filePath Path to the package file
   * @return true if the package is installed successfully, false otherwise.
   *         On error, sets m_LastErrorMessage.
   */
  bool installToPersistentStorage(const std::filesystem::path& filePath);

  /**
   * parseAitFiles()
   *
   * Parses AIT XML files and extracts package information.
   *
   * @param aitFiles Vector of paths to AIT XML files
   * @param packages Vector of PackageInfo (output) - discovered packages with isInstalled=false
   * @return true if at least one valid OpApp descriptor was found, false otherwise.
   *         On error, sets m_LastErrorMessage.
   */
  bool parseAitFiles(const std::vector<std::filesystem::path>& aitFiles, std::vector<PackageInfo>& packages);

  /**
   * movePackageFileToInstallationDirectory()
   *
   * For local installations, moves a package file to the
   * m_Configuration.m_DestinationDirectory directory.
   *
   * @param packageFilePath Path to the package file
   * @return true if the package file is moved successfully, false otherwise.
   *         On error, sets m_LastErrorMessage.
   */
  bool movePackageFileToInstallationDirectory(const std::filesystem::path& packageFilePath);

  /**
   * validateOpAppDescriptor()
   *
   * Validates an AIT application descriptor for OpApp requirements.
   * See TS 102796 Table 7 and TS 103606 Table 7.
   *
   * @param app The AIT application descriptor to validate
   * @param outError Output error message if validation fails
   * @return true if valid, false if invalid (with error details in outError).
   */
  bool validateOpAppDescriptor(const Ait::S_AIT_APP_DESC& app, std::string& outError) const;

  PackageStatus m_PackageStatus = PackageStatus::None;

  // void uninstallPackage(const std::string& packagePath);
  // void updatePackage(const std::string& packagePath);

  std::atomic<bool> m_IsRunning{false};
  std::atomic<OpAppUpdateStatus> m_UpdateStatus{OpAppUpdateStatus::NONE};
  std::mutex m_Mutex;

  std::thread m_WorkerThread;
  Configuration m_Configuration;

  std::string m_LastErrorMessage;
  std::unique_ptr<IHashCalculator> m_HashCalculator;
  std::unique_ptr<IDecryptor> m_Decryptor;
  std::unique_ptr<IAitFetcher> m_AitFetcher;
  std::unique_ptr<IXmlParser> m_XmlParser;

  std::filesystem::path m_CandidatePackageFile;
  std::string m_CandidatePackageHash;

  // The package (from AIT) that is a candidate for installation/update
  PackageInfo m_CandidatePackage;
};

} // namespace orb

#endif /* OP_APP_PACKAGE_MANAGER_H */
