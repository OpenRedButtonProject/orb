#include "OpAppPackageManager.h"
#include <mutex>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <iostream>
#include <base/logging.h> // TODO: use local logging instead
#include "json/json.h" // TODO: We should be using external/jsoncpp library instead

// TODO: Rename these to OpAppHashCalculator.h and OpAppDecryptor.h
#include "OpenSSLHashCalculator.h"
#include "OpenSSLDecryptor.h"

static std::string statusCodeToString(OpAppPackageManager::PackageStatus status)
{
  switch (status) {
    case OpAppPackageManager::PackageStatus::None:
      return "None";
    case OpAppPackageManager::PackageStatus::NoUpdateAvailable:
      return "NoUpdateAvailable";
    case OpAppPackageManager::PackageStatus::NotInstalled:
      return "NotInstalled";
    case OpAppPackageManager::PackageStatus::Installed:
      return "Installed";
    case OpAppPackageManager::PackageStatus::UpdateAvailable:
      return "UpdateAvailable";
    case OpAppPackageManager::PackageStatus::UpdateFailed:
      return "UpdateFailed";
    case OpAppPackageManager::PackageStatus::DecryptionFailed:
      return "DecryptionFailed";
    case OpAppPackageManager::PackageStatus::ConfigurationError:
      return "ConfigurationError";
  }
}

static int readJsonField(
  const std::string& jsonFilePath,
  const std::string& fieldName,
  std::string& fieldValue,
  const std::string& defaultValue = ""
)
{
  // Check if the file exists
  if (!std::filesystem::exists(jsonFilePath)) {
    return -1; // File does not exist
  }

  // Read the JSON file
  std::ifstream jsonFile(jsonFilePath);
  Json::Value json;
  jsonFile >> json;

  // Check if the field exists
  if (json.isMember(fieldName)) {
    fieldValue = json[fieldName].asString();
    if (fieldValue.empty()) {
      fieldValue = defaultValue;
    }
    return 0;
  }
  return -2;
}

// Static member initialization
std::unique_ptr<OpAppPackageManager> OpAppPackageManager::s_Instance = nullptr;
std::mutex OpAppPackageManager::s_InstanceMutex;

OpAppPackageManager::OpAppPackageManager(const Configuration& configuration)
  : m_PackageStatus(PackageStatus::None)
  , m_IsRunning(false)
  , m_IsUpdating(false)
  , m_Mutex()
  , m_WorkerThread()
  , m_Configuration(configuration)
  , m_LastErrorMessage()
  , m_HashCalculator(std::make_unique<OpenSSLHashCalculator>())
  , m_Decryptor(std::make_unique<OpenSSLDecryptor>())
{
}

OpAppPackageManager::OpAppPackageManager(const Configuration& configuration, std::unique_ptr<IHashCalculator> hashCalculator)
  : m_PackageStatus(PackageStatus::None)
  , m_IsRunning(false)
  , m_IsUpdating(false)
  , m_Mutex()
  , m_WorkerThread()
  , m_Configuration(configuration)
  , m_LastErrorMessage()
  , m_HashCalculator(std::move(hashCalculator))
  , m_Decryptor(std::make_unique<OpenSSLDecryptor>())
{
}

OpAppPackageManager::OpAppPackageManager(
  const Configuration& configuration, std::unique_ptr<IHashCalculator> hashCalculator, std::unique_ptr<IDecryptor> decryptor)
  : m_PackageStatus(PackageStatus::None)
  , m_IsRunning(false)
  , m_IsUpdating(false)
  , m_Mutex()
  , m_WorkerThread()
  , m_Configuration(configuration)
  , m_LastErrorMessage()
  , m_HashCalculator(std::move(hashCalculator))
  , m_Decryptor(std::move(decryptor))
{
  if (!m_HashCalculator) {
    m_HashCalculator = std::make_unique<OpenSSLHashCalculator>();
  }
  if (!m_Decryptor) {
    m_Decryptor = std::make_unique<OpenSSLDecryptor>();
  }
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

OpAppPackageManager& OpAppPackageManager::getInstance(
  const Configuration& configuration, std::unique_ptr<IHashCalculator> hashCalculator, std::unique_ptr<IDecryptor> decryptor)
{
  std::lock_guard<std::mutex> lock(s_InstanceMutex);

  if (!s_Instance) {
    s_Instance = std::unique_ptr<OpAppPackageManager>(new OpAppPackageManager(configuration, std::move(hashCalculator), std::move(decryptor)));
  }

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

void OpAppPackageManager::checkForUpdates()
{
  m_IsRunning = true;

  m_PackageStatus = doPackageFileCheck();

  if (m_PackageStatus != PackageStatus::UpdateAvailable) {

    LOG(INFO) << "No update available: [" << statusCodeToString(m_PackageStatus) << "]";
    return;
  }

  // We have an update available. Install it.
  m_IsUpdating = true;
  m_PackageStatus = doPackageInstall();
  m_IsUpdating = false;

  if (m_PackageStatus != PackageStatus::Installed) {
    LOG(ERROR) << "Update failed: [" << statusCodeToString(m_PackageStatus) << "]";
    return;
  }

  LOG(INFO) << "Update installed: [" << statusCodeToString(m_PackageStatus) << "]";

  // Keep the worker thread running by adding a small delay
  // This prevents the thread from exiting immediately
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

OpAppPackageManager::PackageStatus OpAppPackageManager::doPackageFileCheck()
{
  // Check the m_PackageLocation for any new packages
  // Get the list of files in m_PackageLocation with file ending with m_PackageSuffix
  PackageOperationResult packageFilesResult = getPackageFiles();

  // Check if there was an error getting package files
  if (!packageFilesResult.success) {
    m_LastErrorMessage = packageFilesResult.errorMessage;
    return PackageStatus::ConfigurationError;
  }

  // Get the actual package files
  std::vector<std::string> packageFiles = packageFilesResult.packageFiles;

  // Process found no package files, no change
  if (packageFiles.empty()) {
    return PackageStatus::NoUpdateAvailable;
  }

  // We have exactly one package file (getPackageFiles would have returned error if more than one)
  std::string packageFile = packageFiles[0];

  // Check if the package is installed by comparing hashes
  if (isPackageInstalled(packageFile)) {
    return PackageStatus::Installed;
  }

  // Save the package file name to the m_CandidatePackageFile variable
  m_CandidatePackageFile = packageFile;

  return PackageStatus::UpdateAvailable;
}

bool OpAppPackageManager::isPackageInstalled(const std::string& packagePath)
{
  // Calculate the given files SHA-256 hash and compare it to the hash of the installed package
  // If the hashes are the same, the package is installed
  // If the hashes are different, the package is not installed
  // If the package is not installed, return false
  // If the package is installed, return true

  // Get the hash of the package file
  m_CandidatePackageHash = calculateSHA256Hash(packagePath);

  // Get the hash of the installed package
  std::string installedPackageHash;
  int result = readJsonField(m_Configuration.m_PackageHashFilePath, "hash", installedPackageHash);
  if (result == 0) {
    // Compare the hashes and return the result
    return m_CandidatePackageHash == installedPackageHash;
  }
  else if (result == -1) {
    // File does not exist, so the package is not installed
    LOG(INFO) << "Package receipt file does not exist: " << m_Configuration.m_PackageHashFilePath;
    return false;
  }

  // else:Error reading the file
  LOG(ERROR) << "Error reading package receipt file: " << m_Configuration.m_PackageHashFilePath;
  return false;
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

std::string OpAppPackageManager::calculateFileSHA256Hash(const std::string& filePath) const
{
  return calculateSHA256Hash(filePath);
}

OpAppPackageManager::PackageStatus OpAppPackageManager::doPackageInstall()
{
  // Check if the package file is set
  if (m_CandidatePackageFile.empty()) {
    return PackageStatus::ConfigurationError;
  }

  // Check if the package file exists
  if (!std::filesystem::exists(m_CandidatePackageFile)) {
    return PackageStatus::ConfigurationError;
  }

  // Steps to install the package:
  // 1. Ensure the destination directory exists
  if (!std::filesystem::exists(m_Configuration.m_DestinationDirectory)) {
    std::filesystem::create_directories(m_Configuration.m_DestinationDirectory);
  }

  // 2. Copy the package file to the installation directory
  std::string packageFileName = std::filesystem::path(m_CandidatePackageFile).filename().string();
  std::string workingFilePath = m_Configuration.m_DestinationDirectory + "/" + packageFileName;
  std::error_code errorCode;
  std::filesystem::copy(m_CandidatePackageFile, workingFilePath, errorCode);
  if (errorCode) {
    LOG(ERROR) << "Error copying package file to working directory: " << errorCode.message();
    return PackageStatus::ConfigurationError;
  }
  else {
    LOG(INFO) << "Package file copied to working directory: " << workingFilePath;
  }

  // Decrypt the package file
  PackageOperationResult decryptResult = decryptPackageFile(workingFilePath);

  // Verify the package file
  if (!decryptResult.success) {
    LOG(ERROR) << "Decryption failed: " << decryptResult.errorMessage;
    return PackageStatus::DecryptionFailed;
  }

  LOG(INFO) << "Decryption successful";

  // // Verify the package file
  // if (!verifyPackageFile(decryptResult.packageFiles[0])) {
  //   LOG(ERROR) << "Package file verification failed";
  //   return PackageStatus::ConfigurationError;
  // }
  // LOG(INFO) << "Package file verification successful";

  // // 3. Update the package receipt file with the candidate hash
  // // Create the JSON hash file
  // std::ofstream hashFile(m_Configuration.m_PackageHashFilePath);
  // hashFile << "{\"hash\": \"" << m_CandidatePackageHash << "\"}";
  // hashFile.close();

  // 4. Return the status of the installation
  return PackageStatus::Installed;
}

PackageOperationResult OpAppPackageManager::decryptPackageFile(const std::string& filePath) const
{
  return m_Decryptor->decrypt(filePath);
}

bool OpAppPackageManager::verifyPackageFile(const std::string& filePath) const
{
  return false;
}
