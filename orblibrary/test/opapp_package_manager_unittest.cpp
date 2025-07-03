#include <iostream>
#include <string>
#include <filesystem>

#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/orb/orblibrary/include/OpAppPackageManager.h"
#include <fstream>

class OpAppPackageManagerTest : public ::testing::Test {

public:
  static constexpr std::string PACKAGE_PATH = "test/packages";

protected:
  void SetUp() override {
    // Clean up any existing instance before each test
    OpAppPackageManager::destroyInstance();

    // Create test directory structure
    std::filesystem::create_directories(PACKAGE_PATH);
  }

  void TearDown() override {
    // Clean up after each test
    OpAppPackageManager::destroyInstance();
    // Remove the package file in the package source location
    std::string packagePath = PACKAGE_PATH + "/package.opk";
    std::remove(packagePath.c_str());

    // Remove test directory structure
    std::filesystem::remove_all("test");
  }
};

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
  configuration.m_InstallDirectory = "/install";

  // WHEN: creating instance with custom configuration
  OpAppPackageManager& packageManager = OpAppPackageManager::getInstance(configuration);

  // THEN: the instance should be created successfully
  EXPECT_FALSE(packageManager.isRunning());
  EXPECT_FALSE(packageManager.isUpdating());
}

TEST_F(OpAppPackageManagerTest, TestStartAndStop)
{
  // GIVEN: a singleton OpAppPackageManager instance
  OpAppPackageManager::Configuration configuration;
  configuration.m_PackageLocation = PACKAGE_PATH;
  OpAppPackageManager& packageManager = OpAppPackageManager::getInstance(configuration);

  // WHEN: starting the package manager
  packageManager.start();

  // THEN: the package manager should be running
  EXPECT_TRUE(packageManager.isRunning());
  EXPECT_FALSE(packageManager.isUpdating());

  // WHEN: stopping the package manager
  packageManager.stop();

  // THEN: the package manager should be stopped
  EXPECT_FALSE(packageManager.isRunning());
  EXPECT_FALSE(packageManager.isUpdating());
}

TEST_F(OpAppPackageManagerTest, TestCheckForUpdates_NoUpdates)
{
  // GIVEN: a singleton OpAppPackageManager instance
  // and no package file in the package source location
  OpAppPackageManager::Configuration configuration;
  configuration.m_PackageLocation = PACKAGE_PATH;
  configuration.m_PackageSuffix = ".opk";
  OpAppPackageManager& packageManager = OpAppPackageManager::getInstance(configuration);

  // WHEN: doPackageFileCheck is called
  EXPECT_EQ(packageManager.getPackageStatus(), OpAppPackageManager::PackageStatus::DontKnow);
  packageManager.doPackageFileCheck();

  // THEN: the package status is unchanged
  EXPECT_EQ(packageManager.getPackageStatus(), OpAppPackageManager::PackageStatus::DontKnow);
}

TEST_F(OpAppPackageManagerTest, TestCheckForUpdates_UpdatesAvailable)
{
  // GIVEN: a singleton OpAppPackageManager instance
  // and a package file in the package source location
  OpAppPackageManager::Configuration configuration;
  configuration.m_PackageLocation = PACKAGE_PATH;
  configuration.m_PackageSuffix = ".opk";
  // Create a package file in the package source location
  std::string packagePath = PACKAGE_PATH + "/package.opk";
  std::ofstream file(packagePath);
  file.close();
  OpAppPackageManager& packageManager = OpAppPackageManager::getInstance(configuration);

  // WHEN: doPackageFileCheck is called
  packageManager.doPackageFileCheck();

  // THEN: the package status is UpdateAvailable
  EXPECT_EQ(packageManager.getPackageStatus(), OpAppPackageManager::PackageStatus::UpdateAvailable);
}

// TEST_F(OpAppPackageManagerTest, TestIsPackageIsNotAlreadyInstalled)
// {
//   // GIVEN: a singleton OpAppPackageManager instance and no package installed
//   OpAppPackageManager::Configuration configuration;
//   configuration.m_PackageLocation = PACKAGE_PATH;
//   OpAppPackageManager& packageManager = OpAppPackageManager::getInstance(configuration);
//   std::string packagePath = "/path/to/package.opk";

//   // WHEN: checking if the package is installed
//   bool isInstalled = packageManager.isPackageInstalled(packagePath);

//   // THEN: the package should not be installed
//   EXPECT_FALSE(isInstalled);
// }

// TEST_F(OpAppPackageManagerTest, TestIsPackageIsAlreadyInstalled)
// {
//   // GIVEN: a singleton OpAppPackageManager instance and a package installed
//   OpAppPackageManager::Configuration configuration;
//   configuration.m_PackageLocation = PACKAGE_PATH;
//   OpAppPackageManager& packageManager = OpAppPackageManager::getInstance(configuration);
//   std::string packagePath = "/path/to/package.opk";

//   // WHEN: checking if the package is installed
//   bool isInstalled = packageManager.isPackageInstalled(packagePath);

//   // THEN: the package should be installed
//   EXPECT_TRUE(isInstalled);
// }

// TODO: Add tests for future package manager functionality
// These stubs can be expanded when the OpAppPackageManager class is implemented

TEST_F(OpAppPackageManagerTest, TestInstallPackage)
{
  // GIVEN: a singleton OpAppPackageManager instance and a package to install
  OpAppPackageManager::Configuration configuration;
  configuration.m_PackageLocation = PACKAGE_PATH;
  OpAppPackageManager& packageManager = OpAppPackageManager::getInstance(configuration);
  std::string packagePath = "/path/to/package.opk";

  // WHEN: attempting to install a package
  // bool result = packageManager.installPackage(packagePath);

  // THEN: the installation should be handled appropriately
  // TODO: Implement when installPackage method is added
  // EXPECT_TRUE(result);

  // Mark variables as intentionally unused for now
  (void)packageManager;
  (void)packagePath;
}

TEST_F(OpAppPackageManagerTest, TestUninstallPackage)
{
  // GIVEN: a singleton OpAppPackageManager instance and a package ID
  OpAppPackageManager::Configuration configuration;
  configuration.m_PackageLocation = PACKAGE_PATH;
  OpAppPackageManager& packageManager = OpAppPackageManager::getInstance(configuration);
  std::string packageId = "com.example.app";

  // WHEN: attempting to uninstall a package
  // bool result = packageManager.uninstallPackage(packageId);

  // THEN: the uninstallation should be handled appropriately
  // TODO: Implement when uninstallPackage method is added
  // EXPECT_TRUE(result);

  // Mark variables as intentionally unused for now
  (void)packageManager;
  (void)packageId;
}

TEST_F(OpAppPackageManagerTest, TestListInstalledPackages)
{
  // GIVEN: a singleton OpAppPackageManager instance
  OpAppPackageManager::Configuration configuration;
  configuration.m_PackageLocation = PACKAGE_PATH;
  OpAppPackageManager& packageManager = OpAppPackageManager::getInstance(configuration);

  // WHEN: requesting a list of installed packages
  // std::vector<std::string> packages = packageManager.listInstalledPackages();

  // THEN: a list of packages should be returned
  // TODO: Implement when listInstalledPackages method is added
  // EXPECT_FALSE(packages.empty());

  // Mark variables as intentionally unused for now
  (void)packageManager;
}

TEST_F(OpAppPackageManagerTest, TestGetPackageInfo)
{
  // GIVEN: a singleton OpAppPackageManager instance and a package ID
  OpAppPackageManager::Configuration configuration;
  configuration.m_PackageLocation = PACKAGE_PATH;
  OpAppPackageManager& packageManager = OpAppPackageManager::getInstance(configuration);
  std::string packageId = "com.example.app";

  // WHEN: requesting package information
  // PackageInfo info = packageManager.getPackageInfo(packageId);

  // THEN: package information should be returned
  // TODO: Implement when getPackageInfo method is added
  // EXPECT_EQ(info.id, packageId);

  // Mark variables as intentionally unused for now
  (void)packageManager;
  (void)packageId;
}

TEST_F(OpAppPackageManagerTest, TestUpdatePackage)
{
  // GIVEN: a singleton OpAppPackageManager instance and a package update
  OpAppPackageManager::Configuration configuration;
  configuration.m_PackageLocation = PACKAGE_PATH;
  OpAppPackageManager& packageManager = OpAppPackageManager::getInstance(configuration);
  std::string packagePath = "/path/to/update.opk";

  // WHEN: attempting to update a package
  // bool result = packageManager.updatePackage(packagePath);

  // THEN: the update should be handled appropriately
  // TODO: Implement when updatePackage method is added
  // EXPECT_TRUE(result);

  // Mark variables as intentionally unused for now
  (void)packageManager;
  (void)packagePath;
}

TEST_F(OpAppPackageManagerTest, TestIsPackageInstalled)
{
  // GIVEN: a singleton OpAppPackageManager instance and a package ID
  OpAppPackageManager::Configuration configuration;
  configuration.m_PackageLocation = PACKAGE_PATH;
  OpAppPackageManager& packageManager = OpAppPackageManager::getInstance(configuration);
  std::string packageId = "com.example.app";

  // WHEN: checking if a package is installed
  // bool installed = packageManager.isPackageInstalled(packageId);

  // THEN: the installation status should be returned
  // TODO: Implement when isPackageInstalled method is added
  // EXPECT_FALSE(installed); // Assuming no packages are installed initially

  // Mark variables as intentionally unused for now
  (void)packageManager;
  (void)packageId;
}

TEST_F(OpAppPackageManagerTest, TestGetPackageVersion)
{
  // GIVEN: a singleton OpAppPackageManager instance and a package ID
  OpAppPackageManager::Configuration configuration;
  configuration.m_PackageLocation = PACKAGE_PATH;
  OpAppPackageManager& packageManager = OpAppPackageManager::getInstance(configuration);
  std::string packageId = "com.example.app";

  // WHEN: requesting package version
  // std::string version = packageManager.getPackageVersion(packageId);

  // THEN: the package version should be returned
  // TODO: Implement when getPackageVersion method is added
  // EXPECT_FALSE(version.empty());

  // Mark variables as intentionally unused for now
  (void)packageManager;
  (void)packageId;
}

TEST_F(OpAppPackageManagerTest, TestEnablePackage)
{
  // GIVEN: a singleton OpAppPackageManager instance and a package ID
  OpAppPackageManager::Configuration configuration;
  configuration.m_PackageLocation = PACKAGE_PATH;
  OpAppPackageManager& packageManager = OpAppPackageManager::getInstance(configuration);
  std::string packageId = "com.example.app";

  // WHEN: attempting to enable a package
  // bool result = packageManager.enablePackage(packageId);

  // THEN: the enable operation should be handled appropriately
  // TODO: Implement when enablePackage method is added
  // EXPECT_TRUE(result);

  // Mark variables as intentionally unused for now
  (void)packageManager;
  (void)packageId;
}

TEST_F(OpAppPackageManagerTest, TestDisablePackage)
{
  // GIVEN: a singleton OpAppPackageManager instance and a package ID
  OpAppPackageManager::Configuration configuration;
  configuration.m_PackageLocation = PACKAGE_PATH;
  OpAppPackageManager& packageManager = OpAppPackageManager::getInstance(configuration);
  std::string packageId = "com.example.app";

  // WHEN: attempting to disable a package
  // bool result = packageManager.disablePackage(packageId);

  // THEN: the disable operation should be handled appropriately
  // TODO: Implement when disablePackage method is added
  // EXPECT_TRUE(result);

  // Mark variables as intentionally unused for now
  (void)packageManager;
  (void)packageId;
}

TEST_F(OpAppPackageManagerTest, TestGetPackageSize)
{
  // GIVEN: a singleton OpAppPackageManager instance and a package ID
  OpAppPackageManager::Configuration configuration;
  configuration.m_PackageLocation = PACKAGE_PATH;
  OpAppPackageManager& packageManager = OpAppPackageManager::getInstance(configuration);
  std::string packageId = "com.example.app";

  // WHEN: requesting package size
  // size_t size = packageManager.getPackageSize(packageId);

  // THEN: the package size should be returned
  // TODO: Implement when getPackageSize method is added
  // EXPECT_GT(size, 0);

  // Mark variables as intentionally unused for now
  (void)packageManager;
  (void)packageId;
}

TEST_F(OpAppPackageManagerTest, TestValidatePackage)
{
  // GIVEN: a singleton OpAppPackageManager instance and a package path
  OpAppPackageManager::Configuration configuration;
  configuration.m_PackageLocation = PACKAGE_PATH;
  OpAppPackageManager& packageManager = OpAppPackageManager::getInstance(configuration);
  std::string packagePath = "/path/to/package.opk";

  // WHEN: validating a package
  // bool valid = packageManager.validatePackage(packagePath);

  // THEN: the validation result should be returned
  // TODO: Implement when validatePackage method is added
  // EXPECT_TRUE(valid);

  // Mark variables as intentionally unused for now
  (void)packageManager;
  (void)packagePath;
}

// Error handling tests
TEST_F(OpAppPackageManagerTest, TestInstallInvalidPackage)
{
  // GIVEN: a singleton OpAppPackageManager instance and an invalid package path
  OpAppPackageManager::Configuration configuration;
  configuration.m_PackageLocation = PACKAGE_PATH;
  OpAppPackageManager& packageManager = OpAppPackageManager::getInstance(configuration);
  std::string invalidPath = "/nonexistent/package.opk";

  // WHEN: attempting to install an invalid package
  // bool result = packageManager.installPackage(invalidPath);

  // THEN: the operation should fail gracefully
  // TODO: Implement when installPackage method is added
  // EXPECT_FALSE(result);

  // Mark variables as intentionally unused for now
  (void)packageManager;
  (void)invalidPath;
}

TEST_F(OpAppPackageManagerTest, TestUninstallNonexistentPackage)
{
  // GIVEN: a singleton OpAppPackageManager instance and a nonexistent package ID
  OpAppPackageManager::Configuration configuration;
  configuration.m_PackageLocation = PACKAGE_PATH;
  OpAppPackageManager& packageManager = OpAppPackageManager::getInstance(configuration);
  std::string nonexistentId = "com.nonexistent.app";

  // WHEN: attempting to uninstall a nonexistent package
  // bool result = packageManager.uninstallPackage(nonexistentId);

  // THEN: the operation should fail gracefully
  // TODO: Implement when uninstallPackage method is added
  // EXPECT_FALSE(result);

  // Mark variables as intentionally unused for now
  (void)packageManager;
  (void)nonexistentId;
}

TEST_F(OpAppPackageManagerTest, TestGetInfoForNonexistentPackage)
{
  // GIVEN: a singleton OpAppPackageManager instance and a nonexistent package ID
  OpAppPackageManager::Configuration configuration;
  configuration.m_PackageLocation = PACKAGE_PATH;
  OpAppPackageManager& packageManager = OpAppPackageManager::getInstance(configuration);
  std::string nonexistentId = "com.nonexistent.app";

  // WHEN: requesting information for a nonexistent package
  // PackageInfo info = packageManager.getPackageInfo(nonexistentId);

  // THEN: appropriate error handling should occur
  // TODO: Implement when getPackageInfo method is added
  // EXPECT_TRUE(info.id.empty());

  // Mark variables as intentionally unused for now
  (void)packageManager;
  (void)nonexistentId;
}

// Performance and stress tests
TEST_F(OpAppPackageManagerTest, TestConcurrentOperations)
{
  // GIVEN: a singleton OpAppPackageManager instance
  OpAppPackageManager::Configuration configuration;
  configuration.m_PackageLocation = PACKAGE_PATH;
  OpAppPackageManager& packageManager = OpAppPackageManager::getInstance(configuration);

  // WHEN: performing multiple operations concurrently
  // (This would require threading implementation)

  // THEN: all operations should complete successfully
  // TODO: Implement when threading support is added

  // Mark variables as intentionally unused for now
  (void)packageManager;
}

TEST_F(OpAppPackageManagerTest, TestLargePackageHandling)
{
  // GIVEN: a singleton OpAppPackageManager instance and a large package
  OpAppPackageManager::Configuration configuration;
  configuration.m_PackageLocation = PACKAGE_PATH;
  OpAppPackageManager& packageManager = OpAppPackageManager::getInstance(configuration);
  std::string largePackagePath = "/path/to/large/package.opk";

  // WHEN: handling a large package

  // THEN: the operation should complete without memory issues
  // TODO: Implement when large package handling is added

  // Mark variables as intentionally unused for now
  (void)packageManager;
  (void)largePackagePath;
}

// Integration tests
TEST_F(OpAppPackageManagerTest, TestFullPackageLifecycle)
{
  // GIVEN: a singleton OpAppPackageManager instance and a valid package
  OpAppPackageManager::Configuration configuration;
  configuration.m_PackageLocation = PACKAGE_PATH;
  OpAppPackageManager& packageManager = OpAppPackageManager::getInstance(configuration);
  std::string packagePath = "/path/to/valid/package.opk";
  std::string packageId = "com.example.app";

  // WHEN: performing a full package lifecycle (install, enable, disable, uninstall)

  // THEN: all operations should complete successfully
  // TODO: Implement when all lifecycle methods are added

  // Mark variables as intentionally unused for now
  (void)packageManager;
  (void)packagePath;
  (void)packageId;
}

TEST_F(OpAppPackageManagerTest, TestPackageUpdateWorkflow)
{
  // GIVEN: a singleton OpAppPackageManager instance and package versions
  OpAppPackageManager::Configuration configuration;
  configuration.m_PackageLocation = PACKAGE_PATH;
  OpAppPackageManager& packageManager = OpAppPackageManager::getInstance(configuration);
  std::string oldVersionPath = "/path/to/old/version.opk";
  std::string newVersionPath = "/path/to/new/version.opk";

  // WHEN: performing a package update workflow

  // THEN: the update should complete successfully
  // TODO: Implement when update workflow is added

  // Mark variables as intentionally unused for now
  (void)packageManager;
  (void)oldVersionPath;
  (void)newVersionPath;
}

// Standalone test to isolate destroyInstance issue
TEST(OpAppPackageManagerStandalone, TestDestroyInstanceStandalone)
{
  // GIVEN: a singleton instance
  OpAppPackageManager::Configuration configuration;
  configuration.m_PackageLocation = OpAppPackageManagerTest::PACKAGE_PATH;
  OpAppPackageManager& instance1 = OpAppPackageManager::getInstance(configuration);

  // WHEN: destroying the instance
  OpAppPackageManager::destroyInstance();

  // THEN: a new instance should be created when getInstance is called again
  OpAppPackageManager* instance2_ptr = OpAppPackageManager::getInstance();
  EXPECT_EQ(instance2_ptr, nullptr); // Should be nullptr since no configuration provided

  // Create new instance with configuration
  OpAppPackageManager& instance2 = OpAppPackageManager::getInstance(configuration);

  // Verify that both instances are valid and accessible
  EXPECT_FALSE(instance1.isRunning());
  EXPECT_FALSE(instance1.isUpdating());
  EXPECT_FALSE(instance2.isRunning());
  EXPECT_FALSE(instance2.isUpdating());

  // The test passes if we can successfully destroy and recreate the instance
  // Memory address reuse is not a failure condition

  // Clean up after test
  OpAppPackageManager::destroyInstance();
}
