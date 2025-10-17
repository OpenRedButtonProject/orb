#ifndef OP_APP_PACKAGE_MANAGER_H
#define OP_APP_PACKAGE_MANAGER_H

#include <string>
#include <thread>
#include <atomic>
#include <memory>
#include <vector>
#include <mutex>
#include <functional>

// Error handling structure for package operations
struct PackageOperationResult {
  bool success;
  std::string errorMessage;
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



  // Singleton instance management
  // getInstance() returns nullptr if not configured yet
  static OpAppPackageManager* getInstance();
  // getInstance(configuration) creates and configures the instance
  static OpAppPackageManager& getInstance(const Configuration& configuration);
  // getInstance(configuration, hashCalculator) creates instance with custom hash calculator (for testing)
  static OpAppPackageManager& getInstance(
    const Configuration& configuration, std::unique_ptr<IHashCalculator> hashCalculator);
  // getInstance(configuration, decryptor) creates instance with custom decryptor (for testing)
  static OpAppPackageManager& getInstance(
    const Configuration& configuration, std::unique_ptr<IDecryptor> decryptor);
  // getInstance(configuration, hashCalculator, decryptor) creates instance with custom hash calculator and decryptor (for testing)
  static OpAppPackageManager& getInstance(
    const Configuration& configuration, std::unique_ptr<IHashCalculator> hashCalculator, std::unique_ptr<IDecryptor> decryptor);
  static void destroyInstance();

  // Destructor must be public for std::unique_ptr to work
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

  // Public method for calculating SHA256 hash (useful for testing and external use)
  std::string calculateFileSHA256Hash(const std::string& filePath) const;

  // Package status methods
  PackageOperationResult getPackageFiles();

  // Error handling
  std::string getLastErrorMessage() const { return m_LastErrorMessage; }
  void clearLastError() { m_LastErrorMessage.clear(); }

  // Make the test interface a friend class
  friend class OpAppPackageManagerTestInterface;

private:
  // Private constructor for singleton pattern
  OpAppPackageManager(const Configuration& configuration);
  // Private constructor with custom hash calculator (for testing)
  OpAppPackageManager(
    const Configuration& configuration, std::unique_ptr<IHashCalculator> hashCalculator);
  // Private constructor with custom hash calculator and decryptor (for testing)
  OpAppPackageManager(
    const Configuration& configuration,
    std::unique_ptr<IHashCalculator> hashCalculator,
    std::unique_ptr<IDecryptor> decryptor);

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

  PackageStatus m_PackageStatus;

  // void uninstallPackage(const std::string& packagePath);
  // void updatePackage(const std::string& packagePath);
  // PackageInfo getPackageInfo();

  std::atomic<bool> m_IsRunning;
  std::atomic<bool> m_IsUpdating;
  std::mutex m_Mutex;

  std::thread m_WorkerThread;
  Configuration m_Configuration;

  // Singleton instance
  static std::unique_ptr<OpAppPackageManager> s_Instance;
  static std::mutex s_InstanceMutex;

  std::string m_LastErrorMessage;
  std::unique_ptr<IHashCalculator> m_HashCalculator;
  std::unique_ptr<IDecryptor> m_Decryptor;

  std::string m_CandidatePackageFile;
  std::string m_CandidatePackageHash;
};

#endif /* OP_APP_PACKAGE_MANAGER_H */
