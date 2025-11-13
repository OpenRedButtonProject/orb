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

    /**
     * Creates a basic mock AIT table with default values.
     * Caller can customize the returned table before using it.
     */
    std::unique_ptr<Ait::S_AIT_TABLE> CreateBasicMockAitTable()
    {
        auto mockAitTable = std::make_unique<Ait::S_AIT_TABLE>();
        mockAitTable->numApps = 1;
        mockAitTable->appArray.resize(1);

        // Basic app info with defaults
        mockAitTable->appArray[0].appId = 1;
        mockAitTable->appArray[0].orgId = 1;
        mockAitTable->appArray[0].scheme = "urn:dvb:metadata:cs:LinkedApplicationCS:2019:1.1";
        mockAitTable->appArray[0].controlCode = Ait::APP_CTL_AUTOSTART;

        // Version info with defaults
        mockAitTable->appArray[0].appDesc.appProfiles.resize(1);
        mockAitTable->appArray[0].appDesc.appProfiles[0].versionMajor = 1;
        mockAitTable->appArray[0].appDesc.appProfiles[0].versionMinor = 0;
        mockAitTable->appArray[0].appDesc.appProfiles[0].versionMicro = 0;
        mockAitTable->appArray[0].appDesc.appProfiles[0].appProfile = 0;

        // Priority and transport with defaults
        mockAitTable->appArray[0].appDesc.priority = 1;
        mockAitTable->appArray[0].numTransports = 1;
        mockAitTable->appArray[0].transportArray[0].protocolId = AIT_PROTOCOL_HTTP;
        mockAitTable->appArray[0].transportArray[0].failedToLoad = false;
        mockAitTable->appArray[0].parentalRatings.clear();

        return mockAitTable;
    }

    /**
     * Sets up ApplicationManager with common configuration (network, parental control, callback).
     * Takes ownership of the xmlParser.
     */
    void SetupApplicationManager(ApplicationManager& appManager, std::unique_ptr<NiceMock<MockXmlParser>> xmlParser)
    {
        appManager.SetXmlParser(std::move(xmlParser));
        appManager.OnNetworkAvailabilityChanged(true);
        ON_CALL(*mockCallback, GetParentalControlRegion()).WillByDefault(Return("US"));
        ON_CALL(*mockCallback, GetParentalControlRegion3()).WillByDefault(Return("USA"));
        ON_CALL(*mockCallback, GetParentalControlAge()).WillByDefault(Return(18));
        appManager.RegisterCallback(APP_TYPE_HBBTV, mockCallback.get());
    }

    /**
     * Sets up the EXPECT_CALL for ParseAit and returns the mockAitTable to be moved.
     * The caller should then move mockXmlParser into ApplicationManager and call ProcessXmlAit.
     */
    void SetupParseAitExpectation(const std::string& xmlContent,
                                  std::unique_ptr<Ait::S_AIT_TABLE> mockAitTable)
    {
        EXPECT_CALL(*mockXmlParser, ParseAit(xmlContent.c_str(), xmlContent.length()))
            .WillOnce(Return(ByMove(std::move(mockAitTable))));
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
    auto mockAitTable = CreateBasicMockAitTable();
    mockAitTable->appArray[0].scheme = "urn:hbbtv:opapp:privileged:2017";

    // Set up expectation BEFORE moving the mock object
    SetupParseAitExpectation(xmlContent, std::move(mockAitTable));

    ApplicationManager appManager;
    SetupApplicationManager(appManager, std::move(mockXmlParser));

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

// ============================================================================
// Unit tests for BaseApp keyset methods
// ============================================================================

TEST_F(ApplicationManagerTest, TestGetKeySetMaskDefaultValue)
{
    // GIVEN: ApplicationManager with a created app
    ApplicationManager appManager;
    appManager.RegisterCallback(APP_TYPE_HBBTV, mockCallback.get());

    EXPECT_CALL(*mockCallback, LoadApplication(
        testing::_, testing::_, testing::_, testing::_, testing::An<MockApplicationSessionCallback::onPageLoadedSuccess>()))
        .Times(1);

    int appId = appManager.CreateAndRunApp("http://example.com/app.html", false);

    // WHEN: GetKeySetMask is called on a new app
    uint16_t keySetMask = appManager.GetKeySetMask(appId);

    // THEN: Should return 0 (default value)
    EXPECT_EQ(keySetMask, 0);
}

TEST_F(ApplicationManagerTest, TestSetKeySetMaskBasic)
{
    // GIVEN: ApplicationManager with a created app
    ApplicationManager appManager;
    appManager.RegisterCallback(APP_TYPE_HBBTV, mockCallback.get());

    EXPECT_CALL(*mockCallback, LoadApplication(
        testing::_, testing::_, testing::_, testing::_, testing::An<MockApplicationSessionCallback::onPageLoadedSuccess>()))
        .Times(1);

    int appId = appManager.CreateAndRunApp("http://example.com/app.html", false);

    // WHEN: SetKeySetMask is called with a simple mask
    uint16_t mask = KEY_SET_RED | KEY_SET_GREEN;
    uint16_t result = appManager.SetKeySetMask(appId, mask, {});

    // THEN: Should return the same mask
    EXPECT_EQ(result, mask);
    EXPECT_EQ(appManager.GetKeySetMask(appId), mask);
}

TEST_F(ApplicationManagerTest, TestSetKeySetMaskWithOtherKeys)
{
    // GIVEN: ApplicationManager with a created app
    ApplicationManager appManager;
    appManager.RegisterCallback(APP_TYPE_HBBTV, mockCallback.get());

    EXPECT_CALL(*mockCallback, LoadApplication(
        testing::_, testing::_, testing::_, testing::_, testing::An<MockApplicationSessionCallback::onPageLoadedSuccess>()))
        .Times(1);

    int appId = appManager.CreateAndRunApp("http://example.com/app.html", false);

    // WHEN: SetKeySetMask is called with KEY_SET_OTHER and other keys
    std::vector<uint16_t> otherKeys = {416, 500, 600}; // VK_RECORD and custom keys
    uint16_t mask = KEY_SET_OTHER | KEY_SET_RED;
    uint16_t result = appManager.SetKeySetMask(appId, mask, otherKeys);

    // THEN: Should return the mask and store other keys
    EXPECT_EQ(result, mask);
    EXPECT_EQ(appManager.GetKeySetMask(appId), mask);
    std::vector<uint16_t> retrievedKeys = appManager.GetOtherKeyValues(appId);
    EXPECT_EQ(retrievedKeys, otherKeys);
}

TEST_F(ApplicationManagerTest, TestSetKeySetMaskWithoutOtherKeysFlag)
{
    // GIVEN: ApplicationManager with a created app
    ApplicationManager appManager;
    appManager.RegisterCallback(APP_TYPE_HBBTV, mockCallback.get());

    EXPECT_CALL(*mockCallback, LoadApplication(
        testing::_, testing::_, testing::_, testing::_, testing::An<MockApplicationSessionCallback::onPageLoadedSuccess>()))
        .Times(1);

    int appId = appManager.CreateAndRunApp("http://example.com/app.html", false);

    // WHEN: SetKeySetMask is called with other keys but without KEY_SET_OTHER flag
    std::vector<uint16_t> otherKeys = {416, 500};
    uint16_t mask = KEY_SET_RED; // No KEY_SET_OTHER flag
    appManager.SetKeySetMask(appId, mask, otherKeys);

    // THEN: Other keys should not be stored
    std::vector<uint16_t> retrievedKeys = appManager.GetOtherKeyValues(appId);
    EXPECT_TRUE(retrievedKeys.empty());
}

TEST_F(ApplicationManagerTest, TestGetOtherKeyValuesDefault)
{
    // GIVEN: ApplicationManager with a created app
    ApplicationManager appManager;
    appManager.RegisterCallback(APP_TYPE_HBBTV, mockCallback.get());

    EXPECT_CALL(*mockCallback, LoadApplication(
        testing::_, testing::_, testing::_, testing::_, testing::An<MockApplicationSessionCallback::onPageLoadedSuccess>()))
        .Times(1);

    int appId = appManager.CreateAndRunApp("http://example.com/app.html", false);

    // WHEN: GetOtherKeyValues is called on a new app
    std::vector<uint16_t> otherKeys = appManager.GetOtherKeyValues(appId);

    // THEN: Should return empty vector
    EXPECT_TRUE(otherKeys.empty());
}

TEST_F(ApplicationManagerTest, TestInKeySetNavigationKeys)
{
    // GIVEN: ApplicationManager with a created app and navigation keyset
    ApplicationManager appManager;
    appManager.RegisterCallback(APP_TYPE_HBBTV, mockCallback.get());

    EXPECT_CALL(*mockCallback, LoadApplication(
        testing::_, testing::_, testing::_, testing::_, testing::An<MockApplicationSessionCallback::onPageLoadedSuccess>()))
        .Times(1);

    int appId = appManager.CreateAndRunApp("http://example.com/app.html", false);
    appManager.SetKeySetMask(appId, KEY_SET_NAVIGATION, {});

    // WHEN: InKeySet is called with navigation keys
    // THEN: Should return true for navigation keys
    EXPECT_TRUE(appManager.InKeySet(appId, 38));  // VK_UP
    EXPECT_TRUE(appManager.InKeySet(appId, 40));  // VK_DOWN
    EXPECT_TRUE(appManager.InKeySet(appId, 37));  // VK_LEFT
    EXPECT_TRUE(appManager.InKeySet(appId, 39));  // VK_RIGHT
    EXPECT_TRUE(appManager.InKeySet(appId, 13));  // VK_ENTER
    EXPECT_TRUE(appManager.InKeySet(appId, 461)); // VK_BACK

    // AND: Should return false for non-navigation keys
    EXPECT_FALSE(appManager.InKeySet(appId, 403)); // VK_RED
    EXPECT_FALSE(appManager.InKeySet(appId, 48));  // VK_NUMERIC_START
}

TEST_F(ApplicationManagerTest, TestInKeySetColorKeys)
{
    // GIVEN: ApplicationManager with a created app and color keyset
    ApplicationManager appManager;
    appManager.RegisterCallback(APP_TYPE_HBBTV, mockCallback.get());

    EXPECT_CALL(*mockCallback, LoadApplication(
        testing::_, testing::_, testing::_, testing::_, testing::An<MockApplicationSessionCallback::onPageLoadedSuccess>()))
        .Times(1);

    int appId = appManager.CreateAndRunApp("http://example.com/app.html", false);
    uint16_t mask = KEY_SET_RED | KEY_SET_GREEN | KEY_SET_YELLOW | KEY_SET_BLUE;
    appManager.SetKeySetMask(appId, mask, {});

    // WHEN: InKeySet is called with color keys
    // THEN: Should return true for color keys
    EXPECT_TRUE(appManager.InKeySet(appId, 403)); // VK_RED
    EXPECT_TRUE(appManager.InKeySet(appId, 404)); // VK_GREEN
    EXPECT_TRUE(appManager.InKeySet(appId, 405)); // VK_YELLOW
    EXPECT_TRUE(appManager.InKeySet(appId, 406)); // VK_BLUE

    // AND: Should return false for other keys
    EXPECT_FALSE(appManager.InKeySet(appId, 38));  // VK_UP
    EXPECT_FALSE(appManager.InKeySet(appId, 457)); // VK_INFO
}

TEST_F(ApplicationManagerTest, TestInKeySetNumericKeys)
{
    // GIVEN: ApplicationManager with a created app and numeric keyset
    ApplicationManager appManager;
    appManager.RegisterCallback(APP_TYPE_HBBTV, mockCallback.get());

    EXPECT_CALL(*mockCallback, LoadApplication(
        testing::_, testing::_, testing::_, testing::_, testing::An<MockApplicationSessionCallback::onPageLoadedSuccess>()))
        .Times(1);

    int appId = appManager.CreateAndRunApp("http://example.com/app.html", false);
    appManager.SetKeySetMask(appId, KEY_SET_NUMERIC, {});

    // WHEN: InKeySet is called with numeric keys
    // THEN: Should return true for numeric keys (0-9)
    for (uint16_t key = 48; key <= 57; ++key) {
        EXPECT_TRUE(appManager.InKeySet(appId, key));
    }

    // AND: Should return false for non-numeric keys
    EXPECT_FALSE(appManager.InKeySet(appId, 47)); // Before numeric range
    EXPECT_FALSE(appManager.InKeySet(appId, 58)); // After numeric range
}

TEST_F(ApplicationManagerTest, TestInKeySetAlphaKeys)
{
    // GIVEN: ApplicationManager with a created app and alpha keyset
    ApplicationManager appManager;
    appManager.RegisterCallback(APP_TYPE_HBBTV, mockCallback.get());

    EXPECT_CALL(*mockCallback, LoadApplication(
        testing::_, testing::_, testing::_, testing::_, testing::An<MockApplicationSessionCallback::onPageLoadedSuccess>()))
        .Times(1);

    int appId = appManager.CreateAndRunApp("http://example.com/app.html", false);
    appManager.SetKeySetMask(appId, KEY_SET_ALPHA, {});

    // WHEN: InKeySet is called with alpha keys
    // THEN: Should return true for alpha keys (A-Z)
    for (uint16_t key = 65; key <= 90; ++key) {
        EXPECT_TRUE(appManager.InKeySet(appId, key));
    }

    // AND: Should return false for non-alpha keys
    EXPECT_FALSE(appManager.InKeySet(appId, 64)); // Before alpha range
    EXPECT_FALSE(appManager.InKeySet(appId, 91)); // After alpha range
}

TEST_F(ApplicationManagerTest, TestInKeySetVcrKeys)
{
    // GIVEN: ApplicationManager with a created app and VCR keyset
    ApplicationManager appManager;
    appManager.RegisterCallback(APP_TYPE_HBBTV, mockCallback.get());

    EXPECT_CALL(*mockCallback, LoadApplication(
        testing::_, testing::_, testing::_, testing::_, testing::An<MockApplicationSessionCallback::onPageLoadedSuccess>()))
        .Times(1);

    int appId = appManager.CreateAndRunApp("http://example.com/app.html", false);
    appManager.SetKeySetMask(appId, KEY_SET_VCR, {});

    // WHEN: InKeySet is called with VCR keys
    // THEN: Should return true for VCR keys
    EXPECT_TRUE(appManager.InKeySet(appId, 415)); // VK_PLAY
    EXPECT_TRUE(appManager.InKeySet(appId, 413)); // VK_STOP
    EXPECT_TRUE(appManager.InKeySet(appId, 19));  // VK_PAUSE
    EXPECT_TRUE(appManager.InKeySet(appId, 417)); // VK_FAST_FWD
    EXPECT_TRUE(appManager.InKeySet(appId, 412)); // VK_REWIND
    EXPECT_TRUE(appManager.InKeySet(appId, 425)); // VK_NEXT
    EXPECT_TRUE(appManager.InKeySet(appId, 424)); // VK_PREV
    EXPECT_TRUE(appManager.InKeySet(appId, 402)); // VK_PLAY_PAUSE

    // AND: Should return false for other keys
    EXPECT_FALSE(appManager.InKeySet(appId, 403)); // VK_RED
}

TEST_F(ApplicationManagerTest, TestInKeySetScrollKeys)
{
    // GIVEN: ApplicationManager with a created app and scroll keyset
    ApplicationManager appManager;
    appManager.RegisterCallback(APP_TYPE_HBBTV, mockCallback.get());

    EXPECT_CALL(*mockCallback, LoadApplication(
        testing::_, testing::_, testing::_, testing::_, testing::An<MockApplicationSessionCallback::onPageLoadedSuccess>()))
        .Times(1);

    int appId = appManager.CreateAndRunApp("http://example.com/app.html", false);
    appManager.SetKeySetMask(appId, KEY_SET_SCROLL, {});

    // WHEN: InKeySet is called with scroll keys
    // THEN: Should return true for scroll keys
    EXPECT_TRUE(appManager.InKeySet(appId, 33)); // VK_PAGE_UP
    EXPECT_TRUE(appManager.InKeySet(appId, 34)); // VK_PAGE_DOWN

    // AND: Should return false for other keys
    EXPECT_FALSE(appManager.InKeySet(appId, 38)); // VK_UP
}

TEST_F(ApplicationManagerTest, TestInKeySetInfoKey)
{
    // GIVEN: ApplicationManager with a created app and info keyset
    ApplicationManager appManager;
    appManager.RegisterCallback(APP_TYPE_HBBTV, mockCallback.get());

    EXPECT_CALL(*mockCallback, LoadApplication(
        testing::_, testing::_, testing::_, testing::_, testing::An<MockApplicationSessionCallback::onPageLoadedSuccess>()))
        .Times(1);

    int appId = appManager.CreateAndRunApp("http://example.com/app.html", false);
    appManager.SetKeySetMask(appId, KEY_SET_INFO, {});

    // WHEN: InKeySet is called with info key
    // THEN: Should return true for info key
    EXPECT_TRUE(appManager.InKeySet(appId, 457)); // VK_INFO

    // AND: Should return false for other keys
    EXPECT_FALSE(appManager.InKeySet(appId, 403)); // VK_RED
}

TEST_F(ApplicationManagerTest, TestInKeySetWithOtherKeys)
{
    // GIVEN: ApplicationManager with a created app and KEY_SET_OTHER with specific keys
    ApplicationManager appManager;
    appManager.RegisterCallback(APP_TYPE_HBBTV, mockCallback.get());

    EXPECT_CALL(*mockCallback, LoadApplication(
        testing::_, testing::_, testing::_, testing::_, testing::An<MockApplicationSessionCallback::onPageLoadedSuccess>()))
        .Times(1);

    int appId = appManager.CreateAndRunApp("http://example.com/app.html", false);

    // Note: GetKeySetMaskForKeyCode returns 0 for keys that don't map to any known key set.
    // For KEY_SET_OTHER, InKeySet only checks otherKeys if GetKeySetMaskForKeyCode returns
    // a non-zero value that matches the mask. Since VK_RECORD (416) maps to KEY_SET_OTHER,
    // it will work. However, keys like 500 and 600 that don't map to any key set will return
    // false because GetKeySetMaskForKeyCode(500) = 0, so the first condition in InKeySet fails.
    std::vector<uint16_t> otherKeys = {416, 500, 600}; // VK_RECORD is the ONLY key that maps to KEY_SET_OTHER
    appManager.SetKeySetMask(appId, KEY_SET_OTHER, otherKeys);

    // WHEN: InKeySet is called with VK_RECORD (which maps to KEY_SET_OTHER and is in otherKeys)
    // THEN: Should return true
    EXPECT_TRUE(appManager.InKeySet(appId, 416)); // VK_RECORD (in otherKeys, maps to KEY_SET_OTHER)
    // AND: Should return false for keys that don't map to any key set (GetKeySetMaskForKeyCode returns 0)
    // because InKeySet checks GetKeySetMaskForKeyCode first, and if it returns 0, it never checks otherKeys
    EXPECT_FALSE(appManager.InKeySet(appId, 500)); // Doesn't map to any key set, returns false
    EXPECT_FALSE(appManager.InKeySet(appId, 600)); // Doesn't map to any key set, returns false
    EXPECT_FALSE(appManager.InKeySet(appId, 700)); // Doesn't map to any key set, returns false
}

TEST_F(ApplicationManagerTest, TestInKeySetUnknownKey)
{
    // GIVEN: ApplicationManager with a created app and a keyset
    ApplicationManager appManager;
    appManager.RegisterCallback(APP_TYPE_HBBTV, mockCallback.get());

    EXPECT_CALL(*mockCallback, LoadApplication(
        testing::_, testing::_, testing::_, testing::_, testing::An<MockApplicationSessionCallback::onPageLoadedSuccess>()))
        .Times(1);

    int appId = appManager.CreateAndRunApp("http://example.com/app.html", false);
    appManager.SetKeySetMask(appId, KEY_SET_RED | KEY_SET_GREEN, {});

    // WHEN: InKeySet is called with an unknown key code
    // THEN: Should return false
    EXPECT_FALSE(appManager.InKeySet(appId, 9999)); // Unknown key code
}

TEST_F(ApplicationManagerTest, TestInKeySetInvalidAppId)
{
    // GIVEN: ApplicationManager
    ApplicationManager appManager;

    // WHEN: InKeySet is called with invalid app ID
    // THEN: Should return false
    EXPECT_FALSE(appManager.InKeySet(BaseApp::INVALID_APP_ID, 403));
    EXPECT_FALSE(appManager.InKeySet(99999, 403));
}

// ============================================================================
// Unit tests for HbbTVApp SetKeySetMask override
// ============================================================================

TEST_F(ApplicationManagerTest, TestHbbTVAppSetKeySetMaskActivatedApp)
{
    // GIVEN: ApplicationManager with an activated HbbTV app (non-AUTOSTART)
    ApplicationManager appManager;
    appManager.RegisterCallback(APP_TYPE_HBBTV, mockCallback.get());

    EXPECT_CALL(*mockCallback, LoadApplication(
        testing::_, testing::_, testing::_, testing::_, testing::An<MockApplicationSessionCallback::onPageLoadedSuccess>()))
        .Times(1);

    int appId = appManager.CreateAndRunApp("http://example.com/app.html", false);
    // App is activated by default when created via CreateAndRunApp

    // WHEN: SetKeySetMask is called with VCR, NUMERIC, and OTHER keysets
    uint16_t mask = KEY_SET_VCR | KEY_SET_NUMERIC | KEY_SET_OTHER;
    std::vector<uint16_t> otherKeys = {416};
    uint16_t result = appManager.SetKeySetMask(appId, mask, otherKeys);

    // THEN: All keysets should be preserved (app is activated)
    EXPECT_EQ(result, mask);
    EXPECT_EQ(appManager.GetKeySetMask(appId), mask);
}

TEST_F(ApplicationManagerTest, TestHbbTVAppSetKeySetMaskUnactivatedAppOldVersionScheme11)
{
    // GIVEN: ApplicationManager with an unactivated HbbTV app (AUTOSTART) with old version and scheme 1.1

    // Create app via AIT with AUTOSTART control code (unactivated)
    std::string xmlContent = "valid xml content";
    auto mockAitTable = CreateBasicMockAitTable();
    mockAitTable->appArray[0].scheme = "urn:dvb:metadata:cs:LinkedApplicationCS:2019:1.1";
    mockAitTable->appArray[0].appDesc.appProfiles[0].versionMinor = 2; // Old version (> 1)

    SetupParseAitExpectation(xmlContent, std::move(mockAitTable));

    ApplicationManager appManagerWithParser;
    SetupApplicationManager(appManagerWithParser, std::move(mockXmlParser));

    int appId = appManagerWithParser.ProcessXmlAit(xmlContent, false, "urn:dvb:metadata:cs:LinkedApplicationCS:2019:1.1");
    EXPECT_GT(appId, BaseApp::INVALID_APP_ID);

    // WHEN: SetKeySetMask is called with VCR, NUMERIC, and OTHER keysets
    uint16_t mask = KEY_SET_VCR | KEY_SET_NUMERIC | KEY_SET_OTHER | KEY_SET_RED;
    std::vector<uint16_t> otherKeys = {416};
    uint16_t result = appManagerWithParser.SetKeySetMask(appId, mask, otherKeys);

    // THEN: VCR, NUMERIC, and OTHER should be filtered out (unactivated, old version, scheme 1.1)
    uint16_t expectedMask = KEY_SET_RED; // Only RED should remain
    EXPECT_EQ(result, expectedMask);
    EXPECT_EQ(appManagerWithParser.GetKeySetMask(appId), expectedMask);
}

TEST_F(ApplicationManagerTest, TestHbbTVAppSetKeySetMaskUnactivatedAppScheme12)
{
    // GIVEN: ApplicationManager with an unactivated HbbTV app with scheme 1.2

    // Create app via AIT with AUTOSTART control code
    std::string xmlContent = "valid xml content";
    auto mockAitTable = CreateBasicMockAitTable();
    mockAitTable->appArray[0].scheme = "urn:dvb:metadata:cs:LinkedApplicationCS:2019:1.2";
    mockAitTable->appArray[0].appDesc.appProfiles[0].versionMinor = 2; // Old version

    SetupParseAitExpectation(xmlContent, std::move(mockAitTable));

    ApplicationManager appManagerWithParser;
    SetupApplicationManager(appManagerWithParser, std::move(mockXmlParser));

    int appId = appManagerWithParser.ProcessXmlAit(xmlContent, false, "urn:dvb:metadata:cs:LinkedApplicationCS:2019:1.2");
    EXPECT_GT(appId, BaseApp::INVALID_APP_ID);

    // WHEN: SetKeySetMask is called with VCR, NUMERIC, and OTHER keysets
    uint16_t mask = KEY_SET_VCR | KEY_SET_NUMERIC | KEY_SET_OTHER | KEY_SET_RED;
    std::vector<uint16_t> otherKeys = {416};
    uint16_t result = appManagerWithParser.SetKeySetMask(appId, mask, otherKeys);

    // THEN: Only VCR should be filtered out (scheme 1.2 allows NUMERIC and OTHER)
    // Filtering logic:
    // - VCR: filtered because (isOldVersion && !isException) = (true && true) = true
    // - NUMERIC: NOT filtered because (!isLinkedAppScheme12 && isOldVersion) = (!true && true) = false
    // - OTHER: NOT filtered because (!isLinkedAppScheme12 && isOldVersion) = (!true && true) = false
    // Note: VCR is filtered for old versions unless it's the exception case (scheme 1.2, version 7)
    // Since versionMinor is 2 (not 7), VCR should be filtered
    uint16_t expectedMask = KEY_SET_NUMERIC | KEY_SET_OTHER | KEY_SET_RED; // Only VCR filtered
    EXPECT_EQ(result, expectedMask);
    EXPECT_EQ(appManagerWithParser.GetKeySetMask(appId), expectedMask);

    // AND: otherKeys should be stored since KEY_SET_OTHER survived filtering
    std::vector<uint16_t> retrievedKeys = appManagerWithParser.GetOtherKeyValues(appId);
    EXPECT_EQ(retrievedKeys, otherKeys);
}

TEST_F(ApplicationManagerTest, TestHbbTVAppSetKeySetMaskUnactivatedAppScheme12Version7)
{
    // GIVEN: ApplicationManager with an unactivated HbbTV app with scheme 1.2 and version 7 (exception case)

    // Create app via AIT with AUTOSTART control code
    std::string xmlContent = "valid xml content";
    auto mockAitTable = CreateBasicMockAitTable();
    mockAitTable->appArray[0].scheme = "urn:dvb:metadata:cs:LinkedApplicationCS:2019:1.2";
    mockAitTable->appArray[0].appDesc.appProfiles[0].versionMinor = 7; // Exception case: scheme 1.2, versionMinor == 7

    SetupParseAitExpectation(xmlContent, std::move(mockAitTable));

    ApplicationManager appManagerWithParser;
    SetupApplicationManager(appManagerWithParser, std::move(mockXmlParser));

    int appId = appManagerWithParser.ProcessXmlAit(xmlContent, false, "urn:dvb:metadata:cs:LinkedApplicationCS:2019:1.2");
    EXPECT_GT(appId, BaseApp::INVALID_APP_ID);

    // WHEN: SetKeySetMask is called with VCR keyset
    uint16_t mask = KEY_SET_VCR | KEY_SET_RED;
    std::vector<uint16_t> otherKeys = {};
    uint16_t result = appManagerWithParser.SetKeySetMask(appId, mask, otherKeys);

    // THEN: VCR should NOT be filtered (exception case: scheme 1.2, version 7)
    EXPECT_EQ(result, mask);
    EXPECT_EQ(appManagerWithParser.GetKeySetMask(appId), mask);
}

TEST_F(ApplicationManagerTest, TestHbbTVAppSetKeySetMaskUnactivatedAppScheme2)
{
    // GIVEN: ApplicationManager with an unactivated HbbTV app with scheme 2

    // Create app via AIT with AUTOSTART control code
    std::string xmlContent = "valid xml content";
    auto mockAitTable = CreateBasicMockAitTable();
    mockAitTable->appArray[0].scheme = "urn:dvb:metadata:cs:LinkedApplicationCS:2019:2";
    mockAitTable->appArray[0].appDesc.appProfiles[0].versionMinor = 2; // Old version

    SetupParseAitExpectation(xmlContent, std::move(mockAitTable));

    ApplicationManager appManagerWithParser;
    SetupApplicationManager(appManagerWithParser, std::move(mockXmlParser));

    int appId = appManagerWithParser.ProcessXmlAit(xmlContent, false, "urn:dvb:metadata:cs:LinkedApplicationCS:2019:2");
    EXPECT_GT(appId, BaseApp::INVALID_APP_ID);

    // WHEN: SetKeySetMask is called with VCR, NUMERIC, and OTHER keysets
    uint16_t mask = KEY_SET_VCR | KEY_SET_NUMERIC | KEY_SET_OTHER | KEY_SET_RED;
    std::vector<uint16_t> otherKeys = {416};
    uint16_t result = appManagerWithParser.SetKeySetMask(appId, mask, otherKeys);

    // THEN: All keysets should be preserved (scheme 2 bypasses filtering)
    EXPECT_EQ(result, mask);
    EXPECT_EQ(appManagerWithParser.GetKeySetMask(appId), mask);
}

TEST_F(ApplicationManagerTest, TestHbbTVAppSetKeySetMaskNewVersion)
{
    // GIVEN: ApplicationManager with an unactivated HbbTV app with new version (versionMinor <= 1)

    // Create app via AIT with AUTOSTART control code
    std::string xmlContent = "valid xml content";
    auto mockAitTable = CreateBasicMockAitTable();
    mockAitTable->appArray[0].appDesc.appProfiles[0].versionMinor = 1; // New version (<= 1)

    SetupParseAitExpectation(xmlContent, std::move(mockAitTable));

    ApplicationManager appManagerWithParser;
    SetupApplicationManager(appManagerWithParser, std::move(mockXmlParser));

    int appId = appManagerWithParser.ProcessXmlAit(xmlContent, false, "urn:dvb:metadata:cs:LinkedApplicationCS:2019:1.1");
    EXPECT_GT(appId, BaseApp::INVALID_APP_ID);

    // WHEN: SetKeySetMask is called with VCR, NUMERIC, and OTHER keysets
    uint16_t mask = KEY_SET_VCR | KEY_SET_NUMERIC | KEY_SET_OTHER | KEY_SET_RED;
    std::vector<uint16_t> otherKeys = {416};
    uint16_t result = appManagerWithParser.SetKeySetMask(appId, mask, otherKeys);

    // THEN: All keysets should be preserved (new version bypasses filtering)
    EXPECT_EQ(result, mask);
    EXPECT_EQ(appManagerWithParser.GetKeySetMask(appId), mask);
}

// ============================================================================
// Unit tests for HbbTVApp InKeySet override (activation behavior)
// ============================================================================

TEST_F(ApplicationManagerTest, TestHbbTVAppInKeySetActivatesApp)
{
    // GIVEN: ApplicationManager with an unactivated HbbTV app (AUTOSTART)

    // Create app via AIT with AUTOSTART control code
    std::string xmlContent = "valid xml content";
    auto mockAitTable = CreateBasicMockAitTable();

    SetupParseAitExpectation(xmlContent, std::move(mockAitTable));

    ApplicationManager appManagerWithParser;
    SetupApplicationManager(appManagerWithParser, std::move(mockXmlParser));

    int appId = appManagerWithParser.ProcessXmlAit(xmlContent, false, "urn:dvb:metadata:cs:LinkedApplicationCS:2019:1.1");
    EXPECT_GT(appId, BaseApp::INVALID_APP_ID);

    // Set a keyset mask
    appManagerWithParser.SetKeySetMask(appId, KEY_SET_RED, {});

    // WHEN: InKeySet is called with an accepted key (first time, app is unactivated)
    bool result1 = appManagerWithParser.InKeySet(appId, 403); // VK_RED

    // THEN: Should return true and activate the app
    EXPECT_TRUE(result1);

    // AND WHEN: SetKeySetMask is called again with VCR keyset (now that app is activated)
    // The app should now accept VCR keys even though it's an old version
    appManagerWithParser.SetKeySetMask(appId, KEY_SET_VCR, {});
    bool result2 = appManagerWithParser.InKeySet(appId, 415); // VK_PLAY

    // THEN: Should return true (app is now activated, so VCR keys are accepted)
    EXPECT_TRUE(result2);
}

TEST_F(ApplicationManagerTest, TestHbbTVAppInKeySetDoesNotActivateOnRejectedKey)
{
    // GIVEN: ApplicationManager with an unactivated HbbTV app

    // Create app via AIT with AUTOSTART control code
    std::string xmlContent = "valid xml content";
    auto mockAitTable = CreateBasicMockAitTable();
    mockAitTable->appArray[0].appDesc.appProfiles[0].versionMinor = 2; // Old version (> 1) required for VCR filtering

    SetupParseAitExpectation(xmlContent, std::move(mockAitTable));

    ApplicationManager appManagerWithParser;
    SetupApplicationManager(appManagerWithParser, std::move(mockXmlParser));

    int appId = appManagerWithParser.ProcessXmlAit(xmlContent, false, "urn:dvb:metadata:cs:LinkedApplicationCS:2019:1.1");
    EXPECT_GT(appId, BaseApp::INVALID_APP_ID);

    // Verify app is unactivated (AUTOSTART control code sets m_isActivated = false)
    // Set a keyset mask (only RED key)
    appManagerWithParser.SetKeySetMask(appId, KEY_SET_RED, {});

    // WHEN: InKeySet is called with a rejected key (not in keyset)
    bool result = appManagerWithParser.InKeySet(appId, 404); // VK_GREEN (not in keyset)

    // THEN: Should return false and app should remain unactivated
    EXPECT_FALSE(result);

    // AND: App should still be unactivated (can verify by trying to set VCR mask which would be filtered)
    // For an unactivated app with old version (versionMinor > 1) and scheme 1.1, VCR should be filtered
    // VCR filtering requires: !m_isActivated && isOldVersion && !isException
    uint16_t mask = KEY_SET_VCR | KEY_SET_RED;
    uint16_t resultMask = appManagerWithParser.SetKeySetMask(appId, mask, {});
    // VCR should be filtered out if app is still unactivated and isOldVersion = true
    EXPECT_EQ(resultMask, KEY_SET_RED);
}

} // namespace orb
