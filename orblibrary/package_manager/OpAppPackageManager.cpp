#include "OpAppPackageManager.h"
#include <mutex>
#include <filesystem>
#include <base/logging.h>

// Static member initialization
std::unique_ptr<OpAppPackageManager> OpAppPackageManager::s_Instance = nullptr;
std::mutex OpAppPackageManager::s_InstanceMutex;

OpAppPackageManager::OpAppPackageManager(const Configuration& configuration)
  : m_PackageStatus(PackageStatus::DontKnow),
    m_IsRunning(false),
    m_IsUpdating(false),
    m_Configuration(configuration)
{
}

OpAppPackageManager::~OpAppPackageManager()
{
  // Ensure the worker thread is stopped before destruction
  if (m_IsRunning) {
    stop();
  }
}

// Singleton instance management
OpAppPackageManager* OpAppPackageManager::getInstance()
{
  std::lock_guard<std::mutex> lock(s_InstanceMutex);

  // Return nullptr if no instance has been created yet
  return s_Instance.get();
}

OpAppPackageManager& OpAppPackageManager::getInstance(const Configuration& configuration)
{
  std::lock_guard<std::mutex> lock(s_InstanceMutex);

  if (!s_Instance) {
    s_Instance = std::unique_ptr<OpAppPackageManager>(new OpAppPackageManager(configuration));
  }
  // If instance already exists, we just return the existing instance
  // This is a design decision - you could also log a warning or handle it differently

  return *s_Instance;
}

void OpAppPackageManager::destroyInstance()
{
  std::lock_guard<std::mutex> lock(s_InstanceMutex);
  if (s_Instance) {
    s_Instance.reset();
    s_Instance = nullptr; // Explicitly set to nullptr to ensure it's destroyed
  }
}

void OpAppPackageManager::start()
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  if (m_IsRunning) {
    return;
  }

  m_WorkerThread = std::thread(&OpAppPackageManager::checkForUpdates, this);

  m_IsRunning = true;
  m_IsUpdating = false;
}

void OpAppPackageManager::stop()
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  if (!m_IsRunning) {
    return;
  }

  m_WorkerThread.join();

  m_IsRunning = false;
  m_IsUpdating = false;
}

bool OpAppPackageManager::isRunning() const
{
  return m_IsRunning;
}

bool OpAppPackageManager::isUpdating() const
{
    return m_IsUpdating;
}

OpAppPackageManager::PackageStatus OpAppPackageManager::getPackageStatus() const
{
  return m_PackageStatus;
}

void OpAppPackageManager::checkForUpdates()
{
  m_IsRunning = true;

  doPackageFileCheck();

  // Keep the worker thread running by adding a small delay
  // This prevents the thread from exiting immediately
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

void OpAppPackageManager::doPackageFileCheck()
{
  // Check the m_PackageLocation for any new packages
  // Get the list of files in m_PackageLocation with file ending with m_PackageSuffix
  std::vector<std::string> packageFiles = getPackageFiles();

  // Process found package files if any
  if (packageFiles.empty()) {
    LOG(INFO) << "No package files found in " + m_Configuration.m_PackageLocation;
    return;
  }

  m_PackageStatus = PackageStatus::UpdateAvailable;
}

bool OpAppPackageManager::isPackageInstalled(const std::string& packagePath) const
{
  return false;
}

std::vector<std::string> OpAppPackageManager::getPackageFiles()
{
  std::vector<std::string> packageFiles;
  // Check if the package location directory exists
  if (!std::filesystem::exists(m_Configuration.m_PackageLocation)) {
    return packageFiles;
  }

  // Iterate through files in the package location directory
  for (const auto& entry : std::filesystem::directory_iterator(m_Configuration.m_PackageLocation)) {
    if (entry.is_regular_file()) {
      std::string filename = entry.path().filename().string();
      if (filename.find(m_Configuration.m_PackageSuffix) != std::string::npos) {
        packageFiles.push_back(entry.path().string());
      }
    }
  }

  return packageFiles;
}
