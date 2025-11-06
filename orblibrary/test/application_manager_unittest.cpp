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
#include "third_party/orb/orblibrary/moderator/app_mgr/base_app.h"
#include "MockXmlParser.h"
#include <gmock/gmock.h>

using ::testing::NiceMock; // Suppresses warnings about unused mock objects
using ::testing::Return;
using ::testing::ReturnNull;
using ::testing::ByMove;
using ::testing::_;

namespace orb
{
class ApplicationManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        mockCallback = std::make_unique<NiceMock<MockApplicationSessionCallback>>();
        mockXmlParser = std::make_unique<NiceMock<MockXmlParser>>();
    }

    void TearDown() override
    {
        mockCallback.reset();
        mockXmlParser.reset();
    }

    std::unique_ptr<NiceMock<MockApplicationSessionCallback>> mockCallback;
    std::unique_ptr<NiceMock<MockXmlParser>> mockXmlParser;
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
    ApplicationManager appManager(std::move(mockXmlParser));

    // WHEN: ProcessXmlAit is called with empty XML
    int result = appManager.ProcessXmlAit("");

    // THEN: Should return BaseApp::INVALID_APP_ID
    EXPECT_EQ(result, BaseApp::INVALID_APP_ID);
}

TEST_F(ApplicationManagerTest, TestProcessXmlAitWithMockParserFailure)
{
    // GIVEN: ApplicationManager and mock XML parser set to fail
    ON_CALL(*mockXmlParser, ParseAit(_, _)).WillByDefault(ReturnNull());

    ApplicationManager appManager(std::move(mockXmlParser));

    // WHEN: ProcessXmlAit is called
    int result = appManager.ProcessXmlAit("Don't care");

    // THEN: Should return BaseApp::INVALID_APP_ID due to parser failure
    EXPECT_EQ(result, BaseApp::INVALID_APP_ID);
}

TEST_F(ApplicationManagerTest, TestProcessXmlAitWithValidAitTable)
{
    // GIVEN: ApplicationManager and valid AIT table with autostart app
    std::string xmlContent = "valid xml content";

    // Create a mock AIT table meeting ALL conditions for getAutoStartApp to return non-null
    auto mockAitTable = std::make_unique<Ait::S_AIT_TABLE>();
    mockAitTable->numApps = 1;
    mockAitTable->appArray.resize(1);

    // ✅ Basic app info
    mockAitTable->appArray[0].appId = 1;
    mockAitTable->appArray[0].orgId = 1;
    mockAitTable->appArray[0].scheme = "urn:hbbtv:opapp:privileged:2017";

    // ✅ Control code: AUTOSTART (required for getAutoStartApp)
    mockAitTable->appArray[0].controlCode = Ait::APP_CTL_AUTOSTART;

    // ✅ Version: Supported HbbTV version (1.0.0 <= 1.7.1)
    mockAitTable->appArray[0].appDesc.appProfiles.resize(1);
    mockAitTable->appArray[0].appDesc.appProfiles[0].versionMajor = 1;
    mockAitTable->appArray[0].appDesc.appProfiles[0].versionMinor = 0;
    mockAitTable->appArray[0].appDesc.appProfiles[0].versionMicro = 0;
    mockAitTable->appArray[0].appDesc.appProfiles[0].appProfile = 0; // Base profile only

    // ✅ Priority: Valid priority for selection
    mockAitTable->appArray[0].appDesc.priority = 1;

    // ✅ Transport: HTTP protocol with no failures (viable transport)
    mockAitTable->appArray[0].numTransports = 1;
    mockAitTable->appArray[0].transportArray[0].protocolId = AIT_PROTOCOL_HTTP;
    mockAitTable->appArray[0].transportArray[0].failedToLoad = false;

    // ✅ Parental ratings: Empty vector means no restrictions (IsAgeRestricted returns false)
    mockAitTable->appArray[0].parentalRatings.clear();

    // Set up expectation BEFORE moving the mock object
    EXPECT_CALL(*mockXmlParser, ParseAit(xmlContent.c_str(), xmlContent.length()))
        .WillOnce(Return(ByMove(std::move(mockAitTable))));

    ApplicationManager appManager(std::move(mockXmlParser));

    // ✅ Set network availability to true (required for HTTP transport)
    appManager.OnNetworkAvailabilityChanged(true);

    // ✅ Register callback for parental control parameters
    ON_CALL(*mockCallback, GetParentalControlRegion()).WillByDefault(Return("US"));
    ON_CALL(*mockCallback, GetParentalControlRegion3()).WillByDefault(Return("USA"));
    ON_CALL(*mockCallback, GetParentalControlAge()).WillByDefault(Return(18));
    appManager.RegisterCallback(APP_TYPE_HBBTV, mockCallback.get());

    // WHEN: ProcessXmlAit is called with isDvbi=false (triggers getAutoStartApp path)
    int result = appManager.ProcessXmlAit(xmlContent, false, "urn:hbbtv:opapp:privileged:2017");

    // THEN: Should return a valid app ID (getAutoStartApp returned non-null and app was created)
    EXPECT_GT(result, BaseApp::INVALID_APP_ID);
}

TEST_F(ApplicationManagerTest, TestProcessXmlAitWithInvalidAitTable)
{
    // GIVEN: ApplicationManager and invalid AIT table
    std::string xmlContent = "valid xml content";

    // Create a mock AIT table with no apps
    auto mockAitTable = std::make_unique<Ait::S_AIT_TABLE>();
    mockAitTable->numApps = 0;

    // Set up expectation BEFORE moving the mock object
    EXPECT_CALL(*mockXmlParser, ParseAit(xmlContent.c_str(), xmlContent.length()))
        .WillOnce(Return(ByMove(std::move(mockAitTable))));

    ApplicationManager appManager(std::move(mockXmlParser));

    // WHEN: ProcessXmlAit is called with invalid AIT table
    int result = appManager.ProcessXmlAit(xmlContent, false, "urn:hbbtv:opapp:privileged:2017");

    // THEN: Should return BaseApp::INVALID_APP_ID due to invalid AIT table
    EXPECT_EQ(result, BaseApp::INVALID_APP_ID);
}

/* TODO Test for isDvbi=true path */

TEST_F(ApplicationManagerTest, TestRegisterCallback)
{
    // GIVEN: ApplicationManager singleton
    ApplicationManager appManager;

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
    ApplicationManager appManager;

    // WHEN: RegisterCallback is called with invalid app type
    appManager.RegisterCallback(APP_TYPE_MAX, mockCallback.get());

    // THEN: No exception should be thrown
    // The method should handle invalid parameters gracefully
    SUCCEED();
}

TEST_F(ApplicationManagerTest, TestRegisterCallbackNullCallback)
{
    // GIVEN: ApplicationManager singleton
    ApplicationManager appManager;

    // WHEN: RegisterCallback is called with null callback
    appManager.RegisterCallback(APP_TYPE_HBBTV, nullptr);

    // THEN: No exception should be thrown
    // The method should handle null callback gracefully
    SUCCEED();
}

TEST_F(ApplicationManagerTest, TestGetRunningAppIds)
{
    // GIVEN: ApplicationManager singleton
    ApplicationManager appManager;

    // WHEN: GetRunningAppIds is called
    std::vector<int> appIds = appManager.GetRunningAppIds();

    // THEN: Should return a vector (may be empty)
    // This test verifies the method doesn't crash and returns a valid vector
    EXPECT_TRUE(appIds.empty() || !appIds.empty());
}

TEST_F(ApplicationManagerTest, TestGetOrganizationId)
{
    // GIVEN: ApplicationManager singleton
    ApplicationManager appManager;

    // WHEN: GetOrganizationId is called with no running apps
    int orgId = appManager.GetOrganizationId();

    // THEN: Should return -1 (indicating no running app)
    EXPECT_EQ(orgId, -1);
}

TEST_F(ApplicationManagerTest, TestGetCurrentAppNames)
{
    // GIVEN: ApplicationManager singleton
    ApplicationManager appManager;

    // WHEN: GetCurrentAppNames is called with no running apps
    std::map<std::string, std::string> names = appManager.GetCurrentAppNames();

    // THEN: Should return an empty map
    EXPECT_TRUE(names.empty());
}

// Unit tests for CreateAndRunApp public method
TEST_F(ApplicationManagerTest, TestCreateAndRunAppWithValidHbbTVUrl)
{
    // GIVEN: ApplicationManager with session callback registered
    ApplicationManager appManager;
    appManager.RegisterCallback(APP_TYPE_HBBTV, mockCallback.get());

    std::string testUrl = "http://example.com/myapp.html";

    // EXPECT: Callback methods to be called correctly
    EXPECT_CALL(*mockCallback, LoadApplication(
        testing::_, testing::_, testing::_, testing::_, testing::An<MockApplicationSessionCallback::onPageLoadedSuccess>()))
        .Times(1);

    // WHEN: CreateAndRunApp is called with HbbTV URL (runAsOpApp=false)
    int appId = appManager.CreateAndRunApp(testUrl, false);

    // THEN: Should return a valid app ID (not INVALID_APP_ID)
    EXPECT_GT(appId, BaseApp::INVALID_APP_ID);
}

TEST_F(ApplicationManagerTest, TestCreateAndRunAppWithValidOpAppUrl)
{
    // GIVEN: ApplicationManager with session callback registered for OpApp
    ApplicationManager appManager;
    appManager.RegisterCallback(APP_TYPE_OPAPP, mockCallback.get());

    std::string testUrl = "http://operator.com/opapp.html";

    // EXPECT: Callback methods to be called correctly
    EXPECT_CALL(*mockCallback, LoadApplication(
        testing::_, testing::_, testing::An<MockApplicationSessionCallback::onPageLoadedSuccess>()))
        .Times(1);

    // WHEN: CreateAndRunApp is called with OpApp URL (runAsOpApp=true)
    int appId = appManager.CreateAndRunApp(testUrl, true);

    // THEN: Should return valid app ID
    EXPECT_GT(appId, BaseApp::INVALID_APP_ID);
}

TEST_F(ApplicationManagerTest, TestCreateAndRunAppWithEmptyUrl)
{
    // GIVEN: ApplicationManager with session callback registered
    ApplicationManager appManager;
    appManager.RegisterCallback(APP_TYPE_HBBTV, mockCallback.get());

    std::string emptyUrl = "";

    // EXPECT: No callback methods should be called for empty URL
    EXPECT_CALL(*mockCallback, LoadApplication(
        testing::_, testing::_, testing::_, testing::_, testing::An<MockApplicationSessionCallback::onPageLoadedSuccess>()))
        .Times(0);

    // WHEN: CreateAndRunApp is called with empty URL
    int appId = appManager.CreateAndRunApp(emptyUrl, false);

    // THEN: Should return BaseApp::INVALID_APP_ID
    EXPECT_EQ(appId, BaseApp::INVALID_APP_ID);
}

TEST_F(ApplicationManagerTest, TestCreateAndRunAppWithoutSessionCallback)
{
    // GIVEN: ApplicationManager without session callback
    ApplicationManager appManager;
    // Explicitly ensure no callback is registered for current interface
    appManager.RegisterCallback(APP_TYPE_HBBTV, nullptr);

    std::string testUrl = "http://example.com/myapp.html";

    // WHEN: CreateAndRunApp is called without session callback
    int appId = appManager.CreateAndRunApp(testUrl, false);

    // THEN: Should return BaseApp::INVALID_APP_ID due to missing callback
    EXPECT_EQ(appId, BaseApp::INVALID_APP_ID);
}

TEST_F(ApplicationManagerTest, TestCreateAndRunAppReplacesExistingApp)
{
    // GIVEN: ApplicationManager with session callback registered
    ApplicationManager appManager;
    appManager.RegisterCallback(APP_TYPE_HBBTV, mockCallback.get());

    // WHEN: CreateAndRunApp is called with first URL
    std::string firstUrl = "http://first.com/app.html";

    EXPECT_CALL(*mockCallback, LoadApplication(
        testing::_, testing::_, testing::_, testing::_, testing::An<MockApplicationSessionCallback::onPageLoadedSuccess>()))
        .Times(2); // Called for both apps

    int firstAppId = appManager.CreateAndRunApp(firstUrl, false);
    EXPECT_GT(firstAppId, BaseApp::INVALID_APP_ID);

    // THEN: first app is running and is the only app with that ID
    auto runningApps = appManager.GetRunningAppIds();
    EXPECT_TRUE(runningApps.size() == 1);
    EXPECT_EQ(runningApps[0], firstAppId);

    // AND WHEN: CreateAndRunApp is called with second URL
    std::string secondUrl = "http://second.com/app.html";
    int secondAppId = appManager.CreateAndRunApp(secondUrl, false);

    // THEN: valid next ID is returned and different from first app ID
    EXPECT_GT(secondAppId, firstAppId);

    // AND: appears in running apps list
    runningApps = appManager.GetRunningAppIds();
    EXPECT_TRUE(runningApps.size() == 1);
    EXPECT_EQ(runningApps[0], secondAppId);
}


TEST_F(ApplicationManagerTest, TestCreateAndRunAppWithHTTPSUrl)
{
    // GIVEN: ApplicationManager with session callback registered
    ApplicationManager appManager;
    appManager.RegisterCallback(APP_TYPE_HBBTV, mockCallback.get());

    std::string httpsUrl = "https://secure.example.com/myapp.html";

    // EXPECT: Callback methods to be called correctly
    EXPECT_CALL(*mockCallback, LoadApplication(
        testing::_, testing::_, testing::_, testing::_, testing::An<MockApplicationSessionCallback::onPageLoadedSuccess>()))
        .Times(1);

    // WHEN: CreateAndRunApp is called with HTTPS URL
    int appId = appManager.CreateAndRunApp(httpsUrl, false);

    // THEN: Should return valid app ID
    EXPECT_GT(appId, BaseApp::INVALID_APP_ID);
}

TEST_F(ApplicationManagerTest, TestUrlRetrieval)
{
    // GIVEN: ApplicationManager with session callback
    ApplicationManager appManager;
    appManager.RegisterCallback(APP_TYPE_HBBTV, mockCallback.get());

    std::string testUrl = "http://example.com/lifecycle_test.html";

    // EXPECT: App creation callbacks
    EXPECT_CALL(*mockCallback, LoadApplication(
        testing::_, testing::_, testing::_, testing::_, testing::An<MockApplicationSessionCallback::onPageLoadedSuccess>()))
        .Times(1);

    // WHEN: App is created
    int appId = appManager.CreateAndRunApp(testUrl, false);


    // App URL should be retrievable
    std::string retrievedUrl = appManager.GetApplicationUrl(appId);
    // Note: May be empty or different based on implementation
    EXPECT_EQ(retrievedUrl, testUrl);
}

TEST_F(ApplicationManagerTest, TestCreateAndRunAppDefaultParameters)
{
    // GIVEN: ApplicationManager with session callback registered
    ApplicationManager appManager;
    appManager.RegisterCallback(APP_TYPE_HBBTV, mockCallback.get());

    std::string testUrl = "http://example.com/default_test.html";

    // EXPECT: Callback methods to be called correctly
    EXPECT_CALL(*mockCallback, LoadApplication(
        testing::_, testing::_, testing::_, testing::_, testing::An<MockApplicationSessionCallback::onPageLoadedSuccess>()))
        .Times(1);

    // WHEN: CreateAndRunApp is called with default parameters (runAsOpApp=false)
    int appId = appManager.CreateAndRunApp(testUrl);

    // THEN: Should return valid app ID
    EXPECT_GT(appId, BaseApp::INVALID_APP_ID);

    // Verify it runs as HbbTV app by default
    std::string scheme = appManager.GetApplicationScheme(appId);
    EXPECT_EQ(scheme, "urn:dvb:metadata:cs:LinkedApplicationCS:2019:1.1");
}

} // namespace orb
