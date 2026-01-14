#include "OpAppPackageManager.h"
#include "AitFetcher.h"
#include "xml_parser.h"
#include "HttpDownloader.h"

#include <cassert>
#include <mutex>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <iostream>
#include <random>
#include <base/logging.h> // TODO: use local logging instead
#include "json/json.h" // TODO: We should be using external/jsoncpp library instead

// TODO: Rename these to OpAppHashCalculator.h and OpAppDecryptor.h
#include "HashCalculator.h"
#include "Decryptor.h"

namespace orb
{
static int readJsonField(
  const std::filesystem::path& jsonFilePath,
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
  std::unique_ptr<IAitFetcher> aitFetcher)
  : OpAppPackageManager(configuration, std::move(hashCalculator), std::move(decryptor),
                        std::move(aitFetcher), nullptr)
{
}

OpAppPackageManager::OpAppPackageManager(
  const Configuration& configuration,
  std::unique_ptr<IHashCalculator> hashCalculator,
  std::unique_ptr<IDecryptor> decryptor,
  std::unique_ptr<IAitFetcher> aitFetcher,
  std::unique_ptr<IXmlParser> xmlParser)
  : OpAppPackageManager(configuration, std::move(hashCalculator), std::move(decryptor),
                        std::move(aitFetcher), std::move(xmlParser), nullptr)
{
}

OpAppPackageManager::OpAppPackageManager(
  const Configuration& configuration,
  std::unique_ptr<IHashCalculator> hashCalculator,
  std::unique_ptr<IDecryptor> decryptor,
  std::unique_ptr<IAitFetcher> aitFetcher,
  std::unique_ptr<IXmlParser> xmlParser,
  std::unique_ptr<IHttpDownloader> httpDownloader)
  : m_Configuration(configuration)
  , m_HashCalculator(std::move(hashCalculator))
  , m_Decryptor(std::move(decryptor))
  , m_AitFetcher(std::move(aitFetcher))
  , m_XmlParser(std::move(xmlParser))
  , m_HttpDownloader(std::move(httpDownloader))
{
  // Create default implementations if not provided
  if (!m_HashCalculator) {
    m_HashCalculator = std::make_unique<HashCalculator>();
  }
  if (!m_Decryptor) {
    m_Decryptor = std::make_unique<Decryptor>();
  }
  if (!m_AitFetcher) {
    // Pass User-Agent from configuration (TS 103 606 Section 6.1.5.1)
    m_AitFetcher = std::make_unique<AitFetcher>(m_Configuration.m_UserAgent);
  }
  if (!m_XmlParser) {
    m_XmlParser = IXmlParser::create();
  }
  if (!m_HttpDownloader) {
    // Pass User-Agent from configuration (TS 103 606 Section 6.1.7)
    m_HttpDownloader = std::make_unique<HttpDownloader>(30000, m_Configuration.m_UserAgent);
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
      // Check if an installation already exists.
      // If so, return.
      if (!isOpAppInstalled()) {
        doFirstTimeInstallation();
      }
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

bool OpAppPackageManager::isOpAppInstalled()
{
  PackageInfo installedPkg;
  if (loadInstallReceipt(installedPkg)) {
    LOG(INFO) << "OpApp "  << installedPkg.name << " is installed at " << installedPkg.installPath;
    return true;
  }

  return false;
}

void OpAppPackageManager::doFirstTimeInstallation()
{
  // At the moment the same process is used for both first time installation and update.
  checkForUpdates();
}

bool OpAppPackageManager::tryLocalUpdate()
{
  setOpAppUpdateStatus(OpAppUpdateStatus::SOFTWARE_DISCOVERING);
  if (m_Configuration.m_PackageLocation.empty() || m_Configuration.m_InstallReceiptFilePath.empty()) {
    setOpAppUpdateStatus(OpAppUpdateStatus::SOFTWARE_DISCOVERY_FAILED);
    LOG(WARNING) << "Local update failed: Package location or install receipt file path not set";
    return false;
  }

  LOG(INFO) << "Local package check enabled. Checking package file in "
    << m_Configuration.m_PackageLocation << " and comparing hash to installed package receipt in "
    << m_Configuration.m_InstallReceiptFilePath;
  m_PackageStatus = doLocalPackageCheck();

  if (m_PackageStatus != PackageStatus::UpdateAvailable) {
    if (m_PackageStatus == PackageStatus::ConfigurationError) {
      setOpAppUpdateStatus(OpAppUpdateStatus::SOFTWARE_DISCOVERY_FAILED);
      LOG(ERROR) << "Local Update failed: Configuration error";
    }
    else if (m_PackageStatus == PackageStatus::NoUpdateAvailable
      || m_PackageStatus == PackageStatus::Installed) {
      setOpAppUpdateStatus(OpAppUpdateStatus::SOFTWARE_CURRENT);
      LOG(INFO) << "No new update available or no local package file found";
    }
    return false;
  }

  setOpAppUpdateStatus(OpAppUpdateStatus::SOFTWARE_DOWNLOADING);
  // This is the local equivalent to a download operation.
  // Copy file to working directory and update m_CandidatePackageFile to the new location
  if (!movePackageFileToInstallationDirectory(m_CandidatePackageFile)) {
    setOpAppUpdateStatus(OpAppUpdateStatus::SOFTWARE_DOWNLOAD_FAILED);
    LOG(ERROR) << "Error moving package file to installation directory";
    return false;
  }

  setOpAppUpdateStatus(OpAppUpdateStatus::SOFTWARE_DOWNLOADED);

  m_PackageStatus = installFromPackageFile();
  if (m_PackageStatus != PackageStatus::Installed) {
    setOpAppUpdateStatus(OpAppUpdateStatus::SOFTWARE_INSTALLATION_FAILED);
    return false;
  }
  return true;
}

bool OpAppPackageManager::tryRemoteUpdate()
{
  setOpAppUpdateStatus(OpAppUpdateStatus::SOFTWARE_DISCOVERING);
  m_PackageStatus = doRemotePackageCheck();

  if (m_PackageStatus != PackageStatus::UpdateAvailable) {
    if (m_PackageStatus == PackageStatus::ConfigurationError
      || m_PackageStatus == PackageStatus::DiscoveryFailed) {
      setOpAppUpdateStatus(OpAppUpdateStatus::SOFTWARE_DISCOVERY_FAILED);
      LOG(ERROR) << "Remote update failed: ["
        << static_cast<int>(m_PackageStatus) << "]";
    }
    else if (m_PackageStatus == PackageStatus::Installed
      || m_PackageStatus == PackageStatus::NoUpdateAvailable) {
      setOpAppUpdateStatus(OpAppUpdateStatus::SOFTWARE_CURRENT);
      LOG(INFO) << "No new update available";
    }
    return false;
  }

  setOpAppUpdateStatus(OpAppUpdateStatus::SOFTWARE_DOWNLOADING);

  if (!downloadPackageFile(m_CandidatePackage)) {
    setOpAppUpdateStatus(OpAppUpdateStatus::SOFTWARE_DOWNLOAD_FAILED);
    LOG(ERROR) << "Download failed: " << m_LastErrorMessage;
    return false;
  }

  setOpAppUpdateStatus(OpAppUpdateStatus::SOFTWARE_DOWNLOADED);

  m_PackageStatus = installFromPackageFile();
  if (m_PackageStatus != PackageStatus::Installed) {
    setOpAppUpdateStatus(OpAppUpdateStatus::SOFTWARE_INSTALLATION_FAILED);
    return false;
  }
  return true;
}

OpAppPackageManager::PackageStatus OpAppPackageManager::installFromPackageFile()
{
  std::filesystem::path decryptedFile;
  if (!decryptPackageFile(m_CandidatePackageFile, decryptedFile)) {
    LOG(ERROR) << "Decryption failed: " << m_LastErrorMessage;
    return PackageStatus::DecryptionFailed;
  }

  if (!verifyZipPackage(decryptedFile)) {
    LOG(ERROR) << "Package file verification failed: " << m_LastErrorMessage;
    return PackageStatus::VerificationFailed;
  }

  setOpAppUpdateStatus(OpAppUpdateStatus::SOFTWARE_UNPACKING);

  if (!unzipPackageFile(decryptedFile, m_Configuration.m_DestinationDirectory)) {
    LOG(ERROR) << "Unzip failed: " << m_LastErrorMessage;
    return PackageStatus::UnzipFailed;
  }

  if (!verifyUnzippedPackage(m_Configuration.m_DestinationDirectory)) {
    LOG(ERROR) << "Unzipped package verification failed: " << m_LastErrorMessage;
    return PackageStatus::VerificationFailed;
  }

  if (!installToPersistentStorage(m_Configuration.m_DestinationDirectory)) {
    LOG(ERROR) << "Installation to persistent storage failed: " << m_LastErrorMessage;
    return PackageStatus::UpdateFailed;
  }

  return PackageStatus::Installed;
}

void OpAppPackageManager::checkForUpdates()
{
  // Update and first install is the same operation.
  bool wasInstalled = tryLocalUpdate();
  if (!wasInstalled) {
    // No local file or no update available. Do a full remote check.
    wasInstalled = tryRemoteUpdate();
  }

  if (wasInstalled) {
    LOG(INFO) << "OpApp was successfully installed";
    // Call success callback
    if (m_Configuration.m_OnUpdateSuccess) {
      // Not sure if this is the correct argument to pass to the callback.
      m_Configuration.m_OnUpdateSuccess(m_CandidatePackageFile);
    }

    return;
  }

  if (!m_LastErrorMessage.empty()) {
    LOG(ERROR) << "OpApp installation failed: [" << m_LastErrorMessage << "]";
    // Call failure callback for installation errors
    if (m_Configuration.m_OnUpdateFailure) {
      m_Configuration.m_OnUpdateFailure(m_PackageStatus, m_LastErrorMessage);
    }
    return;
  }

  // Keep the worker thread running by adding a small delay
  // This prevents the thread from exiting immediately
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

void OpAppPackageManager::setOpAppUpdateStatus(OpAppUpdateStatus status)
{
  // FREE-317 TODO Send event as per TS 103 606 Section A.2.2.1
  m_UpdateStatus.store(status);
}

OpAppPackageManager::OpAppUpdateStatus OpAppPackageManager::getOpAppUpdateStatus() const {
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

OpAppPackageManager::PackageStatus OpAppPackageManager::doLocalPackageCheck()
{
  // Check the m_PackageLocation for any new packages
  std::vector<std::filesystem::path> packageFiles;
  int numFiles = searchLocalPackageFiles(packageFiles);

  if (numFiles < 0) {
    // Error occurred (e.g., multiple files found) - m_LastErrorMessage already set
    return PackageStatus::ConfigurationError;
  }

  if (numFiles == 0) {
    return PackageStatus::NoUpdateAvailable;
  }

  // Get the hash of the installed package from the install receipt
  std::string installedPackageHash;
  int result = readJsonField(
    m_Configuration.m_InstallReceiptFilePath, "packageHash", installedPackageHash);

  // We have exactly one package file
  std::filesystem::path packageFile = packageFiles[0];
  m_CandidatePackageFile = packageFile;
  m_CandidatePackageHash = calculateFileSHA256Hash(packageFile);

  if (result == 0) /* Successfully read the install receipt */ {
    if (m_CandidatePackageHash == installedPackageHash) {
      return PackageStatus::Installed;
    }
    // Hashes differ - update available
    return PackageStatus::UpdateAvailable;
  }
  else if (result == -1) {
    LOG(INFO) << "Install receipt file does not exist: " << m_Configuration.m_InstallReceiptFilePath;
  }
  else {
    LOG(ERROR) << "Error reading install receipt file: " << m_Configuration.m_InstallReceiptFilePath;
  }

  return PackageStatus::UpdateAvailable;
}

OpAppPackageManager::PackageStatus OpAppPackageManager::doRemotePackageCheck()
{
  // Check for a remote package file via AIT acquisition.
  // Needs the FQDN passed in. Use the FQDN from the Configuration::m_OpAppFqdn.
  if (m_Configuration.m_OpAppFqdn.empty()) {
    LOG(INFO) << "No OpApp FQDN configured, skipping remote package check";
    return PackageStatus::ConfigurationError;
  }

  // Determine AIT output directory
  std::filesystem::path aitDir = m_Configuration.m_AitOutputDirectory;
  if (aitDir.empty()) {
    aitDir = m_Configuration.m_DestinationDirectory / "ait_cache";
  }

  // Clear the AIT directory before acquisition to remove stale files
  std::error_code ec;
  if (std::filesystem::exists(aitDir)) {
    std::filesystem::remove_all(aitDir, ec);
    if (ec) {
      LOG(WARNING) << "Failed to clear AIT directory: " << aitDir
                   << ", error: " << ec.message();
    }
  }

  // Use the injected AIT fetcher to fetch ALL AIT XMLs
  AitFetchResult result = m_AitFetcher->FetchAitXmls(
      m_Configuration.m_OpAppFqdn, true /* network available */, aitDir.string());

  if (!result.success || result.aitFiles.empty()) {
    std::string error = result.fatalError.empty()
        ? "AIT acquisition failed: no AITs acquired"
        : result.fatalError;
    LOG(ERROR) << "AIT acquisition failed: " << error;
    m_LastErrorMessage = error;
    return PackageStatus::ConfigurationError;
  }

  // Log any non-fatal errors encountered during acquisition
  for (const auto& error : result.errors) {
    LOG(WARNING) << "AIT acquisition warning: " << error;
  }

  // Parse the AIT files (convert string paths to filesystem::path)
  std::vector<std::filesystem::path> aitFilePaths(result.aitFiles.begin(), result.aitFiles.end());
  std::vector<PackageInfo> discoveredPackages;
  if (!parseAitFiles(aitFilePaths, discoveredPackages)) {
    LOG(WARNING) << "No applications found in any AIT: " << m_LastErrorMessage;
    return PackageStatus::DiscoveryFailed;
  }

  // TS103606 Section 4.1.2 Only one privileged OpApp per device - use first valid package
  // While it's possible there may be more than one, we only support one.
  const PackageInfo& pkg = discoveredPackages.front();

  // Check if this package is already installed
  PackageInfo installedPkg;
  if (!loadInstallReceipt(installedPkg)) {
    // No existing installation - this is a first-time install
    LOG(INFO) << "New package available for installation: " << pkg.name
              << " (orgId=" << pkg.orgId << ", appId=" << pkg.appId
              << ", v" << pkg.xmlVersion << ")";
    m_CandidatePackage = pkg;
    return PackageStatus::UpdateAvailable;
  }

  // Existing installation found - check if update available
  // Check if the installed package matches the discovered package
  if (pkg.orgId != installedPkg.orgId || pkg.appId != installedPkg.appId) {
    LOG(INFO) << "Package differs from installed package. Uninstall the existing package.";
    return PackageStatus::Installed;
  }

  // orgId and appId match - check if the package is newer
  if (pkg.isNewerThan(installedPkg)) {
    LOG(INFO) << "Update available for " << pkg.name
              << " (installed v" << installedPkg.xmlVersion
              << " -> v" << pkg.xmlVersion << ")";
    m_CandidatePackage = pkg;
    return PackageStatus::UpdateAvailable;
  }

  LOG(INFO) << "Package " << pkg.name << " is up to date (v" << installedPkg.xmlVersion << ")";
  return PackageStatus::Installed;
}

int OpAppPackageManager::searchLocalPackageFiles(std::vector<std::filesystem::path>& outPackageFiles)
{
  outPackageFiles.clear();

  // Check if the package location directory exists
  if (m_Configuration.m_PackageLocation.empty() ||
      !std::filesystem::exists(m_Configuration.m_PackageLocation)) {
    return 0; // No error, just no files (directory doesn't exist, e.g., no SD card)
  }

  // Package file suffixes to search for
  const std::vector<std::string> packageSuffixes = {".cms", ".zip"};

  // Iterate through files in the package location directory
  for (const auto& entry : std::filesystem::directory_iterator(m_Configuration.m_PackageLocation)) {
    if (entry.is_regular_file()) {
      std::string filename = entry.path().filename().string();
      for (const auto& suffix : packageSuffixes) {
        if (filename.length() >= suffix.length() &&
            filename.substr(filename.length() - suffix.length()) == suffix) {
          outPackageFiles.push_back(entry.path());
          break;
        }
      }
    }
  }

  // Check for multiple package files - this is an error condition
  if (outPackageFiles.size() > 1) {
    m_LastErrorMessage = "Multiple package files found in directory '" +
                         m_Configuration.m_PackageLocation.string() + "'. Expected only one package file. Found: ";
    for (size_t i = 0; i < outPackageFiles.size(); ++i) {
      if (i > 0) m_LastErrorMessage += ", ";
      m_LastErrorMessage += outPackageFiles[i].filename().string();
    }
    return -1;
  }

  return static_cast<int>(outPackageFiles.size());
}

std::string OpAppPackageManager::calculateFileSHA256Hash(const std::filesystem::path& filePath) const
{
  return m_HashCalculator->calculateSHA256Hash(filePath);
}

bool OpAppPackageManager::movePackageFileToInstallationDirectory(const std::filesystem::path& packageFilePath) {
  // Steps to install the package:
  // 1. Ensure the destination directory exists
  if (!std::filesystem::exists(m_Configuration.m_DestinationDirectory)) {
    std::filesystem::create_directories(m_Configuration.m_DestinationDirectory);
  }

  // 2. Copy the package file to the installation directory
  std::filesystem::path workingFilePath = m_Configuration.m_DestinationDirectory / packageFilePath.filename();
  std::error_code errorCode;
  std::filesystem::copy(packageFilePath, workingFilePath, errorCode);
  if (errorCode) {
    LOG(ERROR) << "Error copying package file to working directory: " << errorCode.message();
    return false;
  }

  // 3. Update m_CandidatePackageFile to point to the new location
  m_CandidatePackageFile = workingFilePath;

  LOG(INFO) << "Package file copied to working directory: " << workingFilePath;
  return true;
}

bool OpAppPackageManager::decryptPackageFile(const std::filesystem::path& filePath, std::filesystem::path& outFile)
{
  /* From the OpApp HbbTV spec:
  11.3.4.4 Process for decrypting an application package
  */
  std::string decryptError;
  bool result = m_Decryptor->decrypt(filePath, outFile, decryptError);
  if (!result) {
    m_LastErrorMessage = decryptError;
  }
  return result;
}

bool OpAppPackageManager::verifyZipPackage(const std::filesystem::path& filePath)
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
  (void)filePath;  // Suppress unused warning until implementation
  m_LastErrorMessage = "Verification not yet implemented";
  return false;
}

bool OpAppPackageManager::unzipPackageFile(const std::filesystem::path& inFile, const std::filesystem::path& outPath)
{
  (void)inFile;  // Suppress unused warning until implementation
  (void)outPath;  // Suppress unused warning until implementation
  m_LastErrorMessage = "Unzip not yet implemented";
  return false;
}

bool OpAppPackageManager::verifyUnzippedPackage(const std::filesystem::path& filePath)
{
  (void)filePath;  // Suppress unused warning until implementation
  m_LastErrorMessage = "Unzipped package verification not yet implemented";
  return false;
}

bool OpAppPackageManager::installToPersistentStorage(const std::filesystem::path& filePath)
{
  // TODO: Copy files from filePath to m_Configuration.m_OpAppInstallDirectory/orgId/appId

  m_CandidatePackage.installPath = m_Configuration.m_OpAppInstallDirectory /
      std::to_string(m_CandidatePackage.orgId) / std::to_string(m_CandidatePackage.appId);
  m_CandidatePackage.packageHash = m_CandidatePackageHash;

  // Generate ISO timestamp for installedAt
  auto now = std::chrono::system_clock::now();
  auto time_t_now = std::chrono::system_clock::to_time_t(now);
  std::tm tm_now;
  gmtime_r(&time_t_now, &tm_now);
  std::ostringstream oss;
  oss << std::put_time(&tm_now, "%Y-%m-%dT%H:%M:%SZ");
  m_CandidatePackage.installedAt = oss.str();

  // Save the installation receipt
  if (!saveInstallReceipt(m_CandidatePackage)) {
    return false;
  }

  (void)filePath;  // Suppress unused warning until full implementation
  LOG(INFO) << "Installation receipt saved for package orgId=" << m_CandidatePackage.orgId
            << ", appId=" << m_CandidatePackage.appId;
  return true;
}

bool OpAppPackageManager::saveInstallReceipt(const PackageInfo& pkg)
{
  if (m_Configuration.m_InstallReceiptFilePath.empty()) {
    m_LastErrorMessage = "Install receipt file path not configured";
    return false;
  }

  // Ensure parent directory exists
  std::filesystem::path parentDir = m_Configuration.m_InstallReceiptFilePath.parent_path();
  if (!parentDir.empty() && !std::filesystem::exists(parentDir)) {
    std::error_code ec;
    std::filesystem::create_directories(parentDir, ec);
    if (ec) {
      m_LastErrorMessage = "Failed to create directory for install receipt: " + ec.message();
      return false;
    }
  }

  // Build JSON object with all package info
  Json::Value root;
  root["orgId"] = pkg.orgId;
  root["appId"] = pkg.appId;
  root["xmlVersion"] = pkg.xmlVersion;
  root["name"] = pkg.name;
  root["baseUrl"] = pkg.baseUrl;
  root["location"] = pkg.location;
  root["installPath"] = pkg.installPath.string();
  root["packageHash"] = pkg.packageHash;
  root["installedAt"] = pkg.installedAt;

  // Write to a temporary file first, then rename for atomic write
  std::filesystem::path tempFile = m_Configuration.m_InstallReceiptFilePath;
  tempFile += ".tmp";

  std::ofstream outFile(tempFile);
  if (!outFile) {
    m_LastErrorMessage = "Failed to open install receipt file for writing: " + tempFile.string();
    return false;
  }

  Json::StreamWriterBuilder builder;
  builder["indentation"] = "  ";
  std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
  writer->write(root, &outFile);
  outFile.close();

  if (outFile.fail()) {
    m_LastErrorMessage = "Failed to write install receipt file: " + tempFile.string();
    return false;
  }

  // Atomic rename
  std::error_code ec;
  std::filesystem::rename(tempFile, m_Configuration.m_InstallReceiptFilePath, ec);
  if (ec) {
    m_LastErrorMessage = "Failed to rename install receipt file: " + ec.message();
    std::filesystem::remove(tempFile, ec);  // Clean up temp file
    return false;
  }

  return true;
}

bool OpAppPackageManager::loadInstallReceipt(PackageInfo& outPackage) const
{
  if (m_Configuration.m_InstallReceiptFilePath.empty()) {
    return false;
  }

  if (!std::filesystem::exists(m_Configuration.m_InstallReceiptFilePath)) {
    return false;
  }

  std::ifstream jsonFile(m_Configuration.m_InstallReceiptFilePath);
  if (!jsonFile) {
    LOG(ERROR) << "Failed to open install receipt file: " << m_Configuration.m_InstallReceiptFilePath;
    return false;
  }

  Json::Value root;
  Json::CharReaderBuilder builder;
  std::string errors;
  if (!Json::parseFromStream(builder, jsonFile, &root, &errors)) {
    LOG(ERROR) << "Failed to parse install receipt JSON: " << errors;
    return false;
  }

  // Read fields - support both old format (hash only) and new format (full receipt)
  outPackage.orgId = root.get("orgId", 0).asUInt();
  outPackage.appId = static_cast<uint16_t>(root.get("appId", 0).asUInt());
  outPackage.xmlVersion = root.get("xmlVersion", 0).asUInt();
  outPackage.name = root.get("name", "").asString();
  outPackage.baseUrl = root.get("baseUrl", "").asString();
  outPackage.location = root.get("location", "").asString();
  outPackage.installPath = root.get("installPath", "").asString();
  outPackage.installedAt = root.get("installedAt", "").asString();
  outPackage.packageHash = root["packageHash"].asString();
  return true;
}

bool OpAppPackageManager::validateOpAppDescriptor(const Ait::S_AIT_APP_DESC& app, std::string& outError) const
{
  // Basic validation. See TS 102796 Table 7 and TS 103606 Table 7.
  if ((app.xmlType & Ait::XML_TYP_OPAPP) != Ait::XML_TYP_OPAPP) {
    outError = "Unexpected application type: " + std::to_string(app.xmlType) +
               " expected OPAPP TYPE (0x80 or 0x81)";
    LOG(WARNING) << "AIT validation failed: " << outError;
    return false;
  }

  if (app.appUsage != "urn:hbbtv:opapp:privileged:2017" && app.appUsage != "urn:hbbtv:opapp:specific:2017") {
    outError = "Unexpected application usage: " + app.appUsage +
               " expected 'urn:hbbtv:opapp:privileged:2017' or 'urn:hbbtv:opapp:specific:2017'";
    LOG(WARNING) << "AIT validation failed: " << outError;
    return false;
  }

  LOG(INFO) << "AIT application descriptor has expected application usage: " << app.appUsage;
  // TODO Check against the bilateral agreement on this device.
  // This will require a callback to the moderator to check the bilateral agreement.
  // If it fails then set an error, e.g. "Application not supported by this device."

  if (app.numTransports == 0) {
    outError = "No transport defined for application";
    LOG(WARNING) << "AIT validation failed: " << outError;
    return false;
  }

  if (app.transportArray[0].protocolId != Ait::PROTOCOL_HTTP) {
    outError = "Unexpected transport protocol: " + std::to_string(app.transportArray[0].protocolId) +
               " expected HTTPTransportType (0x3)";
    LOG(WARNING) << "AIT validation failed: " << outError;
    return false;
  }

  // The following are warnings only - we still process the descriptor
  if (app.controlCode != Ait::APP_CTL_AUTOSTART) {
    LOG(WARNING) << "AIT application descriptor has unexpected control code: "
                 << static_cast<int>(app.controlCode) << " expected AUTOSTART (0x1)";
  }

  if (app.appDesc.visibility != Ait::VISIBLE_ALL) {
    LOG(WARNING) << "AIT application descriptor has unexpected visibility: "
                 << static_cast<int>(app.appDesc.visibility) << " expected VISIBLE_ALL (0x3)";
  }

  if (app.appDesc.serviceBound) {
    LOG(WARNING) << "AIT application descriptor has unexpected serviceBound=true, expected false";
  }

  outError.clear();
  return true;
}

bool OpAppPackageManager::parseAitFiles(
    const std::vector<std::filesystem::path>& aitFiles, std::vector<PackageInfo>& packages)
{
  packages.clear();

  if (aitFiles.empty()) {
    m_LastErrorMessage = "No AIT files provided";
    return false;
  }

  std::vector<std::string> errors;

  for (const auto& aitFile : aitFiles) {
    // Read file content
    std::ifstream file(aitFile, std::ios::binary);
    if (!file) {
      std::string msg = "Failed to open AIT file: " + aitFile.string();
      LOG(WARNING) << msg;
      errors.push_back(msg);
      continue;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();

    // Parse the AIT XML
    auto aitTable = m_XmlParser->ParseAit(content.c_str(), content.size());
    if (!aitTable) {
      std::string msg = "Failed to parse AIT file: " + aitFile.string();
      LOG(WARNING) << msg;
      errors.push_back(msg);
      continue;
    }

    LOG(INFO) << "Parsed AIT from " << aitFile << ": "
              << static_cast<int>(aitTable->numApps) << " app(s)";

    if (aitTable->numApps != 1) {
      LOG(WARNING) << "AIT table has " << static_cast<int>(aitTable->numApps)
                   << " application descriptors, expected 1";
    }

    // Process each application descriptor in the AIT table
    for (const auto& app : aitTable->appArray) {
      std::string validationError;
      if (!validateOpAppDescriptor(app, validationError)) {
        errors.push_back(validationError);
        continue;
      }

      // Extract package info from AIT descriptor
      PackageInfo pkgInfo;
      pkgInfo.orgId = app.orgId;
      pkgInfo.appId = app.appId;
      pkgInfo.xmlVersion = app.xmlVersion;
      pkgInfo.baseUrl = app.transportArray[0].url.baseUrl;
      pkgInfo.location = app.location;

      if (app.appName.numLangs > 0) {
        pkgInfo.name = app.appName.names[0].name;
      }

      LOG(INFO) << "  App: orgId=" << pkgInfo.orgId << ", appId=" << pkgInfo.appId
                << ", baseUrl=" << pkgInfo.baseUrl
                << ", xmlVersion=" << pkgInfo.xmlVersion
                << ", location=" << pkgInfo.location
                << ", name=" << pkgInfo.name;

      packages.push_back(pkgInfo);
    }
  }

  if (packages.empty()) {
    m_LastErrorMessage = errors.empty() ? "No valid OpApp descriptors found" :
                         "No valid OpApp descriptors found. Errors: " + errors[0];
    for (size_t i = 1; i < errors.size() && i < 3; ++i) {
      m_LastErrorMessage += "; " + errors[i];
    }
    return false;
  }

  return true;
}

bool OpAppPackageManager::downloadPackageFile(const PackageInfo& packageInfo)
{
  // TS 103 606 V1.2.1 Section 6.1.7 - Package Download
  // - HTTP GET request to download the encrypted application package
  // - User-Agent header per ETSI TS 102 796 Section 7.3.2.4 (set in HttpDownloader constructor)
  // - Reject if Content-Type is not application/vnd.hbbtv.opapp.pkg
  // - Retry: max 3 attempts with random delay between 60-600 seconds between requests
  //   (configurable via Configuration for testing)

  const int maxAttempts = m_Configuration.m_DownloadMaxAttempts;
  const int retryDelayMin = m_Configuration.m_DownloadRetryDelayMinSeconds;
  const int retryDelayMax = m_Configuration.m_DownloadRetryDelayMaxSeconds;
  static constexpr const char* EXPECTED_CONTENT_TYPE = "application/vnd.hbbtv.opapp.pkg";

  std::string downloadUrl = packageInfo.getAppUrl();
  if (downloadUrl.empty()) {
    m_LastErrorMessage = "Package URL is empty";
    LOG(ERROR) << "Package download failed: " << m_LastErrorMessage;
    return false;
  }

  LOG(INFO) << "Starting package download from: " << downloadUrl;

  // Ensure destination directory exists
  if (!std::filesystem::exists(m_Configuration.m_DestinationDirectory)) {
    std::error_code ec;
    std::filesystem::create_directories(m_Configuration.m_DestinationDirectory, ec);
    if (ec) {
      m_LastErrorMessage = "Failed to create destination directory: " + ec.message();
      LOG(ERROR) << "Package download failed: " << m_LastErrorMessage;
      return false;
    }
  }

  // Destination file path for the downloaded package
  std::filesystem::path downloadedFilePath =
      m_Configuration.m_DestinationDirectory / "downloaded_package.cms";

  // Random number generator for retry delay
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> delayDist(retryDelayMin, retryDelayMax);

  std::string lastError;
  for (int attempt = 1; attempt <= maxAttempts; ++attempt) {
    LOG(INFO) << "Download attempt " << attempt << " of " << maxAttempts;

    auto result = m_HttpDownloader->DownloadToFile(downloadUrl, downloadedFilePath);

    if (!result) {
      lastError = "HTTP request failed (network error or timeout)";
      LOG(WARNING) << "Download attempt " << attempt << " failed: " << lastError;
    }
    else if (!result->IsSuccess()) {
      lastError = "HTTP request failed with status code: " + std::to_string(result->GetStatusCode());
      LOG(WARNING) << "Download attempt " << attempt << " failed: " << lastError;
    }
    else {
      // Check Content-Type header (TS 103 606 Section 6.1.7)
      std::string contentType = result->GetContentType();
      if (contentType != EXPECTED_CONTENT_TYPE) {
        lastError = "Invalid Content-Type: '" + contentType +
                    "', expected '" + std::string(EXPECTED_CONTENT_TYPE) + "'";
        LOG(WARNING) << "Download attempt " << attempt << " failed: " << lastError;
        // Remove the downloaded file as it's not valid
        std::error_code ec;
        std::filesystem::remove(downloadedFilePath, ec);
      }
      else {
        // Success!
        LOG(INFO) << "Package downloaded successfully to: " << downloadedFilePath;
        m_CandidatePackageFile = downloadedFilePath;
        m_CandidatePackageHash = calculateFileSHA256Hash(downloadedFilePath);
        return true;
      }
    }

    // If not the last attempt, wait before retrying (skip if delay is 0 for testing)
    if (attempt < maxAttempts && retryDelayMax > 0) {
      int delaySeconds = delayDist(gen);
      LOG(INFO) << "Waiting " << delaySeconds << " seconds before retry...";
      std::this_thread::sleep_for(std::chrono::seconds(delaySeconds));
    }
  }

  // All attempts failed
  m_LastErrorMessage = "Package download failed after " + std::to_string(maxAttempts) +
                       " attempts. Last error: " + lastError;
  LOG(ERROR) << m_LastErrorMessage;
  return false;
}

} // namespace orb
