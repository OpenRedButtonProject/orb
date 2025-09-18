#include <iostream>
#include <string>
#include <json/json.h>
#include <memory>

#include "testing/gtest/include/gtest/gtest.h"
#include "VideoWindow.hpp"
#include "MockOrbBrowser.h"
#include "JsonRpcService.h"
#include "OrbConstants.h"
#include "log.hpp"

using namespace orb;
using namespace orb::networkServices;

// Mock JsonRpcService for testing
class MockJsonRpcService : public JsonRpcService {
public:
    MockJsonRpcService() : JsonRpcService(8080, "/test", nullptr) {}
};

class VideoWindowTest : public ::testing::Test {
protected:
    void SetUp() override {
        m_mockOrbBrowser = std::make_unique<MockOrbBrowser>();
        m_mockJsonRpcService = std::make_shared<MockJsonRpcService>();
        m_videoWindow = std::make_unique<VideoWindow>(m_mockOrbBrowser.get());
        m_videoWindow->setWebSocketService(m_mockJsonRpcService);
    }

    void TearDown() override {
        m_videoWindow.reset();
        m_mockJsonRpcService.reset();
        m_mockOrbBrowser.reset();
    }

    std::unique_ptr<MockOrbBrowser> m_mockOrbBrowser;
    std::shared_ptr<MockJsonRpcService> m_mockJsonRpcService;
    std::unique_ptr<VideoWindow> m_videoWindow;
};

TEST_F(VideoWindowTest, TestConstructor) {
    // Test that VideoWindow can be constructed with a browser
    EXPECT_NE(m_videoWindow, nullptr);
}

TEST_F(VideoWindowTest, TestSetWebSocketService) {
    // Test that WebSocket service can be set
    auto newService = std::make_shared<MockJsonRpcService>();
    m_videoWindow->setWebSocketService(newService);

    // This test verifies the method doesn't crash
    // The actual service reference is private, so we can't directly verify it
    SUCCEED();
}

TEST_F(VideoWindowTest, TestHandleRequestSelectChannel) {
    // Test handling select channel event
    std::string properties = "{\"channelType\":1,\"idType\":2,\"ipBroadcastID\":\"testBroadcast\"}";

    bool result = m_videoWindow->handleRequest("VideoWindow.selectChannel", properties);

    EXPECT_TRUE(result);
}

TEST_F(VideoWindowTest, TestHandleRequestPause) {
    // Test handling pause event
    std::string properties = "{}";

    bool result = m_videoWindow->handleRequest("VideoWindow.pause", properties);

    EXPECT_TRUE(result);
}

TEST_F(VideoWindowTest, TestHandleRequestResume) {
    // Test handling resume event
    std::string properties = "{}";

    bool result = m_videoWindow->handleRequest("VideoWindow.resume", properties);

    EXPECT_TRUE(result);
}

TEST_F(VideoWindowTest, TestHandleRequestUnknownEvent) {
    // Test handling unknown event type
    std::string properties = "{}";

    bool result = m_videoWindow->handleRequest("UnknownEvent", properties);

    EXPECT_FALSE(result);
}

TEST_F(VideoWindowTest, TestHandleRequestInvalidJson) {
    // Test handling event with invalid JSON properties
    std::string properties = "invalid json";

    bool result = m_videoWindow->handleRequest("VideoWindow.selectChannel", properties);

    EXPECT_FALSE(result);
}


TEST_F(VideoWindowTest, TestDispatchChannelStatusChangedEventConnecting) {
    // Test dispatching channel status changed event for connecting status
    Json::Value params;
    params["status"] = 1; // PLAYBACK_STATUS_CONNECTING

    std::string result = m_videoWindow->DispatchChannelStatusChangedEvent(params);

    // Verify the result contains the expected status code
    EXPECT_FALSE(result.empty());
    Json::Value resultVal = Json::Value();
    EXPECT_TRUE(orb::JsonUtil::decodeJson(result, &resultVal));
    EXPECT_EQ(resultVal["method"].asString(), "VideoWindow.ChannelStatusChanged");
    EXPECT_EQ(resultVal["params"]["statusCode"].asInt(), CHANNEL_STATUS_CONNECTING);
}

TEST_F(VideoWindowTest, TestDispatchChannelStatusChangedEventPresenting) {
    // Test dispatching channel status changed event for presenting status
    Json::Value params;
    params["status"] = 2; // PLAYBACK_STATUS_PRESENTING

    std::string result = m_videoWindow->DispatchChannelStatusChangedEvent(params);

    // Verify the result contains the expected status code
    EXPECT_FALSE(result.empty());
    Json::Value resultVal;
    EXPECT_TRUE(orb::JsonUtil::decodeJson(result, &resultVal));
    EXPECT_EQ(resultVal["method"].asString(), "VideoWindow.ChannelStatusChanged");
    EXPECT_EQ(resultVal["params"]["statusCode"].asInt(), CHANNEL_STATUS_PRESENTING);
}

TEST_F(VideoWindowTest, TestDispatchChannelStatusChangedEventStopped) {
    // Test dispatching channel status changed event for stopped status
    Json::Value params;
    params["status"] = 3; // PLAYBACK_STATUS_STOPPED

    std::string result = m_videoWindow->DispatchChannelStatusChangedEvent(params);

    // Verify the result contains the expected status code
    EXPECT_FALSE(result.empty());
    Json::Value resultVal;
    EXPECT_TRUE(orb::JsonUtil::decodeJson(result, &resultVal));
    EXPECT_EQ(resultVal["method"].asString(), "VideoWindow.ChannelStatusChanged");
    EXPECT_EQ(resultVal["params"]["statusCode"].asInt(), CHANNEL_STATUS_INTERRUPTED);
}

TEST_F(VideoWindowTest, TestDispatchChannelStatusChangedEventWithError) {
    // Test dispatching channel status changed event with error
    Json::Value params;
    params["status"] = 1;
    params["error"] = CHANNEL_STATUS_NO_SIGNAL; // Error code : CHANNEL_STATUS_NO_SIGNAL

    std::string result = m_videoWindow->DispatchChannelStatusChangedEvent(params);

    // Verify the result contains the error information
    EXPECT_FALSE(result.empty());
    Json::Value resultVal;
    EXPECT_TRUE(orb::JsonUtil::decodeJson(result, &resultVal));
    EXPECT_EQ(resultVal["params"]["statusCode"].asInt(), CHANNEL_STATUS_NO_SIGNAL);
    EXPECT_EQ(resultVal["params"]["permanentError"].asBool(), true);
}

