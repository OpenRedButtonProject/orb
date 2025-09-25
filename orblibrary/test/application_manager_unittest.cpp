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
#include "MockApplicationSessionCallback.h"

namespace orb
{

class ApplicationManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Set up test environment
        mockCallback = std::make_unique<MockApplicationSessionCallback>();
    }

    void TearDown() override
    {
        // Clean up test environment
        mockCallback.reset();
    }

    std::unique_ptr<MockApplicationSessionCallback> mockCallback;
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

TEST_F(ApplicationManagerTest, TestProcessXmlAitInvalidXml)
{
    // GIVEN: ApplicationManager singleton and invalid XML
    ApplicationManager& appManager = ApplicationManager::instance();
    std::string invalidXml = "This is not valid XML";

    // WHEN: ProcessXmlAit is called with invalid XML
    int result = appManager.ProcessXmlAit(invalidXml);

    // THEN: Should return INVALID_APP_ID
    EXPECT_EQ(result, INVALID_APP_ID);
}

TEST_F(ApplicationManagerTest, TestProcessXmlAitValidXmlAit)
{
    // GIVEN: ApplicationManager singleton and valid XML AIT
    ApplicationManager& appManager = ApplicationManager::instance();

    // Register the mock callback
    appManager.RegisterCallback(APP_TYPE_HBBTV, mockCallback.get());
    appManager.SetCurrentInterface(APP_TYPE_HBBTV);

    // Valid XML AIT content (simplified)
    std::string validXmlAit = R"(
        <?xml version="1.0" encoding="UTF-8"?>
        <ait xmlns="urn:dvb:metadata:ait:2019" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
            <application>
                <applicationIdentifier>
                    <orgId>1</orgId>
                    <appId>1</appId>
                </applicationIdentifier>
                <applicationDescriptor>
                    <applicationName>
                        <name>Test App</name>
                    </applicationName>
                    <applicationBoundary>http://example.com/</applicationBoundary>
                    <applicationTransport>http://example.com/app.html</applicationTransport>
                    <applicationLocation>http://example.com/app.html</applicationLocation>
                    <applicationMimeType>application/x-hbbtv</applicationMimeType>
                </applicationDescriptor>
            </application>
        </ait>
    )";

    // WHEN: ProcessXmlAit is called with valid XML AIT
    int result = appManager.ProcessXmlAit(validXmlAit);

    // THEN: Should return a valid app ID (not INVALID_APP_ID)
    // Note: The actual result depends on the XML parsing and app creation logic
    // This test verifies that the method doesn't crash and returns some value
    EXPECT_NE(result, INVALID_APP_ID);
}

TEST_F(ApplicationManagerTest, TestProcessXmlAitWithDvbiFlag)
{
    // GIVEN: ApplicationManager singleton and valid XML AIT
    ApplicationManager& appManager = ApplicationManager::instance();

    // Register the mock callback
    appManager.RegisterCallback(APP_TYPE_HBBTV, mockCallback.get());
    appManager.SetCurrentInterface(APP_TYPE_HBBTV);

    // Valid XML AIT content
    std::string validXmlAit = R"(
        <?xml version="1.0" encoding="UTF-8"?>
        <ait xmlns="urn:dvb:metadata:ait:2019" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
            <application>
                <applicationIdentifier>
                    <orgId>1</orgId>
                    <appId>1</appId>
                </applicationIdentifier>
                <applicationDescriptor>
                    <applicationName>
                        <name>Test DVB-I App</name>
                    </applicationName>
                    <applicationBoundary>http://example.com/</applicationBoundary>
                    <applicationTransport>http://example.com/app.html</applicationTransport>
                    <applicationLocation>http://example.com/app.html</applicationLocation>
                    <applicationMimeType>application/x-hbbtv</applicationMimeType>
                </applicationDescriptor>
            </application>
        </ait>
    )";

    // WHEN: ProcessXmlAit is called with isDvbi=true
    int result = appManager.ProcessXmlAit(validXmlAit, true);

    // THEN: Should return a valid app ID (not INVALID_APP_ID)
    // For DVB-I apps, this should return the current HbbTV app ID
    EXPECT_NE(result, INVALID_APP_ID);
}

TEST_F(ApplicationManagerTest, TestProcessXmlAitWithCustomScheme)
{
    // GIVEN: ApplicationManager singleton and valid XML AIT
    ApplicationManager& appManager = ApplicationManager::instance();

    // Register the mock callback
    appManager.RegisterCallback(APP_TYPE_HBBTV, mockCallback.get());
    appManager.SetCurrentInterface(APP_TYPE_HBBTV);

    // Valid XML AIT content
    std::string validXmlAit = R"(
        <?xml version="1.0" encoding="UTF-8"?>
        <ait xmlns="urn:dvb:metadata:ait:2019" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
            <application>
                <applicationIdentifier>
                    <orgId>1</orgId>
                    <appId>1</appId>
                </applicationIdentifier>
                <applicationDescriptor>
                    <applicationName>
                        <name>Test App with Custom Scheme</name>
                    </applicationName>
                    <applicationBoundary>http://example.com/</applicationBoundary>
                    <applicationTransport>http://example.com/app.html</applicationTransport>
                    <applicationLocation>http://example.com/app.html</applicationLocation>
                    <applicationMimeType>application/x-hbbtv</applicationMimeType>
                </applicationDescriptor>
            </application>
        </ait>
    )";

    // WHEN: ProcessXmlAit is called with custom scheme
    std::string customScheme = "urn:hbbtv:opapp:privileged:2017";
    int result = appManager.ProcessXmlAit(validXmlAit, false, customScheme);

    // THEN: Should return a valid app ID (not INVALID_APP_ID)
    EXPECT_NE(result, INVALID_APP_ID);
}

} // namespace orb
