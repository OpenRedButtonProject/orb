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
 * ORB MediaSynchroniser
 *
 */

#include <sys/sysinfo.h>

#include "BroadcastInterface.hpp"
#include "log.hpp"
#include "IOrbBrowser.h"
#include "BroadcastUtil.h"
#include "JsonUtil.h"

#define EMPTY_STRING ""

using namespace std;

namespace orb
{

// constants for the names of the methods
const string SET_VIDEO_RECTANGLE = "setVideoRectangle";
const string GET_CURRENT_CHANNEL = "getCurrentChannel";
const string GET_CURRENT_CHANNEL_FOR_EVENT = "getCurrentChannelForEvent";
const string GET_CHANNEL_LIST = "getChannelList";
const string SET_CHANNEL_TO_NULL = "setChannelToNull";
const string GET_VOLUME = "getVolume";
const string SET_VOLUME = "setVolume";
const string SET_CHANNEL_TO_CCID = "setChannelToCcid";
const string SET_CHANNEL_TO_TRIPLET = "setChannelToTriplet";
const string SET_CHANNEL_TO_DSD = "setChannelToDsd";
const string GET_PROGRAMMES = "getProgrammes";
const string GET_COMPONENTS = "getComponents";
const string GET_PRIVATE_AUDIO_COMPONENT = "getPrivateAudioComponent";
const string GET_PRIVATE_VIDEO_COMPONENT = "getPrivateVideoComponent";
const string OVERRIDE_COMPONENT_SELECTION = "overrideComponentSelection";
const string RESTORE_COMPONENT_SELECTION = "restoreComponentSelection";
const string START_SEARCH = "startSearch";
const string ABORT_SEARCH = "abortSearch";
const string ADD_STREAM_EVENT_LISTENER = "addStreamEventListener";
const string REMOVE_STREAM_EVENT_LISTENER = "removeStreamEventListener";
const string SET_PRESENTATION_SUSPENDED = "setPresentationSuspended";

BroadcastInterface::BroadcastInterface(IOrbBrowser* browser, std::shared_ptr<IPlatform> platform)
    : mOrbBrowser(browser)
    , mPlatform(platform)
{

}

string BroadcastInterface::executeRequest(string method, Json::Value token, Json::Value params)
{
    // TODO Set up proper responses
    string response = R"({"Response": "BroadcastInterface request [)" + method + R"(] not implemented"})";

    LOGI("Request with method [" + method + "] received: " + JsonUtil::convertJsonToString(params));
    Json::Value responseJson;
    if (method == SET_VIDEO_RECTANGLE)
    {
        int x = params["x"].asInt();
        int y = params["y"].asInt();
        int width = params["width"].asInt();
        int height = params["height"].asInt();
        mPlatform->Broadcast_SetVideoRectangle(x, y, width, height);
        responseJson["result"] = true;
        if (BroadcastUtil::isIpChannel(mPlatform->Broadcast_GetCurrentChannel())) {
            mWebSocketServer->SendIPPlayerSetVideoWindow(0, x, y, width, height);
        }

        return JsonUtil::convertJsonToString(responseJson);
    }
    else if (method == GET_CURRENT_CHANNEL)
    {
        responseJson["result"] = BroadcastUtil::convertChannelToJson(*mPlatform->Broadcast_GetCurrentChannel());
        return JsonUtil::convertJsonToString(responseJson);
    }
    else if (method == GET_CURRENT_CHANNEL_FOR_EVENT)
    {
        // TODO: Implement getCurrentChannelForEvent functionality
        responseJson["result"] = BroadcastUtil::convertChannelToJson(*mPlatform->Broadcast_GetCurrentChannel());
        return JsonUtil::convertJsonToString(responseJson);
    }
    else if (method == GET_CHANNEL_LIST)
    {
        responseJson["result"] = BroadcastUtil::convertChannelListToJson(mPlatform->Broadcast_GetChannelList());
        LOGI("getChannelList called - returning " + JsonUtil::convertJsonToString(responseJson));
        return JsonUtil::convertJsonToString(responseJson);
    }
    else if (method == SET_CHANNEL_TO_NULL)
    {
        // TODO: Implement setChannelToNull functionality
        response = R"({"result": -1})";
        LOGI("setChannelToNull called - returning -1 (not implemented)");
    }
    else if (method == GET_VOLUME)
    {
        // TODO: Implement getVolume functionality
        response = R"({"result": 50})";
        LOGI("getVolume called - returning 50 (not implemented)");
    }
    else if (method == SET_VOLUME)
    {
        // TODO: Implement setVolume functionality
        response = R"({"result": true})";
        LOGI("setVolume called - returning true (not implemented)");
    }
    else if (method == SET_CHANNEL_TO_CCID)
    {
        // TODO: Implement setChannelToCcid functionality
        int ret = mPlatform->Broadcast_SetChannelToCcid(
            params["ccid"].asString(),
            params["trickplay"].asBool(),
            params["contentAccessDescriptorURL"].asString(),
            params["quiet"].asInt());
        std::shared_ptr<Channel> currentChannel = mPlatform->Broadcast_GetCurrentChannel();

        if (BroadcastUtil::isIpChannel(currentChannel)) {
            LOGI("setChannelToCcid called - sending IPPlayerSelectChannel");
            mWebSocketServer->SendIPPlayerSelectChannel(
                currentChannel->GetChannelType(),
                currentChannel->GetIdType(),
                currentChannel->GetIpBroadcastId());
        }
        responseJson["result"] = ret;
        return JsonUtil::convertJsonToString(responseJson);
    }
    else if (method == SET_CHANNEL_TO_TRIPLET)
    {
        // TODO: Implement setChannelToTriplet functionality
        response = R"({"result": -1})";
        LOGI("setChannelToTriplet called - returning -1 (not implemented)");
    }
    else if (method == SET_CHANNEL_TO_DSD)
    {
        // TODO: Implement setChannelToDsd functionality
        response = R"({"result": -1})";
        LOGI("setChannelToDsd called - returning -1 (not implemented)");
    }
    else if (method == GET_PROGRAMMES)
    {
        // TODO: Implement getProgrammes functionality
        response = R"({"result": []})";
        LOGI("getProgrammes called - returning empty array (not implemented)");
    }
    else if (method == GET_COMPONENTS)
    {
        // TODO: Implement getComponents functionality
        std::string ccId = params["ccid"].asString();
        if (mIPChannelSessionMap.find(ccId) != mIPChannelSessionMap.end()) {
            responseJson["result"] = mIPChannelSessionMap[ccId].componentsInfo;
        } else {
            responseJson["result"] = Json::Value();
        }
        return JsonUtil::convertJsonToString(responseJson);
    }
    else if (method == GET_PRIVATE_AUDIO_COMPONENT)
    {
        // TODO: Implement getPrivateAudioComponent functionality
        response = R"({"result": null})";
        LOGI("getPrivateAudioComponent called - returning null (not implemented)");
    }
    else if (method == GET_PRIVATE_VIDEO_COMPONENT)
    {
        // TODO: Implement getPrivateVideoComponent functionality
        response = R"({"result": null})";
        LOGI("getPrivateVideoComponent called - returning null (not implemented)");
    }
    else if (method == OVERRIDE_COMPONENT_SELECTION)
    {
        // TODO: Implement overrideComponentSelection functionality
        // no response
        LOGI("overrideComponentSelection called (not implemented)");
    }
    else if (method == RESTORE_COMPONENT_SELECTION)
    {
        // TODO: Implement restoreComponentSelection functionality
        // no response
        LOGI("restoreComponentSelection called (not implemented)");
    }
    else if (method == START_SEARCH)
    {
        // TODO: Implement startSearch functionality
        // no response
        LOGI("startSearch called (not implemented)");
    }
    else if (method == ABORT_SEARCH)
    {
        // TODO: Implement abortSearch functionality
        // no response
        LOGI("abortSearch called (not implemented)");
    }
    else if (method == ADD_STREAM_EVENT_LISTENER)
    {
        // TODO: Implement addStreamEventListener functionality
        response = R"({"result": 1})";
        LOGI("addStreamEventListener called - returning 1 (not implemented)");
    }
    else if (method == REMOVE_STREAM_EVENT_LISTENER)
    {
        // TODO: Implement removeStreamEventListener functionality
        // no response
        LOGI("removeStreamEventListener called (not implemented)");
    }
    else if (method == SET_PRESENTATION_SUSPENDED)
    {
        bool presentationSuspended = params["presentationSuspended"].asBool();
        mPlatform->Broadcast_SetPresentationSuspended(presentationSuspended);
        std::shared_ptr<Channel> currentChannel = mPlatform->Broadcast_GetCurrentChannel();
        if (BroadcastUtil::isIpChannel(currentChannel)) {
            int sessionId = mIPChannelSessionMap[currentChannel->GetCcid()].sessionId;
            if (presentationSuspended) {
                mWebSocketServer->SendIPPlayerPause(sessionId);
            } else {
                mWebSocketServer->SendIPPlayerResume(sessionId);
            }
        }
        responseJson["result"] = -1;
        return JsonUtil::convertJsonToString(responseJson);
    }
    else // Unknown Method
    {
        response = R"({"error": "BroadcastInterface request [)" + method + R"(] invalid method"})";
        LOGE("Invalid Method [" + method +"]");
    }

    return response;
}

// define constants for the names of the events
const string CHANNEL_STATUS_CHANGED = "ChannelStatusChanged";
const string COMPONENT_CHANGED = "ComponentChanged";
const string PROGRAMMES_CHANGED = "ProgrammesChanged";
const string PARENTAL_RATING_CHANGE = "ParentalRatingChange";
const string PARENTAL_RATING_ERROR = "ParentalRatingError";
const string SELECTED_COMPONENT_CHANGED = "SelectedComponentChanged";
const string STREAM_EVENT = "StreamEvent";
const string SERVICE_INSTANCE_CHANGED = "ServiceInstanceChanged";

void BroadcastInterface::DispatchChannelStatusChangedEvent(const int onetId, const int transId,
    const int servId, const int statusCode, const bool permanentError, int ipSessionId)
{
    Json::Value prop;
    prop["statusCode"] = statusCode;
    prop["permanentError"] = permanentError;

    std::shared_ptr<Channel> currentChannel = mPlatform->Broadcast_GetCurrentChannel();
    if (BroadcastUtil::isIpChannel(currentChannel)
        && mIPChannelSessionMap[currentChannel->GetCcid()].sessionId == ipSessionId) {
        // if the current channel is an IP channel, use the onid, tsid and sid of the current channel
        prop["onetId"] = currentChannel->GetOnid();
        prop["transId"] = currentChannel->GetTsid();
        prop["servId"] = currentChannel->GetSid();
    } else {
        prop["onetId"] = onetId;
        prop["transId"] = transId;
        prop["servId"] = servId;
    }

    LOGD("DispatchChannelStatusChangedEvent => onetId: " << prop["onetId"].asInt() << ", transId: " << prop["transId"].asInt() <<
        ", servId: " << prop["servId"].asInt() << ", statusCode: " << prop["statusCode"].asInt() << ", statusCode: " << prop["statusCode"].asInt());
    std::string properties =  JsonUtil::convertJsonToString(prop);
    mOrbBrowser->dispatchEvent(CHANNEL_STATUS_CHANGED, properties);
}

void BroadcastInterface::DispatchServiceInstanceChangedEvent(const int index)
{
    LOGI("dispatchServiceInstanceChangedEvent => index: " << index);
    Json::Value prop;
    prop["serviceInstanceIndex"] = index;
    std::string properties = JsonUtil::convertJsonToString(prop);
    mOrbBrowser->dispatchEvent(SERVICE_INSTANCE_CHANGED, properties);
}

void BroadcastInterface::DispatchParentalRatingChangeEvent(const bool blocked)
{
    LOGI("DispatchParentalRatingChangeEvent => blocked: " << blocked);
    Json::Value prop;
    prop["blocked"] = blocked;
    std::string properties = JsonUtil::convertJsonToString(prop);
    mOrbBrowser->dispatchEvent(PARENTAL_RATING_CHANGE, properties);
}

void BroadcastInterface::DispatchParentalRatingErrorEvent(std::string contentID,
    std::vector<ParentalRating> ratings, std::string DRMSystemID)
{
    LOGI("DispatchParentalRatingErrorEvent => contentID: " << contentID);
    Json::Value prop;
    Json::Value ratingsArray(Json::arrayValue);
    for (unsigned int i = 0; i < ratings.size(); ++i)
    {
        Json::Value rating;
        rating["name"] = ratings[i].name;
        rating["scheme"] = ratings[i].scheme;
        rating["value"] = ratings[i].value;
        rating["labels"] = ratings[i].labels;
        rating["region"] = ratings[i].region;
        ratingsArray.append(rating);
    }
    prop["contentID"] = contentID.c_str();
    prop["ratings"] = ratingsArray;
    prop["DRMSystemID"] = DRMSystemID.c_str();
    std::string properties = JsonUtil::convertJsonToString(prop);
    mOrbBrowser->dispatchEvent(PARENTAL_RATING_ERROR, properties);
}

void BroadcastInterface::DispatchSelectedComponentChangedEvent(const int componentType)
{
    LOGI("DispatchSelectedComponentChangedEvent => componentType: " << componentType);
    Json::Value prop;
    prop["componentType"] = componentType;
    std::string properties = JsonUtil::convertJsonToString(prop);
    mOrbBrowser->dispatchEvent(SELECTED_COMPONENT_CHANGED, properties);
}

void BroadcastInterface::DispatchComponentChangedEvent(const int componentType, int ipSessionId, Json::Value componentsInfo)
{
    LOGD("DispatchComponentChangedEvent => componentType: " << componentType);
    Json::Value prop;
    prop["componentType"] = componentType;
    std::string properties = JsonUtil::convertJsonToString(prop);
    std::shared_ptr<Channel> currentChannel = mPlatform->Broadcast_GetCurrentChannel();
    if (BroadcastUtil::isIpChannel(currentChannel)
        && mIPChannelSessionMap[currentChannel->GetCcid()].sessionId == ipSessionId) {
        mIPChannelSessionMap[currentChannel->GetCcid()].componentsInfo = componentsInfo;
    } else if (ipSessionId >= 0) {
        LOGE("DispatchComponentChangedEvent => current channel is an IP channel, but sessionId does not match. "
            << "sessionId: " << ipSessionId << ", current channel sessionId: " << mIPChannelSessionMap[currentChannel->GetCcid()].sessionId);
    }

    mOrbBrowser->dispatchEvent(COMPONENT_CHANGED, properties);
}

void BroadcastInterface::DispatchStreamEvent(const int id, const std::string name, const std::string data,
    const std::string text, const std::string status, const DASHEvent dashEvent)
{
    LOGI("DispatchStreamEvent => id: " << id << ", name: " << name << ", data: " << data << ", text: "
        << text << ", status: " << status);
    Json::Value prop;

    Json::Value dash;
    dash["id"] = dashEvent.id;
    dash["startTime"] = dashEvent.startTime;
    dash["duration"] = dashEvent.duration;
    dash["contentEncoding"] = dashEvent.contentEncoding;

    prop["id"] = id;
    prop["name"] = name.c_str();
    prop["data"] = data.c_str();
    prop["text"] = text.c_str();
    prop["status"] = status.c_str();
    prop["DASHEvent"] = dash;
    std::string properties = JsonUtil::convertJsonToString(prop);
    mOrbBrowser->dispatchEvent(STREAM_EVENT, properties);
}

void BroadcastInterface::DispatchProgrammesChangedEvent()
{
    LOGI("dispatchProgrammesChangedEvent");
    mOrbBrowser->dispatchEvent(PROGRAMMES_CHANGED, "{}");
}

void BroadcastInterface::SetWebSocketServer(std::shared_ptr<orb::networkServices::JsonRpcService> webSocketServer)
{
    mWebSocketServer = webSocketServer;
}

void BroadcastInterface::CreateIPChannelSession(const int sessionId)
{
    std::shared_ptr<Channel> currentChannel = mPlatform->Broadcast_GetCurrentChannel();
    if (BroadcastUtil::isIpChannel(currentChannel)) {
        LOGD("CreateIPChannelSession => sessionId: " << sessionId);
        mIPChannelSessionMap[currentChannel->GetCcid()].sessionId = sessionId;
    }
}

} // namespace orb
