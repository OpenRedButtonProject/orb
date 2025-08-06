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
#include "log.h"
#include "IOrbBrowser.h"

#define EMPTY_STRING ""

using namespace std;

namespace orb
{

BroadcastInterface::BroadcastInterface(IOrbBrowser* browser)
    : mOrbBrowser(browser)
{ }

string BroadcastInterface::executeRequest(string method, Json::Value token, Json::Value params)
{
    // TODO Set up proper responses
    string response = R"({"Response": "BroadcastInterface request [)" + method + R"(] not implemented"})";

    LOGI("Request with method [" + method + "] received");
    if (method == "setVideoRectangle")
    {
        // no response
        LOGI("");
    }
    else if (method == "getCurrentChannel")
    {
        // TODO: Implement getCurrentChannel functionality
        // This should return the current broadcast channel information
        response = R"({"result": null})";
        LOGI("getCurrentChannel called - returning null (not implemented)");
    }
    else if (method == "getCurrentChannelForEvent")
    {
        // TODO: Implement getCurrentChannelForEvent functionality
        response = R"({"result": null})";
        LOGI("getCurrentChannelForEvent called - returning null (not implemented)");
    }
    else if (method == "getChannelList")
    {
        // TODO: Implement getChannelList functionality
        response = R"({"result": []})";
        LOGI("getChannelList called - returning empty array (not implemented)");
    }
    else if (method == "setChannelToNull")
    {
        // TODO: Implement setChannelToNull functionality
        response = R"({"result": -1})";
        LOGI("setChannelToNull called - returning -1 (not implemented)");
    }
    else if (method == "getVolume")
    {
        // TODO: Implement getVolume functionality
        response = R"({"result": 50})";
        LOGI("getVolume called - returning 50 (not implemented)");
    }
    else if (method == "setVolume")
    {
        // TODO: Implement setVolume functionality
        response = R"({"result": true})";
        LOGI("setVolume called - returning true (not implemented)");
    }
    else if (method == "setChannelToCcid")
    {
        // TODO: Implement setChannelToCcid functionality
        response = R"({"result": -1})";
        LOGI("setChannelToCcid called - returning -1 (not implemented)");
    }
    else if (method == "setChannelToTriplet")
    {
        // TODO: Implement setChannelToTriplet functionality
        response = R"({"result": -1})";
        LOGI("setChannelToTriplet called - returning -1 (not implemented)");
    }
    else if (method == "setChannelToDsd")
    {
        // TODO: Implement setChannelToDsd functionality
        response = R"({"result": -1})";
        LOGI("setChannelToDsd called - returning -1 (not implemented)");
    }
    else if (method == "getProgrammes")
    {
        // TODO: Implement getProgrammes functionality
        response = R"({"result": []})";
        LOGI("getProgrammes called - returning empty array (not implemented)");
    }
    else if (method == "getComponents")
    {
        // TODO: Implement getComponents functionality
        response = R"({"result": []})";
        LOGI("getComponents called - returning empty array (not implemented)");
    }
    else if (method == "getPrivateAudioComponent")
    {
        // TODO: Implement getPrivateAudioComponent functionality
        response = R"({"result": null})";
        LOGI("getPrivateAudioComponent called - returning null (not implemented)");
    }
    else if (method == "getPrivateVideoComponent")
    {
        // TODO: Implement getPrivateVideoComponent functionality
        response = R"({"result": null})";
        LOGI("getPrivateVideoComponent called - returning null (not implemented)");
    }
    else if (method == "overrideComponentSelection")
    {
        // TODO: Implement overrideComponentSelection functionality
        // no response
        LOGI("overrideComponentSelection called (not implemented)");
    }
    else if (method == "restoreComponentSelection")
    {
        // TODO: Implement restoreComponentSelection functionality
        // no response
        LOGI("restoreComponentSelection called (not implemented)");
    }
    else if (method == "startSearch")
    {
        // TODO: Implement startSearch functionality
        // no response
        LOGI("startSearch called (not implemented)");
    }
    else if (method == "abortSearch")
    {
        // TODO: Implement abortSearch functionality
        // no response
        LOGI("abortSearch called (not implemented)");
    }
    else if (method == "addStreamEventListener")
    {
        // TODO: Implement addStreamEventListener functionality
        response = R"({"result": 1})";
        LOGI("addStreamEventListener called - returning 1 (not implemented)");
    }
    else if (method == "removeStreamEventListener")
    {
        // TODO: Implement removeStreamEventListener functionality
        // no response
        LOGI("removeStreamEventListener called (not implemented)");
    }
    else if (method == "setPresentationSuspended")
    {
        // TODO: Implement setPresentationSuspended functionality
        // no response
        LOGI("setPresentationSuspended called (not implemented)");
    }
    else // Unknown Method
    {
        response = R"({"error": "BroadcastInterface request [)" + method + R"(] invalid method"})";
        LOGE("Invalid Method [" + method +"]");
    }

    return response;
}

void BroadcastInterface::DispatchChannelStatusChangedEvent(const int onetId, const int transId,
    const int servId, const int statusCode, const bool permanentError)
{
    LOGI("DispatchChannelStatusChangedEvent => onetId: " << onetId << ", transId: " << transId <<
        ", servId: " << servId << ", statusCode: " << statusCode << ", statusCode: " << statusCode);
    Json::StreamWriterBuilder writerBuilder;
    Json::Value prop;
    writerBuilder["indentation"] = EMPTY_STRING; // optional?
    prop["onetId"] = onetId;
    prop["transId"] = transId;
    prop["servId"] = servId;
    prop["statusCode"] = statusCode;
    prop["permanentError"] = permanentError;
    std::string properties = Json::writeString(writerBuilder, prop);
    mOrbBrowser->dispatchEvent("ChannelStatusChanged", properties);
}

void BroadcastInterface::DispatchServiceInstanceChangedEvent(const int index)
{
    LOGI("dispatchServiceInstanceChangedEvent => index: " << index);
    Json::StreamWriterBuilder writerBuilder;
    Json::Value prop;
    writerBuilder["indentation"] = EMPTY_STRING; // optional?
    prop["serviceInstanceIndex"] = index;
    std::string properties = Json::writeString(writerBuilder, prop);
    mOrbBrowser->dispatchEvent("ServiceInstanceChanged", properties);
}

void BroadcastInterface::DispatchParentalRatingChangeEvent(const bool blocked)
{
    LOGI("DispatchParentalRatingChangeEvent => blocked: " << blocked);
    Json::StreamWriterBuilder writerBuilder;
    Json::Value prop;
    writerBuilder["indentation"] = EMPTY_STRING; // optional?
    prop["blocked"] = blocked;
    std::string properties = Json::writeString(writerBuilder, prop);
    mOrbBrowser->dispatchEvent("ParentalRatingChange", properties);
}

void BroadcastInterface::DispatchParentalRatingErrorEvent(std::string contentID,
    std::vector<ParentalRating> ratings, std::string DRMSystemID)
{
    LOGI("DispatchParentalRatingErrorEvent => contentID: " << contentID);
    Json::StreamWriterBuilder writerBuilder;
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
    writerBuilder["indentation"] = EMPTY_STRING; // optional?
    prop["contentID"] = contentID.c_str();
    prop["ratings"] = ratingsArray;
    prop["DRMSystemID"] = DRMSystemID.c_str();
    std::string properties = Json::writeString(writerBuilder, prop);
    mOrbBrowser->dispatchEvent("ParentalRatingError", properties);
}

void BroadcastInterface::DispatchSelectedComponentChangedEvent(const int componentType)
{
    LOGI("DispatchSelectedComponentChangedEvent => componentType: " << componentType);
    Json::StreamWriterBuilder writerBuilder;
    Json::Value prop;
    writerBuilder["indentation"] = EMPTY_STRING; // optional?
    prop["componentType"] = componentType;
    std::string properties = Json::writeString(writerBuilder, prop);
    mOrbBrowser->dispatchEvent("ComponentChanged", properties);
}

void BroadcastInterface::DispatchStreamEvent(const int id, const std::string name, const std::string data,
    const std::string text, const std::string status, const DASHEvent dashEvent)
{
    LOGI("DispatchStreamEvent => id: " << id << ", name: " << name << ", data: " << data << ", text: "
        << text << ", status: " << status);
    Json::StreamWriterBuilder writerBuilder;
    Json::Value prop;
    
    Json::Value dash;
    dash["id"] = dashEvent.id;
    dash["startTime"] = dashEvent.startTime;
    dash["duration"] = dashEvent.duration;
    dash["contentEncoding"] = dashEvent.contentEncoding;
        
    writerBuilder["indentation"] = EMPTY_STRING; // optional?
    prop["id"] = id;
    prop["name"] = name.c_str();
    prop["data"] = data.c_str();
    prop["text"] = text.c_str();
    prop["status"] = status.c_str();
    prop["DASHEvent"] = dash;
    std::string properties = Json::writeString(writerBuilder, prop);
    mOrbBrowser->dispatchEvent("ParentalRatingError", properties);
}

void BroadcastInterface::DispatchProgrammesChangedEvent()
{
    LOGI("dispatchProgrammesChangedEvent");
    mOrbBrowser->dispatchEvent("ProgrammesChanged", "{}");
}

} // namespace orb
