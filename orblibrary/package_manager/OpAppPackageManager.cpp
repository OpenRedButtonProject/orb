#include "OpAppPackageManager.h"
#include <mutex>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <openssl/sha.h>
#include <iostream>

// Static member initialization
std::unique_ptr<OpAppPackageManager> OpAppPackageManager::s_Instance = nullptr;
std::mutex OpAppPackageManager::s_InstanceMutex;

OpAppPackageManager::OpAppPackageManager(const Configuration& configuration)
  : m_PackageStatus(PackageStatus::DontKnow)
  , m_IsRunning(false)
  , m_IsUpdating(false)
  , m_Mutex()
  , m_WorkerThread()
  , m_Configuration(configuration)
  , m_LastErrorMessage()
  , m_HashCalculator(std::make_unique<OpenSSLHashCalculator>())
{
}

OpAppPackageManager::OpAppPackageManager(const Configuration& configuration, std::unique_ptr<IHashCalculator> hashCalculator)
  : m_PackageStatus(PackageStatus::DontKnow)
  , m_IsRunning(false)
  , m_IsUpdating(false)
  , m_Mutex()
  , m_WorkerThread()
  , m_Configuration(configuration)
  , m_LastErrorMessage()
  , m_HashCalculator(std::move(hashCalculator))
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

OpAppPackageManager& OpAppPackageManager::getInstance(const Configuration& configuration, std::unique_ptr<IHashCalculator> hashCalculator)
{
  std::lock_guard<std::mutex> lock(s_InstanceMutex);

  if (!s_Instance) {
    s_Instance = std::unique_ptr<OpAppPackageManager>(new OpAppPackageManager(configuration, std::move(hashCalculator)));
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
  PackageOperationResult packageFilesResult = getPackageFiles();

  // Check if there was an error getting package files
  if (!packageFilesResult.success) {
    m_PackageStatus = PackageStatus::ConfigurationError;
    m_LastErrorMessage = packageFilesResult.errorMessage;
    return;
  }

  // Get the actual package files
  std::vector<std::string> packageFiles = packageFilesResult.packageFiles;

  // Process found no package files, no change
  if (packageFiles.empty()) {
    return;
  }

  // We have exactly one package file (getPackageFiles would have returned error if more than one)
  std::string packageFile = packageFiles[0];

  // Check if the package is installed by comparing hashes
  if (isPackageInstalled(packageFile)) {
    m_PackageStatus = PackageStatus::Installed;
  } else {
    m_PackageStatus = PackageStatus::UpdateAvailable;
  }
}

bool OpAppPackageManager::isPackageInstalled(const std::string& packagePath) const
{
  // Calculate the given files SHA-256 hash and compare it to the hash of the installed package
  // If the hashes are the same, the package is installed
  // If the hashes are different, the package is not installed
  // If the package is not installed, return false
  // If the package is installed, return true

  // Get the hash of the package file
  std::string packageHash = calculateSHA256Hash(packagePath);

  // Get the hash of the installed package
  std::string installedPackageHash = calculateSHA256Hash(m_Configuration.m_PackageHashFilePath);

  // Compare the hashes and return the result
  return packageHash == installedPackageHash;
}

PackageOperationResult OpAppPackageManager::getPackageFiles()
{
  std::vector<std::string> packageFiles;

  // Check if the package location directory exists
  if (!std::filesystem::exists(m_Configuration.m_PackageLocation)) {
    return PackageOperationResult(true, "", packageFiles); // No error, just no files
  }

  // Iterate through files in the package location directory
  for (const auto& entry : std::filesystem::directory_iterator(m_Configuration.m_PackageLocation)) {
    if (entry.is_regular_file()) {
      std::string filename = entry.path().filename().string();
      if (filename.length() >= m_Configuration.m_PackageSuffix.length() &&
          filename.substr(filename.length() - m_Configuration.m_PackageSuffix.length()) == m_Configuration.m_PackageSuffix) {
        packageFiles.push_back(entry.path().string());
      }
    }
  }

  // Check for multiple package files
  if (packageFiles.size() > 1) {
    std::string errorMessage = "Multiple package files found in directory '" +
                              m_Configuration.m_PackageLocation + "'. Expected only one package file. Found: ";
    for (size_t i = 0; i < packageFiles.size(); ++i) {
      if (i > 0) errorMessage += ", ";
      errorMessage += std::filesystem::path(packageFiles[i]).filename().string();
    }
    return PackageOperationResult(false, errorMessage, packageFiles);
  }

  return PackageOperationResult(true, "", packageFiles);
}

std::string OpAppPackageManager::calculateSHA256Hash(const std::string& filePath) const
{
  return m_HashCalculator->calculateSHA256Hash(filePath);
}

std::string OpenSSLHashCalculator::calculateSHA256Hash(const std::string& filePath) const
{
  std::ifstream file(filePath, std::ios::binary);
  if (!file.is_open()) {
    return ""; // Return empty string if file cannot be opened
  }

  // Initialize SHA256 context
  SHA256_CTX sha256;
  SHA256_Init(&sha256);

  // Read file in chunks and update hash
  const size_t bufferSize = 4096; // 4KB chunks
  std::vector<char> buffer(bufferSize);

  while (file.read(buffer.data(), bufferSize)) {
    SHA256_Update(&sha256, buffer.data(), file.gcount());
  }

  // Handle any remaining bytes
  if (file.gcount() > 0) {
    SHA256_Update(&sha256, buffer.data(), file.gcount());
  }

  // Finalize the hash
  unsigned char hash[SHA256_DIGEST_LENGTH];
  SHA256_Final(hash, &sha256);

  // Convert hash to hexadecimal string
  std::stringstream ss;
  ss << std::hex << std::setfill('0');
  for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
    ss << std::setw(2) << static_cast<int>(hash[i]);
  }

  return ss.str();
}

std::string OpAppPackageManager::calculateFileSHA256Hash(const std::string& filePath) const
{
  return calculateSHA256Hash(filePath);
}
