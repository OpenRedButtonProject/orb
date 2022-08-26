/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

#include "BroadcastRequestHandler.h"
#include "Channel.h"
#include "ORBEvents.h"
#include "ORBPlatform.h"
#include "ORB.h"

using namespace WPEFramework::Plugin;

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
#define BROADCAST_SELECT_COMPONENT "selectComponent"
#define BROADCAST_UNSELECT_COMPONENT "unselectComponent"
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
    JsonObject token,
    std::string method,
    JsonObject params,
    JsonObject& response)
{
    bool ret = true;

    // Broadcast.setVideoRectangle
    if (method == BROADCAST_SET_VIDEO_RECTANGLE)
    {
        int x = params["x"].Number();
        int y = params["y"].Number();
        int w = params["width"].Number();
        int h = params["height"].Number();
        ORB::instance(nullptr)->GetORBPlatform()->Broadcast_SetVideoRectangle(x, y, w, h);
        response.FromString("{}");
    }
    // Broadcast.getCurrentChannel
    else if (method == BROADCAST_GET_CURRENT_CHANNEL)
    {
        if (!IsRequestAllowed(token, ApplicationManager::MethodRequirement::FOR_BROADCAST_APP_ONLY))
        {
            response = MakeErrorResponse("SecurityError");
            return ret;
        }
        std::shared_ptr<Channel> currentChannel = ORB::instance(
            nullptr)->GetORBPlatform()->Broadcast_GetCurrentChannel();
        if (currentChannel->GetCcid().empty())
        {
            response = MakeErrorResponse("Current channel unknown");
            return ret;
        }
        response.Set("result", currentChannel->ToJsonObject());
    }
    // Broadcast.getCurrentChannelForEvent
    else if (method == BROADCAST_GET_CURRENT_CHANNEL_FOR_EVENT)
    {
        if (!IsRequestAllowed(token,
            ApplicationManager::MethodRequirement::FOR_BROADCAST_OR_TRANSITIONING_APP_ONLY))
        {
            response = MakeErrorResponse("SecurityError");
            return ret;
        }
        std::shared_ptr<Channel> currentChannel = ORB::instance(
            nullptr)->GetORBPlatform()->Broadcast_GetCurrentChannel();
        response.Set("result", currentChannel->ToJsonObject());
    }
    // Broadcast.getChannelList
    else if (method == BROADCAST_GET_CHANNEL_LIST)
    {
        if (!IsRequestAllowed(token, ApplicationManager::MethodRequirement::FOR_BROADCAST_APP_ONLY))
        {
            response = MakeErrorResponse("SecurityError");
            return ret;
        }
        std::vector<Channel> channelList = ORB::instance(
            nullptr)->GetORBPlatform()->Broadcast_GetChannelList();
        ArrayType<JsonValue> array;
        for (auto channel : channelList)
        {
            array.Add(channel.ToJsonObject());
        }
        JsonValue jsonChannelList;
        jsonChannelList.Array(array);
        response.Set("result", jsonChannelList);
    }
    // Broadcast.setChannelToCcid
    else if (method == BROADCAST_SET_CHANNEL_TO_CCID)
    {
        if (!IsRequestAllowed(token, ApplicationManager::MethodRequirement::FOR_BROADCAST_APP_ONLY))
        {
            response = MakeErrorResponse("NotRunning");
            return ret;
        }

        std::string ccid = params["ccid"].String();
        bool trickPlay = params["trickplay"].Boolean();
        std::string contentAccessDescriptorURL = params["contentAccessDescriptorURL"].String();
        bool quiet = params["quiet"].Boolean();
        int channelChangeError = -1;

        bool success = ORB::instance(nullptr)->GetORBPlatform()->Broadcast_SetChannelToCcid(ccid,
            trickPlay, contentAccessDescriptorURL, quiet, &channelChangeError);
        response["success"] = success;
        if (!success)
        {
            response["errorState"] = channelChangeError;
        }

        return ret;
    }
    // Broadcast.setChannelToNull
    else if (method == BROADCAST_SET_CHANNEL_TO_NULL)
    {
        if (!IsRequestAllowed(token, ApplicationManager::MethodRequirement::FOR_BROADCAST_APP_ONLY))
        {
            response = MakeErrorResponse("NotRunning");
            return ret;
        }

        bool trickPlay = params["trickplay"].Boolean();
        std::string contentAccessDescriptorURL = params["contentAccessDescriptorURL"].String();
        bool quiet = params["quiet"].Boolean();
        int channelChangeError = -1;

        bool success = ORB::instance(nullptr)->GetORBPlatform()->Broadcast_SetChannelToNull(
            trickPlay, contentAccessDescriptorURL, quiet, &channelChangeError);
        response["success"] = success;
        if (!success)
        {
            response["errorState"] = channelChangeError;
        }

        return ret;
    }
    // Broadcast.setChannelToTriplet
    else if (method == BROADCAST_SET_CHANNEL_TO_TRIPLET)
    {
        if (!IsRequestAllowed(token, ApplicationManager::MethodRequirement::FOR_BROADCAST_APP_ONLY))
        {
            response = MakeErrorResponse("NotRunning");
            return ret;
        }

        int idType = params["idType"].Number();
        int onid = params["onid"].Number();
        int tsid = params["tsid"].Number();
        int sid = params["sid"].Number();
        int sourceID = params["sourceID"].Number();
        std::string ipBroadcastID = params["ipBroadcastID"].String();
        bool trickPlay = params["trickplay"].Boolean();
        std::string contentAccessDescriptorURL = params["contentAccessDescriptorURL"].String();
        bool quiet = params["quiet"].Boolean();
        int channelChangeError = -1;

        bool success = ORB::instance(nullptr)->GetORBPlatform()->Broadcast_SetChannelToTriplet(
            idType, onid, tsid, sid, sourceID, ipBroadcastID,
            trickPlay, contentAccessDescriptorURL, quiet, &channelChangeError);
        response["success"] = success;
        if (!success)
        {
            response["errorState"] = channelChangeError;
        }

        return ret;
    }
    // Broadcast.setChannelToDsd
    else if (method == BROADCAST_SET_CHANNEL_TO_DSD)
    {
        if (!IsRequestAllowed(token, ApplicationManager::MethodRequirement::FOR_BROADCAST_APP_ONLY))
        {
            response = MakeErrorResponse("NotRunning");
            return ret;
        }

        std::string dsd = params["dsd"].String();
        int sid = params["sid"].Number();
        bool trickPlay = params["trickplay"].Boolean();
        std::string contentAccessDescriptorURL = params["contentAccessDescriptorURL"].String();
        bool quiet = params["quiet"].Boolean();
        int channelChangeError = -1;

        bool success = ORB::instance(nullptr)->GetORBPlatform()->Broadcast_SetChannelToDsd(dsd, sid,
            trickPlay, contentAccessDescriptorURL, quiet, &channelChangeError);
        response["success"] = success;
        if (!success)
        {
            response["errorState"] = channelChangeError;
        }

        return ret;
    }
    // Broadcast.getProgrammes
    else if (method == BROADCAST_GET_PROGRAMMES)
    {
        if (!IsRequestAllowed(token, ApplicationManager::MethodRequirement::FOR_BROADCAST_APP_ONLY))
        {
            response = MakeErrorResponse("SecurityError");
            return ret;
        }
        std::string ccid = params["ccid"].String();
        std::vector<Programme> programmes = ORB::instance(
            nullptr)->GetORBPlatform()->Broadcast_GetProgrammes(ccid);
        ArrayType<JsonValue> array;
        for (auto programme : programmes)
        {
            array.Add(programme.ToJsonObject());
        }
        JsonValue jsonProgrammeList;
        jsonProgrammeList.Array(array);
        response.Set("result", jsonProgrammeList);
    }
    // Broadcast.getComponents
    else if (method == BROADCAST_GET_COMPONENTS)
    {
        if (!IsRequestAllowed(token, ApplicationManager::MethodRequirement::FOR_BROADCAST_APP_ONLY))
        {
            response = MakeErrorResponse("SecurityError");
            return ret;
        }
        std::string ccid = params["ccid"].String();

        int componentType;

        if (params.HasLabel("typeCode"))
        {
            componentType = params["typeCode"].Number();
        }
        else
        {
            componentType = -1;
        }


        std::vector<Component> components = ORB::instance(
            nullptr)->GetORBPlatform()->Broadcast_GetComponents(ccid, componentType);
        ArrayType<JsonValue> array;
        for (auto component : components)
        {
            array.Add(component.ToJsonObject());
        }
        JsonValue jsonComponentList;
        jsonComponentList.Array(array);
        response.Set("result", jsonComponentList);
    }
    // Broadcast.selectComponent
    else if (method == BROADCAST_SELECT_COMPONENT)
    {
        if (!IsRequestAllowed(token, ApplicationManager::MethodRequirement::FOR_BROADCAST_APP_ONLY))
        {
            response = MakeErrorResponse("SecurityError");
            return ret;
        }
        int componentType = params["type"].Number();
        int pid = params["pid"].Number();
        ORB::instance(nullptr)->GetORBPlatform()->Broadcast_SelectComponent(componentType, pid);
        response.FromString("{}");
    }
    // Broadcast.unselectComponent
    else if (method == BROADCAST_UNSELECT_COMPONENT)
    {
        if (!IsRequestAllowed(token, ApplicationManager::MethodRequirement::FOR_BROADCAST_APP_ONLY))
        {
            response = MakeErrorResponse("SecurityError");
            return ret;
        }
        int componentType = params["type"].Number();
        ORB::instance(nullptr)->GetORBPlatform()->Broadcast_UnselectComponent(componentType);
        response.FromString("{}");
    }
    // Broadcast.startSearch
    else if (method == BROADCAST_START_SEARCH)
    {
        if (!IsRequestAllowed(token, ApplicationManager::MethodRequirement::FOR_BROADCAST_APP_ONLY))
        {
            response = MakeErrorResponse("SecurityError");
            return ret;
        }
        std::string queryAsString = params["query"].String();
        std::shared_ptr<Query> query = std::make_shared<Query>(queryAsString);
        int offset = params["offset"].Number();
        int count = params["count"].Number();
        ArrayType<JsonValue> array = params["channelConstraints"].Array();
        std::vector<std::string> channelConstraints;
        for (int i = 0; i < array.Length(); i++)
        {
            channelConstraints.push_back(array[i].String());
        }
        std::shared_ptr<MetadataSearchTask> searchTask = std::make_shared<MetadataSearchTask>(query,
            offset, count, channelConstraints);
        ORB::instance(nullptr)->AddMetadataSearchTask(query->GetQueryId(), searchTask);
        searchTask->Run();
        response.FromString("{}");
    }
    // Broadcast.abortSearch
    else if (method == BROADCAST_ABORT_SEARCH)
    {
        if (!IsRequestAllowed(token, ApplicationManager::MethodRequirement::FOR_BROADCAST_APP_ONLY))
        {
            response = MakeErrorResponse("SecurityError");
            return ret;
        }
        int queryId = params["queryId"].Number();
        std::shared_ptr<MetadataSearchTask> searchTask = ORB::instance(
            nullptr)->GetMetadataSearchTask(queryId);
        if (searchTask)
        {
            searchTask->Stop();
            ORB::instance(nullptr)->RemoveMetadataSearchTask(queryId);
        }
        std::vector<std::string> searchResults;
        MetadataSearchTask::OnMetadataSearchCompleted(queryId, SEARCH_STATUS_ABORTED,
            searchResults);
        response.FromString("{}");
    }
    // Broadcast.addStreamEventListener
    else if (method == BROADCAST_ADD_STREAM_EVENT_LISTENER)
    {
        std::string targetUrl = params["targetURL"].String();
        std::string eventName = params["eventName"].String();
        int componentTag = params["componentTag"].Number();
        int streamEventId = params["streamEventId"].Number();
        int id = AddStreamEventListener(targetUrl, eventName, componentTag, streamEventId);
        response["subscribed"] = (id != -1);
        response["id"] = id;
    }
    // Broadcast.removeStreamEventListener
    else if (method == BROADCAST_REMOVE_STREAM_EVENT_LISTENER)
    {
        int id = params["id"].Number();
        RemoveStreamEventListener(id);
        response.FromString("{}");
    }
    // Broadcast.setPresentationSuspended
    else if (method == BROADCAST_SET_PRESENTATION_SUSPENDED)
    {
        if (!IsRequestAllowed(token, ApplicationManager::MethodRequirement::FOR_BROADCAST_APP_ONLY))
        {
            response = MakeErrorResponse("SecurityError");
            return ret;
        }
        bool presentationSuspended = params["presentationSuspended"].Boolean();
        ORB::instance(nullptr)->GetORBPlatform()->Broadcast_SetPresentationSuspended(
            presentationSuspended);
        response.FromString("{}");
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
    static int subscriberId = 0;
    subscriberId++;

    fprintf(stderr,
        "[BroadcastRequestHandler::AddStreamEventListener] targetUrl=%s eventName=%s componentTag=%d streamEventId=%d",
        targetUrl.c_str(),
        eventName.c_str(),
        componentTag,
        streamEventId
        );

    bool result = false;

    if (targetUrl.rfind("dvb:", 0) == 0)
    {
        result = ORB::instance(nullptr)->GetORBPlatform()->Dsmcc_SubscribeToStreamEventByName(
            targetUrl, eventName, subscriberId);
    }
    else
    {
        result = ORB::instance(nullptr)->GetORBPlatform()->Dsmcc_SubscribeStreamEventId(
            eventName, componentTag, streamEventId, subscriberId);
    }

    if (result)
    {
        return subscriberId;
    }

    return -1;
}

/**
 * Remove the specified DSM-CC stream event listener.
 *
 * @param id The listener id
 */
void BroadcastRequestHandler::RemoveStreamEventListener(int id)
{
    fprintf(stderr, "[BroadcastRequestHandler::RemoveStreamEventListener] id=%d\n", id);
    ORB::instance(nullptr)->GetORBPlatform()->Dsmcc_UnsubscribeFromStreamEvents(id);
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
bool BroadcastRequestHandler::IsRequestAllowed(JsonObject token,
    ApplicationManager::MethodRequirement methodType)
{
    std::string uri = "";
    int appId = 0;

    JsonObject payload = token["payload"].Object();
    std::string payloadAsString;
    payload.ToString(payloadAsString);
    fprintf(stderr, "[BroadcastRequestHandler::IsRequestAllowed] payload=%s\n",
        payloadAsString.c_str());

    if (payload.HasLabel("appId"))
    {
        appId = static_cast<int>(payload["appId"].Number());
    }
    if (payload.HasLabel("uri"))
    {
        uri = payload["uri"].String();
    }
    return ORB::instance(nullptr)->GetApplicationManager()->IsRequestAllowed(appId, uri,
        methodType);
}
} // namespace orb
