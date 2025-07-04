#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/orb/components/network_services/json_rpc/JsonRpcService.h"
#include "third_party/orb/components/network_services/json_rpc/JsonRpcCallback.h"
#include <thread>
#include <chrono>

using namespace orb::networkServices;

TEST(JsonRpcService, TestJsonRpcServiceStartAndStop) {
    // GIVEN: a JsonRpcService object
    std::unique_ptr<JsonRpcService::ISessionCallback> sessionCallback = std::make_unique<JsonRpcCallback>();
    JsonRpcService jsonRpcService(8090, "/jsonrpc", std::move(sessionCallback));
    //WHEN: the service is started
    EXPECT_TRUE(jsonRpcService.Start());
    std::this_thread::sleep_for(std::chrono::seconds(1));
    jsonRpcService.Stop();
}


TEST(JsonRpcService, TestConnectionSendMessage) {
    // GIVEN: a WebSocketConnection object
    struct lws *wsi = nullptr; // Mock or create a valid lws pointer
    WebSocketService::WebSocketConnection connection(wsi, "/test");

    // WHEN: sending a message
    std::string message = "Hello, WebSocket!";
    connection.SendMessage(message);

    // THEN: the message should be added to the write queue
    EXPECT_EQ(connection.GetQueueSize(), 1);
}
    

TEST(JsonRpcService, TestConnectionClose) {
    // GIVEN: a WebSocketConnection object
    struct lws *wsi = nullptr; // Mock or create a valid lws pointer
    WebSocketService::WebSocketConnection connection(wsi, "/test");

    // WHEN: closing the connection
    connection.Close();

    // THEN: the write queue should contain a close fragment
    EXPECT_EQ(connection.GetQueueSize(), 1);
}

// write cases test following methods in JsonRpcService:

TEST(JSonRpcService, TestSendIPPlayerSelectChannel) {
// GIVEN: a JsonRpcService object
std::unique_ptr<JsonRpcService::ISessionCallback> sessionCallback = std::make_unique<JsonRpcCallback>();
JsonRpcService jsonRpcService(8090, "/jsonrpc", std::move(sessionCallback));
// WHEN: sending a select channel message
jsonRpcService.SendIPPlayerSelectChannel(1, 2, "testBroadcastId");
}

TEST(JSonRpcService, TestSendIPPlayerPlay) {
    // GIVEN: a JsonRpcService object
    std::unique_ptr<JsonRpcService::ISessionCallback> sessionCallback = std::make_unique<JsonRpcCallback>();
    JsonRpcService jsonRpcService(8090, "/jsonrpc", std::move(sessionCallback));
    // WHEN: sending a play message
    jsonRpcService.SendIPPlayerPlay(1);
}

TEST(JSonRpcService, TestSendIPPlayerPause) {
    // GIVEN: a JsonRpcService object
    std::unique_ptr<JsonRpcService::ISessionCallback> sessionCallback = std::make_unique<JsonRpcCallback>();
    JsonRpcService jsonRpcService(8090, "/jsonrpc", std::move(sessionCallback));
    // WHEN: sending a pause message
    jsonRpcService.SendIPPlayerPause(1);
}   

TEST(JSonRpcService, TestSendIPPlayerStop) {
    // GIVEN: a JsonRpcService object
    std::unique_ptr<JsonRpcService::ISessionCallback> sessionCallback = std::make_unique<JsonRpcCallback>();
    JsonRpcService jsonRpcService(8090, "/jsonrpc", std::move(sessionCallback));
    // WHEN: sending a stop message
    jsonRpcService.SendIPPlayerStop(1);
}   

TEST(JSonRpcService, TestSendIPPlayerResume) {
    // GIVEN: a JsonRpcService object
    std::unique_ptr<JsonRpcService::ISessionCallback> sessionCallback = std::make_unique<JsonRpcCallback>();
    JsonRpcService jsonRpcService(8090, "/jsonrpc", std::move(sessionCallback));
    // WHEN: sending a resume message
    jsonRpcService.SendIPPlayerResume(1);
}   

TEST(JSonRpcService, TestSendIPPlayerSeek) {
    // GIVEN: a JsonRpcService object
    std::unique_ptr<JsonRpcService::ISessionCallback> sessionCallback = std::make_unique<JsonRpcCallback>();
    JsonRpcService jsonRpcService(8090, "/jsonrpc", std::move(sessionCallback));
    // WHEN: sending a seek message
    jsonRpcService.SendIPPlayerSeek(1, 100, 0);
}   

TEST(JSonRpcService, TestSendIPPlayerSetVideoWindow) {
    // GIVEN: a JsonRpcService object
    std::unique_ptr<JsonRpcService::ISessionCallback> sessionCallback = std::make_unique<JsonRpcCallback>();
    JsonRpcService jsonRpcService(8090, "/jsonrpc", std::move(sessionCallback));
    // WHEN: sending a set video window message
    jsonRpcService.SendIPPlayerSetVideoWindow(1, 0, 0, 800, 600);
}       

TEST(JSonRpcService, TestSendIPPlayerSetRelativeVolume) {
    // GIVEN: a JsonRpcService object
    std::unique_ptr<JsonRpcService::ISessionCallback> sessionCallback = std::make_unique<JsonRpcCallback>();
    JsonRpcService jsonRpcService(8090, "/jsonrpc", std::move(sessionCallback));
    // WHEN: sending a set relative volume message
    jsonRpcService.SendIPPlayerSetRelativeVolume(1, 50);
}   

TEST(JSonRpcService, TestSendIPPlayerSelectComponents) {
    // GIVEN: a JsonRpcService object
    std::unique_ptr<JsonRpcService::ISessionCallback> sessionCallback = std::make_unique<JsonRpcCallback>();
    JsonRpcService jsonRpcService(8090, "/jsonrpc", std::move(sessionCallback));
    // WHEN: sending a select components message
    std::vector<int> videoComponents = {1, 2};
    std::vector<int> audioComponents = {3, 4};
    std::vector<int> subtitleComponents = {5, 6};
    jsonRpcService.SendIPPlayerSelectComponents(1, videoComponents, audioComponents, subtitleComponents);
}   

TEST(JSonRpcService, TestSendIPPlayerResolveTimeline) {
    // GIVEN: a JsonRpcService object
    std::unique_ptr<JsonRpcService::ISessionCallback> sessionCallback = std::make_unique<JsonRpcCallback>();
    JsonRpcService jsonRpcService(8090, "/jsonrpc", std::move(sessionCallback));
    // WHEN: sending a resolve timeline message
    jsonRpcService.SendIPPlayerResolveTimeline(1, "testTimelineSelector");
}   

TEST(JsonRpcService, TestRequestIPPlaybackStatusUpdate) {
    // GIVEN: a JsonRpcService object
    std::unique_ptr<JsonRpcService::ISessionCallback> sessionCallback = std::make_unique<JsonRpcCallback>();
    JsonRpcService jsonRpcService(8090, "/jsonrpc", std::move(sessionCallback));
    // WHEN: requesting IP playback status update
    Json::Value obj;
    obj["id"] = "1";
    obj["params"] = Json::Value(Json::objectValue);
    obj["params"]["status"] = "update";
    auto status = jsonRpcService.RequestIPPlaybackStatusUpdate(1, obj);
    EXPECT_EQ(status, JsonRpcService::JsonRpcStatus::SUCCESS);
}

TEST(JsonRpcService, TestRequestIPPlaybackMediaPositionUpdate) {
    // GIVEN: a JsonRpcService object
    std::unique_ptr<JsonRpcService::ISessionCallback> sessionCallback = std::make_unique<JsonRpcCallback>();
    JsonRpcService jsonRpcService(8090, "/jsonrpc", std::move(sessionCallback));
    // WHEN: requesting IP playback media position update
    Json::Value obj;
    obj["id"] = "1";
    obj["params"] = Json::Value(Json::objectValue);
    obj["params"]["position"] = 100;
    auto status = jsonRpcService.RequestIPPlaybackMediaPositionUpdate(1, obj);
    EXPECT_EQ(status, JsonRpcService::JsonRpcStatus::SUCCESS);
}

TEST(JsonRpcService, TestRequestIPPlaybackSetComponents) {
    // GIVEN: a JsonRpcService object
    std::unique_ptr<JsonRpcService::ISessionCallback> sessionCallback = std::make_unique<JsonRpcCallback>();
    JsonRpcService jsonRpcService(8090, "/jsonrpc", std::move(sessionCallback));
    // WHEN: requesting IP playback set components
    Json::Value obj;
    obj["id"] = "1";
    obj["params"] = Json::Value(Json::objectValue);
    obj["params"]["components"] = Json::arrayValue;
    obj["params"]["components"].append("video");
    obj["params"]["components"].append("audio");
    auto status = jsonRpcService.RequestIPPlaybackSetComponents(1, obj);
    EXPECT_EQ(status, JsonRpcService::JsonRpcStatus::SUCCESS);
}

TEST(JsonRpcService, TestRequestIPPlaybackSetTimelineMapping) {
    // GIVEN: a JsonRpcService object
    std::unique_ptr<JsonRpcService::ISessionCallback> sessionCallback = std::make_unique<JsonRpcCallback>();
    JsonRpcService jsonRpcService(8090, "/jsonrpc", std::move(sessionCallback));
    // WHEN: requesting IP playback set timeline mapping
    Json::Value obj;
    obj["id"] = "1";
    obj["params"] = Json::Value(Json::objectValue);
    obj["params"]["timeline"] = "testTimeline";
    auto status = jsonRpcService.RequestIPPlaybackSetTimelineMapping(1, obj);
    EXPECT_EQ(status, JsonRpcService::JsonRpcStatus::SUCCESS);
}

TEST(JsonRpcService, TestRequestIPPlaybackSetPresentFollowing) {
    // GIVEN: a JsonRpcService object
    std::unique_ptr<JsonRpcService::ISessionCallback> sessionCallback = std::make_unique<JsonRpcCallback>();
    JsonRpcService jsonRpcService(8090, "/jsonrpc", std::move(sessionCallback));
    // WHEN: requesting IP playback set present following
    Json::Value obj;
    obj["id"] = "1";
    obj["params"] = Json::Value(Json::objectValue);
    obj["params"]["presentFollowing"] = true;
    auto status = jsonRpcService.RequestIPPlaybackSetPresentFollowing(1, obj);
    EXPECT_EQ(status, JsonRpcService::JsonRpcStatus::SUCCESS);
}
