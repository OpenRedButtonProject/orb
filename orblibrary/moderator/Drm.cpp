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
 */

#include "Drm.hpp"
#include "log.h"
#include "JsonUtil.h"
#include <sstream>

using namespace std;

namespace orb
{

// Method name constants
const string DRM_GET_SUPPORTED_DRM_SYSTEM_IDS = "getSupportedDRMSystemIDs";
const string DRM_SEND_DRM_MESSAGE = "sendDRMMessage";
const string DRM_CAN_PLAY_CONTENT = "canPlayContent";
const string DRM_CAN_RECORD_CONTENT = "canRecordContent";
const string DRM_SET_ACTIVE_DRM = "setActiveDRM";

// Parameter name constants
const string DRM_RESULT = "result";
const string DRM_SYSTEM_ID = "DRMSystemID";
const string DRM_PRIVATE_DATA = "DRMPrivateData";


std::string Drm::executeRequest(std::string method, std::string token, std::unique_ptr<IJson> params)
{
    LOGI("Drm executeRequest method: " << method);
    // convert params to Json::Value, so we do not change other code.
    // This is workaround for now, we need to change the code to use IJson instead of Json::Value
    // in separate Ticket.

    Json::Value paramsJson;
    if (!JsonUtil::decodeJson(params->toString(), &paramsJson)) {
        LOGE("Drm executeRequest: Invalid params");
        return "{\"error\": \"Invalid params\"}";
    }

    Json::Value response(Json::objectValue);

    if (method == DRM_GET_SUPPORTED_DRM_SYSTEM_IDS)
    {
        response = handleGetSupportedDRMSystemIDs();
    }
    else if (method == DRM_SEND_DRM_MESSAGE)
    {
        response = handleSendDRMMessage(paramsJson);
    }
    else if (method == DRM_CAN_PLAY_CONTENT)
    {
        response = handleCanPlayContent(paramsJson);
    }
    else if (method == DRM_CAN_RECORD_CONTENT)
    {
        response = handleCanRecordContent(paramsJson);
    }
    else if (method == DRM_SET_ACTIVE_DRM)
    {
        response = handleSetActiveDRM(paramsJson);
    }
    else
    {
        response["error"] = "Drm request [" + method + "] invalid method";
    }

    // Convert response to JSON string
    return JsonUtil::convertJsonToString(response);
}

Json::Value Drm::handleGetSupportedDRMSystemIDs()
{
    LOGI("Drm handleGetSupportedDRMSystemIDs");

    Json::Value response(Json::objectValue);
    Json::Value drmSystems(Json::arrayValue);
    // Mock implementation - return empty array for now
    // In a real implementation, this would query the platform for supported DRM systems
    response[DRM_RESULT] = drmSystems;

    return response;
}

Json::Value Drm::handleSendDRMMessage(const Json::Value& params)
{
    LOGI("Drm handleSendDRMMessage");

    Json::Value response;

    // Extract parameters
    string msgID = params.get("msgID", "").asString();
    string msgType = params.get("msgType", "").asString();
    string msg = params.get("msg", "").asString();
    string drmSystemID = params.get(DRM_SYSTEM_ID, "").asString();
    bool block = params.get("block", false).asBool();

    LOGI("Drm sendDRMMessage - msgID: " << msgID << ", msgType: " << msgType
         << ", DRMSystemID: " << drmSystemID << ", block: " << (block ? "true" : "false"));

    // Mock implementation - return empty result for now
    // In a real implementation, this would send the message to the DRM system
    response[DRM_RESULT] = "";

    return response;
}

Json::Value Drm::handleCanPlayContent(const Json::Value& params)
{
    LOGI("Drm handleCanPlayContent");

    Json::Value response(Json::objectValue);

    // Extract parameters
    string drmPrivateData = params.get(DRM_PRIVATE_DATA, "").asString();
    string drmSystemID = params.get(DRM_SYSTEM_ID, "").asString();

    LOGI("Drm canPlayContent - DRMSystemID: " << drmSystemID);

    // Mock implementation - return false for now
    // In a real implementation, this would check if the content can be played
    response[DRM_RESULT] = false;

    return response;
}

Json::Value Drm::handleCanRecordContent(const Json::Value& params)
{
    LOGI("Drm handleCanRecordContent");

    Json::Value response(Json::objectValue);

    // Extract parameters
    string drmPrivateData = params.get(DRM_PRIVATE_DATA, "").asString();
    string drmSystemID = params.get(DRM_SYSTEM_ID, "").asString();

    LOGI("Drm canRecordContent - DRMSystemID: " << drmSystemID);

    // Mock implementation - return false for now
    // In a real implementation, this would check if the content can be recorded
    response[DRM_RESULT] = false;

    return response;
}

Json::Value Drm::handleSetActiveDRM(const Json::Value& params)
{
    LOGI("Drm handleSetActiveDRM");

    Json::Value response(Json::objectValue);

    // Extract parameters
    string drmSystemID = params.get(DRM_SYSTEM_ID, "").asString();

    LOGI("Drm setActiveDRM - DRMSystemID: " << drmSystemID);

    // Mock implementation - return false for now
    // In a real implementation, this would set the active DRM system
    response[DRM_RESULT] = false;

    return response;
}

} // namespace orb
