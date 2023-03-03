/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */
#include "DrmRequestHandler.h"
#include "ORBEngine.h"
#include "JsonUtil.h"

#define DRM_GET_SUPPORTED_DRM_SYSTEM_IDS "getSupportedDRMSystemIDs"
#define DRM_SEND_DRM_MESSAGE "sendDRMMessage"
#define DRM_CAN_PLAY_CONTENT "canPlayContent"
#define DRM_CAN_RECORD_CONTENT "canRecordContent"
#define DRM_SET_ACTIVE_DRM "setActiveDRM"

namespace orb
{
/**
 * Constructor.
 */
DrmRequestHandler::DrmRequestHandler()
{
}

/**
 * Destructor.
 */
DrmRequestHandler::~DrmRequestHandler()
{
}

/**
 * @brief DrmRequestHandler::Handle
 *
 * Handles the given Drm request.
 *
 * @param token    (in)  The request token
 * @param method   (in)  The requested method
 * @param params   (in)  A JSON object containing the input parameters (if any)
 * @param response (out) A JSON object containing the response
 *
 * @return true in success, otherwise false
 */
bool DrmRequestHandler::Handle(
   json token,
   std::string method,
   json params,
   json& response)
{
   ORBPlatform *orbPlatform = ORBEngine::GetSharedInstance().GetORBPlatform();

   bool ret = true;
   response = "{}"_json;

   // Drm.getSupportedDRMSystemIDs
   if (method == DRM_GET_SUPPORTED_DRM_SYSTEM_IDS)
   {
      std::vector<DrmSystemStatus> result = orbPlatform->Drm_GetSupportedDrmSystemIds();
      json json_result = json::array();
      for (DrmSystemStatus drmSystemStatus : result)
      {
         json_result.push_back(JsonUtil::DrmSystemStatusToJsonObject(drmSystemStatus));
      }
      response.emplace("result", json_result);
   }
   // Drm.sendDRMMessage
   else if (method == DRM_SEND_DRM_MESSAGE)
   {
      std::string messageId = params.value("msgID", "");
      std::string messageType = params.value("msgType", "");
      std::string message = params.value("msg", "");
      std::string drmSystemId = params.value("DRMSystemID", "");
      bool blocked = params.value("blocked", false);
      std::string result = orbPlatform->Drm_SendDrmMessage(messageId, messageType, message, drmSystemId, blocked);
      response.emplace("result", result);
   }
   // Drm.canPlayContent
   else if (method == DRM_CAN_PLAY_CONTENT)
   {
      std::string drmPrivateData = params.value("DRMPrivateData", "");
      std::string drmSystemId = params.value("DRMSystemID", "");
      bool result = orbPlatform->Drm_CanPlayContent(drmPrivateData, drmSystemId);
      response.emplace("result", result);
   }
   // Drm.canRecordContent
   else if (method == DRM_CAN_RECORD_CONTENT)
   {
      std::string drmPrivateData = params.value("DRMPrivateData", "");
      std::string drmSystemId = params.value("DRMSystemID", "");
      bool result = orbPlatform->Drm_CanRecordContent(drmPrivateData, drmSystemId);
      response.emplace("result", result);
   }
   // Drm.setActiveDRM
   else if (method == DRM_SET_ACTIVE_DRM)
   {
      std::string drmSystemId = params.value("DRMSystemID", "");
      bool result = orbPlatform->Drm_SetActiveDrm(drmSystemId);
      response.emplace("result", result);
   }
   // UnknownMethod
   else
   {
      response = ORBBridgeRequestHandler::MakeErrorResponse("UnknownMethod");
      ret = false;
   }

   orbPlatform = nullptr;
   return ret;
}
} // namespace orb