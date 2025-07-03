#ifndef OP_APP_PACKAGE_MANAGER_H
#define OP_APP_PACKAGE_MANAGER_H

#include <string>
#include <thread>
#include <atomic>
#include <memory>

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
      std::string m_DestinationDirectory;
      std::string m_InstallDirectory;
  };

  enum class PackageStatus {
    DontKnow,
    NotInstalled,
    Installed,
    UpdateAvailable,
    UpdateInProgress,
    UpdateFailed,
    UpdateSuccess
  };

  // Singleton instance management
  // getInstance() returns nullptr if not configured yet
  static OpAppPackageManager* getInstance();
  // getInstance(configuration) creates and configures the instance
  static OpAppPackageManager& getInstance(const Configuration& configuration);
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

private:
  // Private constructor for singleton pattern
  OpAppPackageManager(const Configuration& configuration);

  std::vector<std::string> getPackageFiles();
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
};

#endif /* OP_APP_PACKAGE_MANAGER_H */
