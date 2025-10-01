#include <iostream>
#include <string>
#include <memory>

#include "testing/gtest/include/gtest/gtest.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "Moderator.h"
#include "MockOrbBrowser.h"
#include "MockJson.h"
#include "MockFactory.h"
#include "MockAppMgrInterface.h"
#include "MockComponentBase.h"


/**
 * Test fixture class for Moderator unit tests
 * Provides common setup and helper methods for all test cases
 */
class ModeratorTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create mock objects
        mockBrowser = std::make_unique<orb::MockOrbBrowser>();
        mockFactory = std::make_unique<orb::MockFactory>();
        mockAppMgrInterface = std::make_unique<orb::MockAppMgrInterface>(mockBrowser.get(), orb::APP_TYPE_HBBTV);
        mockDrm = std::make_unique<orb::MockComponentBase>();
    }

    void TearDown() override {}

    /**
     * Create a Moderator instance with the current mock objects
     */
    std::unique_ptr<orb::Moderator> createModerator() {
        return std::make_unique<orb::Moderator>(
            mockBrowser.get(),
            orb::APP_TYPE_HBBTV,
            std::move(mockFactory)
        );
    }

    void createAppMgrInterfaceAndDrm() {
      EXPECT_CALL(*mockFactory, createAppMgrInterface(mockBrowser.get(), orb::APP_TYPE_HBBTV))
         .WillOnce(::testing::Return(::testing::ByMove(std::move(mockAppMgrInterface))));
      EXPECT_CALL(*mockFactory, createDrm())
         .WillOnce(::testing::Return(::testing::ByMove(std::move(mockDrm))));
    }

    void setupJsonParsing(const std::string& input, bool parseResult, const std::string& methodValue = "") {
        auto mockJson = std::make_unique<orb::MockJson>();
        EXPECT_CALL(*mockJson, parse(input))
            .WillOnce(::testing::Return(parseResult));

        if (!methodValue.empty()) {
            EXPECT_CALL(*mockJson, getString("method"))
             .WillOnce(::testing::Return(methodValue));
        }

        EXPECT_CALL(*mockFactory, createJson(""))
            .WillOnce(::testing::Return(::testing::ByMove(std::move(mockJson))));
        createAppMgrInterfaceAndDrm();
    }

    /**
     * Set up expectations for error request handling
     */
    void setupErrorRequestHandling(const std::string& input) {
      auto mockJson = std::make_unique<orb::MockJson>();
        EXPECT_CALL(*mockJson, parse(input))
            .WillOnce(::testing::Return(true));
        EXPECT_CALL(*mockJson, hasParam("error", orb::JsonType::JSON_TYPE_OBJECT))
            .WillOnce(::testing::Return(true));
        EXPECT_CALL(*mockFactory, createJson(""))
            .WillOnce(::testing::Return(::testing::ByMove(std::move(mockJson))));
        createAppMgrInterfaceAndDrm();
    }

    /**
     * Set up expectations for invalid method handling
     */
    void setupNoMethodHandling(const std::string& input) {
      auto mockJson = std::make_unique<orb::MockJson>();
      EXPECT_CALL(*mockJson, parse(input))
            .WillOnce(::testing::Return(true));
        EXPECT_CALL(*mockJson, hasParam("method", orb::JsonType::JSON_TYPE_STRING))
            .WillOnce(::testing::Return(false));
        EXPECT_CALL(*mockJson, hasParam("error", orb::JsonType::JSON_TYPE_OBJECT))
            .WillOnce(::testing::Return(false));
        EXPECT_CALL(*mockFactory, createJson(""))
            .WillOnce(::testing::Return(::testing::ByMove(std::move(mockJson))));
        createAppMgrInterfaceAndDrm();
    }

    /**
     * Set up expectations for valid method handling
     */
    void setupHandleOrbRequest(const std::string& input, const std::string& methodValue, const std::string& resultValue) {
        auto mockJson = std::make_unique<orb::MockJson>();
        auto mockParams = std::make_unique<orb::MockJson>();
        EXPECT_CALL(*mockJson, parse(input))
            .WillOnce(::testing::Return(true));
        EXPECT_CALL(*mockJson, getString("method"))
            .WillOnce(::testing::Return(methodValue));
        EXPECT_CALL(*mockJson, getString("token"))
            .WillOnce(::testing::Return("token"));
        EXPECT_CALL(*mockJson, hasParam("method", orb::JsonType::JSON_TYPE_STRING))
            .WillOnce(::testing::Return(true));
        EXPECT_CALL(*mockJson, hasParam("error", orb::JsonType::JSON_TYPE_OBJECT))
            .WillOnce(::testing::Return(false));

         int appType = orb::APP_TYPE_HBBTV;
         EXPECT_CALL(*mockJson, setInteger("params",  appType, "applicationType"))
            .WillOnce(::testing::Return());

        EXPECT_CALL(*mockJson, getObject("params"))
            .WillOnce(::testing::Return(::testing::ByMove(std::move(mockParams))));

        EXPECT_CALL(*mockFactory, createJson(""))
            .WillOnce(::testing::Return(::testing::ByMove(std::move(mockJson))));

        createAppMgrInterfaceAndDrm();
    }

    void setupValidMethodHandlingForSendRequestToClient(const std::string& input, const std::string& methodValue, const std::string& resultValue) {
      auto mockJson = std::make_unique<orb::MockJson>();
      EXPECT_CALL(*mockJson, parse(input))
          .WillOnce(::testing::Return(true));
      EXPECT_CALL(*mockJson, getString("method"))
          .WillOnce(::testing::Return(methodValue));
      EXPECT_CALL(*mockJson, hasParam("method", orb::JsonType::JSON_TYPE_STRING))
          .WillOnce(::testing::Return(true));
      EXPECT_CALL(*mockJson, hasParam("error", orb::JsonType::JSON_TYPE_OBJECT))
          .WillOnce(::testing::Return(false));
      int appType = orb::APP_TYPE_HBBTV;
      EXPECT_CALL(*mockJson, setInteger("params",  appType, "applicationType"))
          .WillOnce(::testing::Return());
      EXPECT_CALL(*mockJson, toString())
          .WillOnce(::testing::Return(methodValue));

      EXPECT_CALL(*mockBrowser, sendRequestToClient(methodValue))
          .WillOnce(::testing::Return(resultValue));

      EXPECT_CALL(*mockFactory, createJson(""))
          .WillOnce(::testing::Return(::testing::ByMove(std::move(mockJson))));
      createAppMgrInterfaceAndDrm();
    }

    // Mock objects available to all test methods
    std::unique_ptr<orb::MockOrbBrowser> mockBrowser;
    std::unique_ptr<orb::MockFactory> mockFactory;
    std::unique_ptr<orb::MockAppMgrInterface> mockAppMgrInterface;
    std::unique_ptr<orb::MockComponentBase> mockDrm;
};

TEST_F(ModeratorTest, HandleOrbRequestEmptyRequest)
{
    setupJsonParsing("", false);
    auto moderator = createModerator();
    std::string response = moderator->handleOrbRequest("");
    EXPECT_EQ(response, R"({"error": "Invalid Request"})");
}

TEST_F(ModeratorTest, HandleOrbRequestInvalidJsonRequest)
{
    setupJsonParsing("invalid json", false);
    auto moderator = createModerator();
    std::string response = moderator->handleOrbRequest("invalid json");
    EXPECT_EQ(response, R"({"error": "Invalid Request"})");
}

TEST_F(ModeratorTest, HandleOrbRequestNoMethod)
{
    setupNoMethodHandling(R"({ "NotAMethod": { "Some": "Value" }})");
    auto moderator = createModerator();
    std::string response = moderator->handleOrbRequest(R"({ "NotAMethod": { "Some": "Value" }})");
    EXPECT_EQ(response, R"({"error": "No method"})");
}

TEST_F(ModeratorTest, HandleOrbRequestErrorRequest)
{
    setupErrorRequestHandling(R"({ "error": { "Some": "Value" }})");
    auto moderator = createModerator();
    std::string response = moderator->handleOrbRequest(R"({ "error": { "Some": "Value" }})");
    EXPECT_EQ(response, R"({"error": "Error Request"})");
}

TEST_F(ModeratorTest, HandleOrbRequestForApplicationManager)
{
    EXPECT_CALL(*mockAppMgrInterface, executeRequest("showApplication", "token", ::testing::_))
        .WillOnce(::testing::Return(R"({"result": ""})"));
    setupHandleOrbRequest(R"({ "method": "Manager.showApplication" })", "Manager.showApplication", R"({"result": ""})");
    auto moderator = createModerator();
    std::string response = moderator->handleOrbRequest(R"({ "method": "Manager.showApplication" })");
    EXPECT_EQ(response, R"({"result": ""})");
}

TEST_F(ModeratorTest, HandleOrbRequestForDrm)
{
    EXPECT_CALL(*mockDrm, executeRequest("setActiveDRM", "token", ::testing::_))
        .WillOnce(::testing::Return(R"({"result": false})"));
    setupHandleOrbRequest(R"({ "method": "Drm.setActiveDRM" })", "Drm.setActiveDRM", R"({"result": false})");
    auto moderator = createModerator();
    std::string response = moderator->handleOrbRequest(R"({ "method": "Drm.setActiveDRM" })");
    EXPECT_EQ(response, R"({"result": false})");
}

TEST_F(ModeratorTest, HandleOrbRequestForNetwork)
{
    std::string result = R"({"Response": "Network request [resolveHostAddress] not implemented"})";
    setupHandleOrbRequest(R"({ "method": "Network.resolveHostAddress" })", "Network.resolveHostAddress", result);
    auto moderator = createModerator();
    std::string response = moderator->handleOrbRequest(R"({ "method": "Network.resolveHostAddress" })");
    EXPECT_EQ(response, result);
}

TEST_F(ModeratorTest, HandleOrbRequestForSendRequestToClient)
{
    std::string result = R"({"result": "OrbClient Response"})";
    setupValidMethodHandlingForSendRequestToClient(R"({ "method": "Broadcast.SetChannel" })", "Broadcast.SetChannel", result);
    auto moderator = createModerator();
    std::string response = moderator->handleOrbRequest(R"({ "method": "Broadcast.SetChannel" })");
    EXPECT_EQ(response, result);
}

// add test to test handleBridgeEvent
TEST_F(ModeratorTest, HandleBridgeEventForChannelStatusChange)
{
    std::string etype = orb::CHANNEL_STATUS_CHANGE;
    std::string properties = R"({ "statusCode": -2, "onetId": 1, "transId": 1, "servId": 1 })";
    auto mockJson = std::make_unique<orb::MockJson>();
    EXPECT_CALL(*mockJson, parse(properties))
        .WillOnce(::testing::Return(true));
    EXPECT_CALL(*mockJson, getInteger("statusCode"))
        .WillOnce(::testing::Return(orb::CHANNEL_STATUS_CONNECTING));
    EXPECT_CALL(*mockJson, getInteger("onetId"))
        .WillOnce(::testing::Return(1));
    EXPECT_CALL(*mockJson, getInteger("transId"))
        .WillOnce(::testing::Return(1));
    EXPECT_CALL(*mockJson, getInteger("servId"))
        .WillOnce(::testing::Return(1));

    EXPECT_CALL(*mockFactory, createJson(""))
      .WillOnce(::testing::Return(::testing::ByMove(std::move(mockJson))));

    EXPECT_CALL(*mockAppMgrInterface, onChannelChange(1, 1, 1))
        .WillOnce(::testing::Return());

    createAppMgrInterfaceAndDrm();
    auto moderator = createModerator();
    bool consumed = moderator->handleBridgeEvent(etype, properties);
    EXPECT_FALSE(consumed);
}

TEST_F(ModeratorTest, HandleBridgeEventForNetworkStatusChange)
{
    std::string etype = orb::NETWORK_STATUS;
    std::string properties = R"({ "available": true })";
    auto mockJson = std::make_unique<orb::MockJson>();
    EXPECT_CALL(*mockJson, parse(properties))
        .WillOnce(::testing::Return(true));
    EXPECT_CALL(*mockJson, getBool("available"))
        .WillOnce(::testing::Return(true));

    EXPECT_CALL(*mockFactory, createJson(""))
        .WillOnce(::testing::Return(::testing::ByMove(std::move(mockJson))));

    EXPECT_CALL(*mockAppMgrInterface, onNetworkStatusChange(true))
        .WillOnce(::testing::Return());

    createAppMgrInterfaceAndDrm();
    auto moderator = createModerator();
    bool consumed = moderator->handleBridgeEvent(etype, properties);
    EXPECT_TRUE(consumed);
}

TEST_F(ModeratorTest, ProcessAitSection)
{
  const std::vector<uint8_t>& section =
  {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};
  EXPECT_CALL(*mockAppMgrInterface, processAitSection(1, 1, section))
      .WillOnce(::testing::Return());

  createAppMgrInterfaceAndDrm();
  auto moderator = createModerator();
  moderator->processAitSection(1, 1, section);
}

TEST_F(ModeratorTest, ProcessXmlAit)
{
  const std::vector<uint8_t>& xmlait =
  {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};
  EXPECT_CALL(*mockAppMgrInterface, processXmlAit(xmlait))
      .WillOnce(::testing::Return());
  createAppMgrInterfaceAndDrm();
  auto moderator = createModerator();
  moderator->processXmlAit(xmlait);
}
