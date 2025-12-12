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

namespace orb
{

// Error handling structure for package operations
struct PackageOperationResult {
  bool success;
  std::string errorMessage;
  int statusCode; // See TS 103 606 V1.2.1 (2024-03) A.2.2.1
  std::vector<std::string> packageFiles;

  PackageOperationResult() : success(true) {}
  PackageOperationResult(bool s, const std::string& msg) : success(s), errorMessage(msg) {}
  PackageOperationResult(bool s, const std::string& msg, const std::vector<std::string>& files)
    : success(s), errorMessage(msg), packageFiles(files) {}
};


// Hash calculation interface for testing
class IHashCalculator {
public:
  virtual ~IHashCalculator() = default;
  virtual std::string calculateSHA256Hash(const std::string& filePath) const = 0;
};

class IDecryptor {
public:
  virtual ~IDecryptor() = default;
  virtual PackageOperationResult decrypt(const std::string& filePath) const = 0;
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
    NotInstalled,
    Installed,
    UpdateAvailable,
    UpdateFailed,
    DecryptionFailed,
    VerificationFailed,
    ConfigurationError
  };

  // Callback function types for update completion
  using UpdateSuccessCallback = std::function<void(const std::string& packagePath)>;
  using UpdateFailureCallback = std::function<void(PackageStatus status, const std::string& errorMessage)>;

  struct PackageInfo {
    std::string m_PackageName;
    std::string m_PackageVersion;
    std::string m_PackageDescription;
    std::string m_PackageAuthor;
    std::string m_PackageLicense;
  };

  struct Configuration {
      std::string m_OpAppFqdn; /* Fully Qualified Domain Name (Section 6.1.4 of TS 103 606 V1.2.1) */
      std::string m_PackageLocation;
      std::string m_PackageSuffix;
      std::string m_PrivateKeyFilePath;
      std::string m_PublicKeyFilePath;
      std::string m_CertificateFilePath;
      std::string m_PackageHashFilePath;
      std::string m_DestinationDirectory; /* Directory where the package is decrypted, unzipped and verified */
      std::string m_OpAppInstallDirectory; /* Directory where the OpApp is installed */
      UpdateSuccessCallback m_OnUpdateSuccess; /* Callback called when update completes successfully */
      UpdateFailureCallback m_OnUpdateFailure; /* Callback called when update fails */
  };

  // Constructors
  explicit OpAppPackageManager(const Configuration& configuration);
  // Constructor with custom hash calculator (for testing)
  OpAppPackageManager(
    const Configuration& configuration, std::unique_ptr<IHashCalculator> hashCalculator);
  // Constructor with custom hash calculator and decryptor (for testing)
  OpAppPackageManager(
    const Configuration& configuration,
    std::unique_ptr<IHashCalculator> hashCalculator,
    std::unique_ptr<IDecryptor> decryptor);

  ~OpAppPackageManager();

  // Prevent copying and moving
  OpAppPackageManager(const OpAppPackageManager&) = delete;
  OpAppPackageManager& operator=(const OpAppPackageManager&) = delete;
  OpAppPackageManager(OpAppPackageManager&&) = delete;
  OpAppPackageManager& operator=(OpAppPackageManager&&) = delete;

  void start();
  void stop();
  bool isRunning() const;
  bool isUpdating() const;
  bool isPackageInstalled(const std::string& packagePath);
  void checkForUpdates();

  OpAppUpdateStatus getOpAppUpdateStatus() const;

  /* Returns the URL of the currently installed OpApp, otherwise empty string */
  std::string getOpAppUrl() const;

  // Public method for calculating SHA256 hash (useful for testing and external use)
  std::string calculateFileSHA256Hash(const std::string& filePath) const;

  // Search the local package location 'Configuration::m_PackageLocation' for package files.
  // Returns a PackageOperationResult containing the list of package files found.
  // If no package files are found, the success flag is false and the error message is set.
  // If multiple package files are found, the success flag is false and the error message is set.
  // If a single package file is found, the success flag is true and the package file is set.
  // If an error occurs, the success flag is false and the error message is set.
  PackageOperationResult getPackageFiles();

  // Error handling
  std::string getLastErrorMessage() const { return m_LastErrorMessage; }
  void clearLastError() { m_LastErrorMessage.clear(); }

  // Make the test interface a friend class
  friend class OpAppPackageManagerTestInterface;

private:
  /**
   * doPackageFileCheck()
   *
   * Checks for the existence of a *single* OpApp package file, ending with the package suffix
   * in the directory set by m_PackageLocation, and checks its SHA256 hash against any existing
   * hash found at m_PackageHashFilePath.
   *
   * If the package file is found, it is saved to m_CandidatePackageFile.
   *
   * Returns:
   *  PackageStatus::NoUpdateAvailable if no package file is found.
   *  PackageStatus::Installed if the package file exists and the hash is the same.
   *  PackageStatus::UpdateAvailable if the package file exists and the hash is different.
   *  PackageStatus::ConfigurationError if multiple package files are found.
   *  PackageStatus::ConfigurationError for any other error.
   */
  PackageStatus doPackageFileCheck();

  /**
   * doRemotePackageCheck()
   *
   * Checks for a remote package file and compares hash to installed package hash.
   *
   * Returns:
   *  PackageStatus::NoUpdateAvailable if no remote package file is found.
   *  PackageStatus::UpdateAvailable if the remote package file exists and the hash is different.
   *  PackageStatus::Installed if the remote package file is already installed.
   */
  PackageStatus doRemotePackageCheck();

  /**
   * tryPackageInstall()
   *
   * Attempts to install the package file found in m_CandidatePackageFile.
   *
   * Returns:
   *  PackageStatus::Installed if the package is installed successfully.
   *  PackageStatus::DecryptionFailed if the package file cannot be decrypted.
   *  PackageStatus::VerificationFailed if the package file cannot be verified.
   *  PackageStatus::ConfigurationError if the package file cannot be found.
   *  PackageStatus::ConfigurationError for any other error.
   */
  PackageStatus tryPackageInstall();

  /**
   * decryptPackageFile()
   *
   * Decrypts the package file found in m_CandidatePackageFile.
   *
   * Returns:
   *  PackageOperationResult::success if the package file is decrypted successfully.
   *  PackageOperationResult::errorMessage if the package file cannot be decrypted.
   *  PackageOperationResult::packageFiles if the package file is decrypted successfully.
   */
  PackageOperationResult decryptPackageFile(const std::string& filePath) const;

  /**
   * verifyPackageFile()
   *
   * Verifies the package file found in m_CandidatePackageFile. See reference 6.1.8.
   *
   * Returns:
   *  PackageOperationResult::success if the package is compatible and a new version of OpApp.
   *  PackageOperationResult::errorMessage if the package is not compatible with the OpApp or same or older version.
   */
  PackageOperationResult verifyPackageFile(const std::string& filePath) const;

  /**
   * unzipPackageFile()
   *
   * Unzips the package file found in m_CandidatePackageFile.
   *
   * Returns:
   *  true if the package is unzipped successfully.
   *  false if the package cannot be unzipped.
   */
  bool unzipPackageFile(const std::string& filePath) const;

  PackageStatus m_PackageStatus = PackageStatus::None;

  // void uninstallPackage(const std::string& packagePath);
  // void updatePackage(const std::string& packagePath);
  // PackageInfo getPackageInfo();

  std::atomic<bool> m_IsRunning{false};
  std::atomic<bool> m_IsUpdating{false}; // TODO replace with OpAppUpdateStatus
  std::atomic<OpAppUpdateStatus> m_UpdateStatus{OpAppUpdateStatus::NONE};
  std::mutex m_Mutex;

  std::thread m_WorkerThread;
  Configuration m_Configuration;

  std::string m_LastErrorMessage;
  std::unique_ptr<IHashCalculator> m_HashCalculator;
  std::unique_ptr<IDecryptor> m_Decryptor;

  std::string m_CandidatePackageFile;
  std::string m_CandidatePackageHash;
};

} // namespace orb

#endif /* OP_APP_PACKAGE_MANAGER_H */
