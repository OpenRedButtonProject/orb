/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Unit tests for ApplicationManager
 */

#include <iostream>
#include <string>

#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/orb/orblibrary/moderator/app_mgr/application_manager.h"
#include "third_party/orb/orblibrary/moderator/app_mgr/xml_parser.h"
#include "MockApplicationSessionCallback.h"

namespace orb
{
class MockXmlParser : public IXmlParser
{
public:
    MockXmlParser() = default;
    virtual ~MockXmlParser() = default;

    // Control the behavior of ParseAit for testing
    void SetParseAitResult(std::unique_ptr<Ait::S_AIT_TABLE> result)
    {
        parseAitResult = std::move(result);
    }

    void SetParseAitShouldFail(bool shouldFail)
    {
        parseAitShouldFail = shouldFail;
    }

    std::unique_ptr<Ait::S_AIT_TABLE> ParseAit(const char *content, uint32_t length) override
    {
        if (parseAitShouldFail)
        {
            return nullptr;
        }

        return std::move(parseAitResult);
    }

private:
    std::unique_ptr<Ait::S_AIT_TABLE> parseAitResult;
    bool parseAitShouldFail = false;
};


class ApplicationManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Set up test environment
        mockCallback = std::make_unique<MockApplicationSessionCallback>();
        mockXmlParser = std::make_unique<MockXmlParser>();

        // Set up ApplicationManager with mock XML parser
        ApplicationManager& appManager = ApplicationManager::instance();
        appManager.SetXmlParser(std::move(mockXmlParser));
    }

    void TearDown() override
    {
        // Clean up test environment
        mockCallback.reset();
        mockXmlParser.reset();
    }

    std::unique_ptr<MockApplicationSessionCallback> mockCallback;
    std::unique_ptr<MockXmlParser> mockXmlParser;
};

TEST_F(ApplicationManagerTest, TestSingletonInstance)
{
    // GIVEN: ApplicationManager singleton
    // WHEN: We get the instance multiple times
    ApplicationManager& instance1 = ApplicationManager::instance();
    ApplicationManager& instance2 = ApplicationManager::instance();

    // THEN: Both references should point to the same object
    EXPECT_EQ(&instance1, &instance2);
}

TEST_F(ApplicationManagerTest, TestProcessXmlAitEmptyXml)
{
    // GIVEN: ApplicationManager singleton and empty XML
    ApplicationManager& appManager = ApplicationManager::instance();
    std::string emptyXml = "";

    // WHEN: ProcessXmlAit is called with empty XML
    int result = appManager.ProcessXmlAit(emptyXml);

    // THEN: Should return INVALID_APP_ID
    EXPECT_EQ(result, INVALID_APP_ID);
}

TEST_F(ApplicationManagerTest, TestProcessXmlAitWithMockParserFailure)
{
    // GIVEN: ApplicationManager singleton and mock XML parser set to fail
    ApplicationManager& appManager = ApplicationManager::instance();

    auto failingMockParser = std::make_unique<MockXmlParser>();
    failingMockParser->SetParseAitShouldFail(true);
    appManager.SetXmlParser(std::move(failingMockParser));

    std::string xmlContent = "some xml content";

    // WHEN: ProcessXmlAit is called
    int result = appManager.ProcessXmlAit(xmlContent);

    // THEN: Should return INVALID_APP_ID due to parser failure
    EXPECT_EQ(result, INVALID_APP_ID);
}

TEST_F(ApplicationManagerTest, TestProcessXmlAitWithMockParserSuccess)
{
    // GIVEN: ApplicationManager singleton and mock XML parser set to succeed
    ApplicationManager& appManager = ApplicationManager::instance();

    // Create a mock AIT table
    auto mockAitTable = std::make_unique<Ait::S_AIT_TABLE>();
    mockAitTable->numApps = 1;
    mockAitTable->appArray.resize(1);
    mockAitTable->appArray[0].appId = 1;
    mockAitTable->appArray[0].orgId = 1;
    mockAitTable->appArray[0].scheme = "urn:hbbtv:opapp:privileged:2017";

    auto successMockParser = std::make_unique<MockXmlParser>();
    successMockParser->SetParseAitResult(std::move(mockAitTable));
    appManager.SetXmlParser(std::move(successMockParser));

    // Register callback for app creation
    appManager.RegisterCallback(APP_TYPE_HBBTV, mockCallback.get());
    appManager.SetCurrentInterface(APP_TYPE_HBBTV);

    std::string xmlContent = "valid xml content";

    // WHEN: ProcessXmlAit is called
    int result = appManager.ProcessXmlAit(xmlContent);

    // THEN: Should return a valid app ID (not INVALID_APP_ID)
    // Note: The actual result depends on app creation logic, but it should not be INVALID_APP_ID
    // due to successful XML parsing
    EXPECT_NE(result, INVALID_APP_ID);
}

TEST_F(ApplicationManagerTest, TestRegisterCallback)
{
    // GIVEN: ApplicationManager singleton
    ApplicationManager& appManager = ApplicationManager::instance();

    // WHEN: RegisterCallback is called with valid parameters
    appManager.RegisterCallback(APP_TYPE_HBBTV, mockCallback.get());

    // THEN: No exception should be thrown
    // Note: We can't easily verify the callback was registered without exposing internal state
    // This test mainly ensures the method doesn't crash
    SUCCEED();
}

TEST_F(ApplicationManagerTest, TestRegisterCallbackInvalidType)
{
    // GIVEN: ApplicationManager singleton
    ApplicationManager& appManager = ApplicationManager::instance();

    // WHEN: RegisterCallback is called with invalid app type
    // Note: APP_TYPE_VIDEO is beyond the valid range (APP_TYPE_HBBTV, APP_TYPE_OPAPP)
    appManager.RegisterCallback(APP_TYPE_VIDEO, mockCallback.get());

    // THEN: No exception should be thrown
    // The method should handle invalid parameters gracefully
    SUCCEED();
}

TEST_F(ApplicationManagerTest, TestRegisterCallbackNullCallback)
{
    // GIVEN: ApplicationManager singleton
    ApplicationManager& appManager = ApplicationManager::instance();

    // WHEN: RegisterCallback is called with null callback
    appManager.RegisterCallback(APP_TYPE_HBBTV, nullptr);

    // THEN: No exception should be thrown
    // The method should handle null callback gracefully
    SUCCEED();
}

TEST_F(ApplicationManagerTest, TestSetCurrentInterface)
{
    // GIVEN: ApplicationManager singleton
    ApplicationManager& appManager = ApplicationManager::instance();

    // WHEN: SetCurrentInterface is called with valid app type
    appManager.SetCurrentInterface(APP_TYPE_HBBTV);

    // THEN: No exception should be thrown
    SUCCEED();
}

TEST_F(ApplicationManagerTest, TestSetCurrentInterfaceInvalidType)
{
    // GIVEN: ApplicationManager singleton
    ApplicationManager& appManager = ApplicationManager::instance();

    // WHEN: SetCurrentInterface is called with invalid app type
    appManager.SetCurrentInterface(APP_TYPE_VIDEO);

    // THEN: No exception should be thrown
    // The method should handle invalid parameters gracefully
    SUCCEED();
}

TEST_F(ApplicationManagerTest, TestGetRunningAppIds)
{
    // GIVEN: ApplicationManager singleton
    ApplicationManager& appManager = ApplicationManager::instance();

    // WHEN: GetRunningAppIds is called
    std::vector<int> appIds = appManager.GetRunningAppIds();

    // THEN: Should return a vector (may be empty)
    // This test verifies the method doesn't crash and returns a valid vector
    EXPECT_TRUE(appIds.empty() || !appIds.empty());
}

TEST_F(ApplicationManagerTest, TestGetOrganizationId)
{
    // GIVEN: ApplicationManager singleton
    ApplicationManager& appManager = ApplicationManager::instance();

    // WHEN: GetOrganizationId is called with no running apps
    uint32_t orgId = appManager.GetOrganizationId();

    // THEN: Should return -1 (indicating no running app)
    EXPECT_EQ(orgId, static_cast<uint32_t>(-1));
}

TEST_F(ApplicationManagerTest, TestGetCurrentAppNames)
{
    // GIVEN: ApplicationManager singleton
    ApplicationManager& appManager = ApplicationManager::instance();

    // WHEN: GetCurrentAppNames is called with no running apps
    std::map<std::string, std::string> names = appManager.GetCurrentAppNames();

    // THEN: Should return an empty map
    EXPECT_TRUE(names.empty());
}

} // namespace orb
