#include <iostream>
#include <string>
#include <filesystem>
#include <algorithm>
#include <cctype>
#include <map>
#include <sstream>
#include <thread>
#include <chrono>
#include <vector>
#include <iomanip>

#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/orb/orblibrary/include/OpAppPackageManager.h"
#include "third_party/orb/orblibrary/package_manager/AitFetcher.h"
#include "third_party/orb/orblibrary/package_manager/IUnzipper.h"
#include "third_party/orb/orblibrary/common/xml_parser.h"
#include "third_party/orb/orblibrary/network/HttpDownloader.h"
#include "OpAppPackageManagerTestInterface.h"
#include <fstream>

using namespace orb;

class MockDecryptor : public IDecryptor {
public:
  MockDecryptor() = default;

  void setDecryptResult(bool success, const std::string& errorMessage = "", const std::filesystem::path& file = {}) {
    m_DecryptSuccess = success;
    m_DecryptError = errorMessage;
    m_DecryptFile = file;
  }

  bool decrypt(
    const std::filesystem::path& filePath,
    std::filesystem::path& outFile,
    std::string& outError) const override {
    m_WasDecryptCalled = true;
    m_LastFilePath = filePath;
    outFile = m_DecryptFile;
    outError = m_DecryptError;
    return m_DecryptSuccess;
  }

  bool wasDecryptCalled() const {
    return m_WasDecryptCalled;
  }

  void setLastFilePath(const std::filesystem::path& filePath) {
    m_LastFilePath = filePath;
  }

  std::filesystem::path getLastFilePath() const {
    return m_LastFilePath;
  }

  void reset() {
    m_WasDecryptCalled = false;
    m_LastFilePath.clear();
  }

private:
  bool m_DecryptSuccess = true;
  std::string m_DecryptError;
  std::filesystem::path m_DecryptFile;
  mutable bool m_WasDecryptCalled = false;
  mutable std::filesystem::path m_LastFilePath;
};

// Mock AIT fetcher for testing remote package check
class MockAitFetcher : public IAitFetcher {
public:
  MockAitFetcher() = default;

  void setFetchResult(const AitFetchResult& result) {
    m_FetchResult = result;
  }

  // Set file content to write when FetchAitXmls is called
  // Maps filename (not full path) to content
  void setFileContent(const std::string& filename, const std::string& content) {
    m_FileContents[filename] = content;
  }

  AitFetchResult FetchAitXmls(
      const std::string& fqdn,
      bool networkAvailable,
      const std::string& outputDirectory) override {
    m_LastFqdn = fqdn;
    m_LastNetworkAvailable = networkAvailable;
    m_LastOutputDirectory = outputDirectory;
    m_WasFetchCalled = true;

    // Create directory and write files if content was provided
    if (!m_FileContents.empty()) {
      std::filesystem::create_directories(outputDirectory);
      std::vector<std::string> createdFiles;
      for (const auto& [filename, content] : m_FileContents) {
        std::string filePath = outputDirectory + "/" + filename;
        std::ofstream file(filePath);
        file << content;
        file.close();
        createdFiles.push_back(filePath);
      }
      // Return result with actual created file paths
      return AitFetchResult(createdFiles, m_FetchResult.errors);
    }

    return m_FetchResult;
  }

  bool wasFetchCalled() const { return m_WasFetchCalled; }
  std::string getLastFqdn() const { return m_LastFqdn; }
  bool getLastNetworkAvailable() const { return m_LastNetworkAvailable; }
  std::string getLastOutputDirectory() const { return m_LastOutputDirectory; }

  void reset() {
    m_WasFetchCalled = false;
    m_LastFqdn.clear();
    m_LastOutputDirectory.clear();
    m_FileContents.clear();
  }

private:
  AitFetchResult m_FetchResult;
  std::map<std::string, std::string> m_FileContents;
  mutable bool m_WasFetchCalled = false;
  mutable std::string m_LastFqdn;
  mutable bool m_LastNetworkAvailable = false;
  mutable std::string m_LastOutputDirectory;
};

// Mock hash calculator for testing
class MockHashCalculator : public IHashCalculator {
public:
  MockHashCalculator() = default;

  // Create an install receipt JSON file with the specified hash
  void createInstallReceiptFile(const std::filesystem::path& filePath, const std::string& hash) {
    std::ofstream jsonFile(filePath);
    jsonFile << R"({"packageHash": ")" << hash << R"("})";
    jsonFile.close();
  }

  // Create an invalid JSON file (missing packageHash field)
  void createInvalidReceiptFile(const std::filesystem::path& filePath) {
    std::ofstream jsonFile(filePath);
    jsonFile << R"({"version": "1.0", "timestamp": "2024-01-01"})";
    jsonFile.close();
  }

  // Set predefined responses for specific file paths (for direct hash calculation)
  void setHashForFile(const std::filesystem::path& filePath, const std::string& hash) {
    m_FileHashes[filePath.string()] = hash;
  }

  // Set default hash for any file not explicitly set
  void setDefaultHash(const std::string& hash) {
    m_DefaultHash = hash;
  }

  std::string calculateSHA256Hash(const std::filesystem::path& filePath) const override {
    auto it = m_FileHashes.find(filePath.string());
    if (it != m_FileHashes.end()) {
      return it->second;
    }
    return m_DefaultHash;
  }

private:
  std::map<std::string, std::string> m_FileHashes;
  std::string m_DefaultHash = "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855"; // Empty file hash
};

// Mock HTTP downloader for testing package download
class MockHttpDownloader : public IHttpDownloader {
public:
  MockHttpDownloader() = default;

  // Configure the mock to return a successful download
  void setDownloadSuccess(const std::string& content, const std::string& contentType) {
    m_DownloadSuccess = true;
    m_DownloadContent = content;
    m_DownloadContentType = contentType;
    m_DownloadStatusCode = 200;
  }

  // Configure the mock to return an HTTP error
  void setDownloadHttpError(int statusCode) {
    m_DownloadSuccess = true;
    m_DownloadStatusCode = statusCode;
    m_DownloadContent = "";
    m_DownloadContentType = "";
  }

  // Configure the mock to return a network failure (nullptr)
  void setDownloadNetworkFailure() {
    m_DownloadSuccess = false;
  }

  // Configure how many times the mock should fail before succeeding
  void setFailuresBeforeSuccess(int failures, const std::string& content, const std::string& contentType) {
    m_FailuresBeforeSuccess = failures;
    m_DownloadContent = content;
    m_DownloadContentType = contentType;
    m_DownloadStatusCode = 200;
    m_DownloadSuccess = true;
  }

  std::shared_ptr<DownloadedObject> Download(const std::string& url) override {
    m_DownloadCallCount++;
    m_LastDownloadUrl = url;

    // Check if we should fail this attempt
    if (m_FailuresBeforeSuccess > 0 && m_DownloadCallCount <= m_FailuresBeforeSuccess) {
      return nullptr;
    }

    if (!m_DownloadSuccess) {
      return nullptr;
    }

    return std::make_shared<DownloadedObject>(m_DownloadContent, m_DownloadContentType, m_DownloadStatusCode);
  }

  std::shared_ptr<DownloadedObject> DownloadToFile(
      const std::string& url, const std::filesystem::path& outputPath) override {
    m_DownloadCallCount++;
    m_LastDownloadUrl = url;
    m_LastOutputPath = outputPath;

    // Check if we should fail this attempt
    if (m_FailuresBeforeSuccess > 0 && m_DownloadCallCount <= m_FailuresBeforeSuccess) {
      return nullptr;
    }

    if (!m_DownloadSuccess) {
      return nullptr;
    }

    // Write content to the output file if download succeeds
    if (m_DownloadStatusCode >= 200 && m_DownloadStatusCode < 300) {
      std::filesystem::create_directories(outputPath.parent_path());
      std::ofstream outFile(outputPath, std::ios::binary);
      outFile.write(m_DownloadContent.c_str(), m_DownloadContent.size());
      outFile.close();
    }

    return std::make_shared<DownloadedObject>(m_DownloadContent, m_DownloadContentType, m_DownloadStatusCode);
  }

  // Getters for verification
  int getDownloadCallCount() const { return m_DownloadCallCount; }
  std::string getLastDownloadUrl() const { return m_LastDownloadUrl; }
  std::filesystem::path getLastOutputPath() const { return m_LastOutputPath; }

  void reset() {
    m_DownloadCallCount = 0;
    m_LastDownloadUrl.clear();
    m_LastOutputPath.clear();
    m_FailuresBeforeSuccess = 0;
  }

private:
  bool m_DownloadSuccess = true;
  std::string m_DownloadContent;
  std::string m_DownloadContentType;
  int m_DownloadStatusCode = 200;
  int m_FailuresBeforeSuccess = 0;

  mutable int m_DownloadCallCount = 0;
  mutable std::string m_LastDownloadUrl;
  mutable std::filesystem::path m_LastOutputPath;
};

// Mock unzipper for testing
class MockUnzipper : public IUnzipper {
public:
  MockUnzipper() = default;

  void setUnzipResult(bool success, const std::string& errorMessage = "") {
    m_UnzipSuccess = success;
    m_UnzipError = errorMessage;
  }

  // Configure the reported uncompressed size (for size limit testing via metadata)
  void setReportedUncompressedSize(size_t size) {
    m_ReportedUncompressedSize = size;
  }

  // Configure a file to be "inside" the mock ZIP
  void setFileInZip(const std::string& path, const std::vector<uint8_t>& content) {
    m_FilesInZip[path] = content;
  }

  void setFileInZip(const std::string& path, const std::string& content) {
    m_FilesInZip[path] = std::vector<uint8_t>(content.begin(), content.end());
  }

  bool unzip(
      const std::filesystem::path& zipFile,
      const std::filesystem::path& destDir,
      std::string& outError) const override {
    m_WasUnzipCalled = true;
    m_LastZipFile = zipFile;
    m_LastDestDir = destDir;

    if (!m_UnzipSuccess) {
      outError = m_UnzipError;
      return false;
    }

    // Create destination directory
    std::filesystem::create_directories(destDir);
    return true;
  }

  bool getTotalUncompressedSize(
      const std::filesystem::path& zipFile,
      size_t& outSize,
      std::string& outError) const override {
    m_WasGetSizeCalled = true;
    m_LastZipFile = zipFile;

    if (m_ReportedUncompressedSize == SIZE_MAX) {
      outError = "Failed to read ZIP metadata";
      return false;
    }

    outSize = m_ReportedUncompressedSize;
    return true;
  }

  bool readFileFromZip(
      const std::filesystem::path& zipFile,
      const std::string& filePathInZip,
      std::vector<uint8_t>& outContent,
      std::string& outError) const override {
    m_WasReadFileCalled = true;
    m_LastZipFile = zipFile;
    m_LastFilePathInZip = filePathInZip;

    auto it = m_FilesInZip.find(filePathInZip);
    if (it == m_FilesInZip.end()) {
      outError = "File not found in ZIP: " + filePathInZip;
      return false;
    }

    outContent = it->second;
    return true;
  }

  // Getters for verification
  bool wasUnzipCalled() const { return m_WasUnzipCalled; }
  bool wasGetSizeCalled() const { return m_WasGetSizeCalled; }
  bool wasReadFileCalled() const { return m_WasReadFileCalled; }
  std::filesystem::path getLastZipFile() const { return m_LastZipFile; }
  std::filesystem::path getLastDestDir() const { return m_LastDestDir; }
  std::string getLastFilePathInZip() const { return m_LastFilePathInZip; }

  void reset() {
    m_WasUnzipCalled = false;
    m_WasGetSizeCalled = false;
    m_WasReadFileCalled = false;
    m_LastZipFile.clear();
    m_LastDestDir.clear();
    m_LastFilePathInZip.clear();
    m_FilesInZip.clear();
  }

private:
  bool m_UnzipSuccess = true;
  std::string m_UnzipError;
  size_t m_ReportedUncompressedSize = 0;
  std::map<std::string, std::vector<uint8_t>> m_FilesInZip;

  mutable bool m_WasUnzipCalled = false;
  mutable bool m_WasGetSizeCalled = false;
  mutable bool m_WasReadFileCalled = false;
  mutable std::filesystem::path m_LastZipFile;
  mutable std::filesystem::path m_LastDestDir;
  mutable std::string m_LastFilePathInZip;
};

// Helper function to generate valid OpApp AIT XML for testing
static std::string createValidOpAppAitXml(
    uint32_t orgId,
    uint16_t appId,
    const std::string& appName,
    const std::string& baseUrl,
    const std::string& location = "index.html",
    const std::string& controlCode = "AUTOSTART",
    const std::string& appUsage = "urn:hbbtv:opapp:privileged:2017")
{
  std::ostringstream ss;
  ss << R"(<?xml version="1.0" encoding="UTF-8"?>
<mhp:ServiceDiscovery xmlns:mhp="urn:dvb:mhp:2009">
  <mhp:ApplicationDiscovery DomainName="test.example.com">
    <mhp:ApplicationList>
      <mhp:Application>
        <mhp:appName Language="eng">)" << appName << R"(</mhp:appName>
        <mhp:applicationIdentifier>
          <mhp:orgId>)" << orgId << R"(</mhp:orgId>
          <mhp:appId>)" << appId << R"(</mhp:appId>
        </mhp:applicationIdentifier>
        <mhp:applicationDescriptor>
          <mhp:type>
            <mhp:OtherApp>application/vnd.hbbtv.opapp.pkg</mhp:OtherApp>
          </mhp:type>
          <mhp:controlCode>)" << controlCode << R"(</mhp:controlCode>
          <mhp:visibility>VISIBLE_ALL</mhp:visibility>
          <mhp:serviceBound>false</mhp:serviceBound>
          <mhp:priority>1</mhp:priority>
          <mhp:version>1</mhp:version>
        </mhp:applicationDescriptor>
        <mhp:applicationUsageDescriptor>
          <mhp:ApplicationUsage>)" << appUsage << R"(</mhp:ApplicationUsage>
        </mhp:applicationUsageDescriptor>
        <mhp:applicationTransport xsi:type="mhp:HTTPTransportType" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
          <mhp:URLBase>)" << baseUrl << R"(</mhp:URLBase>
        </mhp:applicationTransport>
        <mhp:applicationLocation>)" << location << R"(</mhp:applicationLocation>
      </mhp:Application>
    </mhp:ApplicationList>
  </mhp:ApplicationDiscovery>
</mhp:ServiceDiscovery>)";
  return ss.str();
}

class OpAppPackageManagerTest : public ::testing::Test {

public:
  // Static member declaration
  static std::string PACKAGE_PATH;

protected:
  void SetUp() override {
    // Create test directory structure in temporary directory
    PACKAGE_PATH = std::filesystem::temp_directory_path() / "orb_test_packages";
    std::filesystem::create_directories(PACKAGE_PATH);
  }

  void TearDown() override {
    // Remove test directory structure entirely - handles all cleanup
    std::filesystem::remove_all(PACKAGE_PATH);
  }

  // =========================================================================
  // Helper Methods - Reduce code duplication across tests
  // =========================================================================

  // Create a basic configuration with common defaults
  static OpAppPackageManager::Configuration createBasicConfiguration() {
    OpAppPackageManager::Configuration config;
    config.m_PackageLocation = PACKAGE_PATH;
    return config;
  }

  // Create a configuration with receipt file path configured
  static OpAppPackageManager::Configuration createConfigurationWithReceipt() {
    auto config = createBasicConfiguration();
    config.m_InstallReceiptFilePath = PACKAGE_PATH + "/install_receipt.json";
    return config;
  }

  // Create a configuration with receipt and destination directory
  static OpAppPackageManager::Configuration createConfigurationWithReceiptAndDest() {
    auto config = createConfigurationWithReceipt();
    config.m_WorkingDirectory = PACKAGE_PATH + "/install";
    return config;
  }

  // Create a configuration for download tests with zero retry delays
  static OpAppPackageManager::Configuration createConfigurationForDownloadTests() {
    auto config = createBasicConfiguration();
    config.m_WorkingDirectory = PACKAGE_PATH + "/dest";
    // Set zero delays for fast test execution
    config.m_DownloadRetryDelayMinSeconds = 0;
    config.m_DownloadRetryDelayMaxSeconds = 0;
    return config;
  }

  // Create a package file at the default location
  static std::filesystem::path createPackageFile(const std::string& content = "") {
    std::filesystem::path packagePath = std::filesystem::path(PACKAGE_PATH) / "package.cms";
    std::ofstream file(packagePath);
    if (!content.empty()) {
      file << content;
    }
    file.close();
    return packagePath;
  }

  // Create a package file with a specific name
  static std::filesystem::path createPackageFile(const std::string& filename, const std::string& content) {
    std::filesystem::path packagePath = std::filesystem::path(PACKAGE_PATH) / filename;
    std::ofstream file(packagePath);
    if (!content.empty()) {
      file << content;
    }
    file.close();
    return packagePath;
  }

  // Create a valid receipt file with specified org/app IDs and hash
  static void createReceiptFile(
      const std::filesystem::path& path,
      uint32_t orgId = 12345,
      uint16_t appId = 100,
      const std::string& packageHash = "abc123",
      uint32_t xmlVersion = 1,
      const std::string& name = "Test OpApp") {
    std::ofstream receiptFile(path);
    receiptFile << R"({
    "orgId": )" << orgId << R"(,
    "appId": )" << appId << R"(,
    "xmlVersion": )" << xmlVersion << R"(,
    "name": ")" << name << R"(",
    "packageHash": ")" << packageHash << R"(",
    "installedAt": "2026-01-01T00:00:00Z"
  })";
    receiptFile.close();
  }

  // Create an invalid (non-JSON) receipt file
  static void createInvalidReceiptFile(const std::filesystem::path& path) {
    std::ofstream receiptFile(path);
    receiptFile << "this is not valid json content";
    receiptFile.close();
  }

  // Create an empty receipt file
  static void createEmptyReceiptFile(const std::filesystem::path& path) {
    std::ofstream receiptFile(path);
    receiptFile.close();
  }

  // Create a test file with specified content
  static std::filesystem::path createTestFile(const std::string& filename, const std::string& content = "") {
    std::filesystem::path filePath = std::filesystem::path(PACKAGE_PATH) / filename;
    std::ofstream file(filePath);
    if (!content.empty()) {
      file << content;
    }
    file.close();
    return filePath;
  }

  // Create a test directory
  static std::filesystem::path createTestDirectory(const std::string& dirname) {
    std::filesystem::path dirPath = std::filesystem::path(PACKAGE_PATH) / dirname;
    std::filesystem::create_directories(dirPath);
    return dirPath;
  }

  // Create an AIT XML file at the specified path
  static std::filesystem::path createAitXmlFile(
      const std::string& filename,
      uint32_t orgId,
      uint16_t appId,
      const std::string& appName,
      const std::string& baseUrl) {
    std::filesystem::path aitPath = std::filesystem::path(PACKAGE_PATH) / filename;
    std::ofstream aitFile(aitPath);
    aitFile << createValidOpAppAitXml(orgId, appId, appName, baseUrl);
    aitFile.close();
    return aitPath;
  }
};

// Static member definition
std::string OpAppPackageManagerTest::PACKAGE_PATH;

TEST_F(OpAppPackageManagerTest, TestDefaultInitialization)
{
  // GIVEN: a configuration object
  auto configuration = createBasicConfiguration();

  // WHEN: creating instance with configuration
  OpAppPackageManager packageManager(configuration);

  // THEN: the instance should be in a valid initial state
  EXPECT_FALSE(packageManager.isRunning());
}

TEST_F(OpAppPackageManagerTest, TestConfigurationInitialization)
{
  // GIVEN: a configuration object with custom values
  auto configuration = createBasicConfiguration();
  configuration.m_PrivateKeyFilePath = "/keys/private.key";
  configuration.m_PublicKeyFilePath = "/keys/public.key";
  configuration.m_CertificateFilePath = "/certs/cert.pem";
  configuration.m_WorkingDirectory = "/dest";
  configuration.m_OpAppInstallDirectory = "/install";

  // WHEN: creating instance with custom configuration
  OpAppPackageManager packageManager(configuration);

  // THEN: the instance should be created successfully
  EXPECT_FALSE(packageManager.isRunning());
}

TEST_F(OpAppPackageManagerTest, TestStartAndStop)
{
  // GIVEN: an OpAppPackageManager instance with no package file
  auto configuration = createBasicConfiguration();
  OpAppPackageManager packageManager(configuration);

  // WHEN: starting the package manager
  packageManager.start();

  // THEN: the package manager should be running
  EXPECT_TRUE(packageManager.isRunning());

  // Wait until the worker thread completes (max 1 second)
  int maxWaitTime = 1000;
  int waitTime = 0;
  while (packageManager.isRunning() && waitTime < maxWaitTime) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    waitTime += 10;
  }

  // THEN: the package manager should be stopped
  EXPECT_FALSE(packageManager.isRunning());

  // Ensure proper cleanup by waiting for thread completion
  packageManager.stop();
}

TEST_F(OpAppPackageManagerTest, TestCheckForUpdates_NoUpdates)
{
  // GIVEN: a test interface instance and no package file
  auto configuration = createBasicConfiguration();
  auto testInterface = OpAppPackageManagerTestInterface::create(configuration);

  // WHEN: doLocalPackageCheck is called
  OpAppPackageManager::PackageStatus status = testInterface->doLocalPackageCheck();

  // THEN: the package status is NoUpdateAvailable (no package files found)
  EXPECT_EQ(status, OpAppPackageManager::PackageStatus::NoUpdateAvailable);
}

TEST_F(OpAppPackageManagerTest, TestCheckForUpdates_UpdatesAvailable)
{
  // GIVEN: a test interface instance and a package file
  auto configuration = createBasicConfiguration();
  createPackageFile();
  auto testInterface = OpAppPackageManagerTestInterface::create(configuration);

  // WHEN: doLocalPackageCheck is called
  OpAppPackageManager::PackageStatus status = testInterface->doLocalPackageCheck();

  // THEN: the package status is UpdateAvailable
  EXPECT_EQ(status, OpAppPackageManager::PackageStatus::UpdateAvailable);
}

TEST_F(OpAppPackageManagerTest, TestCheckForUpdates_UpdatesAvailable_NoExistingPackage)
{
  // GIVEN: a test interface instance with receipt path but no receipt file
  auto configuration = createConfigurationWithReceipt();
  createPackageFile();
  auto testInterface = OpAppPackageManagerTestInterface::create(configuration);

  // WHEN: doLocalPackageCheck is called
  OpAppPackageManager::PackageStatus status = testInterface->doLocalPackageCheck();

  // THEN: the package status is UpdateAvailable
  EXPECT_EQ(status, OpAppPackageManager::PackageStatus::UpdateAvailable);
}

TEST_F(OpAppPackageManagerTest, TestCheckForUpdates_UpdatesAvailable_ExistingPackage_HashSame)
{
  // GIVEN: a package manager and a mock hash calculator with identical hashes
  auto mockCalculator = std::make_unique<MockHashCalculator>();
  const std::string hash = "test_hash_1234567890abcdef";
  mockCalculator->setHashForFile(PACKAGE_PATH + "/package.cms", hash);
  mockCalculator->createInstallReceiptFile(PACKAGE_PATH + "/install_receipt.json", hash);

  auto configuration = createConfigurationWithReceipt();
  createPackageFile();

  // WHEN: checking package status
  OpAppPackageManager::Dependencies deps;
  deps.hashCalculator = std::move(mockCalculator);
  auto testInterface = OpAppPackageManagerTestInterface::create(configuration, std::move(deps));
  OpAppPackageManager::PackageStatus status = testInterface->doLocalPackageCheck();

  // THEN: the package status is Installed
  EXPECT_EQ(status, OpAppPackageManager::PackageStatus::Installed);
}

TEST_F(OpAppPackageManagerTest, TestCheckForUpdates_UpdatesAvailable_NoReceiptFile)
{
  // GIVEN: a package manager and a mock hash calculator, but no receipt file exists
  auto mockCalculator = std::make_unique<MockHashCalculator>();
  mockCalculator->setHashForFile(PACKAGE_PATH + "/package.cms", "package_hash_abcdef123456");

  auto configuration = createConfigurationWithReceipt();
  createPackageFile();

  // WHEN: checking package status
  OpAppPackageManager::Dependencies deps;
  deps.hashCalculator = std::move(mockCalculator);
  auto testInterface = OpAppPackageManagerTestInterface::create(configuration, std::move(deps));
  OpAppPackageManager::PackageStatus status = testInterface->doLocalPackageCheck();

  // THEN: the package should be considered update available (no receipt means not installed)
  EXPECT_EQ(status, OpAppPackageManager::PackageStatus::UpdateAvailable);
}

TEST_F(OpAppPackageManagerTest, TestCheckForUpdates_UpdatesAvailable_InvalidReceiptFile)
{
  // GIVEN: a package manager and a mock hash calculator with an invalid JSON receipt file
  auto mockCalculator = std::make_unique<MockHashCalculator>();
  mockCalculator->setHashForFile(PACKAGE_PATH + "/package.cms", "package_hash_abcdef123456");
  mockCalculator->createInvalidReceiptFile(PACKAGE_PATH + "/install_receipt.json");

  auto configuration = createConfigurationWithReceipt();
  createPackageFile();

  // WHEN: checking package status
  OpAppPackageManager::Dependencies deps;
  deps.hashCalculator = std::move(mockCalculator);
  auto testInterface = OpAppPackageManagerTestInterface::create(configuration, std::move(deps));
  OpAppPackageManager::PackageStatus status = testInterface->doLocalPackageCheck();

  // THEN: the package should be considered update available (invalid receipt means not installed)
  EXPECT_EQ(status, OpAppPackageManager::PackageStatus::UpdateAvailable);
}

TEST_F(OpAppPackageManagerTest, TestCheckForUpdates_UpdatesAvailable_DifferentHash)
{
  // GIVEN: a package manager and a mock hash calculator with different hashes
  auto mockCalculator = std::make_unique<MockHashCalculator>();
  mockCalculator->setHashForFile(PACKAGE_PATH + "/package.cms", "package_hash_abcdef123456");
  mockCalculator->createInstallReceiptFile(PACKAGE_PATH + "/install_receipt.json", "different_hash_789xyz");

  auto configuration = createConfigurationWithReceipt();
  createPackageFile();

  // WHEN: checking package status
  OpAppPackageManager::Dependencies deps;
  deps.hashCalculator = std::move(mockCalculator);
  auto testInterface = OpAppPackageManagerTestInterface::create(configuration, std::move(deps));
  OpAppPackageManager::PackageStatus status = testInterface->doLocalPackageCheck();

  // THEN: the package should be considered update available (different hashes)
  EXPECT_EQ(status, OpAppPackageManager::PackageStatus::UpdateAvailable);
}

TEST_F(OpAppPackageManagerTest, TestInstallFromPackageFile_DecryptCalled)
{
  // GIVEN: an OpAppPackageManager instance and a package file that exists
  auto configuration = createConfigurationWithReceiptAndDest();
  auto packagePath = createPackageFile("test package content");

  // Create a mock decryptor that returns success with package files
  auto mockDecryptor = std::make_unique<MockDecryptor>();
  auto mockHashCalculator = std::make_unique<MockHashCalculator>();
  std::filesystem::path decryptedFile = PACKAGE_PATH + "/decrypted_package.cms";
  mockDecryptor->setDecryptResult(true, "", decryptedFile);

  // Store a reference to the mock decryptor before moving it
  MockDecryptor* mockDecryptorPtr = mockDecryptor.get();

  OpAppPackageManager::Dependencies deps;
  deps.hashCalculator = std::move(mockHashCalculator);
  deps.decryptor = std::move(mockDecryptor);
  auto testInterface = OpAppPackageManagerTestInterface::create(configuration, std::move(deps));
  testInterface->setCandidatePackageFile(packagePath);

  // WHEN: attempting to install a package
  testInterface->installFromPackageFile();

  // THEN: the decrypt method should be called
  EXPECT_TRUE(mockDecryptorPtr->wasDecryptCalled());
}

TEST_F(OpAppPackageManagerTest, TestInstallFromPackageFile_DecryptFailed)
{
  // GIVEN: an OpAppPackageManager instance and a package file that exists
  auto configuration = createConfigurationWithReceiptAndDest();
  auto packagePath = createPackageFile("test package content");

  // Create a mock decryptor that returns a failure
  auto mockDecryptor = std::make_unique<MockDecryptor>();
  auto mockHashCalculator = std::make_unique<MockHashCalculator>();
  mockDecryptor->setDecryptResult(false, "Decryption failed");

  OpAppPackageManager::Dependencies deps;
  deps.hashCalculator = std::move(mockHashCalculator);
  deps.decryptor = std::move(mockDecryptor);
  auto testInterface = OpAppPackageManagerTestInterface::create(configuration, std::move(deps));
  testInterface->setCandidatePackageFile(packagePath);

  // WHEN: attempting to install a package directly
  OpAppPackageManager::PackageStatus status = testInterface->installFromPackageFile();

  // THEN: the installation should fail with DecryptionFailed status
  EXPECT_EQ(status, OpAppPackageManager::PackageStatus::DecryptionFailed);
}

TEST_F(OpAppPackageManagerTest, TestTryLocalUpdate_DecryptFailed_CallsFailureCallback)
{
  // GIVEN: an OpAppPackageManager instance with a package file that will fail decryption
  auto configuration = createConfigurationWithReceiptAndDest();
  std::filesystem::create_directories(configuration.m_WorkingDirectory);
  auto packagePath = createPackageFile("test package content");

  // Set up callbacks to verify the failure is reported correctly
  bool failureCallbackCalled = false;
  std::string failureErrorMessage;
  configuration.m_OnUpdateFailure = [&](OpAppPackageManager::PackageStatus, const std::string& errorMessage) {
    failureCallbackCalled = true;
    failureErrorMessage = errorMessage;
  };

  // Create a mock decryptor that returns a failure
  auto mockDecryptor = std::make_unique<MockDecryptor>();
  auto mockHashCalculator = std::make_unique<MockHashCalculator>();
  mockDecryptor->setDecryptResult(false, "Decryption failed");

  // Set up hash calculator to indicate update available (different hashes)
  mockHashCalculator->setHashForFile(packagePath.string(), "new_hash_abc123");
  mockHashCalculator->createInstallReceiptFile(PACKAGE_PATH + "/install_receipt.json", "old_hash_xyz789");

  OpAppPackageManager::Dependencies deps;
  deps.hashCalculator = std::move(mockHashCalculator);
  deps.decryptor = std::move(mockDecryptor);
  auto testInterface = OpAppPackageManagerTestInterface::create(configuration, std::move(deps));

  // WHEN: checking for updates (which runs asynchronously)
  testInterface->checkForUpdates();

  // Wait for async operation to complete
  std::this_thread::sleep_for(std::chrono::milliseconds(200));

  // THEN: the failure callback should be called with the decryption error message
  EXPECT_TRUE(failureCallbackCalled);
  EXPECT_NE(failureErrorMessage.find("Decryption failed"), std::string::npos);
}

TEST_F(OpAppPackageManagerTest, TestInstallFromPackageFile_ReturnsCorrectPackageStatus)
{
  // GIVEN: an OpAppPackageManager instance
  auto configuration = createConfigurationWithReceiptAndDest();
  auto packagePath = createPackageFile("test package content");

  // Create a mock decryptor that returns success with a decrypted file
  auto mockDecryptor = std::make_unique<MockDecryptor>();
  auto mockHashCalculator = std::make_unique<MockHashCalculator>();
  std::filesystem::path decryptedFile = PACKAGE_PATH + "/decrypted_package.zip";
  mockDecryptor->setDecryptResult(true, "", decryptedFile);

  OpAppPackageManager::Dependencies deps;
  deps.hashCalculator = std::move(mockHashCalculator);
  deps.decryptor = std::move(mockDecryptor);
  auto testInterface = OpAppPackageManagerTestInterface::create(configuration, std::move(deps));
  testInterface->setCandidatePackageFile(packagePath);

  // WHEN: attempting to install a package (decryption succeeds but verification will fail)
  OpAppPackageManager::PackageStatus status = testInterface->installFromPackageFile();

  // THEN: the installation should fail at verification (since we don't have a valid zip)
  EXPECT_NE(status, OpAppPackageManager::PackageStatus::Installed);
}

// =============================================================================
// Direct Method Tests
// =============================================================================

TEST_F(OpAppPackageManagerTest, TestMovePackageFileToInstallationDirectory_Success)
{
  // GIVEN: an OpAppPackageManager instance with a package file
  auto configuration = createBasicConfiguration();
  configuration.m_WorkingDirectory = PACKAGE_PATH + "/install";
  auto packagePath = createPackageFile("test package content");

  auto testInterface = OpAppPackageManagerTestInterface::create(configuration);

  // WHEN: moving the package file to installation directory
  bool result = testInterface->movePackageFileToInstallationDirectory(packagePath);

  // THEN: the operation should succeed
  EXPECT_TRUE(result);

  // AND: the candidate package file should be updated to the new location
  std::filesystem::path expectedPath = std::filesystem::path(PACKAGE_PATH) / "install" / "package.cms";
  EXPECT_EQ(testInterface->getCandidatePackageFile(), expectedPath);

  // AND: the file should exist at the new location
  EXPECT_TRUE(std::filesystem::exists(expectedPath));
}

TEST_F(OpAppPackageManagerTest, TestMovePackageFileToInstallationDirectory_CreatesDirectory)
{
  // GIVEN: an OpAppPackageManager instance with a destination directory that doesn't exist
  auto configuration = createBasicConfiguration();
  configuration.m_WorkingDirectory = PACKAGE_PATH + "/nonexistent/install";
  auto packagePath = createPackageFile("test package content");

  auto testInterface = OpAppPackageManagerTestInterface::create(configuration);

  // WHEN: moving the package file to installation directory
  bool result = testInterface->movePackageFileToInstallationDirectory(packagePath);

  // THEN: the operation should succeed (directory created automatically)
  EXPECT_TRUE(result);

  // AND: the destination directory should exist
  EXPECT_TRUE(std::filesystem::exists(PACKAGE_PATH + "/nonexistent/install"));
}

TEST_F(OpAppPackageManagerTest, TestMovePackageFileToInstallationDirectory_NonexistentSource)
{
  // GIVEN: an OpAppPackageManager instance with a nonexistent source file
  auto configuration = createBasicConfiguration();
  configuration.m_WorkingDirectory = PACKAGE_PATH + "/install";

  auto testInterface = OpAppPackageManagerTestInterface::create(configuration);

  // WHEN: moving a nonexistent package file
  std::filesystem::path nonexistentPath = std::filesystem::path(PACKAGE_PATH) / "nonexistent.cms";
  bool result = testInterface->movePackageFileToInstallationDirectory(nonexistentPath);

  // THEN: the operation should fail
  EXPECT_FALSE(result);
}

TEST_F(OpAppPackageManagerTest, TestVerifyZipPackage_Success)
{
  // GIVEN: an OpAppPackageManager instance with max size limit and mock unzipper
  auto configuration = createBasicConfiguration();
  configuration.m_MaxUnzippedPackageSize = 50 * 1024 * 1024; // 50 MB
  auto testFile = createTestFile("test.zip", "test content");

  auto mockUnzipper = std::make_unique<MockUnzipper>();
  mockUnzipper->setReportedUncompressedSize(1024 * 1024); // 1 MB - under limit
  MockUnzipper* mockUnzipperPtr = mockUnzipper.get();

  OpAppPackageManager::Dependencies deps;
  deps.unzipper = std::move(mockUnzipper);

  auto testInterface = OpAppPackageManagerTestInterface::create(configuration, std::move(deps));

  // WHEN: verifying a zip package
  bool result = testInterface->verifyZipPackage(testFile);

  // THEN: the verification should succeed
  EXPECT_TRUE(result);
  EXPECT_TRUE(mockUnzipperPtr->wasGetSizeCalled());
}

TEST_F(OpAppPackageManagerTest, TestVerifyZipPackage_ExceedsMaxSize)
{
  // GIVEN: an OpAppPackageManager instance with max size limit
  auto configuration = createBasicConfiguration();
  configuration.m_MaxUnzippedPackageSize = 1024; // 1 KB limit
  auto testFile = createTestFile("test.zip", "test content");

  auto mockUnzipper = std::make_unique<MockUnzipper>();
  mockUnzipper->setReportedUncompressedSize(2048); // 2 KB - exceeds limit
  MockUnzipper* mockUnzipperPtr = mockUnzipper.get();

  OpAppPackageManager::Dependencies deps;
  deps.unzipper = std::move(mockUnzipper);

  auto testInterface = OpAppPackageManagerTestInterface::create(configuration, std::move(deps));

  // WHEN: verifying a zip package that exceeds max size
  bool result = testInterface->verifyZipPackage(testFile);

  // THEN: the verification should fail due to size limit
  EXPECT_FALSE(result);
  EXPECT_TRUE(mockUnzipperPtr->wasGetSizeCalled());
  EXPECT_TRUE(testInterface->getLastErrorMessage().find("exceeds maximum") != std::string::npos);
}

TEST_F(OpAppPackageManagerTest, TestVerifyZipPackage_NoSizeLimit)
{
  // GIVEN: an OpAppPackageManager instance with no size limit configured
  auto configuration = createBasicConfiguration();
  configuration.m_MaxUnzippedPackageSize = 0; // No limit
  auto testFile = createTestFile("test.zip", "test content");

  auto mockUnzipper = std::make_unique<MockUnzipper>();
  MockUnzipper* mockUnzipperPtr = mockUnzipper.get();

  OpAppPackageManager::Dependencies deps;
  deps.unzipper = std::move(mockUnzipper);

  auto testInterface = OpAppPackageManagerTestInterface::create(configuration, std::move(deps));

  // WHEN: verifying a zip package
  bool result = testInterface->verifyZipPackage(testFile);

  // THEN: the verification should succeed without checking size
  EXPECT_TRUE(result);
  EXPECT_FALSE(mockUnzipperPtr->wasGetSizeCalled()); // Size check skipped when limit is 0
}

TEST_F(OpAppPackageManagerTest, TestUnzipPackageFile_Success)
{
  // GIVEN: an OpAppPackageManager instance with a mock unzipper
  // Note: Size validation is now done in verifyZipPackage() before unzip
  auto configuration = createBasicConfiguration();
  configuration.m_WorkingDirectory = PACKAGE_PATH + "/work";
  auto testFile = createTestFile("test.zip", "test content");
  std::filesystem::path outPath = PACKAGE_PATH + "/work/100/12345";

  auto mockUnzipper = std::make_unique<MockUnzipper>();
  mockUnzipper->setUnzipResult(true);
  MockUnzipper* mockUnzipperPtr = mockUnzipper.get();

  OpAppPackageManager::Dependencies deps;
  deps.unzipper = std::move(mockUnzipper);

  auto testInterface = OpAppPackageManagerTestInterface::create(configuration, std::move(deps));

  // WHEN: unzipping a package file
  bool result = testInterface->unzipPackageFile(testFile, outPath);

  // THEN: the unzip should succeed
  EXPECT_TRUE(result);
  EXPECT_TRUE(mockUnzipperPtr->wasUnzipCalled());
  EXPECT_EQ(mockUnzipperPtr->getLastZipFile(), testFile);
  EXPECT_EQ(mockUnzipperPtr->getLastDestDir(), outPath);
}

TEST_F(OpAppPackageManagerTest, TestUnzipPackageFile_Failure)
{
  // GIVEN: an OpAppPackageManager instance with a mock unzipper that fails
  auto configuration = createBasicConfiguration();
  configuration.m_WorkingDirectory = PACKAGE_PATH + "/work";
  auto testFile = createTestFile("test.zip", "test content");
  std::filesystem::path outPath = PACKAGE_PATH + "/work/unzipped";

  auto mockUnzipper = std::make_unique<MockUnzipper>();
  mockUnzipper->setUnzipResult(false, "Mock unzip error");

  OpAppPackageManager::Dependencies deps;
  deps.unzipper = std::move(mockUnzipper);

  auto testInterface = OpAppPackageManagerTestInterface::create(configuration, std::move(deps));

  // WHEN: unzipping a package file that fails
  bool result = testInterface->unzipPackageFile(testFile, outPath);

  // THEN: the unzip should fail
  EXPECT_FALSE(result);
  EXPECT_EQ(testInterface->getLastErrorMessage(), "Mock unzip error");
}

// Note: Size limit checking is now done in verifyZipPackage() using ZIP metadata,
// before extraction. See TestVerifyZipPackage_ExceedsMaxSize test above.

TEST_F(OpAppPackageManagerTest, TestInstallToPersistentStorage_SavesReceipt)
{
  // GIVEN: an OpAppPackageManager instance with a candidate package set
  auto configuration = createConfigurationWithReceipt();
  configuration.m_WorkingDirectory = PACKAGE_PATH + "/working";
  configuration.m_OpAppInstallDirectory = PACKAGE_PATH + "/opapps";

  // Set up candidate package info (needed to determine source/dest paths)
  PackageInfo candidatePkg;
  candidatePkg.orgId = 12345;
  candidatePkg.appId = 100;
  candidatePkg.xmlVersion = 2;
  candidatePkg.name = "Test OpApp";
  candidatePkg.baseUrl = "https://test.example.com/";
  candidatePkg.location = "index.html";

  // Create the source directory structure: <working>/<appId>/<orgId>/
  std::filesystem::path srcDir = configuration.m_WorkingDirectory /
      std::to_string(candidatePkg.appId) / std::to_string(candidatePkg.orgId);
  std::filesystem::create_directories(srcDir);

  // Create some test files in the source directory
  std::ofstream(srcDir / "index.html") << "<html><body>Test App</body></html>";
  std::ofstream(srcDir / "app.js") << "console.log('hello');";

  auto testInterface = OpAppPackageManagerTestInterface::create(configuration);
  testInterface->setCandidatePackage(candidatePkg);
  testInterface->setCandidatePackageHash("abc123def456");

  // WHEN: installing to persistent storage (pass working directory root)
  bool result = testInterface->installToPersistentStorage(configuration.m_WorkingDirectory);

  // THEN: the install should succeed and receipt should be saved
  EXPECT_TRUE(result);
  EXPECT_TRUE(std::filesystem::exists(configuration.m_InstallReceiptFilePath));

  // AND: files should be copied to the install directory
  std::filesystem::path destDir = configuration.m_OpAppInstallDirectory /
      std::to_string(candidatePkg.appId) / std::to_string(candidatePkg.orgId);
  EXPECT_TRUE(std::filesystem::exists(destDir / "index.html"));
  EXPECT_TRUE(std::filesystem::exists(destDir / "app.js"));

  // AND: source directory should be removed (moved, not copied)
  EXPECT_FALSE(std::filesystem::exists(srcDir));

  // AND: the receipt should contain the correct data
  PackageInfo loadedPkg;
  EXPECT_TRUE(testInterface->loadInstallReceipt(loadedPkg));
  EXPECT_EQ(loadedPkg.orgId, uint32_t(12345));
  EXPECT_EQ(loadedPkg.appId, uint16_t(100));
  EXPECT_EQ(loadedPkg.xmlVersion, uint32_t(2));
  EXPECT_EQ(loadedPkg.name, "Test OpApp");
  EXPECT_EQ(loadedPkg.packageHash, "abc123def456");
  EXPECT_EQ(loadedPkg.installPath, destDir);
  EXPECT_FALSE(loadedPkg.installedAt.empty());
  // installedUrl: appId=100 (0x64), orgId=12345 (0x3039)
  EXPECT_EQ(loadedPkg.installedUrl, "hbbtv-package://64.3039");
}

TEST_F(OpAppPackageManagerTest, TestInstallToPersistentStorage_FailsWhenSourceMissing)
{
  // GIVEN: an OpAppPackageManager with candidate package but no source directory
  auto configuration = createConfigurationWithReceipt();
  configuration.m_WorkingDirectory = PACKAGE_PATH + "/working";
  configuration.m_OpAppInstallDirectory = PACKAGE_PATH + "/opapps";

  PackageInfo candidatePkg;
  candidatePkg.orgId = 12345;
  candidatePkg.appId = 100;

  auto testInterface = OpAppPackageManagerTestInterface::create(configuration);
  testInterface->setCandidatePackage(candidatePkg);

  // Note: source directory <working>/100/12345 is NOT created

  // WHEN: installing to persistent storage
  bool result = testInterface->installToPersistentStorage(configuration.m_WorkingDirectory);

  // THEN: the install should fail with appropriate error
  EXPECT_FALSE(result);
  EXPECT_TRUE(testInterface->getLastErrorMessage().find("Source directory does not exist") != std::string::npos);
}

TEST_F(OpAppPackageManagerTest, TestSaveInstallReceipt_CreatesDirectory)
{
  // GIVEN: an OpAppPackageManager with a receipt path in a non-existent directory
  auto configuration = createBasicConfiguration();
  configuration.m_InstallReceiptFilePath = PACKAGE_PATH + "/newdir/subdir/install_receipt.json";

  auto testInterface = OpAppPackageManagerTestInterface::create(configuration);

  // Set up a package to save
  PackageInfo pkg;
  pkg.orgId = 99999;
  pkg.appId = 1;
  pkg.xmlVersion = 1;
  pkg.name = "Directory Test App";
  pkg.packageHash = "hash123";
  pkg.installedAt = "2026-01-09T12:00:00Z";

  // WHEN: saving the install receipt
  bool result = testInterface->saveInstallReceipt(pkg);

  // THEN: the directory should be created and receipt saved
  EXPECT_TRUE(result);
  EXPECT_TRUE(std::filesystem::exists(configuration.m_InstallReceiptFilePath));
}

TEST_F(OpAppPackageManagerTest, TestLoadInstallReceipt_ReturnsPackageInfo)
{
  // GIVEN: an existing install receipt JSON file
  auto configuration = createBasicConfiguration();
  configuration.m_InstallReceiptFilePath = PACKAGE_PATH + "/existing_receipt.json";

  // Create a detailed receipt file
  std::ofstream receiptFile(configuration.m_InstallReceiptFilePath);
  receiptFile << R"({
    "orgId": 54321,
    "appId": 200,
    "xmlVersion": 5,
    "name": "Existing App",
    "baseUrl": "https://existing.example.com/",
    "location": "app.html",
    "installPath": "/opt/orb/opapps/54321/200",
    "packageHash": "existinghash789",
    "installedAt": "2025-06-15T10:30:00Z"
  })";
  receiptFile.close();

  auto testInterface = OpAppPackageManagerTestInterface::create(configuration);

  // WHEN: loading the install receipt
  PackageInfo loadedPkg;
  bool result = testInterface->loadInstallReceipt(loadedPkg);

  // THEN: the package info should be loaded correctly
  EXPECT_TRUE(result);
  EXPECT_EQ(loadedPkg.orgId, uint32_t(54321));
  EXPECT_EQ(loadedPkg.appId, uint16_t(200));
  EXPECT_EQ(loadedPkg.xmlVersion, uint32_t(5));
  EXPECT_EQ(loadedPkg.name, "Existing App");
  EXPECT_EQ(loadedPkg.baseUrl, "https://existing.example.com/");
  EXPECT_EQ(loadedPkg.location, "app.html");
  EXPECT_EQ(loadedPkg.installPath, "/opt/orb/opapps/54321/200");
  EXPECT_EQ(loadedPkg.packageHash, "existinghash789");
  EXPECT_EQ(loadedPkg.installedAt, "2025-06-15T10:30:00Z");
}

TEST_F(OpAppPackageManagerTest, TestLoadInstallReceipt_FileNotFound)
{
  // GIVEN: a configuration with a non-existent receipt file
  auto configuration = createBasicConfiguration();
  configuration.m_InstallReceiptFilePath = PACKAGE_PATH + "/nonexistent_receipt.json";

  auto testInterface = OpAppPackageManagerTestInterface::create(configuration);

  // WHEN: loading the install receipt
  PackageInfo loadedPkg;
  bool result = testInterface->loadInstallReceipt(loadedPkg);

  // THEN: should return false (file not found)
  EXPECT_FALSE(result);
}

TEST_F(OpAppPackageManagerTest, TestLoadInstallReceipt_CanCheckOrgAppId)
{
  // GIVEN: an existing install receipt for a specific org/app ID
  auto configuration = createConfigurationWithReceipt();
  createReceiptFile(configuration.m_InstallReceiptFilePath, 12345, 100, "installedhash", 3, "Installed App");

  auto testInterface = OpAppPackageManagerTestInterface::create(configuration);

  // WHEN: loading the receipt and checking org/app IDs
  PackageInfo outPkg;
  bool result = testInterface->loadInstallReceipt(outPkg);

  // THEN: should load successfully and have correct org/app IDs
  EXPECT_TRUE(result);
  EXPECT_EQ(outPkg.orgId, uint32_t(12345));
  EXPECT_EQ(outPkg.appId, uint16_t(100));
  EXPECT_EQ(outPkg.xmlVersion, uint32_t(3));

  // Client code can check if it matches a specific org/app ID
  bool matchesExpected = (outPkg.orgId == 12345 && outPkg.appId == 100);
  EXPECT_TRUE(matchesExpected);

  // And can check for non-matching IDs
  bool matchesDifferent = (outPkg.orgId == 99999 && outPkg.appId == 1);
  EXPECT_FALSE(matchesDifferent);
}

TEST_F(OpAppPackageManagerTest, TestGetOpAppUrl_ReturnsInstalledUrl)
{
  // GIVEN: an existing install receipt with installedUrl
  auto configuration = createConfigurationWithReceipt();

  // Create a receipt file with installedUrl
  std::ofstream receiptFile(configuration.m_InstallReceiptFilePath);
  receiptFile << R"({
    "orgId": 12345,
    "appId": 100,
    "xmlVersion": 1,
    "name": "Test App",
    "installedUrl": "hbbtv-package://64.3039"
  })";
  receiptFile.close();

  auto testInterface = OpAppPackageManagerTestInterface::create(configuration);

  // WHEN: calling getOpAppUrl
  std::string url = testInterface->getPackageManager().getOpAppUrl();

  // THEN: should return the installed URL from the receipt
  EXPECT_EQ(url, "hbbtv-package://64.3039");
}

TEST_F(OpAppPackageManagerTest, TestGetOpAppUrl_ReturnsEmptyWhenNoReceipt)
{
  // GIVEN: a configuration with no receipt file
  auto configuration = createBasicConfiguration();
  configuration.m_InstallReceiptFilePath = PACKAGE_PATH + "/nonexistent_receipt.json";

  auto testInterface = OpAppPackageManagerTestInterface::create(configuration);

  // WHEN: calling getOpAppUrl with no receipt
  std::string url = testInterface->getPackageManager().getOpAppUrl();

  // THEN: should return empty string
  EXPECT_TRUE(url.empty());
}

TEST_F(OpAppPackageManagerTest, TestGetCandidatePackageFile_ReturnsSetValue)
{
  // GIVEN: an OpAppPackageManager instance
  auto configuration = createBasicConfiguration();
  auto testInterface = OpAppPackageManagerTestInterface::create(configuration);

  // WHEN: setting a candidate package file
  std::filesystem::path expectedPath = "/path/to/candidate.cms";
  testInterface->setCandidatePackageFile(expectedPath);

  // THEN: getCandidatePackageFile should return the set value
  EXPECT_EQ(testInterface->getCandidatePackageFile(), expectedPath);
}

// =============================================================================
// isOpAppInstalled Tests
// =============================================================================

TEST_F(OpAppPackageManagerTest, TestIsOpAppInstalled_NoReceiptFile_ReturnsFalse)
{
  // GIVEN: an OpAppPackageManager with no install receipt file
  auto configuration = createConfigurationWithReceipt();
  auto testInterface = OpAppPackageManagerTestInterface::create(configuration);

  // WHEN: checking if OpApp is installed
  bool result = testInterface->getPackageManager().isOpAppInstalled();

  // THEN: should return false (no receipt file means not installed)
  EXPECT_FALSE(result);
}

TEST_F(OpAppPackageManagerTest, TestIsOpAppInstalled_NoReceiptPathConfigured_ReturnsFalse)
{
  // GIVEN: an OpAppPackageManager with no receipt path configured
  auto configuration = createBasicConfiguration();
  auto testInterface = OpAppPackageManagerTestInterface::create(configuration);

  // WHEN: checking if OpApp is installed
  bool result = testInterface->getPackageManager().isOpAppInstalled();

  // THEN: should return false (no receipt path configured)
  EXPECT_FALSE(result);
}

TEST_F(OpAppPackageManagerTest, TestIsOpAppInstalled_ValidReceiptFile_ReturnsTrue)
{
  // GIVEN: an OpAppPackageManager with a valid install receipt file
  auto configuration = createConfigurationWithReceipt();
  createReceiptFile(configuration.m_InstallReceiptFilePath);

  auto testInterface = OpAppPackageManagerTestInterface::create(configuration);

  // WHEN: checking if OpApp is installed
  bool result = testInterface->getPackageManager().isOpAppInstalled();

  // THEN: should return true (valid receipt file exists)
  EXPECT_TRUE(result);
}

TEST_F(OpAppPackageManagerTest, TestIsOpAppInstalled_InvalidReceiptFile_ReturnsFalse)
{
  // GIVEN: an OpAppPackageManager with an invalid/corrupt receipt file
  auto configuration = createConfigurationWithReceipt();
  createInvalidReceiptFile(configuration.m_InstallReceiptFilePath);

  auto testInterface = OpAppPackageManagerTestInterface::create(configuration);

  // WHEN: checking if OpApp is installed
  bool result = testInterface->getPackageManager().isOpAppInstalled();

  // THEN: should return false (invalid receipt means not properly installed)
  EXPECT_FALSE(result);
}

TEST_F(OpAppPackageManagerTest, TestIsOpAppInstalled_EmptyReceiptFile_ReturnsFalse)
{
  // GIVEN: an OpAppPackageManager with an empty receipt file
  auto configuration = createConfigurationWithReceipt();
  createEmptyReceiptFile(configuration.m_InstallReceiptFilePath);

  auto testInterface = OpAppPackageManagerTestInterface::create(configuration);

  // WHEN: checking if OpApp is installed
  bool result = testInterface->getPackageManager().isOpAppInstalled();

  // THEN: should return false (empty receipt means not properly installed)
  EXPECT_FALSE(result);
}

// =============================================================================
// STUB TESTS - Placeholder tests for future functionality
// These tests have no assertions and exist only as templates for future implementation.
// Remove or implement when the corresponding functionality is added.
// =============================================================================

// TODO(STUB): Implement when uninstallPackage method is added
TEST_F(OpAppPackageManagerTest, STUB_TestUninstallPackage)
{
  auto configuration = createBasicConfiguration();
  auto testInterface = OpAppPackageManagerTestInterface::create(configuration);
  (void)testInterface;
}

// TODO(STUB): Implement when listInstalledPackages method is added
TEST_F(OpAppPackageManagerTest, STUB_TestListInstalledPackages)
{
  auto configuration = createBasicConfiguration();
  auto testInterface = OpAppPackageManagerTestInterface::create(configuration);
  (void)testInterface;
}

// TODO(STUB): Implement when updatePackage method is added
TEST_F(OpAppPackageManagerTest, STUB_TestUpdatePackage)
{
  auto configuration = createBasicConfiguration();
  auto testInterface = OpAppPackageManagerTestInterface::create(configuration);
  (void)testInterface;
}


// TODO(STUB): Implement when getPackageVersion method is added
TEST_F(OpAppPackageManagerTest, STUB_TestGetPackageVersion)
{
  auto configuration = createBasicConfiguration();
  auto testInterface = OpAppPackageManagerTestInterface::create(configuration);
  (void)testInterface;
}

// TODO(STUB): Implement when validatePackage method is added
TEST_F(OpAppPackageManagerTest, STUB_TestValidatePackage)
{
  auto configuration = createBasicConfiguration();
  auto testInterface = OpAppPackageManagerTestInterface::create(configuration);
  (void)testInterface;
}

// TODO(STUB): Implement when installPackage method handles invalid paths
TEST_F(OpAppPackageManagerTest, STUB_TestInstallInvalidPackage)
{
  auto configuration = createBasicConfiguration();
  auto testInterface = OpAppPackageManagerTestInterface::create(configuration);
  (void)testInterface;
}

// TODO(STUB): Implement concurrent operation tests when threading support is added
TEST_F(OpAppPackageManagerTest, STUB_TestConcurrentOperations)
{
  auto configuration = createBasicConfiguration();
  auto testInterface = OpAppPackageManagerTestInterface::create(configuration);
  (void)testInterface;
}

// TODO(STUB): Implement large package handling tests
TEST_F(OpAppPackageManagerTest, STUB_TestLargePackageHandling)
{
  auto configuration = createBasicConfiguration();
  auto testInterface = OpAppPackageManagerTestInterface::create(configuration);
  (void)testInterface;
}

// TODO(STUB): Implement full lifecycle tests when all methods are available
TEST_F(OpAppPackageManagerTest, STUB_TestFullPackageLifecycle)
{
  auto configuration = createBasicConfiguration();
  auto testInterface = OpAppPackageManagerTestInterface::create(configuration);
  (void)testInterface;
}

// TODO(STUB): Implement package update workflow tests
TEST_F(OpAppPackageManagerTest, STUB_TestPackageUpdateWorkflow)
{
  auto configuration = createBasicConfiguration();
  auto testInterface = OpAppPackageManagerTestInterface::create(configuration);
  (void)testInterface;
}

TEST_F(OpAppPackageManagerTest, TestUpdateCallbacks_NoUpdateAvailable_NoCallbacksCalled)
{
  // GIVEN: an OpAppPackageManager instance with callbacks
  auto configuration = createConfigurationWithReceipt();

  bool successCallbackCalled = false;
  bool failureCallbackCalled = false;
  configuration.m_OnUpdateSuccess = [&](const std::string&) { successCallbackCalled = true; };
  configuration.m_OnUpdateFailure = [&](OpAppPackageManager::PackageStatus, const std::string&) { failureCallbackCalled = true; };

  auto testInterface = OpAppPackageManagerTestInterface::create(configuration);

  // WHEN: checking for updates when no package file exists
  testInterface->checkForUpdates();

  // THEN: neither callback should be called for DiscoveryFailed
  EXPECT_FALSE(successCallbackCalled);
  EXPECT_FALSE(failureCallbackCalled);
}

TEST_F(OpAppPackageManagerTest, TestUpdateCallbacks_Installed_NoCallbacksCalled)
{
  // GIVEN: an OpAppPackageManager instance with callbacks and installed package
  auto configuration = createConfigurationWithReceipt();
  auto packagePath = createPackageFile("test package content");

  const std::string hash = "test_hash_1234567890abcdef";
  auto mockCalculator = std::make_unique<MockHashCalculator>();
  mockCalculator->setHashForFile(packagePath.string(), hash);
  mockCalculator->createInstallReceiptFile(configuration.m_InstallReceiptFilePath.string(), hash);

  bool successCallbackCalled = false;
  bool failureCallbackCalled = false;
  configuration.m_OnUpdateSuccess = [&](const std::string&) { successCallbackCalled = true; };
  configuration.m_OnUpdateFailure = [&](OpAppPackageManager::PackageStatus, const std::string&) { failureCallbackCalled = true; };

  OpAppPackageManager::Dependencies deps;
  deps.hashCalculator = std::move(mockCalculator);
  auto testInterface = OpAppPackageManagerTestInterface::create(configuration, std::move(deps));

  // WHEN: checking for updates with installed package
  OpAppPackageManager::PackageStatus status = testInterface->doLocalPackageCheck();

  // THEN: status should be Installed and neither callback should be called
  EXPECT_EQ(status, OpAppPackageManager::PackageStatus::Installed);
  EXPECT_FALSE(successCallbackCalled);
  EXPECT_FALSE(failureCallbackCalled);
}

TEST_F(OpAppPackageManagerTest, TestUpdateCallbacks_ConfigurationError_CallsFailureCallback)
{
  // GIVEN: an OpAppPackageManager instance with callbacks
  auto configuration = createConfigurationWithReceipt();

  // Create multiple package files to trigger ConfigurationError
  createPackageFile("package1.cms", "");
  createPackageFile("package2.cms", "");

  bool successCallbackCalled = false;
  bool failureCallbackCalled = false;
  OpAppPackageManager::PackageStatus failureStatus;
  std::string failureErrorMessage;

  configuration.m_OnUpdateSuccess = [&](const std::string&) { successCallbackCalled = true; };
  configuration.m_OnUpdateFailure = [&](OpAppPackageManager::PackageStatus status, const std::string& errorMessage) {
    failureCallbackCalled = true;
    failureStatus = status;
    failureErrorMessage = errorMessage;
  };

  auto testInterface = OpAppPackageManagerTestInterface::create(configuration);

  // WHEN: checking for updates with multiple package files (ConfigurationError)
  testInterface->checkForUpdates();
  std::this_thread::sleep_for(std::chrono::milliseconds(200));

  // THEN: status should be ConfigurationError and failure callback should be called
  EXPECT_FALSE(successCallbackCalled);
  EXPECT_TRUE(failureCallbackCalled);
  EXPECT_EQ(failureStatus, OpAppPackageManager::PackageStatus::ConfigurationError);
  EXPECT_FALSE(failureErrorMessage.empty());
}

TEST_F(OpAppPackageManagerTest, TestUpdateCallbacks_NoCallbacksSet)
{
  // GIVEN: an OpAppPackageManager instance without callbacks
  auto configuration = createConfigurationWithReceipt();
  auto testInterface = OpAppPackageManagerTestInterface::create(configuration);

  // WHEN: checking for updates when no package file exists
  // THEN: function should complete successfully (NoUpdateAvailable since no packages found)
  OpAppPackageManager::PackageStatus status = testInterface->doLocalPackageCheck();
  EXPECT_EQ(status, OpAppPackageManager::PackageStatus::NoUpdateAvailable);
}

TEST_F(OpAppPackageManagerTest, TestCalculateSHA256Hash)
{
  // GIVEN: an OpAppPackageManager instance and a test file
  auto configuration = createBasicConfiguration();
  auto testInterface = OpAppPackageManagerTestInterface::create(configuration);
  auto testFilePath = createTestFile("test_file.txt", "Hello, World! This is a test file for SHA256 hashing.");

  // WHEN: calculating SHA256 hash of the test file
  std::string hash = testInterface->calculateFileSHA256Hash(testFilePath.string());

  // THEN: the hash should be a valid SHA256 hash (64 hex characters)
  EXPECT_EQ(hash.length(), size_t(64));
  EXPECT_TRUE(std::all_of(hash.begin(), hash.end(), ::isxdigit));
  EXPECT_FALSE(hash.empty());
}

TEST_F(OpAppPackageManagerTest, TestCalculateSHA256HashEmptyFile)
{
  // GIVEN: an OpAppPackageManager instance and an empty test file
  auto configuration = createBasicConfiguration();
  auto testInterface = OpAppPackageManagerTestInterface::create(configuration);
  auto testFilePath = createTestFile("empty_file.txt");

  // WHEN: calculating SHA256 hash of the empty file
  std::string hash = testInterface->calculateFileSHA256Hash(testFilePath.string());

  // THEN: the hash should be the SHA256 hash of an empty file
  EXPECT_EQ(hash, "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");
}

TEST_F(OpAppPackageManagerTest, TestCalculateSHA256HashNonexistentFile)
{
  // GIVEN: an OpAppPackageManager instance
  auto configuration = createBasicConfiguration();
  auto testInterface = OpAppPackageManagerTestInterface::create(configuration);

  // WHEN: calculating SHA256 hash of a nonexistent file
  std::string hash = testInterface->calculateFileSHA256Hash(PACKAGE_PATH + "/nonexistent_file.txt");

  // THEN: the hash should be empty string
  EXPECT_TRUE(hash.empty());
}

TEST_F(OpAppPackageManagerTest, TestCheckForUpdates_MultiplePackageFiles_ReturnsError)
{
  // GIVEN: an OpAppPackageManager instance with multiple package files
  auto configuration = createConfigurationWithReceipt();
  createPackageFile("package1.cms", "");
  createPackageFile("package2.cms", "");

  auto testInterface = OpAppPackageManagerTestInterface::create(configuration);

  // WHEN: doLocalPackageCheck is called
  OpAppPackageManager::PackageStatus status = testInterface->doLocalPackageCheck();

  // THEN: it should set status to ConfigurationError
  EXPECT_EQ(status, OpAppPackageManager::PackageStatus::ConfigurationError);
  EXPECT_FALSE(testInterface->getLastErrorMessage().empty());
}

TEST_F(OpAppPackageManagerTest, TestGetPackageFiles_MultiplePackageFiles_ReturnsError)
{
  // GIVEN: an OpAppPackageManager instance with multiple package files
  auto configuration = createBasicConfiguration();
  createPackageFile("package1.cms", "");
  createPackageFile("package2.cms", "");

  auto testInterface = OpAppPackageManagerTestInterface::create(configuration);

  // WHEN: searchLocalPackageFiles is called
  std::vector<std::filesystem::path> packageFiles;
  int result = testInterface->searchLocalPackageFiles(packageFiles);

  // THEN: it should return -1 (error) for multiple files
  EXPECT_EQ(result, -1);
  EXPECT_FALSE(testInterface->getLastErrorMessage().empty());
  EXPECT_EQ(packageFiles.size(), size_t(2));
  EXPECT_NE(testInterface->getLastErrorMessage().find("Multiple package files found"), std::string::npos);
}

TEST_F(OpAppPackageManagerTest, TestGetPackageFiles_SinglePackageFile_ReturnsSuccess)
{
  // GIVEN: an OpAppPackageManager instance with a single package file
  auto configuration = createBasicConfiguration();
  auto packagePath = createPackageFile();

  auto testInterface = OpAppPackageManagerTestInterface::create(configuration);

  // WHEN: searchLocalPackageFiles is called
  std::vector<std::filesystem::path> packageFiles;
  int result = testInterface->searchLocalPackageFiles(packageFiles);

  // THEN: it should return 1 (one file found)
  EXPECT_EQ(result, 1);
  EXPECT_TRUE(testInterface->getLastErrorMessage().empty());
  EXPECT_EQ(packageFiles.size(), size_t(1));
  EXPECT_EQ(packageFiles[0], packagePath);
}

TEST_F(OpAppPackageManagerTest, TestGetPackageFiles_NoPackageFiles_ReturnsSuccess)
{
  // GIVEN: an OpAppPackageManager instance with no package files
  auto configuration = createBasicConfiguration();
  auto testInterface = OpAppPackageManagerTestInterface::create(configuration);

  // WHEN: searchLocalPackageFiles is called
  std::vector<std::filesystem::path> packageFiles;
  int result = testInterface->searchLocalPackageFiles(packageFiles);

  // THEN: it should return 0 (no files found)
  EXPECT_EQ(result, 0);
  EXPECT_TRUE(testInterface->getLastErrorMessage().empty());
  EXPECT_TRUE(packageFiles.empty());
}

TEST_F(OpAppPackageManagerTest, TestClearLastError)
{
  // GIVEN: an OpAppPackageManager instance with an error
  auto configuration = createBasicConfiguration();
  auto testInterface = OpAppPackageManagerTestInterface::create(configuration);

  // Create an error condition (multiple package files)
  createPackageFile("package1.cms", "");
  createPackageFile("package2.cms", "");

  // Trigger error condition
  testInterface->doLocalPackageCheck();
  EXPECT_FALSE(testInterface->getLastErrorMessage().empty());

  // WHEN: clearLastError is called
  testInterface->clearLastError();

  // THEN: the error message should be cleared
  EXPECT_TRUE(testInterface->getLastErrorMessage().empty());
}

// =============================================================================
// AIT Fetcher and Parser Tests
// =============================================================================

TEST_F(OpAppPackageManagerTest, TestDoRemotePackageCheck_NoFqdn_ReturnsConfigurationError)
{
  // GIVEN: an OpAppPackageManager with no FQDN configured
  auto configuration = createBasicConfiguration();
  configuration.m_WorkingDirectory = PACKAGE_PATH + "/dest";

  auto mockAitFetcher = std::make_unique<MockAitFetcher>();
  MockAitFetcher* mockAitFetcherPtr = mockAitFetcher.get();

  OpAppPackageManager::Dependencies deps;
  deps.aitFetcher = std::move(mockAitFetcher);
  auto testInterface = OpAppPackageManagerTestInterface::create(configuration, std::move(deps));

  // WHEN: doRemotePackageCheck is called
  OpAppPackageManager::PackageStatus status = testInterface->doRemotePackageCheck();

  // THEN: should return ConfigurationError and not call fetcher
  EXPECT_EQ(status, OpAppPackageManager::PackageStatus::ConfigurationError);
  EXPECT_FALSE(mockAitFetcherPtr->wasFetchCalled());
}

TEST_F(OpAppPackageManagerTest, TestDoRemotePackageCheck_FetchFails_ReturnsConfigurationError)
{
  // GIVEN: an OpAppPackageManager with FQDN configured and mock fetcher that fails
  auto configuration = createBasicConfiguration();
  configuration.m_WorkingDirectory = PACKAGE_PATH + "/dest";
  configuration.m_OpAppFqdn = "test.example.com";

  auto mockAitFetcher = std::make_unique<MockAitFetcher>();
  mockAitFetcher->setFetchResult(AitFetchResult("DNS lookup failed"));
  MockAitFetcher* mockAitFetcherPtr = mockAitFetcher.get();

  OpAppPackageManager::Dependencies deps;
  deps.aitFetcher = std::move(mockAitFetcher);
  auto testInterface = OpAppPackageManagerTestInterface::create(configuration, std::move(deps));

  // WHEN: doRemotePackageCheck is called
  OpAppPackageManager::PackageStatus status = testInterface->doRemotePackageCheck();

  // THEN: should return ConfigurationError and have called fetcher
  EXPECT_EQ(status, OpAppPackageManager::PackageStatus::ConfigurationError);
  EXPECT_TRUE(mockAitFetcherPtr->wasFetchCalled());
  EXPECT_EQ(mockAitFetcherPtr->getLastFqdn(), "test.example.com");
}

TEST_F(OpAppPackageManagerTest, TestDoRemotePackageCheck_NoAitFiles_ReturnsConfigurationError)
{
  // GIVEN: an OpAppPackageManager with FQDN and mock fetcher returning empty result
  auto configuration = createBasicConfiguration();
  configuration.m_WorkingDirectory = PACKAGE_PATH + "/dest";
  configuration.m_OpAppFqdn = "test.example.com";

  auto mockAitFetcher = std::make_unique<MockAitFetcher>();
  mockAitFetcher->setFetchResult(AitFetchResult(std::vector<std::string>{}, std::vector<std::string>{}));

  OpAppPackageManager::Dependencies deps;
  deps.aitFetcher = std::move(mockAitFetcher);
  auto testInterface = OpAppPackageManagerTestInterface::create(configuration, std::move(deps));

  // WHEN: doRemotePackageCheck is called
  OpAppPackageManager::PackageStatus status = testInterface->doRemotePackageCheck();

  // THEN: should return ConfigurationError (no AITs acquired)
  EXPECT_EQ(status, OpAppPackageManager::PackageStatus::ConfigurationError);
}

TEST_F(OpAppPackageManagerTest, TestParseAitFiles_EmptyFileList_ReturnsFalse)
{
  // GIVEN: a test interface with real XML parser
  auto configuration = createBasicConfiguration();
  OpAppPackageManager::Dependencies deps;
  deps.xmlParser = std::make_unique<XmlParser>();
  auto testInterface = OpAppPackageManagerTestInterface::create(configuration, std::move(deps));

  // WHEN: parseAitFiles is called with empty list
  std::vector<PackageInfo> packages;
  bool result = testInterface->parseAitFiles(std::vector<std::filesystem::path>{}, packages);

  // THEN: should return failure and have no descriptors
  EXPECT_FALSE(result);
  EXPECT_FALSE(testInterface->getLastErrorMessage().empty());
  EXPECT_TRUE(packages.empty());
}

TEST_F(OpAppPackageManagerTest, TestParseAitFiles_NonexistentFile_ReturnsFalse)
{
  // GIVEN: a test interface with real XML parser
  auto configuration = createBasicConfiguration();
  OpAppPackageManager::Dependencies deps;
  deps.xmlParser = std::make_unique<XmlParser>();
  auto testInterface = OpAppPackageManagerTestInterface::create(configuration, std::move(deps));

  // WHEN: parseAitFiles is called with nonexistent file
  std::vector<PackageInfo> packages;
  bool result = testInterface->parseAitFiles({std::filesystem::path("/nonexistent/ait.xml")}, packages);

  // THEN: should return failure and have no descriptors
  EXPECT_FALSE(result);
  EXPECT_FALSE(testInterface->getLastErrorMessage().empty());
  EXPECT_TRUE(packages.empty());
}

TEST_F(OpAppPackageManagerTest, TestParseAitFiles_InvalidXml_ReturnsFalse)
{
  // GIVEN: a test interface with real XML parser and an invalid XML file
  auto configuration = createBasicConfiguration();
  auto invalidXmlPath = createTestFile("invalid.xml", "This is not valid XML content");

  OpAppPackageManager::Dependencies deps;
  deps.xmlParser = std::make_unique<XmlParser>();
  auto testInterface = OpAppPackageManagerTestInterface::create(configuration, std::move(deps));

  // WHEN: parseAitFiles is called with invalid XML
  std::vector<PackageInfo> packages;
  bool result = testInterface->parseAitFiles({invalidXmlPath}, packages);

  // THEN: should return failure and have no descriptors
  EXPECT_FALSE(result);
  EXPECT_FALSE(testInterface->getLastErrorMessage().empty());
  EXPECT_TRUE(packages.empty());
}

TEST_F(OpAppPackageManagerTest, TestParseAitFiles_ValidAitXml_ExtractsDescriptors)
{
  // GIVEN: a test interface with real XML parser and a valid OpApp AIT XML file
  auto configuration = createBasicConfiguration();
  auto aitXmlPath = createAitXmlFile("valid_ait.xml", 12345, 1, "Test OpApp", "https://test.example.com/app/");

  OpAppPackageManager::Dependencies deps;
  deps.xmlParser = std::make_unique<XmlParser>();
  auto testInterface = OpAppPackageManagerTestInterface::create(configuration, std::move(deps));

  // WHEN: parseAitFiles is called with valid AIT XML
  std::vector<PackageInfo> packages;
  bool result = testInterface->parseAitFiles({aitXmlPath}, packages);

  // THEN: should return success and have extracted descriptors
  EXPECT_TRUE(result);
  ASSERT_EQ(packages.size(), size_t(1));
  EXPECT_EQ(packages[0].orgId, uint32_t(12345));
  EXPECT_EQ(packages[0].appId, uint16_t(1));
  EXPECT_EQ(packages[0].baseUrl, "https://test.example.com/app/");
  EXPECT_EQ(packages[0].location, "index.html");
  EXPECT_EQ(packages[0].name, "Test OpApp");
}

TEST_F(OpAppPackageManagerTest, TestParseAitFiles_MultipleAits_CombinesApps)
{
  // GIVEN: a test interface with real XML parser and multiple OpApp AIT XML files
  auto configuration = createBasicConfiguration();
  auto ait1Path = createAitXmlFile("ait1.xml", 11111, 1, "App One", "https://test1.example.com/");
  auto ait2Path = createAitXmlFile("ait2.xml", 22222, 2, "App Two", "https://test2.example.com/");

  OpAppPackageManager::Dependencies deps;
  deps.xmlParser = std::make_unique<XmlParser>();
  auto testInterface = OpAppPackageManagerTestInterface::create(configuration, std::move(deps));

  // WHEN: parseAitFiles is called with multiple AIT files
  std::vector<PackageInfo> packages;
  bool result = testInterface->parseAitFiles({ait1Path, ait2Path}, packages);

  // THEN: should return success and combine apps from both files
  EXPECT_TRUE(result);
  EXPECT_EQ(packages.size(), size_t(2));

  // Verify both apps are present
  bool foundApp1 = false;
  bool foundApp2 = false;
  for (const auto& desc : packages) {
    if (desc.orgId == 11111 && desc.appId == 1) foundApp1 = true;
    if (desc.orgId == 22222 && desc.appId == 2) foundApp2 = true;
  }
  EXPECT_TRUE(foundApp1);
  EXPECT_TRUE(foundApp2);
}

TEST_F(OpAppPackageManagerTest, TestParseAitFiles_ClearsOldDescriptors)
{
  // GIVEN: a test interface with real XML parser and previously parsed descriptors
  auto configuration = createBasicConfiguration();
  auto aitXmlPath = createAitXmlFile("test_ait.xml", 99999, 9, "Test App", "https://test.example.com/");

  OpAppPackageManager::Dependencies deps;
  deps.xmlParser = std::make_unique<XmlParser>();
  auto testInterface = OpAppPackageManagerTestInterface::create(configuration, std::move(deps));

  // First parse
  std::vector<PackageInfo> packages;
  testInterface->parseAitFiles({aitXmlPath}, packages);
  size_t firstCount = packages.size();

  // WHEN: parseAitFiles is called again with the same file
  packages.clear();
  testInterface->parseAitFiles({aitXmlPath}, packages);

  // THEN: should have same count (cleared and repopulated, not appended)
  EXPECT_EQ(packages.size(), firstCount);
}

TEST_F(OpAppPackageManagerTest, TestPackageInfo_DefaultValues)
{
  // GIVEN: a default-constructed PackageInfo
  PackageInfo pkg;

  // THEN: all values should be default initialized
  EXPECT_EQ(pkg.orgId, uint32_t(0));
  EXPECT_EQ(pkg.appId, uint16_t(0));
  EXPECT_EQ(pkg.xmlVersion, uint32_t(0));
  EXPECT_TRUE(pkg.baseUrl.empty());
  EXPECT_TRUE(pkg.location.empty());
  EXPECT_TRUE(pkg.name.empty());
  EXPECT_TRUE(pkg.installPath.empty());
  EXPECT_TRUE(pkg.packageHash.empty());
  EXPECT_TRUE(pkg.installedAt.empty());
}

TEST_F(OpAppPackageManagerTest, TestPackageInfo_IsSameApp)
{
  // GIVEN: two packages with same org/app ID
  PackageInfo pkg1;
  pkg1.orgId = 12345;
  pkg1.appId = 100;
  pkg1.xmlVersion = 1;

  PackageInfo pkg2;
  pkg2.orgId = 12345;
  pkg2.appId = 100;
  pkg2.xmlVersion = 2;

  // THEN: isSameApp should return true
  EXPECT_TRUE(pkg1.isSameApp(pkg2));

  // WHEN: appId differs
  pkg2.appId = 101;

  // THEN: isSameApp should return false
  EXPECT_FALSE(pkg1.isSameApp(pkg2));
}

TEST_F(OpAppPackageManagerTest, TestPackageInfo_IsNewerThan)
{
  // GIVEN: two packages with same identity, different versions
  PackageInfo pkg1;
  pkg1.orgId = 12345;
  pkg1.appId = 100;
  pkg1.xmlVersion = 1;

  PackageInfo pkg2;
  pkg2.orgId = 12345;
  pkg2.appId = 100;
  pkg2.xmlVersion = 2;

  // THEN: pkg2 is newer than pkg1
  EXPECT_TRUE(pkg2.isNewerThan(pkg1));
  EXPECT_FALSE(pkg1.isNewerThan(pkg2));
}

TEST_F(OpAppPackageManagerTest, TestPackageInfo_GetAppUrl)
{
  // GIVEN: a package with baseUrl and location
  PackageInfo pkg;
  pkg.baseUrl = "https://example.com/apps";
  pkg.location = "index.html";

  // THEN: getPackageUrl should combine them correctly
  EXPECT_EQ(pkg.getPackageUrl(), "https://example.com/apps/index.html");

  // WHEN: baseUrl ends with slash
  pkg.baseUrl = "https://example.com/apps/";

  // THEN: should not add extra slash
  EXPECT_EQ(pkg.getPackageUrl(), "https://example.com/apps/index.html");

  // WHEN: location starts with slash
  pkg.baseUrl = "https://example.com/apps";
  pkg.location = "/index.html";

  // THEN: should not add extra slash
  EXPECT_EQ(pkg.getPackageUrl(), "https://example.com/apps/index.html");
}

TEST_F(OpAppPackageManagerTest, TestPackageInfo_GenerateInstalledUrl)
{
  // GIVEN: a package with appId and orgId
  // TS 103 606 Section 9.4.1: Format is hbbtv-package://appid.orgid
  // - encoded as lowercase hexadecimal with no leading zeros
  PackageInfo pkg;

  // WHEN: appId=100 (0x64), orgId=12345 (0x3039)
  pkg.appId = 100;
  pkg.orgId = 12345;
  EXPECT_EQ(pkg.generateInstalledUrl(), "hbbtv-package://64.3039");

  // WHEN: small values (no leading zeros in hex)
  pkg.appId = 1;
  pkg.orgId = 16;  // 0x10
  EXPECT_EQ(pkg.generateInstalledUrl(), "hbbtv-package://1.10");

  // WHEN: values that would have uppercase in hex
  pkg.appId = 171;   // 0xab
  pkg.orgId = 43981; // 0xabcd
  EXPECT_EQ(pkg.generateInstalledUrl(), "hbbtv-package://ab.abcd");

  // WHEN: max uint16_t appId
  pkg.appId = 65535; // 0xffff
  pkg.orgId = 255;   // 0xff
  EXPECT_EQ(pkg.generateInstalledUrl(), "hbbtv-package://ffff.ff");
}

TEST_F(OpAppPackageManagerTest, TestDoRemotePackageCheck_ValidAit_ReturnsUpdateAvailable)
{
  // GIVEN: an OpAppPackageManager with FQDN and mock fetcher that writes valid OpApp AIT file
  auto configuration = createBasicConfiguration();
  configuration.m_WorkingDirectory = PACKAGE_PATH + "/dest";
  configuration.m_OpAppFqdn = "test.example.com";

  auto mockAitFetcher = std::make_unique<MockAitFetcher>();
  mockAitFetcher->setFileContent("ait_0.xml",
      createValidOpAppAitXml(12345, 1, "Test OpApp", "https://test.example.com/app/"));

  OpAppPackageManager::Dependencies deps;
  deps.aitFetcher = std::move(mockAitFetcher);
  deps.xmlParser = std::make_unique<XmlParser>();
  auto testInterface = OpAppPackageManagerTestInterface::create(configuration, std::move(deps));

  // WHEN: doRemotePackageCheck is called
  OpAppPackageManager::PackageStatus status = testInterface->doRemotePackageCheck();

  // THEN: should return UpdateAvailable
  EXPECT_EQ(status, OpAppPackageManager::PackageStatus::UpdateAvailable);
}

TEST_F(OpAppPackageManagerTest, TestDoRemotePackageCheck_AitWithNoApps_ReturnsNoUpdate)
{
  // GIVEN: an OpAppPackageManager with FQDN and AIT file with no applications
  auto configuration = createBasicConfiguration();
  configuration.m_WorkingDirectory = PACKAGE_PATH + "/dest";
  configuration.m_OpAppFqdn = "test.example.com";

  // AIT with empty application list
  std::string aitContent = R"(<?xml version="1.0" encoding="UTF-8"?>
<mhp:ServiceDiscovery xmlns:mhp="urn:dvb:mhp:2009">
  <mhp:ApplicationDiscovery DomainName="test.example.com">
    <mhp:ApplicationList>
    </mhp:ApplicationList>
  </mhp:ApplicationDiscovery>
</mhp:ServiceDiscovery>)";

  auto mockAitFetcher = std::make_unique<MockAitFetcher>();
  mockAitFetcher->setFileContent("ait_0.xml", aitContent);

  OpAppPackageManager::Dependencies deps;
  deps.aitFetcher = std::move(mockAitFetcher);
  deps.xmlParser = std::make_unique<XmlParser>();
  auto testInterface = OpAppPackageManagerTestInterface::create(configuration, std::move(deps));

  // WHEN: doRemotePackageCheck is called
  OpAppPackageManager::PackageStatus status = testInterface->doRemotePackageCheck();

  // THEN: should return DiscoveryFailed (no apps found)
  EXPECT_EQ(status, OpAppPackageManager::PackageStatus::DiscoveryFailed);
}

// =============================================================================
// AIT Validation Tests
// =============================================================================

// Helper function to create HbbTV (non-OpApp) AIT XML for validation tests
static std::string createHbbTvAitXml(uint32_t orgId, uint16_t appId, const std::string& appName, const std::string& baseUrl)
{
  std::ostringstream ss;
  ss << R"(<?xml version="1.0" encoding="UTF-8"?>
<mhp:ServiceDiscovery xmlns:mhp="urn:dvb:mhp:2009">
  <mhp:ApplicationDiscovery DomainName="test.example.com">
    <mhp:ApplicationList>
      <mhp:Application>
        <mhp:appName Language="eng">)" << appName << R"(</mhp:appName>
        <mhp:applicationIdentifier>
          <mhp:orgId>)" << orgId << R"(</mhp:orgId>
          <mhp:appId>)" << appId << R"(</mhp:appId>
        </mhp:applicationIdentifier>
        <mhp:applicationDescriptor>
          <mhp:type>
            <mhp:OtherApp>application/vnd.hbbtv.xhtml+xml</mhp:OtherApp>
          </mhp:type>
          <mhp:controlCode>AUTOSTART</mhp:controlCode>
          <mhp:visibility>VISIBLE_ALL</mhp:visibility>
          <mhp:serviceBound>false</mhp:serviceBound>
          <mhp:priority>1</mhp:priority>
          <mhp:version>1</mhp:version>
        </mhp:applicationDescriptor>
        <mhp:applicationTransport xsi:type="mhp:HTTPTransportType" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
          <mhp:URLBase>)" << baseUrl << R"(</mhp:URLBase>
        </mhp:applicationTransport>
        <mhp:applicationLocation>index.html</mhp:applicationLocation>
      </mhp:Application>
    </mhp:ApplicationList>
  </mhp:ApplicationDiscovery>
</mhp:ServiceDiscovery>)";
  return ss.str();
}

// Helper function to create OpApp AIT XML with invalid usage
static std::string createOpAppAitXmlWithInvalidUsage(uint32_t orgId, uint16_t appId, const std::string& appName, const std::string& baseUrl)
{
  std::ostringstream ss;
  ss << R"(<?xml version="1.0" encoding="UTF-8"?>
<mhp:ServiceDiscovery xmlns:mhp="urn:dvb:mhp:2009">
  <mhp:ApplicationDiscovery DomainName="test.example.com">
    <mhp:ApplicationList>
      <mhp:Application>
        <mhp:appName Language="eng">)" << appName << R"(</mhp:appName>
        <mhp:applicationIdentifier>
          <mhp:orgId>)" << orgId << R"(</mhp:orgId>
          <mhp:appId>)" << appId << R"(</mhp:appId>
        </mhp:applicationIdentifier>
        <mhp:applicationDescriptor>
          <mhp:type>
            <mhp:OtherApp>application/vnd.hbbtv.opapp.pkg</mhp:OtherApp>
          </mhp:type>
          <mhp:controlCode>AUTOSTART</mhp:controlCode>
          <mhp:visibility>VISIBLE_ALL</mhp:visibility>
          <mhp:serviceBound>false</mhp:serviceBound>
          <mhp:priority>1</mhp:priority>
          <mhp:version>1</mhp:version>
        </mhp:applicationDescriptor>
        <mhp:applicationUsageDescriptor>
          <mhp:ApplicationUsage>urn:invalid:usage:2017</mhp:ApplicationUsage>
        </mhp:applicationUsageDescriptor>
        <mhp:applicationTransport xsi:type="mhp:HTTPTransportType" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
          <mhp:URLBase>)" << baseUrl << R"(</mhp:URLBase>
        </mhp:applicationTransport>
        <mhp:applicationLocation>index.html</mhp:applicationLocation>
      </mhp:Application>
    </mhp:ApplicationList>
  </mhp:ApplicationDiscovery>
</mhp:ServiceDiscovery>)";
  return ss.str();
}

TEST_F(OpAppPackageManagerTest, TestParseAitFiles_NonOpAppType_Rejected)
{
  // GIVEN: an AIT XML with HbbTV app type (not OpApp)
  auto configuration = createBasicConfiguration();
  auto aitXmlPath = createTestFile("hbbtv_ait.xml", createHbbTvAitXml(12345, 1, "HbbTV App", "https://test.example.com/"));

  OpAppPackageManager::Dependencies deps;
  deps.xmlParser = std::make_unique<XmlParser>();
  auto testInterface = OpAppPackageManagerTestInterface::create(configuration, std::move(deps));

  // WHEN: parseAitFiles is called
  std::vector<PackageInfo> packages;
  bool result = testInterface->parseAitFiles({aitXmlPath}, packages);

  // THEN: should return failure (no valid OpApp descriptors)
  EXPECT_FALSE(result);
  EXPECT_TRUE(packages.empty());
}

TEST_F(OpAppPackageManagerTest, TestParseAitFiles_InvalidAppUsage_Rejected)
{
  // GIVEN: an OpApp AIT XML with invalid applicationUsage
  auto configuration = createBasicConfiguration();
  auto aitXmlPath = createTestFile("invalid_usage_ait.xml",
      createOpAppAitXmlWithInvalidUsage(12345, 1, "Invalid Usage App", "https://test.example.com/"));

  OpAppPackageManager::Dependencies deps;
  deps.xmlParser = std::make_unique<XmlParser>();
  auto testInterface = OpAppPackageManagerTestInterface::create(configuration, std::move(deps));

  // WHEN: parseAitFiles is called
  std::vector<PackageInfo> packages;
  bool result = testInterface->parseAitFiles({aitXmlPath}, packages);

  // THEN: should return failure (invalid app usage rejected)
  EXPECT_FALSE(result);
  EXPECT_TRUE(packages.empty());
}

TEST_F(OpAppPackageManagerTest, TestParseAitFiles_SpecificOpApp_Accepted)
{
  // GIVEN: an OpApp AIT XML with specific usage
  auto configuration = createBasicConfiguration();
  auto aitXmlPath = createTestFile("specific_ait.xml",
      createValidOpAppAitXml(12345, 1, "Specific OpApp", "https://test.example.com/",
          "index.html", "AUTOSTART", "urn:hbbtv:opapp:specific:2017"));

  OpAppPackageManager::Dependencies deps;
  deps.xmlParser = std::make_unique<XmlParser>();
  auto testInterface = OpAppPackageManagerTestInterface::create(configuration, std::move(deps));

  // WHEN: parseAitFiles is called
  std::vector<PackageInfo> packages;
  bool result = testInterface->parseAitFiles({aitXmlPath}, packages);

  // THEN: should return success (specific usage accepted)
  EXPECT_TRUE(result);
  ASSERT_EQ(packages.size(), size_t(1));
  EXPECT_EQ(packages[0].orgId, uint32_t(12345));
}

TEST_F(OpAppPackageManagerTest, TestParseAitFiles_MixedValidAndInvalid_OnlyValidExtracted)
{
  // GIVEN: an AIT with multiple apps - one valid OpApp, one invalid HbbTV app
  auto configuration = createBasicConfiguration();

  std::string mixedAitContent = R"(<?xml version="1.0" encoding="UTF-8"?>
<mhp:ServiceDiscovery xmlns:mhp="urn:dvb:mhp:2009">
  <mhp:ApplicationDiscovery DomainName="test.example.com">
    <mhp:ApplicationList>
      <mhp:Application>
        <mhp:appName Language="eng">Invalid HbbTV App</mhp:appName>
        <mhp:applicationIdentifier>
          <mhp:orgId>11111</mhp:orgId>
          <mhp:appId>1</mhp:appId>
        </mhp:applicationIdentifier>
        <mhp:applicationDescriptor>
          <mhp:type>
            <mhp:OtherApp>application/vnd.hbbtv.xhtml+xml</mhp:OtherApp>
          </mhp:type>
          <mhp:controlCode>AUTOSTART</mhp:controlCode>
          <mhp:visibility>VISIBLE_ALL</mhp:visibility>
          <mhp:serviceBound>false</mhp:serviceBound>
          <mhp:priority>1</mhp:priority>
          <mhp:version>1</mhp:version>
        </mhp:applicationDescriptor>
        <mhp:applicationTransport xsi:type="mhp:HTTPTransportType" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
          <mhp:URLBase>https://hbbtv.example.com/</mhp:URLBase>
        </mhp:applicationTransport>
        <mhp:applicationLocation>index.html</mhp:applicationLocation>
      </mhp:Application>
      <mhp:Application>
        <mhp:appName Language="eng">Valid OpApp</mhp:appName>
        <mhp:applicationIdentifier>
          <mhp:orgId>22222</mhp:orgId>
          <mhp:appId>2</mhp:appId>
        </mhp:applicationIdentifier>
        <mhp:applicationDescriptor>
          <mhp:type>
            <mhp:OtherApp>application/vnd.hbbtv.opapp.pkg</mhp:OtherApp>
          </mhp:type>
          <mhp:controlCode>AUTOSTART</mhp:controlCode>
          <mhp:visibility>VISIBLE_ALL</mhp:visibility>
          <mhp:serviceBound>false</mhp:serviceBound>
          <mhp:priority>1</mhp:priority>
          <mhp:version>1</mhp:version>
        </mhp:applicationDescriptor>
        <mhp:applicationUsageDescriptor>
          <mhp:ApplicationUsage>urn:hbbtv:opapp:privileged:2017</mhp:ApplicationUsage>
        </mhp:applicationUsageDescriptor>
        <mhp:applicationTransport xsi:type="mhp:HTTPTransportType" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
          <mhp:URLBase>https://opapp.example.com/</mhp:URLBase>
        </mhp:applicationTransport>
        <mhp:applicationLocation>app.html</mhp:applicationLocation>
      </mhp:Application>
    </mhp:ApplicationList>
  </mhp:ApplicationDiscovery>
</mhp:ServiceDiscovery>)";

  auto aitXmlPath = createTestFile("mixed_ait.xml", mixedAitContent);

  OpAppPackageManager::Dependencies deps;
  deps.xmlParser = std::make_unique<XmlParser>();
  auto testInterface = OpAppPackageManagerTestInterface::create(configuration, std::move(deps));

  // WHEN: parseAitFiles is called
  std::vector<PackageInfo> packages;
  bool result = testInterface->parseAitFiles({aitXmlPath}, packages);

  // THEN: should return success with only the valid OpApp extracted
  EXPECT_TRUE(result);
  ASSERT_EQ(packages.size(), size_t(1));
  EXPECT_EQ(packages[0].orgId, uint32_t(22222));
  EXPECT_EQ(packages[0].appId, uint16_t(2));
  EXPECT_EQ(packages[0].baseUrl, "https://opapp.example.com/");
}

// =============================================================================
// downloadPackageFile Tests (TS 103 606 Section 6.1.7)
// =============================================================================

TEST_F(OpAppPackageManagerTest, TestDownloadPackageFile_EmptyUrl_ReturnsFailure)
{
  // GIVEN: a package info with empty URL
  auto configuration = createConfigurationForDownloadTests();

  auto mockDownloader = std::make_unique<MockHttpDownloader>();
  mockDownloader->setDownloadSuccess("test content", "application/vnd.hbbtv.opapp.pkg");

  OpAppPackageManager::Dependencies deps;
  deps.httpDownloader = std::move(mockDownloader);
  auto testInterface = OpAppPackageManagerTestInterface::create(configuration, std::move(deps));

  PackageInfo pkgInfo;
  pkgInfo.baseUrl = "";  // Empty URL
  pkgInfo.location = "";

  // WHEN: downloadPackageFile is called
  bool result = testInterface->downloadPackageFile(pkgInfo);

  // THEN: should return failure with error message
  EXPECT_FALSE(result);
  EXPECT_NE(testInterface->getLastErrorMessage().find("URL is empty"), std::string::npos);
}

TEST_F(OpAppPackageManagerTest, TestDownloadPackageFile_ValidContentType_ReturnsSuccess)
{
  // GIVEN: a valid package URL with correct content type
  auto configuration = createConfigurationForDownloadTests();

  auto mockDownloader = std::make_unique<MockHttpDownloader>();
  mockDownloader->setDownloadSuccess("encrypted package content", "application/vnd.hbbtv.opapp.pkg");
  MockHttpDownloader* mockDownloaderPtr = mockDownloader.get();

  OpAppPackageManager::Dependencies deps;
  deps.httpDownloader = std::move(mockDownloader);
  auto testInterface = OpAppPackageManagerTestInterface::create(configuration, std::move(deps));

  PackageInfo pkgInfo;
  pkgInfo.baseUrl = "https://test.example.com/packages";
  pkgInfo.location = "app.cms";

  // WHEN: downloadPackageFile is called
  bool result = testInterface->downloadPackageFile(pkgInfo);

  // THEN: should return success
  EXPECT_TRUE(result);
  EXPECT_EQ(mockDownloaderPtr->getDownloadCallCount(), 1);
  EXPECT_EQ(mockDownloaderPtr->getLastDownloadUrl(), "https://test.example.com/packages/app.cms");
  EXPECT_EQ(testInterface->getCandidatePackageFile(),
            std::filesystem::path(PACKAGE_PATH) / "dest" / "app.cms");  // Preserves original filename
  EXPECT_TRUE(testInterface->getLastErrorMessage().empty());
}

TEST_F(OpAppPackageManagerTest, TestDownloadPackageFile_InvalidContentType_ReturnsFailure)
{
  // GIVEN: a package URL returning wrong content type
  auto configuration = createConfigurationForDownloadTests();

  auto mockDownloader = std::make_unique<MockHttpDownloader>();
  // Wrong content type per TS 103 606 Section 6.1.7
  mockDownloader->setDownloadSuccess("some content", "application/octet-stream");
  MockHttpDownloader* mockDownloaderPtr = mockDownloader.get();

  OpAppPackageManager::Dependencies deps;
  deps.httpDownloader = std::move(mockDownloader);
  auto testInterface = OpAppPackageManagerTestInterface::create(configuration, std::move(deps));

  PackageInfo pkgInfo;
  pkgInfo.baseUrl = "https://test.example.com/packages/";
  pkgInfo.location = "app.cms";

  // WHEN: downloadPackageFile is called
  bool result = testInterface->downloadPackageFile(pkgInfo);

  // THEN: should return failure after 3 attempts (retry logic)
  EXPECT_FALSE(result);
  EXPECT_EQ(mockDownloaderPtr->getDownloadCallCount(), 3);  // Max retry attempts
  EXPECT_NE(testInterface->getLastErrorMessage().find("Content-Type"), std::string::npos);
}

TEST_F(OpAppPackageManagerTest, TestDownloadPackageFile_HttpError_ReturnsFail)
{
  // GIVEN: a package URL returning HTTP 404
  auto configuration = createConfigurationForDownloadTests();

  auto mockDownloader = std::make_unique<MockHttpDownloader>();
  mockDownloader->setDownloadHttpError(404);
  MockHttpDownloader* mockDownloaderPtr = mockDownloader.get();

  OpAppPackageManager::Dependencies deps;
  deps.httpDownloader = std::move(mockDownloader);
  auto testInterface = OpAppPackageManagerTestInterface::create(configuration, std::move(deps));

  PackageInfo pkgInfo;
  pkgInfo.baseUrl = "https://test.example.com/packages/";
  pkgInfo.location = "app.cms";

  // WHEN: downloadPackageFile is called
  bool result = testInterface->downloadPackageFile(pkgInfo);

  // THEN: should return failure after 3 attempts
  EXPECT_FALSE(result);
  EXPECT_EQ(mockDownloaderPtr->getDownloadCallCount(), 3);  // Max retry attempts
  EXPECT_NE(testInterface->getLastErrorMessage().find("status code"), std::string::npos);
}

TEST_F(OpAppPackageManagerTest, TestDownloadPackageFile_NetworkFailure_ReturnsFail)
{
  // GIVEN: a network failure scenario
  auto configuration = createConfigurationForDownloadTests();

  auto mockDownloader = std::make_unique<MockHttpDownloader>();
  mockDownloader->setDownloadNetworkFailure();
  MockHttpDownloader* mockDownloaderPtr = mockDownloader.get();

  OpAppPackageManager::Dependencies deps;
  deps.httpDownloader = std::move(mockDownloader);
  auto testInterface = OpAppPackageManagerTestInterface::create(configuration, std::move(deps));

  PackageInfo pkgInfo;
  pkgInfo.baseUrl = "https://test.example.com/packages/";
  pkgInfo.location = "app.cms";

  // WHEN: downloadPackageFile is called
  bool result = testInterface->downloadPackageFile(pkgInfo);

  // THEN: should return failure after 3 attempts
  EXPECT_FALSE(result);
  EXPECT_EQ(mockDownloaderPtr->getDownloadCallCount(), 3);  // Max retry attempts
  EXPECT_NE(testInterface->getLastErrorMessage().find("3 attempts"), std::string::npos);
}

TEST_F(OpAppPackageManagerTest, TestDownloadPackageFile_CreatesDestinationDirectory)
{
  // GIVEN: a destination directory that doesn't exist
  auto configuration = createConfigurationForDownloadTests();
  configuration.m_WorkingDirectory = PACKAGE_PATH + "/new/nested/dest";

  auto mockDownloader = std::make_unique<MockHttpDownloader>();
  mockDownloader->setDownloadSuccess("content", "application/vnd.hbbtv.opapp.pkg");

  OpAppPackageManager::Dependencies deps;
  deps.httpDownloader = std::move(mockDownloader);
  auto testInterface = OpAppPackageManagerTestInterface::create(configuration, std::move(deps));

  PackageInfo pkgInfo;
  pkgInfo.baseUrl = "https://test.example.com/";
  pkgInfo.location = "app.cms";

  // WHEN: downloadPackageFile is called
  bool result = testInterface->downloadPackageFile(pkgInfo);

  // THEN: should create directory and succeed
  EXPECT_TRUE(result);
  EXPECT_TRUE(std::filesystem::exists(PACKAGE_PATH + "/new/nested/dest"));
}

TEST_F(OpAppPackageManagerTest, TestDownloadPackageFile_SetsCandidatePackageFile)
{
  // GIVEN: a successful download scenario
  auto configuration = createConfigurationForDownloadTests();

  auto mockDownloader = std::make_unique<MockHttpDownloader>();
  mockDownloader->setDownloadSuccess("package content", "application/vnd.hbbtv.opapp.pkg");

  OpAppPackageManager::Dependencies deps;
  deps.httpDownloader = std::move(mockDownloader);
  auto testInterface = OpAppPackageManagerTestInterface::create(configuration, std::move(deps));

  PackageInfo pkgInfo;
  pkgInfo.baseUrl = "https://test.example.com/";
  pkgInfo.location = "app.cms";

  // WHEN: downloadPackageFile is called
  bool result = testInterface->downloadPackageFile(pkgInfo);

  // THEN: should set candidate package file (preserves original filename from URL)
  EXPECT_TRUE(result);
  std::filesystem::path expectedPath = std::filesystem::path(PACKAGE_PATH) / "dest" / "app.cms";
  EXPECT_EQ(testInterface->getCandidatePackageFile(), expectedPath);
}

TEST_F(OpAppPackageManagerTest, TestDownloadPackageFile_MaxRetryAttempts)
{
  // GIVEN: a scenario that always fails
  auto configuration = createConfigurationForDownloadTests();

  auto mockDownloader = std::make_unique<MockHttpDownloader>();
  mockDownloader->setDownloadNetworkFailure();
  MockHttpDownloader* mockDownloaderPtr = mockDownloader.get();

  OpAppPackageManager::Dependencies deps;
  deps.httpDownloader = std::move(mockDownloader);
  auto testInterface = OpAppPackageManagerTestInterface::create(configuration, std::move(deps));

  PackageInfo pkgInfo;
  pkgInfo.baseUrl = "https://test.example.com/";
  pkgInfo.location = "app.cms";

  // WHEN: downloadPackageFile is called
  bool result = testInterface->downloadPackageFile(pkgInfo);

  // THEN: should attempt exactly 3 times (per TS 103 606 Section 6.1.7)
  EXPECT_FALSE(result);
  EXPECT_EQ(mockDownloaderPtr->getDownloadCallCount(), 3);
}

TEST_F(OpAppPackageManagerTest, TestDownloadPackageFile_HttpServerError_RetriesAndFails)
{
  // GIVEN: a server returning 500 errors
  auto configuration = createConfigurationForDownloadTests();

  auto mockDownloader = std::make_unique<MockHttpDownloader>();
  mockDownloader->setDownloadHttpError(500);
  MockHttpDownloader* mockDownloaderPtr = mockDownloader.get();

  OpAppPackageManager::Dependencies deps;
  deps.httpDownloader = std::move(mockDownloader);
  auto testInterface = OpAppPackageManagerTestInterface::create(configuration, std::move(deps));

  PackageInfo pkgInfo;
  pkgInfo.baseUrl = "https://test.example.com/";
  pkgInfo.location = "app.cms";

  // WHEN: downloadPackageFile is called
  bool result = testInterface->downloadPackageFile(pkgInfo);

  // THEN: should retry 3 times and fail
  EXPECT_FALSE(result);
  EXPECT_EQ(mockDownloaderPtr->getDownloadCallCount(), 3);
  EXPECT_NE(testInterface->getLastErrorMessage().find("500"), std::string::npos);
}

TEST_F(OpAppPackageManagerTest, TestDownloadPackageFile_CorrectUrlConstruction)
{
  // GIVEN: baseUrl without trailing slash and location without leading slash
  auto configuration = createConfigurationForDownloadTests();

  auto mockDownloader = std::make_unique<MockHttpDownloader>();
  mockDownloader->setDownloadSuccess("content", "application/vnd.hbbtv.opapp.pkg");
  MockHttpDownloader* mockDownloaderPtr = mockDownloader.get();

  OpAppPackageManager::Dependencies deps;
  deps.httpDownloader = std::move(mockDownloader);
  auto testInterface = OpAppPackageManagerTestInterface::create(configuration, std::move(deps));

  PackageInfo pkgInfo;
  pkgInfo.baseUrl = "https://test.example.com/packages";
  pkgInfo.location = "v1/app.cms";

  // WHEN: downloadPackageFile is called
  testInterface->downloadPackageFile(pkgInfo);

  // THEN: URL should be correctly constructed with separator
  EXPECT_EQ(mockDownloaderPtr->getLastDownloadUrl(), "https://test.example.com/packages/v1/app.cms");
}

TEST_F(OpAppPackageManagerTest, TestDownloadPackageFile_ConfigurableRetryAttempts)
{
  // GIVEN: a configuration with custom retry attempts
  auto configuration = createConfigurationForDownloadTests();
  configuration.m_DownloadMaxAttempts = 5;  // Custom max attempts

  auto mockDownloader = std::make_unique<MockHttpDownloader>();
  mockDownloader->setDownloadNetworkFailure();
  MockHttpDownloader* mockDownloaderPtr = mockDownloader.get();

  OpAppPackageManager::Dependencies deps;
  deps.httpDownloader = std::move(mockDownloader);
  auto testInterface = OpAppPackageManagerTestInterface::create(configuration, std::move(deps));

  PackageInfo pkgInfo;
  pkgInfo.baseUrl = "https://test.example.com/";
  pkgInfo.location = "app.cms";

  // WHEN: downloadPackageFile is called
  bool result = testInterface->downloadPackageFile(pkgInfo);

  // THEN: should attempt the configured number of times
  EXPECT_FALSE(result);
  EXPECT_EQ(mockDownloaderPtr->getDownloadCallCount(), 5);
}

// =============================================================================
// Integration Tests (require network access)
// =============================================================================

// Integration test - requires network access and a real OpApp FQDN
// Prefix with DISABLED_ so it doesn't run in CI; remove prefix to run manually
TEST_F(OpAppPackageManagerTest, DISABLED_IntegrationTest_RealRemoteFetch)
{
  // GIVEN: Real OpAppPackageManager with all real implementations
  auto configuration = createBasicConfiguration();
  configuration.m_WorkingDirectory = PACKAGE_PATH + "/dest";
  configuration.m_OpAppFqdn = "test.freeviewplay.tv";  // TODO: Replace with a real OpApp FQDN
  configuration.m_UserAgent = "HbbTV/1.6.1 (+DRM;+PVR;+RTSP;+OMID) orb/1.0";

  std::filesystem::create_directories(configuration.m_WorkingDirectory);

  // Track status via callback
  std::atomic<bool> callbackCalled{false};
  std::string lastError;
  OpAppPackageManager::PackageStatus finalStatus = OpAppPackageManager::PackageStatus::None;

  configuration.m_OnUpdateFailure = [&](OpAppPackageManager::PackageStatus status, const std::string& error) {
    finalStatus = status;
    lastError = error;
    callbackCalled = true;
    std::cout << "Update failure callback: status=" << static_cast<int>(status) << ", error=" << error << std::endl;
  };

  configuration.m_OnUpdateSuccess = [&](const std::string& packagePath) {
    finalStatus = OpAppPackageManager::PackageStatus::Installed;
    callbackCalled = true;
    std::cout << "Update success callback: packagePath=" << packagePath << std::endl;
  };

  OpAppPackageManager packageManager(configuration);

  // WHEN: Remote package check is performed
  std::cout << "Starting remote package check for FQDN: " << configuration.m_OpAppFqdn << std::endl;
  packageManager.start();

  // Wait for completion (with timeout)
  auto startTime = std::chrono::steady_clock::now();
  constexpr auto TIMEOUT = std::chrono::seconds(30);

  while (packageManager.isRunning()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    if (std::chrono::steady_clock::now() - startTime > TIMEOUT) {
      std::cerr << "Integration test timed out after 30 seconds" << std::endl;
      packageManager.stop();
      break;
    }
  }

  // THEN: Log the results (actual expectations depend on the real server)
  std::cout << "Package manager finished running" << std::endl;
  if (!lastError.empty()) {
    std::cout << "Last error: " << lastError << std::endl;
  }

  // Diagnostic: Check the downloaded file (filename is preserved from URL, e.g., opapp.cms)
  std::filesystem::path downloadedFile = configuration.m_WorkingDirectory / "opapp.cms";
  if (std::filesystem::exists(downloadedFile)) {
    // Print file size
    auto fileSize = std::filesystem::file_size(downloadedFile);
    std::cout << "Downloaded file: " << downloadedFile << std::endl;
    std::cout << "File size: " << fileSize << " bytes" << std::endl;

    // Read first 256 bytes to determine if binary or text
    std::ifstream file(downloadedFile, std::ios::binary);
    std::vector<char> buffer(std::min(static_cast<uintmax_t>(256), fileSize));
    file.read(buffer.data(), buffer.size());
    size_t bytesRead = file.gcount();

    // Check for binary content (null bytes or high proportion of non-printable chars)
    int nonPrintableCount = 0;
    bool hasNullByte = false;
    for (size_t i = 0; i < bytesRead; ++i) {
      unsigned char c = static_cast<unsigned char>(buffer[i]);
      if (c == 0) {
        hasNullByte = true;
        break;
      }
      if (c < 32 && c != '\n' && c != '\r' && c != '\t') {
        nonPrintableCount++;
      }
    }

    bool isBinary = hasNullByte || (bytesRead > 0 && static_cast<size_t>(nonPrintableCount) > bytesRead / 4);
    std::cout << "File type: " << (isBinary ? "BINARY" : "TEXT") << std::endl;

    if (!isBinary) {
      // Print first 100 characters
      size_t printLen = std::min(static_cast<size_t>(100), bytesRead);
      std::string preview(buffer.data(), printLen);
      std::cout << "First " << printLen << " characters: " << std::endl;
      std::cout << "---" << std::endl;
      std::cout << preview << std::endl;
      std::cout << "---" << std::endl;
    } else {
      // Print hex dump of first 32 bytes
      std::cout << "First 32 bytes (hex): ";
      for (size_t i = 0; i < std::min(static_cast<size_t>(32), bytesRead); ++i) {
        std::cout << std::hex << std::setfill('0') << std::setw(2)
                  << static_cast<int>(static_cast<unsigned char>(buffer[i])) << " ";
      }
      std::cout << std::dec << std::endl;
    }
  } else {
    std::cout << "Downloaded file not found at: " << downloadedFile << std::endl;
  }

  // For a real integration test, you would assert based on expected server state
  EXPECT_FALSE(packageManager.isRunning());
}
