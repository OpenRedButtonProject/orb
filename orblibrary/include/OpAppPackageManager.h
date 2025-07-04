#ifndef OP_APP_PACKAGE_MANAGER_H
#define OP_APP_PACKAGE_MANAGER_H

#include <string>
#include <thread>
#include <atomic>
#include <memory>
#include <vector>
#include <mutex>

// Hash calculation interface for testing
class IHashCalculator {
public:
  virtual ~IHashCalculator() = default;
  virtual std::string calculateSHA256Hash(const std::string& filePath) const = 0;
};

// Default implementation using OpenSSL
class OpenSSLHashCalculator : public IHashCalculator {
public:
  std::string calculateSHA256Hash(const std::string& filePath) const override;
};

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

class OpAppPackageManager
{
public:

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
      std::string m_DestinationDirectory;
      std::string m_InstallDirectory;
  };

  enum class PackageStatus {
    DontKnow,
    NotInstalled,
    Installed,
    UpdateAvailable,
    UpdateFailed,
    ConfigurationError  // New status for configuration errors like multiple packages
  };

  // Singleton instance management
  // getInstance() returns nullptr if not configured yet
  static OpAppPackageManager* getInstance();
  // getInstance(configuration) creates and configures the instance
  static OpAppPackageManager& getInstance(const Configuration& configuration);
  // getInstance(configuration, hashCalculator) creates instance with custom hash calculator (for testing)
  static OpAppPackageManager& getInstance(const Configuration& configuration, std::unique_ptr<IHashCalculator> hashCalculator);
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
  bool isPackageInstalled(const std::string& packagePath) const;
  void checkForUpdates();
  PackageStatus getPackageStatus() const;
  void doPackageFileCheck();

  // Public method for calculating SHA256 hash (useful for testing and external use)
  std::string calculateFileSHA256Hash(const std::string& filePath) const;

  // Package status methods
  PackageOperationResult getPackageFiles();

  // Error handling
  std::string getLastErrorMessage() const { return m_LastErrorMessage; }
  void clearLastError() { m_LastErrorMessage.clear(); }

private:
  // Private constructor for singleton pattern
  OpAppPackageManager(const Configuration& configuration);
  // Private constructor with custom hash calculator (for testing)
  OpAppPackageManager(const Configuration& configuration, std::unique_ptr<IHashCalculator> hashCalculator);

  std::string calculateSHA256Hash(const std::string& filePath) const;
  PackageStatus m_PackageStatus;

  // void installPackage(const std::string& packagePath);
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
};

#endif /* OP_APP_PACKAGE_MANAGER_H */
