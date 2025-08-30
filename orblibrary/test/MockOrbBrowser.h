#ifndef MOCKORBBROWSER_H
#define MOCKORBBROWSER_H

#include "IOrbBrowser.h"
#include "JsonUtil.h"
#include <string>
#include <vector>

class MockOrbBrowser : public orb::IOrbBrowser {
    public:
    void loadApplication(std::string app_id, std::string url) override {
     // TODO: implement
    }
    void showApplication() override {
     // TODO: implement
    }
    void hideApplication() override {
     // TODO: implement
    }
    std::string sendRequestToClient(std::string jsonRequest) override {
     // get the method from the jsonRequest
     Json::Value jsonRequestVal;
     if (!orb::JsonUtil::decodeJson(jsonRequest, &jsonRequestVal)) {
         return "{\"result\":{\"error\":\"Invalid JSON request\"}}";
     }
     std::string method = jsonRequestVal["method"].asString();
     if (method == "Configuration.getCapabilities") {
         return "{\"result\":{\"jsonRpcServerEndpoint\":\"/hbbtv/jsonrpc/\",\"jsonRpcServerPort\":8080}}";
     }
     else if (method == "Configuration.getAudioProfiles") {
         return "{\"result\":{\"AudioProfiles\":[{\"name\":\"AudioProfile1\",\"id\":1},{\"name\":\"AudioProfile2\",\"id\":2}]}}";
     }
     else if (method == "Configuration.getVideoProfiles") {
         return "{\"result\":{\"VideoProfiles\":[{\"name\":\"VideoProfile1\",\"id\":1},{\"name\":\"VideoProfile2\",\"id\":2}]}}";
     } else if (method == "VideoWindow.ChannelStatusChanged") {
         return jsonRequest;
     }
     return "{\"result\":{\"error\":\"Not implemented\"}}";
    }
    void dispatchEvent(const std::string& etype, const std::string& properties) override {
     // TODO: implement
    }
    void notifyKeySetChange(uint16_t keyset, std::vector<uint16_t> otherkeys) override {
    }
 };

#endif