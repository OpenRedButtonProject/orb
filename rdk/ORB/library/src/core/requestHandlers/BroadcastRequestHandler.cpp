/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

#include "BroadcastRequestHandler.h"
#include "Channel.h"
#include "ORBPlatform.h"
#include "ORBEngine.h"
#include "ORBLogging.h"
#include "JsonUtil.h"

#define BROADCAST_SET_VIDEO_RECTANGLE "setVideoRectangle"
#define BROADCAST_GET_CURRENT_CHANNEL "getCurrentChannel"
#define BROADCAST_GET_CURRENT_CHANNEL_FOR_EVENT "getCurrentChannelForEvent"
#define BROADCAST_GET_CHANNEL_LIST "getChannelList"
#define BROADCAST_SET_CHANNEL_TO_CCID "setChannelToCcid"
#define BROADCAST_SET_CHANNEL_TO_NULL "setChannelToNull"
#define BROADCAST_SET_CHANNEL_TO_TRIPLET "setChannelToTriplet"
#define BROADCAST_SET_CHANNEL_TO_DSD "setChannelToDsd"
#define BROADCAST_GET_PROGRAMMES "getProgrammes"
#define BROADCAST_GET_COMPONENTS "getComponents"
#define BROADCAST_GET_PRIVATE_AUDIO_COMPONENT "getPrivateAudioComponent"
#define BROADCAST_GET_PRIVATE_VIDEO_COMPONENT "getPrivateVideoComponent"
#define BROADCAST_OVERRIDE_COMPONENT_SELECTION "overrideComponentSelection"
#define BROADCAST_RESTORE_COMPONENT_SELECTION "restoreComponentSelection"
#define BROADCAST_START_SEARCH "startSearch"
#define BROADCAST_ABORT_SEARCH "abortSearch"
#define BROADCAST_ADD_STREAM_EVENT_LISTENER "addStreamEventListener"
#define BROADCAST_REMOVE_STREAM_EVENT_LISTENER "removeStreamEventListener"
#define BROADCAST_SET_PRESENTATION_SUSPENDED "setPresentationSuspended"

namespace orb {
/**
 * Constructor.
 */
BroadcastRequestHandler::BroadcastRequestHandler()
{
}

/**
 * Destructor.
 */
BroadcastRequestHandler::~BroadcastRequestHandler()
{
}

/**
 * @brief BroadcastRequestHandler::Handle
 *
 * Handles the given Broadcast request.
 *
 * @param token    (in)  The request token
 * @param method   (in)  The requested method
 * @param params   (in)  A JSON object containing the input parameters (if any)
 * @param response (out) A JSON object containing the response
 *
 * @return true in success, otherwise false
 */
bool BroadcastRequestHandler::Handle(
   json token,
   std::string method,
   json params,
   json& response)
{
   bool ret = true;
   response = "{}"_json;

   // Broadcast.setVideoRectangle
   if (method == BROADCAST_SET_VIDEO_RECTANGLE)
   {
      int x = params.value("x", 0);
      int y = params.value("y", 0);
      int w = params.value("width", 0);
      int h = params.value("height", 0);
      ORBEngine::GetSharedInstance().GetORBPlatform()->Broadcast_SetVideoRectangle(x, y, w, h);
   }
   // Broadcast.getCurrentChannel
   else if (method == BROADCAST_GET_CURRENT_CHANNEL)
   {
      if (!IsRequestAllowed(token, ApplicationManager::MethodRequirement::FOR_BROADCAST_APP_ONLY))
      {
         response = MakeErrorResponse("SecurityError");
      }
      else
      {
         std::shared_ptr<Channel> currentChannel =
            ORBEngine::GetSharedInstance().GetORBPlatform()->Broadcast_GetCurrentChannel();

         if (currentChannel->GetCcid().empty())
         {
            response = MakeErrorResponse("Current channel unknown");
         }
         else
         {
            response["result"] = JsonUtil::ChannelToJsonObject(*(currentChannel.get()));
         }
      }
   }
   // Broadcast.getCurrentChannelForEvent
   else if (method == BROADCAST_GET_CURRENT_CHANNEL_FOR_EVENT)
   {
      if (!IsRequestAllowed(token, ApplicationManager::MethodRequirement::FOR_BROADCAST_OR_TRANSITIONING_APP_ONLY))
      {
         response = MakeErrorResponse("SecurityError");
      }
      else
      {
         std::shared_ptr<Channel> currentChannel =
            ORBEngine::GetSharedInstance().GetORBPlatform()->Broadcast_GetCurrentChannel();

         response["result"] = JsonUtil::ChannelToJsonObject(*(currentChannel.get()));
      }
   }
   // Broadcast.getChannelList
   else if (method == BROADCAST_GET_CHANNEL_LIST)
   {
      if (!IsRequestAllowed(token, ApplicationManager::MethodRequirement::FOR_BROADCAST_APP_ONLY))
      {
         response = MakeErrorResponse("SecurityError");
      }
      else
      {
         std::vector<Channel> channelList = ORBEngine::GetSharedInstance().GetORBPlatform()->Broadcast_GetChannelList();
         json array;
         for (auto channel : channelList)
         {
            array.push_back(JsonUtil::ChannelToJsonObject(channel));
         }
         response.emplace("result", array);
      }
   }
   // Broadcast.setChannelToCcid
   else if (method == BROADCAST_SET_CHANNEL_TO_CCID)
   {
      if (!IsRequestAllowed(token, ApplicationManager::MethodRequirement::FOR_RUNNING_APP_ONLY))
      {
         response = MakeErrorResponse("NotRunning");
      }
      else
      {
         std::string ccid = params.value("ccid", "");
         bool trickPlay = params.value("trickplay", false);
         std::string contentAccessDescriptorURL = params.value("contentAccessDescriptorURL", "");
         bool quiet = params.value("quiet", 0);
         Channel::ErrorState errorState = Channel::ErrorState::CHANNEL_ERROR_STATE_UNKNOWN_ERROR;

         bool success =
            ORBEngine::GetSharedInstance().GetORBPlatform()->
            Broadcast_SetChannelToCcid(ccid, trickPlay, contentAccessDescriptorURL, quiet, &errorState);

         response["success"] = success;
         if (!success)
         {
            response["errorState"] = errorState;
         }
      }
   }
   // Broadcast.setChannelToNull
   else if (method == BROADCAST_SET_CHANNEL_TO_NULL)
   {
      if (!IsRequestAllowed(token, ApplicationManager::MethodRequirement::FOR_RUNNING_APP_ONLY))
      {
         response = MakeErrorResponse("NotRunning");
      }
      else
      {
         bool trickPlay = params.value("trickplay", false);
         std::string contentAccessDescriptorURL = params.value("contentAccessDescriptorURL", "");
         bool quiet = params.value("quiet", 0);
         Channel::ErrorState errorState = Channel::ErrorState::CHANNEL_ERROR_STATE_UNKNOWN_ERROR;

         bool success =
            ORBEngine::GetSharedInstance().GetORBPlatform()->
            Broadcast_SetChannelToNull(trickPlay, contentAccessDescriptorURL, quiet, &errorState);

         response["success"] = success;
         if (!success)
         {
            response["errorState"] = errorState;
         }
      }
   }
   // Broadcast.setChannelToTriplet
   else if (method == BROADCAST_SET_CHANNEL_TO_TRIPLET)
   {
      if (!IsRequestAllowed(token, ApplicationManager::MethodRequirement::FOR_RUNNING_APP_ONLY))
      {
         response = MakeErrorResponse("NotRunning");
      }
      else
      {
         int idType = params.value("idType", -1);
         int onid = params.value("onid", -1);
         int tsid = params.value("tsid", -1);
         int sid = params.value("sid", -1);
         int sourceID = params.value("sourceID", -1);
         std::string ipBroadcastID = params.value("ipBroadcastID", "");
         bool trickPlay = params.value("trickplay", false);
         std::string contentAccessDescriptorURL = params.value("contentAccessDescriptorURL", "");
         bool quiet = params.value("quiet", 0);
         Channel::ErrorState errorState = Channel::ErrorState::CHANNEL_ERROR_STATE_UNKNOWN_ERROR;

         bool success =
            ORBEngine::GetSharedInstance().GetORBPlatform()->
            Broadcast_SetChannelToTriplet(
               idType,
               onid,
               tsid,
               sid,
               sourceID,
               ipBroadcastID,
               trickPlay,
               contentAccessDescriptorURL,
               quiet,
               &errorState
               );

         response["success"] = success;
         if (!success)
         {
            response["errorState"] = errorState;
         }
      }
   }
   // Broadcast.setChannelToDsd
   else if (method == BROADCAST_SET_CHANNEL_TO_DSD)
   {
      if (!IsRequestAllowed(token, ApplicationManager::MethodRequirement::FOR_RUNNING_APP_ONLY))
      {
         response = MakeErrorResponse("NotRunning");
      }
      else
      {
         std::string dsd = params.value("dsd", "");
         int sid = params.value("sid", -1);
         bool trickPlay = params.value("trickplay", false);
         std::string contentAccessDescriptorURL = params.value("contentAccessDescriptorURL", "");
         bool quiet = params.value("quiet", 0);
         Channel::ErrorState errorState = Channel::ErrorState::CHANNEL_ERROR_STATE_UNKNOWN_ERROR;

         bool success = ORBEngine::GetSharedInstance().GetORBPlatform()->Broadcast_SetChannelToDsd(dsd, sid, trickPlay, contentAccessDescriptorURL, quiet, &errorState);
         response["success"] = success;
         if (!success)
         {
            response["errorState"] = errorState;
         }
      }
   }
   // Broadcast.getProgrammes
   else if (method == BROADCAST_GET_PROGRAMMES)
   {
      if (!IsRequestAllowed(token, ApplicationManager::MethodRequirement::FOR_BROADCAST_APP_ONLY))
      {
         response = MakeErrorResponse("SecurityError");
      }
      else
      {
         std::string ccid = params.value("ccid", "");
         std::vector<Programme> programmes = ORBEngine::GetSharedInstance().GetORBPlatform()->Broadcast_GetProgrammes(ccid);
         json array;
         for (auto programme : programmes)
         {
            array.push_back(JsonUtil::ProgrammeToJsonObject(programme));
         }
         response.emplace("result", array);
      }
   }
   // Broadcast.getComponents
   else if (method == BROADCAST_GET_COMPONENTS)
   {
      if (!IsRequestAllowed(token, ApplicationManager::MethodRequirement::FOR_BROADCAST_APP_ONLY))
      {
         response = MakeErrorResponse("SecurityError");
      }
      else
      {
         std::string ccid = params.value("ccid", "");

         int componentType = params.value("typeCode", COMPONENT_TYPE_ANY);

         std::vector<Component> components = ORBEngine::GetSharedInstance().GetORBPlatform()->Broadcast_GetComponents(ccid, componentType);
         json array;
         for (auto component : components)
         {
            array.push_back(JsonUtil::ComponentToJsonObject(component));
         }
         response.emplace("result", array);
      }
   }
   // Broadcast.getPrivateAudioComponent
   else if (method == BROADCAST_GET_PRIVATE_AUDIO_COMPONENT)
   {
      if (!IsRequestAllowed(token, ApplicationManager::MethodRequirement::FOR_BROADCAST_APP_ONLY))
      {
         response = MakeErrorResponse("SecurityError");
      }
      else
      {
         std::string componentTag = params.value("componentTag", "");
         std::shared_ptr<Component> component = ORBEngine::GetSharedInstance().GetORBPlatform()->Broadcast_GetPrivateAudioComponent(componentTag);
         if (component != nullptr)
         {
            response.emplace("result", JsonUtil::ComponentToJsonObject(*(component.get())));
         }
         else
         {
            response.emplace("result", "{}_json");
         }
      }
   }
   // Broadcast.getPrivateVideoComponent
   else if (method == BROADCAST_GET_PRIVATE_VIDEO_COMPONENT)
   {
      if (!IsRequestAllowed(token, ApplicationManager::MethodRequirement::FOR_BROADCAST_APP_ONLY))
      {
         response = MakeErrorResponse("SecurityError");
      }
      else
      {
         std::string componentTag = params.value("componentTag", "");
         std::shared_ptr<Component> component = ORBEngine::GetSharedInstance().GetORBPlatform()->Broadcast_GetPrivateVideoComponent(componentTag);
         if (component != nullptr)
         {
            response.emplace("result", JsonUtil::ComponentToJsonObject(*(component.get())));
         }
         else
         {
            response.emplace("result", "{}_json");
         }
      }
   }
   // Broadcast.overrideComponentSelection
   else if (method == BROADCAST_OVERRIDE_COMPONENT_SELECTION)
   {
      if (!IsRequestAllowed(token, ApplicationManager::MethodRequirement::FOR_BROADCAST_APP_ONLY))
      {
         response = MakeErrorResponse("SecurityError");
      }
      else
      {
         int componentType = params.value("type", COMPONENT_TYPE_ANY);
         std::string id = params.value("id", "");

         ORBEngine::GetSharedInstance().GetORBPlatform()->Broadcast_OverrideComponentSelection(
            componentType, id);
      }
   }
   // Broadcast.restoreComponentSelection
   else if (method == BROADCAST_RESTORE_COMPONENT_SELECTION)
   {
      if (!IsRequestAllowed(token, ApplicationManager::MethodRequirement::FOR_BROADCAST_APP_ONLY))
      {
         response = MakeErrorResponse("SecurityError");
      }
      else
      {
         int componentType = params.value("type", -1);
         ORBEngine::GetSharedInstance().GetORBPlatform()->Broadcast_RestoreComponentSelection(componentType);
      }
   }
   // Broadcast.startSearch
   else if (method == BROADCAST_START_SEARCH)
   {
      if (!IsRequestAllowed(token, ApplicationManager::MethodRequirement::FOR_BROADCAST_APP_ONLY))
      {
         response = MakeErrorResponse("SecurityError");
      }
      else
      {
         std::string queryAsString = "{}";
         if (params.contains("query") && params["query"].is_object())
         {
            queryAsString = params["query"].dump();
         }
         std::shared_ptr<Query> query = std::make_shared<Query>(queryAsString);
         int offset = params.value("offset", -1);
         int count = params.value("count", -1);
         json array = params["channelConstraints"];
         std::vector<std::string> channelConstraints;
         for (int i = 0; i < array.size(); i++)
         {
            channelConstraints.push_back(array[i]);
         }
         // cancel existing search task (if any) before starting a new one
         CancelSearch(query->GetQueryId());
         std::shared_ptr<MetadataSearchTask> searchTask = std::make_shared<MetadataSearchTask>(query, offset, count, channelConstraints);
         ORBEngine::GetSharedInstance().AddMetadataSearchTask(query->GetQueryId(), searchTask);
         searchTask->Start();
      }
   }
   // Broadcast.abortSearch
   else if (method == BROADCAST_ABORT_SEARCH)
   {
      if (!IsRequestAllowed(token, ApplicationManager::MethodRequirement::FOR_BROADCAST_APP_ONLY))
      {
         response = MakeErrorResponse("SecurityError");
      }
      else
      {
         int queryId = params.value("queryId", 0);
         CancelSearch(queryId);
         std::vector<std::string> searchResults;
         MetadataSearchTask::OnMetadataSearchCompleted(queryId, SEARCH_STATUS_ABORTED, searchResults);
      }
   }
   // Broadcast.addStreamEventListener
   else if (method == BROADCAST_ADD_STREAM_EVENT_LISTENER)
   {
      std::string targetUrl = params.value("targetURL", "");
      std::string eventName = params.value("eventName", "");
      int componentTag;
      if (params.contains("componentTag") && params["componentTag"].is_string())
      {
         componentTag = std::stoi(params.value("componentTag", "-1"));
      }
      else
      {
         componentTag = params.value("componentTag", -1);
      }

      int streamEventId;
      if (params.contains("streamEventId") && params["streamEventId"].is_string())
      {
         streamEventId = std::stoi(params.value("streamEventId", "-1"));
      }
      else
      {
         streamEventId = params.value("streamEventId", -1);
      }

      int id = AddStreamEventListener(targetUrl, eventName, componentTag, streamEventId);
      json resultObj;
      resultObj["result"] = id;
      response = resultObj;
   }
   // Broadcast.removeStreamEventListener
   else if (method == BROADCAST_REMOVE_STREAM_EVENT_LISTENER)
   {
      int id = params.value("id", -1);
      RemoveStreamEventListener(id);
   }
   // Broadcast.setPresentationSuspended
   else if (method == BROADCAST_SET_PRESENTATION_SUSPENDED)
   {
      if (!IsRequestAllowed(token, ApplicationManager::MethodRequirement::FOR_BROADCAST_APP_ONLY))
      {
         response = MakeErrorResponse("SecurityError");
      }
      else
      {
         bool presentationSuspended = params.value("presentationSuspended", false);
         ORBEngine::GetSharedInstance().GetORBPlatform()->Broadcast_SetPresentationSuspended(presentationSuspended);
      }
   }
   // UnknownMethod
   else
   {
      response = RequestHandler::MakeErrorResponse("UnknownMethod");
      ret = false;
   }

   return ret;
}

/**
 * Add a listener for the specified DSM-CC stream event.
 *
 * @param targetUrl     The URL of the DSM-CC StreamEvent object or an HTTP or HTTPS
 *                      URL referring to an XML event description file
 * @param eventName     The name of the event (of the DSM-CC StreamEvent object) that shall
 *                      be subscribed to
 * @param componentTag  The component tag
 * @param streamEventId The StreamEvent id
 *
 * @return The listener id or -1
 */
int BroadcastRequestHandler::AddStreamEventListener(
   std::string targetUrl,
   std::string eventName,
   int componentTag,
   int streamEventId
   )
{
   int ret = -1;
   static int subscriberId = 0;
   subscriberId++;

   ORB_LOG("targetUrl=%s eventName=%s componentTag=%d streamEventId=%d",
      targetUrl.c_str(),
      eventName.c_str(),
      componentTag,
      streamEventId
      );

   bool result = false;

   if (targetUrl.rfind("dvb:", 0) == 0)
   {
      result = ORBEngine::GetSharedInstance().GetORBPlatform()->Dsmcc_SubscribeToStreamEventByName(
         targetUrl, eventName, subscriberId);
   }
   else
   {
      result = ORBEngine::GetSharedInstance().GetORBPlatform()->Dsmcc_SubscribeStreamEventId(
         eventName, componentTag, streamEventId, subscriberId);
   }

   if (result)
   {
      ret = subscriberId;
   }

   return ret;
}

/**
 * Remove the specified DSM-CC stream event listener.
 *
 * @param id The listener id
 */
void BroadcastRequestHandler::RemoveStreamEventListener(int id)
{
   ORB_LOG("id=%d", id);
   ORBEngine::GetSharedInstance().GetORBPlatform()->Dsmcc_UnsubscribeFromStreamEvents(id);
}

/**
 * @brief BroadcastRequestHandler::IsRequestAllowed
 *
 * Check if the given request is allowed.
 *
 * @param token      The request token
 * @param methodType The requested method type
 *
 * @return true if allowed, false otherwise
 */
bool BroadcastRequestHandler::IsRequestAllowed(json token, ApplicationManager::MethodRequirement methodType)
{
   json payload = token["payload"];
   ORB_LOG("payload=%s", payload.dump());

   int appId = payload.value("appId", 0);
   std::string uri = payload.value("uri", "");

   return ORBEngine::GetSharedInstance().GetApplicationManager()->IsRequestAllowed(appId, uri, methodType);
}

/**
 * @brief BroadcastRequestHandler::CancelSearch
 *
 * Cancel the metadata search task corresponding to the given query id, if such task exists
 *
 * @param queryId The query id
 */
void BroadcastRequestHandler::CancelSearch(int queryId)
{
   std::shared_ptr<MetadataSearchTask> searchTask = ORBEngine::GetSharedInstance().GetMetadataSearchTask(queryId);
   if (searchTask)
   {
      ORB_LOG("Aborting existing search task");
      searchTask->Stop();
      ORBEngine::GetSharedInstance().RemoveMetadataSearchTask(queryId);
   }
}
} // namespace orb
