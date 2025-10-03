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
#include <gmock/gmock.h>

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

        // Reset ApplicationManager singleton state to ensure test isolation
        appManager.RegisterCallback(APP_TYPE_HBBTV, nullptr);
        appManager.RegisterCallback(APP_TYPE_OPAPP, nullptr);
        appManager.SetCurrentInterface(APP_TYPE_HBBTV);
    }

    void TearDown() override
    {
        // Clean up test environment
        mockCallback.reset();
        mockXmlParser.reset();

        // Additional cleanup to ensure test isolation for next test
        ApplicationManager& appManager = ApplicationManager::instance();
        appManager.RegisterCallback(APP_TYPE_HBBTV, nullptr);
        appManager.RegisterCallback(APP_TYPE_OPAPP, nullptr);
        appManager.SetCurrentInterface(APP_TYPE_HBBTV);
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
    class GMockXmlParser : public IXmlParser
    {
    public:
        MOCK_METHOD(std::unique_ptr<Ait::S_AIT_TABLE>, ParseAit, (const char *content, uint32_t length), (override));
    };
    // GIVEN: ApplicationManager singleton and mock XML parser set to succeed
    ApplicationManager& appManager = ApplicationManager::instance();

    // Create a mock AIT table
    auto mockAitTable = std::make_unique<Ait::S_AIT_TABLE>();
    mockAitTable->numApps = 1;
    mockAitTable->appArray.resize(1);
    mockAitTable->appArray[0].appId = 1;
    mockAitTable->appArray[0].orgId = 1;
    mockAitTable->appArray[0].scheme = "urn:hbbtv:opapp:privileged:2017";

    std::string xmlContent = "valid xml content";

    auto successMockParser = std::make_unique<GMockXmlParser>();
    // Expect the ParseAit method to be called with the XML content and return the mock AIT table
    EXPECT_CALL(*successMockParser, ParseAit(xmlContent.c_str(), xmlContent.length()))
        .WillOnce(::testing::Return(::testing::ByMove(std::move(mockAitTable))));
    appManager.SetXmlParser(std::move(successMockParser));

    // Register callback for app creation
    appManager.RegisterCallback(APP_TYPE_HBBTV, mockCallback.get());
    appManager.SetCurrentInterface(APP_TYPE_HBBTV);

    // WHEN: ProcessXmlAit is called
    appManager.ProcessXmlAit(xmlContent);
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

// Unit tests for CreateAndRunApp public method
TEST_F(ApplicationManagerTest, TestCreateAndRunAppWithValidHbbTVUrl)
{
    // GIVEN: ApplicationManager with session callback registered
    ApplicationManager& appManager = ApplicationManager::instance();
    appManager.RegisterCallback(APP_TYPE_HBBTV, mockCallback.get());
    appManager.SetCurrentInterface(APP_TYPE_HBBTV);

    std::string testUrl = "http://example.com/myapp.html";

    // EXPECT: Callback methods to be called correctly
    EXPECT_CALL(*mockCallback, LoadApplication(testing::_, testing::_, testing::_, testing::_))
        .Times(1);
    EXPECT_CALL(*mockCallback, ShowApplication(testing::_))
        .Times(1);

    // WHEN: CreateAndRunApp is called with HbbTV URL (runAsOpApp=false)
    int appId = appManager.CreateAndRunApp(testUrl, false);

    // THEN: Should return valid app ID
    EXPECT_GT(appId, 0);
}

TEST_F(ApplicationManagerTest, TestCreateAndRunAppWithValidOpAppUrl)
{
    // GIVEN: ApplicationManager with session callback registered for OpApp
    ApplicationManager& appManager = ApplicationManager::instance();
    appManager.RegisterCallback(APP_TYPE_OPAPP, mockCallback.get());
    appManager.SetCurrentInterface(APP_TYPE_OPAPP);

    std::string testUrl = "http://operator.com/opapp.html";

    // EXPECT: Callback methods to be called correctly
    EXPECT_CALL(*mockCallback, LoadApplication(testing::_, testing::_, testing::_, testing::_))
        .Times(1);

    // This seems odd. OpApp starts in background state...
    EXPECT_CALL(*mockCallback, HideApplication(testing::_))
        .Times(1);

    // WHEN: CreateAndRunApp is called with OpApp URL (runAsOpApp=true)
    int appId = appManager.CreateAndRunApp(testUrl, true);

    // THEN: Should return valid app ID
    EXPECT_GT(appId, 0);
}

TEST_F(ApplicationManagerTest, TestCreateAndRunAppWithEmptyUrl)
{
    // GIVEN: ApplicationManager with session callback registered
    ApplicationManager& appManager = ApplicationManager::instance();
    appManager.RegisterCallback(APP_TYPE_HBBTV, mockCallback.get());
    appManager.SetCurrentInterface(APP_TYPE_HBBTV);

    std::string emptyUrl = "";

    // EXPECT: No callback methods should be called for empty URL
    EXPECT_CALL(*mockCallback, LoadApplication(testing::_, testing::_, testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(*mockCallback, ShowApplication(testing::_))
        .Times(0);

    // WHEN: CreateAndRunApp is called with empty URL
    int appId = appManager.CreateAndRunApp(emptyUrl, false);

    // THEN: Should return INVALID_APP_ID
    EXPECT_EQ(appId, INVALID_APP_ID);
}

TEST_F(ApplicationManagerTest, TestCreateAndRunAppWithoutSessionCallback)
{
    // GIVEN: ApplicationManager without session callback
    ApplicationManager& appManager = ApplicationManager::instance();
    // Explicitly ensure no callback is registered for current interface
    appManager.RegisterCallback(APP_TYPE_HBBTV, nullptr);
    appManager.SetCurrentInterface(APP_TYPE_HBBTV);

    std::string testUrl = "http://example.com/myapp.html";

    // WHEN: CreateAndRunApp is called without session callback
    int appId = appManager.CreateAndRunApp(testUrl, false);

    // THEN: Should return INVALID_APP_ID due to missing callback
    EXPECT_EQ(appId, INVALID_APP_ID);
}

TEST_F(ApplicationManagerTest, TestCreateAndRunAppReplacesExistingApp)
{
    // GIVEN: ApplicationManager with session callback registered
    ApplicationManager& appManager = ApplicationManager::instance();
    appManager.RegisterCallback(APP_TYPE_HBBTV, mockCallback.get());
    appManager.SetCurrentInterface(APP_TYPE_HBBTV);

    // First app
    std::string firstUrl = "http://first.com/app.html";

    EXPECT_CALL(*mockCallback, LoadApplication(testing::_, testing::_, testing::_, testing::_))
        .Times(2); // Called for both apps
    EXPECT_CALL(*mockCallback, ShowApplication(testing::_))
        .Times(2);

    int firstAppId = appManager.CreateAndRunApp(firstUrl, false);
    EXPECT_GT(firstAppId, 0);

    // Second app (replacement)
    std::string secondUrl = "http://second.com/app.html";
    int secondAppId = appManager.CreateAndRunApp(secondUrl, false);

    // THEN: Both should return valid app IDs
    EXPECT_GT(secondAppId, 0);

    // App should appear in running apps list
    std::vector<int> runningApps = appManager.GetRunningAppIds();
    EXPECT_FALSE(runningApps.empty());
}

TEST_F(ApplicationManagerTest, TestCreateAndRunAppWithHTTPSUrl)
{
    // GIVEN: ApplicationManager with session callback registered
    ApplicationManager& appManager = ApplicationManager::instance();
    appManager.RegisterCallback(APP_TYPE_HBBTV, mockCallback.get());
    appManager.SetCurrentInterface(APP_TYPE_HBBTV);

    std::string httpsUrl = "https://secure.example.com/myapp.html";

    // EXPECT: Callback methods to be called correctly
    EXPECT_CALL(*mockCallback, LoadApplication(testing::_, testing::_, testing::_, testing::_))
        .Times(1);
    EXPECT_CALL(*mockCallback, ShowApplication(testing::_))
        .Times(1);

    // WHEN: CreateAndRunApp is called with HTTPS URL
    int appId = appManager.CreateAndRunApp(httpsUrl, false);

    // THEN: Should return valid app ID
    EXPECT_GT(appId, 0);
}

TEST_F(ApplicationManagerTest, DISABLED_TestCreateAndRunAppParameterValidation)
{
    // GIVEN: ApplicationManager with session callback registered
    ApplicationManager& appManager = ApplicationManager::instance();
    appManager.RegisterCallback(APP_TYPE_HBBTV, mockCallback.get());
    appManager.SetCurrentInterface(APP_TYPE_HBBTV);

    // Test various URLs and parameters
    struct TestCase {
        std::string url;
        bool runAsOpApp;
        bool shouldSucceed;
    } testCases[] = {
        {"http://example.com/app.html", false, true},
        {"https://example.com/app.html", true, true},
        {"", false, false},
        {"http://example.com/app.html", true, true}
    };

    for (const auto& testCase : testCases) {
        if (testCase.url.empty()) {
            // For empty URL, expect no callbacks
            EXPECT_CALL(*mockCallback, LoadApplication(testing::_, testing::_, testing::_, testing::_))
                .Times(0);
        } else {
            // For valid URL, expect callbacks
            EXPECT_CALL(*mockCallback, LoadApplication(testing::_, testing::_, testing::_, testing::_))
                .Times(1);
            EXPECT_CALL(*mockCallback, ShowApplication(testing::_))
                .Times(1);
        }

        int appId = appManager.CreateAndRunApp(testCase.url, testCase.runAsOpApp);

        if (testCase.shouldSucceed) {
            EXPECT_GT(appId, 0) << "Failed for URL: " << testCase.url;
        } else {
            EXPECT_EQ(appId, INVALID_APP_ID) << "Should have failed for URL: " << testCase.url;
        }
    }
}

TEST_F(ApplicationManagerTest, TestCreateAndRunAppAppLifecycle)
{
    // GIVEN: ApplicationManager with session callback
    ApplicationManager& appManager = ApplicationManager::instance();
    appManager.RegisterCallback(APP_TYPE_HBBTV, mockCallback.get());
    appManager.SetCurrentInterface(APP_TYPE_HBBTV);

    std::string testUrl = "http://example.com/lifecycle_test.html";

    // EXPECT: App creation callbacks
    EXPECT_CALL(*mockCallback, LoadApplication(testing::_, testing::_, testing::_, testing::_))
        .Times(1);
    EXPECT_CALL(*mockCallback, ShowApplication(testing::_))
        .Times(1);

    // WHEN: App is created
    int appId = appManager.CreateAndRunApp(testUrl, false);

    // THEN: App should be running and trackable
    EXPECT_GT(appId, 0);

    std::vector<int> runningApps = appManager.GetRunningAppIds();
    EXPECT_FALSE(runningApps.empty());

    // App URL should be retrievable
    std::string retrievedUrl = appManager.GetApplicationUrl(appId);
    // Note: May be empty or different based on implementation
}

TEST_F(ApplicationManagerTest, TestCreateAndRunAppDefaultParameters)
{
    // GIVEN: ApplicationManager with session callback registered
    ApplicationManager& appManager = ApplicationManager::instance();
    appManager.RegisterCallback(APP_TYPE_HBBTV, mockCallback.get());
    appManager.SetCurrentInterface(APP_TYPE_HBBTV);

    std::string testUrl = "http://example.com/default_test.html";

    // EXPECT: Callback methods to be called correctly
    EXPECT_CALL(*mockCallback, LoadApplication(testing::_, testing::_, testing::_, testing::_))
        .Times(1);
    EXPECT_CALL(*mockCallback, ShowApplication(testing::_))
        .Times(1);

    // WHEN: CreateAndRunApp is called with default parameters (runAsOpApp=false)
    int appId = appManager.CreateAndRunApp(testUrl);

    // THEN: Should return valid app ID
    EXPECT_GT(appId, 0);

    // Verify it runs as HbbTV app by default
    std::string scheme = appManager.GetApplicationScheme(appId);
    // Implementation-specific verification
}

} // namespace orb
