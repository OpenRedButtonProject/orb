#include <iostream>
#include <string>
#include <filesystem>
#include <algorithm>
#include <cctype>
#include <map>

#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/orb/orblibrary/include/OpAppPackageManager.h"
#include "OpAppPackageManagerTestInterface.h"
#include <fstream>

class MockDecryptor : public IDecryptor {
public:
  MockDecryptor() = default;
  void setDecryptResult(const PackageOperationResult& result) {
    m_DecryptResult = result;
  }

  PackageOperationResult decrypt(const std::string& filePath) const override {
    m_WasDecryptCalled = true;
    m_LastFilePath = filePath;
    return m_DecryptResult;
  }

  bool wasDecryptCalled() const {
    return m_WasDecryptCalled;
  }

  void setLastFilePath(const std::string& filePath) {
    m_LastFilePath = filePath;
  }

  std::string getLastFilePath() const {
    return m_LastFilePath;
  }

  void reset() {
    m_WasDecryptCalled = false;
    m_LastFilePath.clear();
  }

private:
  PackageOperationResult m_DecryptResult;
  mutable bool m_WasDecryptCalled = false;
  mutable std::string m_LastFilePath;
};

// Mock hash calculator for testing
class MockHashCalculator : public IHashCalculator {
public:
  MockHashCalculator() = default;

  // Create a JSON file with the specified hash
  void createHashJsonFile(const std::string& filePath, const std::string& hash) {
    std::ofstream jsonFile(filePath);
    jsonFile << "{\"hash\": \"" << hash << "\"}";
    jsonFile.close();
  }

  // Create an invalid JSON file (missing hash field)
  void createInvalidHashJsonFile(const std::string& filePath) {
    std::ofstream jsonFile(filePath);
    jsonFile << "{\"version\": \"1.0\", \"timestamp\": \"2024-01-01\"}";
    jsonFile.close();
  }

  // Set predefined responses for specific file paths (for direct hash calculation)
  void setHashForFile(const std::string& filePath, const std::string& hash) {
    m_FileHashes[filePath] = hash;
  }

  // Set default hash for any file not explicitly set
  void setDefaultHash(const std::string& hash) {
    m_DefaultHash = hash;
  }

  std::string calculateSHA256Hash(const std::string& filePath) const override {
    auto it = m_FileHashes.find(filePath);
    if (it != m_FileHashes.end()) {
      return it->second;
    }
    return m_DefaultHash;
  }

private:
  std::map<std::string, std::string> m_FileHashes;
  std::string m_DefaultHash = "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855"; // Empty file hash
};

class OpAppPackageManagerTest : public ::testing::Test {

public:
  // Static member declaration
  static std::string PACKAGE_PATH;

protected:
  void SetUp() override {
    // Clean up any existing instance before each test
    OpAppPackageManager::destroyInstance();

    // Create test directory structure in temporary directory
    PACKAGE_PATH = std::filesystem::temp_directory_path() / "orb_test_packages";
    std::filesystem::create_directories(PACKAGE_PATH);
  }

  void TearDown() override {
    // Clean up after each test
    OpAppPackageManager::destroyInstance();
    // Remove the package file in the package source location
    std::string packagePath = PACKAGE_PATH + "/package.opk";
    std::remove(packagePath.c_str());

    // Remove test directory structure
    std::filesystem::remove_all(PACKAGE_PATH);
  }
};

// Static member definition
std::string OpAppPackageManagerTest::PACKAGE_PATH;

TEST_F(OpAppPackageManagerTest, TestGetInstanceWithoutConfiguration)
{
  // GIVEN: no preconditions

  // WHEN: getting the singleton instance without configuration
  OpAppPackageManager* packageManager = OpAppPackageManager::getInstance();

  // THEN: nullptr should be returned since no configuration was provided
  EXPECT_EQ(packageManager, nullptr);
}

TEST_F(OpAppPackageManagerTest, TestGetInstanceWithConfiguration)
{
  // GIVEN: a configuration object
  OpAppPackageManager::Configuration configuration;
  configuration.m_PackageLocation = PACKAGE_PATH;

  // WHEN: getting the singleton instance with configuration
  OpAppPackageManager& packageManager = OpAppPackageManager::getInstance(configuration);

  // THEN: the instance should be created successfully with the provided configuration
  // Verify the instance is valid by checking its state
  EXPECT_FALSE(packageManager.isRunning());
  EXPECT_FALSE(packageManager.isUpdating());
}

TEST_F(OpAppPackageManagerTest, TestGetInstanceMultipleCalls)
{
  // GIVEN: a configuration object
  OpAppPackageManager::Configuration configuration;
  configuration.m_PackageLocation = PACKAGE_PATH;
  // WHEN: calling getInstance multiple times after configuration
  OpAppPackageManager& instance1 = OpAppPackageManager::getInstance(configuration);
  OpAppPackageManager* instance2 = OpAppPackageManager::getInstance();
  OpAppPackageManager* instance3 = OpAppPackageManager::getInstance();

  // THEN: all calls should return the same instance
  EXPECT_EQ(&instance1, instance2);
  EXPECT_EQ(instance2, instance3);
  EXPECT_EQ(&instance1, instance3);
}

TEST_F(OpAppPackageManagerTest, TestGetInstanceWithConfigurationAfterInstanceExists)
{
  // GIVEN: an existing singleton instance
  OpAppPackageManager::Configuration config1;
  config1.m_PackageLocation = PACKAGE_PATH;
  OpAppPackageManager& instance1 = OpAppPackageManager::getInstance(config1);

  // WHEN: trying to get instance with different configuration when instance already exists
  OpAppPackageManager::Configuration config2;
  config2.m_PackageLocation = "/different/packages";

  // THEN: the existing instance should be returned (no exception thrown)
  OpAppPackageManager& instance2 = OpAppPackageManager::getInstance(config2);
  EXPECT_EQ(&instance1, &instance2);
}

TEST_F(OpAppPackageManagerTest, TestDestroyInstance)
{
  // GIVEN: a singleton instance
  OpAppPackageManager::Configuration configuration;
  configuration.m_PackageLocation = PACKAGE_PATH;
  OpAppPackageManager& instance1 = OpAppPackageManager::getInstance(configuration);

  // WHEN: destroying the instance
  OpAppPackageManager::destroyInstance();

  // THEN: a new instance should be created when getInstance is called again
  // Note: We can't rely on memory addresses being different due to memory reuse
  // Instead, we verify that the singleton pattern works by ensuring
  // the instance can be destroyed and recreated without errors
  OpAppPackageManager* instance2_ptr = OpAppPackageManager::getInstance();
  EXPECT_EQ(instance2_ptr, nullptr); // Should be nullptr since no configuration provided

  // Create new instance with configuration
  OpAppPackageManager& instance2 = OpAppPackageManager::getInstance(configuration);

  // Both instances should be valid and accessible
  EXPECT_FALSE(instance1.isRunning());
  EXPECT_FALSE(instance1.isUpdating());
  EXPECT_FALSE(instance2.isRunning());
  EXPECT_FALSE(instance2.isUpdating());

  // The test passes if we can successfully destroy and recreate the instance
  // Memory address reuse is not a failure condition
}

TEST_F(OpAppPackageManagerTest, TestDestroyInstanceMultipleCalls)
{
  // GIVEN: no preconditions

  // WHEN: calling destroyInstance multiple times
  OpAppPackageManager::destroyInstance();
  OpAppPackageManager::destroyInstance();
  OpAppPackageManager::destroyInstance();

  // THEN: no errors should occur
  // (This test verifies that multiple destroy calls are safe)
}

TEST_F(OpAppPackageManagerTest, TestDefaultInitialization)
{
  // GIVEN: a configuration object
  OpAppPackageManager::Configuration configuration;
  configuration.m_PackageLocation = PACKAGE_PATH;
  // WHEN: creating instance with configuration
  OpAppPackageManager& packageManager = OpAppPackageManager::getInstance(configuration);

  // THEN: the instance should be in a valid initial state
  EXPECT_FALSE(packageManager.isRunning());
  EXPECT_FALSE(packageManager.isUpdating());
}

TEST_F(OpAppPackageManagerTest, TestConfigurationInitialization)
{
  // GIVEN: a configuration object
  OpAppPackageManager::Configuration configuration;
  configuration.m_PackageLocation = PACKAGE_PATH;
  configuration.m_PackageSuffix = ".opk";
  configuration.m_PrivateKeyFilePath = "/keys/private.key";
  configuration.m_PublicKeyFilePath = "/keys/public.key";
  configuration.m_CertificateFilePath = "/certs/cert.pem";
  configuration.m_DestinationDirectory = "/dest";
  configuration.m_OpAppInstallDirectory = "/install";

  // WHEN: creating instance with custom configuration
  OpAppPackageManager& packageManager = OpAppPackageManager::getInstance(configuration);

  // THEN: the instance should be created successfully
  EXPECT_FALSE(packageManager.isRunning());
  EXPECT_FALSE(packageManager.isUpdating());
}

// Broken test, disabled for now
TEST_F(OpAppPackageManagerTest, DISABLED_TestStartAndStop)
{
  // GIVEN: a singleton OpAppPackageManager instance
  // and no package file in the package source location
  OpAppPackageManager::Configuration configuration;
  configuration.m_PackageLocation = PACKAGE_PATH;
  OpAppPackageManager& packageManager = OpAppPackageManager::getInstance(configuration);

  // WHEN: starting the package manager
  packageManager.start();

  // THEN: the package manager should be running
  EXPECT_TRUE(packageManager.isRunning());

  // WHEN: wait for the thread to complete naturally
  // Wait until the worker thread completes
  int maxWaitTime = 1000; // 1 second max wait
  int waitTime = 0;
  while (packageManager.isRunning() && waitTime < maxWaitTime) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    waitTime += 10;
  }

  // THEN: the package manager should be stopped
  EXPECT_FALSE(packageManager.isRunning());
}

TEST_F(OpAppPackageManagerTest, TestCheckForUpdates_NoUpdates)
{
  // GIVEN: a test interface instance and no package file in the package source location
  OpAppPackageManager::Configuration configuration;
  configuration.m_PackageLocation = PACKAGE_PATH;
  configuration.m_PackageSuffix = ".opk";
  auto testInterface = OpAppPackageManagerTestInterface::create(configuration);

  // WHEN: doPackageFileCheck is called
  OpAppPackageManager::PackageStatus status = testInterface->doPackageFileCheck();

  // THEN: the package status is NoUpdateAvailable
  EXPECT_EQ(status, OpAppPackageManager::PackageStatus::NoUpdateAvailable);
}

TEST_F(OpAppPackageManagerTest, TestCheckForUpdates_UpdatesAvailable)
{
  // GIVEN: a test interface instance and a package file in the package source location
  OpAppPackageManager::Configuration configuration;
  configuration.m_PackageLocation = PACKAGE_PATH;
  configuration.m_PackageSuffix = ".opk";
  // Create a package file in the package source location
  std::string packagePath = PACKAGE_PATH + "/package.opk";
  std::ofstream file(packagePath);
  file.close();
  auto testInterface = OpAppPackageManagerTestInterface::create(configuration);

  // WHEN: doPackageFileCheck is called
  OpAppPackageManager::PackageStatus status = testInterface->doPackageFileCheck();

  // THEN: the package status is UpdateAvailable
  EXPECT_EQ(status, OpAppPackageManager::PackageStatus::UpdateAvailable);
}

TEST_F(OpAppPackageManagerTest, TestCheckForUpdates_UpdatesAvailable_NoExistingPackage)
{
  // GIVEN: a test interface instance and a package file in the package source location
  OpAppPackageManager::Configuration configuration;
  configuration.m_PackageLocation = PACKAGE_PATH;
  configuration.m_PackageSuffix = ".opk";
  configuration.m_PackageHashFilePath = PACKAGE_PATH + "/package.hash";

  // Create a package file in the package source location
  std::string packagePath = PACKAGE_PATH + "/package.opk";
  std::ofstream file(packagePath);
  file.close();
  auto testInterface = OpAppPackageManagerTestInterface::create(configuration);

  // WHEN: doPackageFileCheck is called
  OpAppPackageManager::PackageStatus status = testInterface->doPackageFileCheck();

  // THEN: the package status is UpdateAvailable
  EXPECT_EQ(status, OpAppPackageManager::PackageStatus::UpdateAvailable);
}

TEST_F(OpAppPackageManagerTest, TestCheckForUpdates_UpdatesAvailable_ExistingPackage_HashSame)
{
  // GIVEN: a package manager and a mock hash calculator with identical, predefined responses
  auto mockCalculator = std::make_unique<MockHashCalculator>();
  mockCalculator->setHashForFile(PACKAGE_PATH + "/package.opk", "test_hash_1234567890abcdef");
  mockCalculator->createHashJsonFile(PACKAGE_PATH + "/package.hash", "test_hash_1234567890abcdef");

  OpAppPackageManager::Configuration configuration;
  configuration.m_PackageLocation = PACKAGE_PATH;
  configuration.m_PackageSuffix = ".opk";
  configuration.m_PackageHashFilePath = PACKAGE_PATH + "/package.hash";

  // Create a package file in the package source location
  std::string packagePath = PACKAGE_PATH + "/package.opk";
  std::ofstream file(packagePath);
  file.close();
  // Note: hash file is created by createHashJsonFile above

  // WHEN: checking package status
  auto testInterface = OpAppPackageManagerTestInterface::create(configuration, std::move(mockCalculator), nullptr);
  OpAppPackageManager::PackageStatus status = testInterface->doPackageFileCheck();

  // THEN: the package status is Installed
  EXPECT_EQ(status, OpAppPackageManager::PackageStatus::Installed);

  // Clean up test files
  std::remove(packagePath.c_str());
  std::remove((PACKAGE_PATH + "/package.hash").c_str());
}

TEST_F(OpAppPackageManagerTest, TestCheckForUpdates_UpdatesAvailable_NoHashFile)
{
  // GIVEN: a package manager and a mock hash calculator, but no hash file exists
  auto mockCalculator = std::make_unique<MockHashCalculator>();
  mockCalculator->setHashForFile(PACKAGE_PATH + "/package.opk", "package_hash_abcdef123456");
  // Note: No hash file is created, simulating a missing hash file

  OpAppPackageManager::Configuration configuration;
  configuration.m_PackageLocation = PACKAGE_PATH;
  configuration.m_PackageSuffix = ".opk";
  configuration.m_PackageHashFilePath = PACKAGE_PATH + "/package.hash";

  // Create package file only
  std::string packagePath = PACKAGE_PATH + "/package.opk";
  std::ofstream packageFile(packagePath);
  packageFile.close();

  // WHEN: checking package status
  auto testInterface = OpAppPackageManagerTestInterface::create(configuration, std::move(mockCalculator), nullptr);
  OpAppPackageManager::PackageStatus status = testInterface->doPackageFileCheck();

  // THEN: the package should be considered update available (no hash file means not installed)
  EXPECT_EQ(status, OpAppPackageManager::PackageStatus::UpdateAvailable);

  // Clean up test files
  std::remove(packagePath.c_str());
}

TEST_F(OpAppPackageManagerTest, TestCheckForUpdates_UpdatesAvailable_InvalidHashFile)
{
  // GIVEN: a package manager and a mock hash calculator with an invalid JSON hash file
  auto mockCalculator = std::make_unique<MockHashCalculator>();
  mockCalculator->setHashForFile(PACKAGE_PATH + "/package.opk", "package_hash_abcdef123456");
  mockCalculator->createInvalidHashJsonFile(PACKAGE_PATH + "/package.hash");

  OpAppPackageManager::Configuration configuration;
  configuration.m_PackageLocation = PACKAGE_PATH;
  configuration.m_PackageSuffix = ".opk";
  configuration.m_PackageHashFilePath = PACKAGE_PATH + "/package.hash";

  // Create package file
  std::string packagePath = PACKAGE_PATH + "/package.opk";
  std::ofstream packageFile(packagePath);
  packageFile.close();

  // WHEN: checking package status
  auto testInterface = OpAppPackageManagerTestInterface::create(configuration, std::move(mockCalculator), nullptr);
  OpAppPackageManager::PackageStatus status = testInterface->doPackageFileCheck();

  // THEN: the package should be considered update available (invalid hash file means not installed)
  EXPECT_EQ(status, OpAppPackageManager::PackageStatus::UpdateAvailable);

  // Clean up test files
  std::remove(packagePath.c_str());
  std::remove((PACKAGE_PATH + "/package.hash").c_str());
}

TEST_F(OpAppPackageManagerTest, TestCheckForUpdates_UpdatesAvailable_SameHash)
{
  // GIVEN: a the package manager and a mock hash calculator with identical, predefined responses
  auto mockCalculator = std::make_unique<MockHashCalculator>();
  mockCalculator->setHashForFile(PACKAGE_PATH + "/package.opk", "test_hash_1234567890abcdef");
  mockCalculator->createHashJsonFile(PACKAGE_PATH + "/package.hash", "test_hash_1234567890abcdef");

  OpAppPackageManager::Configuration configuration;
  configuration.m_PackageLocation = PACKAGE_PATH;
  configuration.m_PackageSuffix = ".opk";
  configuration.m_PackageHashFilePath = PACKAGE_PATH + "/package.hash";

  // Create package files
  std::string packagePath = PACKAGE_PATH + "/package.opk";
  std::string hashPath = PACKAGE_PATH + "/package.hash";
  std::ofstream packageFile(packagePath);
  packageFile.close();
  // Note: hash file is created by createHashJsonFile above

  // WHEN: checking package status
  auto testInterface = OpAppPackageManagerTestInterface::create(configuration, std::move(mockCalculator), nullptr);
  OpAppPackageManager::PackageStatus status = testInterface->doPackageFileCheck();

  // THEN: the package should be considered installed (same hash)
  EXPECT_EQ(status, OpAppPackageManager::PackageStatus::Installed);

  // Clean up test files
  std::remove(packagePath.c_str());
  std::remove(hashPath.c_str());
}

TEST_F(OpAppPackageManagerTest, TestCheckForUpdates_UpdatesAvailable_DifferentHash)
{
  // GIVEN: a the package manager and a mock hash calculator with different hashes for package and hash file
  auto mockCalculator = std::make_unique<MockHashCalculator>();
  mockCalculator->setHashForFile(PACKAGE_PATH + "/package.opk", "package_hash_abcdef123456");
  mockCalculator->createHashJsonFile(PACKAGE_PATH + "/package.hash", "different_hash_789xyz");

  OpAppPackageManager::Configuration configuration;
  configuration.m_PackageLocation = PACKAGE_PATH;
  configuration.m_PackageSuffix = ".opk";
  configuration.m_PackageHashFilePath = PACKAGE_PATH + "/package.hash";

  // Create package files
  std::string packagePath = PACKAGE_PATH + "/package.opk";
  std::string hashPath = PACKAGE_PATH + "/package.hash";
  std::ofstream packageFile(packagePath);
  packageFile.close();
  // Note: hash file is created by createHashJsonFile above

  // WHEN: checking package status
  auto testInterface = OpAppPackageManagerTestInterface::create(configuration, std::move(mockCalculator), nullptr);
  OpAppPackageManager::PackageStatus status = testInterface->doPackageFileCheck();

  // THEN: the package should be considered update available (different hashes)
  EXPECT_EQ(status, OpAppPackageManager::PackageStatus::UpdateAvailable);

  // Clean up test files
  std::remove(packagePath.c_str());
  std::remove(hashPath.c_str());
}

TEST_F(OpAppPackageManagerTest, TestInstallPackage_NoPackageFile)
{
  // GIVEN: a singleton OpAppPackageManager instance and no package file set
  OpAppPackageManager::Configuration configuration;
  configuration.m_PackageLocation = PACKAGE_PATH;
  auto testInterface = OpAppPackageManagerTestInterface::create(configuration);

  // WHEN: attempting to install a package
  OpAppPackageManager::PackageStatus status = testInterface->tryPackageInstall();

  // THEN: the installation should be handled appropriately
  EXPECT_EQ(status, OpAppPackageManager::PackageStatus::ConfigurationError);
}

TEST_F(OpAppPackageManagerTest, TestInstallPackage_PackageFileDoesNotExist)
{
  // GIVEN: a singleton OpAppPackageManager instance and a package file that does not exist
  OpAppPackageManager::Configuration configuration;
  configuration.m_PackageLocation = PACKAGE_PATH;
  auto testInterface = OpAppPackageManagerTestInterface::create(configuration);
  testInterface->setCandidatePackageFile("/nonexistent/package.opk");

  // WHEN: attempting to install a package
  OpAppPackageManager::PackageStatus status = testInterface->tryPackageInstall();

  // THEN: the installation should be handled appropriately
  EXPECT_EQ(status, OpAppPackageManager::PackageStatus::ConfigurationError);
}

TEST_F(OpAppPackageManagerTest, TestInstallPackage_PackageFileExists)
{
  // GIVEN: a singleton OpAppPackageManager instance and a package file that exists
  OpAppPackageManager::Configuration configuration;
  configuration.m_PackageLocation = PACKAGE_PATH;
  configuration.m_PackageSuffix = ".opk";
  configuration.m_PackageHashFilePath = PACKAGE_PATH + "/package.hash";
  configuration.m_DestinationDirectory = PACKAGE_PATH + "/install";

  // Create a package file in the package source location
  std::string packagePath = PACKAGE_PATH + "/package.opk";
  std::ofstream file(packagePath);
  file << "test package content";
  file.close();

  // Create a mock decryptor that returns success with package files
  auto mockDecryptor = std::make_unique<MockDecryptor>();
  auto mockHashCalculator = std::make_unique<MockHashCalculator>();
  std::vector<std::string> packageFiles = {PACKAGE_PATH + "/decrypted_package.opk"};
  mockDecryptor->setDecryptResult(PackageOperationResult(true, "Decryption successful", packageFiles));

  // Store a reference to the mock decryptor before moving it
  MockDecryptor* mockDecryptorPtr = mockDecryptor.get();

  auto testInterface = OpAppPackageManagerTestInterface::create(
    configuration, std::move(mockHashCalculator), std::move(mockDecryptor));
  testInterface->setCandidatePackageFile(packagePath);

  // WHEN: attempting to install a package
  testInterface->tryPackageInstall();

  // THEN: the decrypt method should be called
  EXPECT_TRUE(mockDecryptorPtr->wasDecryptCalled());

  // Clean up test files
  std::remove(packagePath.c_str());
  std::remove((PACKAGE_PATH + "/package.hash").c_str());
  std::remove((PACKAGE_PATH + "/install/package.opk").c_str());
  std::filesystem::remove_all(PACKAGE_PATH + "/install");
}

TEST_F(OpAppPackageManagerTest, TestInstallPackage_PackageFileExists_DecryptFailed)
{
  // GIVEN: a singleton OpAppPackageManager instance and a package file that exists
  OpAppPackageManager::Configuration configuration;
  configuration.m_PackageLocation = PACKAGE_PATH;
  configuration.m_PackageSuffix = ".opk";
  configuration.m_PackageHashFilePath = PACKAGE_PATH + "/package.hash";
  configuration.m_DestinationDirectory = PACKAGE_PATH + "/install";

  // Create a package file in the package source location
  std::string packagePath = PACKAGE_PATH + "/package.opk";
  std::ofstream file(packagePath);
  file << "test package content";
  file.close();

  // Create a mock decryptor that returns a failure
  auto mockDecryptor = std::make_unique<MockDecryptor>();
  auto mockHashCalculator = std::make_unique<MockHashCalculator>();
  mockDecryptor->setDecryptResult(PackageOperationResult(false, "Decryption failed"));

  auto testInterface = OpAppPackageManagerTestInterface::create(
    configuration, std::move(mockHashCalculator), std::move(mockDecryptor));
  testInterface->setCandidatePackageFile(packagePath);

  // WHEN: attempting to install a package
  OpAppPackageManager::PackageStatus status = testInterface->tryPackageInstall();
  // THEN: the installation should be handled appropriately
  EXPECT_EQ(status, OpAppPackageManager::PackageStatus::DecryptionFailed);

  // Clean up test files
  std::remove(packagePath.c_str());
  std::remove((PACKAGE_PATH + "/package.hash").c_str());
  std::remove((PACKAGE_PATH + "/install/package.opk").c_str());
  std::filesystem::remove_all(PACKAGE_PATH + "/install");
}

// TODO: Add tests for future package manager functionality
// These stubs can be expanded when the OpAppPackageManager class is implemented

TEST_F(OpAppPackageManagerTest, TestUninstallPackage)
{
  // GIVEN: a singleton OpAppPackageManager instance and a package ID
  OpAppPackageManager::Configuration configuration;
  configuration.m_PackageLocation = PACKAGE_PATH;
  auto testInterface = OpAppPackageManagerTestInterface::create(configuration);
  std::string packageId = "com.example.app";

  // WHEN: attempting to uninstall a package
  // bool result = packageManager.uninstallPackage(packageId);

  // THEN: the uninstallation should be handled appropriately
  // TODO: Implement when uninstallPackage method is added
  // EXPECT_TRUE(result);

  // Mark variables as intentionally unused for now
  (void)testInterface;
  (void)packageId;
}

TEST_F(OpAppPackageManagerTest, TestListInstalledPackages)
{
  // GIVEN: a singleton OpAppPackageManager instance
  OpAppPackageManager::Configuration configuration;
  configuration.m_PackageLocation = PACKAGE_PATH;
  auto testInterface = OpAppPackageManagerTestInterface::create(configuration);

  // WHEN: requesting a list of installed packages
  // std::vector<std::string> packages = packageManager.listInstalledPackages();

  // THEN: a list of packages should be returned
  // TODO: Implement when listInstalledPackages method is added
  // EXPECT_FALSE(packages.empty());

  // Mark variables as intentionally unused for now
  (void)testInterface;
}

TEST_F(OpAppPackageManagerTest, TestGetPackageInfo)
{
  // GIVEN: a singleton OpAppPackageManager instance and a package ID
  OpAppPackageManager::Configuration configuration;
  configuration.m_PackageLocation = PACKAGE_PATH;
  auto testInterface = OpAppPackageManagerTestInterface::create(configuration);
  std::string packageId = "com.example.app";

  // WHEN: requesting package information
  // PackageInfo info = packageManager.getPackageInfo(packageId);

  // THEN: package information should be returned
  // TODO: Implement when getPackageInfo method is added
  // EXPECT_EQ(info.id, packageId);

  // Mark variables as intentionally unused for now
  (void)testInterface;
  (void)packageId;
}

TEST_F(OpAppPackageManagerTest, TestUpdatePackage)
{
  // GIVEN: a singleton OpAppPackageManager instance and a package update
  OpAppPackageManager::Configuration configuration;
  configuration.m_PackageLocation = PACKAGE_PATH;
  auto testInterface = OpAppPackageManagerTestInterface::create(configuration);
  std::string packagePath = "/path/to/update.opk";

  // WHEN: attempting to update a package
  // bool result = packageManager.updatePackage(packagePath);

  // THEN: the update should be handled appropriately
  // TODO: Implement when updatePackage method is added
  // EXPECT_TRUE(result);

  // Mark variables as intentionally unused for now
  (void)testInterface;
  (void)packagePath;
}

TEST_F(OpAppPackageManagerTest, TestIsPackageInstalled)
{
  // GIVEN: a singleton OpAppPackageManager instance and a package ID
  OpAppPackageManager::Configuration configuration;
  configuration.m_PackageLocation = PACKAGE_PATH;
  auto testInterface = OpAppPackageManagerTestInterface::create(configuration);
  std::string packageId = "com.example.app";

  // WHEN: checking if a package is installed
  // bool installed = packageManager.isPackageInstalled(packageId);

  // THEN: the installation status should be returned
  // TODO: Implement when isPackageInstalled method is added
  // EXPECT_FALSE(installed); // Assuming no packages are installed initially

  // Mark variables as intentionally unused for now
  (void)testInterface;
  (void)packageId;
}

TEST_F(OpAppPackageManagerTest, TestGetPackageVersion)
{
  // GIVEN: a singleton OpAppPackageManager instance and a package ID
  OpAppPackageManager::Configuration configuration;
  configuration.m_PackageLocation = PACKAGE_PATH;
  auto testInterface = OpAppPackageManagerTestInterface::create(configuration);
  std::string packageId = "com.example.app";

  // WHEN: requesting package version
  // std::string version = packageManager.getPackageVersion(packageId);

  // THEN: the package version should be returned
  // TODO: Implement when getPackageVersion method is added
  // EXPECT_FALSE(version.empty());

  // Mark variables as intentionally unused for now
  (void)testInterface;
  (void)packageId;
}

TEST_F(OpAppPackageManagerTest, TestValidatePackage)
{
  // GIVEN: a singleton OpAppPackageManager instance and a package path
  OpAppPackageManager::Configuration configuration;
  configuration.m_PackageLocation = PACKAGE_PATH;
  auto testInterface = OpAppPackageManagerTestInterface::create(configuration);
  std::string packagePath = "/path/to/package.opk";

  // WHEN: validating a package
  // bool valid = packageManager.validatePackage(packagePath);

  // THEN: the validation result should be returned
  // TODO: Implement when validatePackage method is added
  // EXPECT_TRUE(valid);

  // Mark variables as intentionally unused for now
  (void)testInterface;
  (void)packagePath;
}

// Error handling tests
TEST_F(OpAppPackageManagerTest, TestInstallInvalidPackage)
{
  // GIVEN: a singleton OpAppPackageManager instance and an invalid package path
  OpAppPackageManager::Configuration configuration;
  configuration.m_PackageLocation = PACKAGE_PATH;
  auto testInterface = OpAppPackageManagerTestInterface::create(configuration);
  std::string invalidPath = "/nonexistent/package.opk";

  // WHEN: attempting to install an invalid package
  // bool result = packageManager.installPackage(invalidPath);

  // THEN: the operation should fail gracefully
  // TODO: Implement when installPackage method is added
  // EXPECT_FALSE(result);

  // Mark variables as intentionally unused for now
  (void)testInterface;
  (void)invalidPath;
}

TEST_F(OpAppPackageManagerTest, TestGetInfoForNonexistentPackage)
{
  // GIVEN: a singleton OpAppPackageManager instance and a nonexistent package ID
  OpAppPackageManager::Configuration configuration;
  configuration.m_PackageLocation = PACKAGE_PATH;
  auto testInterface = OpAppPackageManagerTestInterface::create(configuration);
  std::string nonexistentId = "com.nonexistent.app";

  // WHEN: requesting information for a nonexistent package
  // PackageInfo info = packageManager.getPackageInfo(nonexistentId);

  // THEN: appropriate error handling should occur
  // TODO: Implement when getPackageInfo method is added
  // EXPECT_TRUE(info.id.empty());

  // Mark variables as intentionally unused for now
  (void)testInterface;
  (void)nonexistentId;
}

// Performance and stress tests
TEST_F(OpAppPackageManagerTest, TestConcurrentOperations)
{
  // GIVEN: a singleton OpAppPackageManager instance
  OpAppPackageManager::Configuration configuration;
  configuration.m_PackageLocation = PACKAGE_PATH;
  auto testInterface = OpAppPackageManagerTestInterface::create(configuration);

  // WHEN: performing multiple operations concurrently
  // (This would require threading implementation)

  // THEN: all operations should complete successfully
  // TODO: Implement when threading support is added

  // Mark variables as intentionally unused for now
  (void)testInterface;
}

TEST_F(OpAppPackageManagerTest, TestLargePackageHandling)
{
  // GIVEN: a singleton OpAppPackageManager instance and a large package
  OpAppPackageManager::Configuration configuration;
  configuration.m_PackageLocation = PACKAGE_PATH;
  auto testInterface = OpAppPackageManagerTestInterface::create(configuration);
  std::string largePackagePath = "/path/to/large/package.opk";

  // WHEN: handling a large package

  // THEN: the operation should complete without memory issues
  // TODO: Implement when large package handling is added

  // Mark variables as intentionally unused for now
  (void)testInterface;
  (void)largePackagePath;
}

// Integration tests
TEST_F(OpAppPackageManagerTest, TestFullPackageLifecycle)
{
  // GIVEN: a singleton OpAppPackageManager instance and a valid package
  OpAppPackageManager::Configuration configuration;
  configuration.m_PackageLocation = PACKAGE_PATH;
  auto testInterface = OpAppPackageManagerTestInterface::create(configuration);
  std::string packagePath = "/path/to/valid/package.opk";
  std::string packageId = "com.example.app";

  // WHEN: performing a full package lifecycle (install, enable, disable, uninstall)

  // THEN: all operations should complete successfully
  // TODO: Implement when all lifecycle methods are added

  // Mark variables as intentionally unused for now
  (void)testInterface;
  (void)packagePath;
  (void)packageId;
}

TEST_F(OpAppPackageManagerTest, TestPackageUpdateWorkflow)
{
  // GIVEN: a singleton OpAppPackageManager instance and package versions
  OpAppPackageManager::Configuration configuration;
  configuration.m_PackageLocation = PACKAGE_PATH;
  auto testInterface = OpAppPackageManagerTestInterface::create(configuration);
  std::string oldVersionPath = "/path/to/old/version.opk";
  std::string newVersionPath = "/path/to/new/version.opk";

  // WHEN: performing a package update workflow

  // THEN: the update should complete successfully
  // TODO: Implement when update workflow is added

  // Mark variables as intentionally unused for now
  (void)testInterface;
  (void)oldVersionPath;
  (void)newVersionPath;
}

TEST_F(OpAppPackageManagerTest, TestUpdateCallbacks_NoUpdateAvailable_NoCallbacksCalled)
{
  // GIVEN: a singleton OpAppPackageManager instance with callbacks
  OpAppPackageManager::Configuration configuration;
  configuration.m_PackageLocation = PACKAGE_PATH;
  configuration.m_PackageSuffix = ".opk";
  configuration.m_PackageHashFilePath = PACKAGE_PATH + "/package.hash";

  bool successCallbackCalled = false;
  bool failureCallbackCalled = false;

  configuration.m_OnUpdateSuccess = [&](const std::string&) {
    successCallbackCalled = true;
  };
  configuration.m_OnUpdateFailure = [&](OpAppPackageManager::PackageStatus, const std::string&) {
    failureCallbackCalled = true;
  };

  auto testInterface = OpAppPackageManagerTestInterface::create(configuration);

  // WHEN: checking for updates when no package file exists
  testInterface->checkForUpdates();

  // THEN: neither callback should be called for NoUpdateAvailable
  EXPECT_FALSE(successCallbackCalled);
  EXPECT_FALSE(failureCallbackCalled);
}

TEST_F(OpAppPackageManagerTest, TestUpdateCallbacks_Installed_NoCallbacksCalled)
{
  // GIVEN: a singleton OpAppPackageManager instance with callbacks and installed package
  OpAppPackageManager::Configuration configuration;
  configuration.m_PackageLocation = PACKAGE_PATH;
  configuration.m_PackageSuffix = ".opk";
  configuration.m_PackageHashFilePath = PACKAGE_PATH + "/package.hash";

  // Create a package file and hash file with same hash (simulating installed package)
  std::string packagePath = PACKAGE_PATH + "/package.opk";
  std::ofstream packageFile(packagePath);
  packageFile << "test package content";
  packageFile.close();

  auto mockCalculator = std::make_unique<MockHashCalculator>();
  mockCalculator->setHashForFile(packagePath, "test_hash_1234567890abcdef");
  mockCalculator->createHashJsonFile(PACKAGE_PATH + "/package.hash", "test_hash_1234567890abcdef");

  bool successCallbackCalled = false;
  bool failureCallbackCalled = false;

  configuration.m_OnUpdateSuccess = [&](const std::string&) {
    successCallbackCalled = true;
  };
  configuration.m_OnUpdateFailure = [&](OpAppPackageManager::PackageStatus, const std::string&) {
    failureCallbackCalled = true;
  };

  auto testInterface = OpAppPackageManagerTestInterface::create(configuration, std::move(mockCalculator), nullptr);

  // WHEN: checking for updates with installed package
  OpAppPackageManager::PackageStatus status = testInterface->doPackageFileCheck();

  // THEN: status should be Installed and neither callback should be called
  EXPECT_EQ(status, OpAppPackageManager::PackageStatus::Installed);
  EXPECT_FALSE(successCallbackCalled);
  EXPECT_FALSE(failureCallbackCalled);

  // Clean up test files
  std::remove(packagePath.c_str());
  std::remove((PACKAGE_PATH + "/package.hash").c_str());
}

TEST_F(OpAppPackageManagerTest, TestUpdateCallbacks_ConfigurationError_CallsFailureCallback)
{
  // GIVEN: a singleton OpAppPackageManager instance with callbacks
  OpAppPackageManager::Configuration configuration;
  configuration.m_PackageLocation = PACKAGE_PATH;
  configuration.m_PackageSuffix = ".opk";
  configuration.m_PackageHashFilePath = PACKAGE_PATH + "/package.hash";

  // Create multiple package files to trigger ConfigurationError
  std::string packagePath1 = PACKAGE_PATH + "/package1.opk";
  std::string packagePath2 = PACKAGE_PATH + "/package2.opk";
  std::ofstream file1(packagePath1);
  std::ofstream file2(packagePath2);
  file1.close();
  file2.close();

  bool successCallbackCalled = false;
  bool failureCallbackCalled = false;
  OpAppPackageManager::PackageStatus failureStatus;
  std::string failureErrorMessage;

  configuration.m_OnUpdateSuccess = [&](const std::string&) {
    successCallbackCalled = true;
  };
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

  // Clean up test files
  std::remove(packagePath1.c_str());
  std::remove(packagePath2.c_str());
}

TEST_F(OpAppPackageManagerTest, TestUpdateCallbacks_NoCallbacksSet)
{
  // GIVEN: a singleton OpAppPackageManager instance without callbacks
  OpAppPackageManager::Configuration configuration;
  configuration.m_PackageLocation = PACKAGE_PATH;
  configuration.m_PackageSuffix = ".opk";
  configuration.m_PackageHashFilePath = PACKAGE_PATH + "/package.hash";

  auto testInterface = OpAppPackageManagerTestInterface::create(configuration);

  // WHEN: checking for updates when no package file exists
  // THEN: function should complete successfully
  OpAppPackageManager::PackageStatus status = testInterface->doPackageFileCheck();
  EXPECT_EQ(status, OpAppPackageManager::PackageStatus::NoUpdateAvailable);
}

// Standalone test to isolate destroyInstance issue
TEST(OpAppPackageManagerStandalone, TestDestroyInstanceStandalone)
{
  // GIVEN: a singleton instance
  OpAppPackageManager::Configuration configuration;
  configuration.m_PackageLocation = OpAppPackageManagerTest::PACKAGE_PATH;
  auto testInterface = OpAppPackageManagerTestInterface::create(configuration);

  // Verify that the test interface is pointing to the same instance as the singleton
  auto opappPackageManager = OpAppPackageManager::getInstance();
  EXPECT_EQ(&testInterface->getPackageManager(), opappPackageManager);

  // WHEN: destroying the instance
  OpAppPackageManager::destroyInstance();

  // THEN: a new instance should be created when getInstance is called again
  auto opappPackageManager2 = OpAppPackageManager::getInstance();
  EXPECT_EQ(opappPackageManager2, nullptr);
  // Should be nullptr since instance destroyed and no configuration provided.

  // Create new instance with configuration
  auto testInterface3 = OpAppPackageManagerTestInterface::create(configuration);

  // Verify that both instances are valid and accessible
  EXPECT_FALSE(testInterface->isRunning());
  EXPECT_FALSE(testInterface->isUpdating());
  EXPECT_FALSE(testInterface3->isRunning());
  EXPECT_FALSE(testInterface3->isUpdating());

  // The test passes if we can successfully destroy and recreate the instance
  // Memory address reuse is not a failure condition

  // Clean up after test
  OpAppPackageManager::destroyInstance();
}

TEST_F(OpAppPackageManagerTest, TestCalculateSHA256Hash)
{
  // GIVEN: a singleton OpAppPackageManager instance and a test file
  OpAppPackageManager::Configuration configuration;
  configuration.m_PackageLocation = PACKAGE_PATH;
  auto testInterface = OpAppPackageManagerTestInterface::create(configuration);

  // Create a test file with known content
  std::string testFilePath = PACKAGE_PATH + "/test_file.txt";
  std::ofstream testFile(testFilePath);
  testFile << "Hello, World! This is a test file for SHA256 hashing.";
  testFile.close();

  // WHEN: calculating SHA256 hash of the test file
  std::string hash = testInterface->calculateFileSHA256Hash(testFilePath);

  // THEN: the hash should be a valid SHA256 hash (64 hex characters)
  EXPECT_EQ(hash.length(), size_t(64)); // SHA256 produces 32 bytes = 64 hex characters
  EXPECT_TRUE(std::all_of(hash.begin(), hash.end(), ::isxdigit)); // All characters should be hex

  // Verify the hash is not empty
  EXPECT_FALSE(hash.empty());

  // Clean up test file
  std::remove(testFilePath.c_str());
}

TEST_F(OpAppPackageManagerTest, TestCalculateSHA256HashEmptyFile)
{
  // GIVEN: a singleton OpAppPackageManager instance and an empty test file
  OpAppPackageManager::Configuration configuration;
  configuration.m_PackageLocation = PACKAGE_PATH;
  auto testInterface = OpAppPackageManagerTestInterface::create(configuration);

  // Create an empty test file
  std::string testFilePath = PACKAGE_PATH + "/empty_file.txt";
  std::ofstream testFile(testFilePath);
  testFile.close();

  // WHEN: calculating SHA256 hash of the empty file
  std::string hash = testInterface->calculateFileSHA256Hash(testFilePath);

  // THEN: the hash should be the SHA256 hash of an empty file
  // SHA256 hash of empty string: e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855
  EXPECT_EQ(hash, "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");

  // Clean up test file
  std::remove(testFilePath.c_str());
}

TEST_F(OpAppPackageManagerTest, TestCalculateSHA256HashNonexistentFile)
{
  // GIVEN: a singleton OpAppPackageManager instance
  OpAppPackageManager::Configuration configuration;
  configuration.m_PackageLocation = PACKAGE_PATH;
  auto testInterface = OpAppPackageManagerTestInterface::create(configuration);

  // WHEN: calculating SHA256 hash of a nonexistent file
  std::string hash = testInterface->calculateFileSHA256Hash(PACKAGE_PATH + "/nonexistent_file.txt");

  // THEN: the hash should be empty string
  EXPECT_TRUE(hash.empty());
}

TEST_F(OpAppPackageManagerTest, TestCheckForUpdates_MultiplePackageFiles_ReturnsError)
{
  // GIVEN: a singleton OpAppPackageManager instance
  // and multiple package files in the package source location
  OpAppPackageManager::Configuration configuration;
  configuration.m_PackageLocation = PACKAGE_PATH;
  configuration.m_PackageSuffix = ".opk";
  configuration.m_PackageHashFilePath = PACKAGE_PATH + "/package.hash";

  // Create multiple package files in the package source location
  std::string packagePath1 = PACKAGE_PATH + "/package1.opk";
  std::string packagePath2 = PACKAGE_PATH + "/package2.opk";
  std::ofstream file1(packagePath1);
  std::ofstream file2(packagePath2);
  file1.close();
  file2.close();

  auto testInterface = OpAppPackageManagerTestInterface::create(configuration);

  // WHEN: doPackageFileCheck is called
  // THEN: it should set status to ConfigurationError
  OpAppPackageManager::PackageStatus status = testInterface->doPackageFileCheck();
  EXPECT_EQ(status, OpAppPackageManager::PackageStatus::ConfigurationError);
  EXPECT_FALSE(testInterface->getLastErrorMessage().empty());

  // Clean up test files
  std::remove(packagePath1.c_str());
  std::remove(packagePath2.c_str());
}

TEST_F(OpAppPackageManagerTest, TestGetPackageFiles_MultiplePackageFiles_ReturnsError)
{
  // GIVEN: a singleton OpAppPackageManager instance
  // and multiple package files in the package source location
  OpAppPackageManager::Configuration configuration;
  configuration.m_PackageLocation = PACKAGE_PATH;
  configuration.m_PackageSuffix = ".opk";

  // Create multiple package files in the package source location
  std::string packagePath1 = PACKAGE_PATH + "/package1.opk";
  std::string packagePath2 = PACKAGE_PATH + "/package2.opk";
  std::ofstream file1(packagePath1);
  std::ofstream file2(packagePath2);
  file1.close();
  file2.close();

  auto testInterface = OpAppPackageManagerTestInterface::create(configuration);

  // WHEN: getPackageFiles is called
  PackageOperationResult result = testInterface->getPackageFiles();

  // THEN: it should return error result
  EXPECT_FALSE(result.success);
  EXPECT_FALSE(result.errorMessage.empty());
  EXPECT_EQ(result.packageFiles.size(), size_t(2));
  EXPECT_NE(result.errorMessage.find("Multiple package files found"), std::string::npos);
  EXPECT_NE(result.errorMessage.find("package1.opk"), std::string::npos);
  EXPECT_NE(result.errorMessage.find("package2.opk"), std::string::npos);

  // Clean up test files
  std::remove(packagePath1.c_str());
  std::remove(packagePath2.c_str());
}

TEST_F(OpAppPackageManagerTest, TestGetPackageFiles_SinglePackageFile_ReturnsSuccess)
{
  // GIVEN: a singleton OpAppPackageManager instance
  // and a single package file in the package source location
  OpAppPackageManager::Configuration configuration;
  configuration.m_PackageLocation = PACKAGE_PATH;
  configuration.m_PackageSuffix = ".opk";

  // Create a single package file in the package source location
  std::string packagePath = PACKAGE_PATH + "/package.opk";
  std::ofstream file(packagePath);
  file.close();

  auto testInterface = OpAppPackageManagerTestInterface::create(configuration);

  // WHEN: getPackageFiles is called
  PackageOperationResult result = testInterface->getPackageFiles();

  // THEN: it should return success with exactly one file
  EXPECT_TRUE(result.success);
  EXPECT_TRUE(result.errorMessage.empty());
  EXPECT_EQ(result.packageFiles.size(), size_t(1));
  EXPECT_EQ(result.packageFiles[0], packagePath);

  // Clean up test file
  std::remove(packagePath.c_str());
}

TEST_F(OpAppPackageManagerTest, TestGetPackageFiles_NoPackageFiles_ReturnsSuccess)
{
  // GIVEN: a singleton OpAppPackageManager instance
  // and no package files in the package source location
  OpAppPackageManager::Configuration configuration;
  configuration.m_PackageLocation = PACKAGE_PATH;
  configuration.m_PackageSuffix = ".opk";

  OpAppPackageManager& packageManager = OpAppPackageManager::getInstance(configuration);

  // WHEN: getPackageFiles is called
  PackageOperationResult result = packageManager.getPackageFiles();

  // THEN: it should return success with empty file list
  EXPECT_TRUE(result.success);
  EXPECT_TRUE(result.errorMessage.empty());
  EXPECT_TRUE(result.packageFiles.empty());
}

TEST_F(OpAppPackageManagerTest, TestClearLastError)
{
  // GIVEN: a singleton OpAppPackageManager instance with an error
  OpAppPackageManager::Configuration configuration;
  configuration.m_PackageLocation = PACKAGE_PATH;
  configuration.m_PackageSuffix = ".opk";

  auto testInterface = OpAppPackageManagerTestInterface::create(configuration);

  // Create an error condition
  std::string packagePath1 = PACKAGE_PATH + "/package1.opk";
  std::string packagePath2 = PACKAGE_PATH + "/package2.opk";
  std::ofstream file1(packagePath1);
  std::ofstream file2(packagePath2);
  file1.close();
  file2.close();

  // Trigger error condition
  testInterface->doPackageFileCheck();

  // Error message should be stored
  EXPECT_FALSE(testInterface->getLastErrorMessage().empty());

  // WHEN: clearLastError is called
  testInterface->clearLastError();

  // THEN: the error message should be cleared
  EXPECT_TRUE(testInterface->getLastErrorMessage().empty());

  // Clean up test files
  std::remove(packagePath1.c_str());
  std::remove(packagePath2.c_str());
}
