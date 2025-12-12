#include "OpAppPackageManager.h"
#include "OpAppAcquisition.h"

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
#include "HashCalculator.h"
#include "Decryptor.h"

namespace orb
{

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
    case OpAppPackageManager::PackageStatus::VerificationFailed:
      return "VerificationFailed";
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

OpAppPackageManager::OpAppPackageManager(const Configuration& configuration)
  : OpAppPackageManager(configuration, nullptr, nullptr, nullptr)
{
}

OpAppPackageManager::OpAppPackageManager(const Configuration& configuration, std::unique_ptr<IHashCalculator> hashCalculator)
  : OpAppPackageManager(configuration, std::move(hashCalculator), nullptr, nullptr)
{
}

OpAppPackageManager::OpAppPackageManager(
  const Configuration& configuration,
  std::unique_ptr<IHashCalculator> hashCalculator,
  std::unique_ptr<IDecryptor> decryptor)
  : OpAppPackageManager(configuration, std::move(hashCalculator), std::move(decryptor), nullptr)
{
}

OpAppPackageManager::OpAppPackageManager(
  const Configuration& configuration,
  std::unique_ptr<IHashCalculator> hashCalculator,
  std::unique_ptr<IDecryptor> decryptor,
  std::unique_ptr<IOpAppAcquisition> acquisition)
  : m_Configuration(configuration)
  , m_HashCalculator(std::move(hashCalculator))
  , m_Decryptor(std::move(decryptor))
  , m_Acquisition(std::move(acquisition))
{
  // Create default implementations if not provided
  if (!m_HashCalculator) {
    m_HashCalculator = std::make_unique<HashCalculator>();
  }
  if (!m_Decryptor) {
    m_Decryptor = std::make_unique<Decryptor>();
  }
  if (!m_Acquisition) {
    m_Acquisition = std::make_unique<OpAppAcquisition>();
  }
}

OpAppPackageManager::~OpAppPackageManager()
{
  // Ensure the worker thread is stopped before destruction
  stop();
}

void OpAppPackageManager::start()
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  if (m_IsRunning) {
    return;
  }

  m_IsRunning = true;
  m_WorkerThread = std::thread(
    [this]() {
      checkForUpdates();
      m_IsRunning = false;
    });
}

void OpAppPackageManager::stop()
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  if (m_IsRunning) {
    m_IsRunning = false;
  }
  if (m_WorkerThread.joinable()) {
    m_WorkerThread.join();
  }
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
  if (!m_Configuration.m_PackageLocation.empty()) {
    // Checks for local package file and compares hash to installed package hash?
    m_PackageStatus = doPackageFileCheck();
  }

  if (m_PackageStatus == PackageStatus::None || m_PackageStatus == PackageStatus::NoUpdateAvailable) {
    // No local file or no update available.
    // Do a full remote check.
    m_PackageStatus = doRemotePackageCheck();
  }

  if (m_PackageStatus != PackageStatus::UpdateAvailable) {
    if (m_PackageStatus == PackageStatus::ConfigurationError)
    {
      LOG(ERROR) << "Configuration error: [" << statusCodeToString(m_PackageStatus) << "]";
      // Call failure callback for configuration errors
      if (m_Configuration.m_OnUpdateFailure) {
        m_Configuration.m_OnUpdateFailure(m_PackageStatus, m_LastErrorMessage);
      }
    }
    else
    {
      LOG(INFO) << "No new update available: [" << statusCodeToString(m_PackageStatus) << "]";
    }
    return;
  }

  // We have an update available. Install it.
  m_IsUpdating = true;
  m_PackageStatus = tryPackageInstall();
  m_IsUpdating = false;

  if (m_PackageStatus != PackageStatus::Installed) {
    LOG(ERROR) << "Update failed: [" << statusCodeToString(m_PackageStatus) << "]";
    // Call failure callback for install errors
    if (m_Configuration.m_OnUpdateFailure) {
      m_Configuration.m_OnUpdateFailure(m_PackageStatus, m_LastErrorMessage);
    }
    return;
  }

  LOG(INFO) << "Update installed: [" << statusCodeToString(m_PackageStatus) << "]";
  // Call success callback
  if (m_Configuration.m_OnUpdateSuccess) {
    m_Configuration.m_OnUpdateSuccess(m_CandidatePackageFile);
  }

  // Keep the worker thread running by adding a small delay
  // This prevents the thread from exiting immediately
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

OpAppPackageManager::OpAppUpdateStatus OpAppPackageManager::getOpAppUpdateStatus() const {
  // TODO Implement this. Hardcoded to allow opapp to start:
  return m_UpdateStatus.load();
}

std::string OpAppPackageManager::getOpAppUrl() const
{
  // TODO: Construct the URL of the opapp. See Section 9.4.1

  // This should return the URL of the currently installed OpApp, otherwise empty string
  // This is used to update the OpApp URL in the OpApp manager.
  // Set to localhost for testing.
  return "https://taco.freeviewplay.tv/";//"http://10.0.2.2:8080/index.html";
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

OpAppPackageManager::PackageStatus OpAppPackageManager::doRemotePackageCheck()
{
  // Check for a remote package file via AIT acquisition.
  // Needs the FQDN passed in. Use the FQDN from the Configuration::m_OpAppFqdn.
  if (m_Configuration.m_OpAppFqdn.empty()) {
    // No FQDN configured means remote check is not enabled - not an error condition
    // FREE-312: Will leave this in for testing purposes...
    LOG(INFO) << "No OpApp FQDN configured, skipping remote package check";
    return PackageStatus::NoUpdateAvailable;
  }

  // Use the injected acquisition interface to fetch AIT XML
  AcquisitionResult result = m_Acquisition->FetchAitXml(
      m_Configuration.m_OpAppFqdn, true /* network available */);

  if (!result.success) {
    LOG(ERROR) << "AIT acquisition failed: " << result.errorMessage;
    m_LastErrorMessage = result.errorMessage;
    return PackageStatus::ConfigurationError;
  }

  if (result.content.empty()) {
    LOG(ERROR) << "AIT acquisition returned empty content";
    m_LastErrorMessage = "AIT acquisition returned empty content";
    return PackageStatus::ConfigurationError;
  }

  LOG(INFO) << "AIT XML retrieved successfully, content size: " << result.content.size();

  // TODO Once the AIT XML is downloaded, we need to parse it to get the package URL.

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
  m_CandidatePackageHash = calculateFileSHA256Hash(packagePath);

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
    return PackageOperationResult(true, "", packageFiles); // No error, just no files, or no SD card.
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

std::string OpAppPackageManager::calculateFileSHA256Hash(const std::string& filePath) const
{
  return m_HashCalculator->calculateSHA256Hash(filePath);
}

OpAppPackageManager::PackageStatus OpAppPackageManager::tryPackageInstall()
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

  // Verify the package file
  PackageOperationResult verifyResult = verifyPackageFile(decryptResult.packageFiles[0]);
  if (!verifyResult.success) {
    LOG(INFO) << "Package file verification failed: " << verifyResult.errorMessage;
    return PackageStatus::VerificationFailed;
  }

  LOG(INFO) << "Package file verification successful";

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
  /* From the OpApp HbbTV spec:
  11.3.4.4 Process for decrypting an application package
  */
  return m_Decryptor->decrypt(filePath);
}

PackageOperationResult OpAppPackageManager::verifyPackageFile(const std::string& filePath) const
{
  /* From the OpApp HbbTV spec:
  6.1.8 Decrypt, verify, unpack and installation of the application package

    The terminal shall decrypt the encrypted application package as defined in clause 11.3.4.4 using the Terminal
    Packaging Certificate and corresponding private key. The terminal shall verify the signature of the decrypted
    application ZIP package as specified in clause 11.3.4.5.

    The terminal shall consider the application package as valid and verified if all of the following are true:
      •The application zip package passed the verification process defined in clause 11.3.4.5.
      •For application packages signalled by a broadcast AIT, the application loop entry from the initially trusted
      broadcast AIT matches the opapp.ait file contained inside the package.
      •For application packages signalled by an XML AIT, the initially trusted XML AIT file matches the
      opapp.aitx from inside the package.
      •When an already installed operator application is being updated, if a minimum application version number was
      provided when the package was last updated (or installed if this is the first update) then:
      -
      the version number in the application package to be installed is greater than or equal that minimum
      version number;
      otherwise if no minimum application version number was provided at that time;
      -
      •
      the version number in the application package to be installed is higher than the version number of the
      currently installed operator application.
      The combined uncompressed and extracted size of the operator application files is smaller than the maximum
      permitted, subject to the bilateral agreement.

  See 11.3.4.5 Application ZIP package signature verification process

    After decrypting the encrypted application package as defined in clause 11.3.4.4, terminals shall verify the resulting
    CMS SignedData according to the following process.

    Terminals shall use the Operator Signing Root CA to verify the certificates included in the certificates block of
    the CMS SignedData structure as detailed in section 5.1 of IETF RFC 5652 [12].

    Terminals shall extract the application ZIP file from the encapContentInfo block of the CMS SignedData.
    Terminals shall fail and reject the verification if any of the following conditions occur:
    •The certificate chain fails certificate path validation as defined in clause 6 of RFC 5280 [11] (this includes a
    check that none of the certificates have expired). The required check that certificates have not been revoked
    shall be performed by obtaining the appropriate CRLs using the cRLDistributionPoints extension (see table
    23).
    •The Operator Name, as signalled via the Organization ('O=') attribute of the subject field, or the
    organisation_id, as signalled via the CommonName ('CN=') attribute of the subject field do not match those
    defined in the bilateral agreement for the operator whose organisation_id is found during the discovery process
    in clause 6.1.5.
    •The value of the message-digest field contained in the CMS SignedData structure does not match with
    the terminal generating a message-digest of the extracted application ZIP file when applying the hashing
    function communicated via the SignatureAlgorithm field.

    If verification fails, the terminal shall follow the process outlined in clause 6.1.9.
    The following provides an informative example where the decrypted application ZIP file is verified with the Operator
    Signing Root CA. The example only covers validating the operator's certificate chain and the message-digest of the
    application ZIP file. It does not include checking certificates for revocation using CRLs.

  */

  return PackageOperationResult(false, "Verification failed", std::vector<std::string>());
}

bool OpAppPackageManager::unzipPackageFile(const std::string& filePath) const
{
  return false;
}

} // namespace orb
