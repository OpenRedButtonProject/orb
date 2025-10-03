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


std::string Drm::executeRequest(const std::string& method, const std::string& token, const IJson& params)
{
    LOGI("Drm executeRequest method: " << method);

    if (method == DRM_GET_SUPPORTED_DRM_SYSTEM_IDS)
    {
        return handleGetSupportedDRMSystemIDs();
    }
    else if (method == DRM_SEND_DRM_MESSAGE)
    {
        return handleSendDRMMessage(std::move(params));
    }
    else if (method == DRM_CAN_PLAY_CONTENT)
    {
        return handleCanPlayContent(std::move(params));
    }
    else if (method == DRM_CAN_RECORD_CONTENT)
    {
        return handleCanRecordContent(std::move(params));
    }
    else if (method == DRM_SET_ACTIVE_DRM)
    {
        return handleSetActiveDRM(std::move(params));
    }
    else
    {
        return "{\"error\": \"Drm request [" + method + "] invalid method\"}";
    }
}

std::string Drm::handleGetSupportedDRMSystemIDs()
{
    LOGI("Drm handleGetSupportedDRMSystemIDs");

    std::unique_ptr<IJson> json = IJson::create();
    std::vector<int> drmSystems;
    // Mock implementation - return empty array for now
    // In a real implementation, this would query the platform for supported DRM systems
    json->setArray(DRM_RESULT, drmSystems);

    return json->toString();
}

std::string Drm::handleSendDRMMessage(const IJson& params)
{
    LOGI("Drm handleSendDRMMessage");

    std::unique_ptr<IJson> json = IJson::create();

    // Extract parameters
    string msgID = params.getString("msgID");
    string msgType = params.getString("msgType");
    string msg = params.getString("msg");
    string drmSystemID = params.getString(DRM_SYSTEM_ID);
    bool block = params.hasParam("block", IJson::JSON_TYPE_BOOLEAN) && params.getBool("block");

    LOGI("Drm sendDRMMessage - msgID: " << msgID << ", msgType: " << msgType
         << ", DRMSystemID: " << drmSystemID << ", block: " << (block ? "true" : "false"));

    // Mock implementation - return empty result for now
    // In a real implementation, this would send the message to the DRM system
    json->setString(DRM_RESULT, string(""));

    return json->toString();
}

std::string Drm::handleCanPlayContent(const IJson& params)
{
    LOGI("Drm handleCanPlayContent");

    std::unique_ptr<IJson> json = IJson::create();

    // Extract parameters
    string drmPrivateData = params.getString(DRM_PRIVATE_DATA);
    string drmSystemID = params.getString(DRM_SYSTEM_ID);

    LOGI("Drm canPlayContent - DRMSystemID: " << drmSystemID);

    // Mock implementation - return false for now
    // In a real implementation, this would check if the content can be played
    json->setBool(DRM_RESULT, false);

    return json->toString();
}

std::string Drm::handleCanRecordContent(const IJson& params)
{
    LOGI("Drm handleCanRecordContent");

    std::unique_ptr<IJson> json = IJson::create();

    // Extract parameters
    string drmPrivateData = params.getString(DRM_PRIVATE_DATA);
    string drmSystemID = params.getString(DRM_SYSTEM_ID);

    LOGI("Drm canRecordContent - DRMSystemID: " << drmSystemID);

    // Mock implementation - return false for now
    // In a real implementation, this would check if the content can be recorded
    json->setBool(DRM_RESULT, false);

    return json->toString();
}

std::string Drm::handleSetActiveDRM(const IJson& params)
{
    LOGI("Drm handleSetActiveDRM");

    std::unique_ptr<IJson> json = IJson::create();

    // Extract parameters
    string drmSystemID = params.getString(DRM_SYSTEM_ID);

    LOGI("Drm setActiveDRM - DRMSystemID: " << drmSystemID);

    // Mock implementation - return false for now
    // In a real implementation, this would set the active DRM system
    json->setBool(DRM_RESULT, false);

    return json->toString();
}

} // namespace orb
