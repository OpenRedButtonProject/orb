/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

#include "ORBPlatformEventHandlerImpl.h"
#include "ORBPlatform.h"
#include "ORBEngine.h"
#include "ORBLogging.h"
#include "JsonUtil.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace orb
{
/**
 * Constructor.
 */
ORBPlatformEventHandlerImpl::ORBPlatformEventHandlerImpl()
{
    ORB_LOG_NO_ARGS();
}

/**
 * Destructor.
 */
ORBPlatformEventHandlerImpl::~ORBPlatformEventHandlerImpl()
{
    ORB_LOG_NO_ARGS();
}

/**
 * Notify the application manager that the broadcast playback has stopped.
 */
void ORBPlatformEventHandlerImpl::OnBroadcastStopped()
{
    ORB_LOG_NO_ARGS();
    ORBEngine::GetSharedInstance().GetApplicationManager()->OnBroadcastStopped();
}

/**
 * Notify the application manager that an AIT section was received.
 *
 * @param aitPid             The AID PID
 * @param serviceId          The corresponding service id
 * @param aitSectionData     The AIT section data
 * @param aitSectionDataSize The AIT section data size in number of bytes
 */
void ORBPlatformEventHandlerImpl::OnAitSectionReceived(unsigned short aitPid, unsigned short
    serviceId, unsigned char *aitSectionData, unsigned int aitSectionDataSize)
{
    ORB_LOG("aitPid=0x%x serviceId=%hu aitSectionDataSize=%u", aitPid, serviceId,
        aitSectionDataSize);

    bool isConnectedToInternet =
        ORBEngine::GetSharedInstance().GetORBPlatform()->Network_IsConnectedToInternet();
    ORBEngine::GetSharedInstance().GetApplicationManager()->OnNetworkAvailabilityChanged(
        isConnectedToInternet);

    ORBEngine::GetSharedInstance().GetApplicationManager()->ProcessAitSection(aitPid, serviceId,
        aitSectionData, aitSectionDataSize);
}

/**
 * Notify the application manager that the current channel's status has changed.
 * Also dispatch the ChannelStatusChanged bridge event to the current page's JavaScript context.
 *
 * @param onetId         The original network id
 * @param transId        The transport stream id
 * @param servId         The service id
 * @param statusCode     The channel status code (value from Channel::Status or Channel::ErrorState)
 * @param permanentError Permanent error indicator
 */
void ORBPlatformEventHandlerImpl::OnChannelStatusChanged(int onetId, int transId, int servId, int
    statusCode, bool permanentError)
{
    ORB_LOG("onetId=%d transId=%d servId=%d statusCode=%d permanentError=%s",
        onetId, transId, servId, statusCode, permanentError ? "yes" : "no");

    // notify the application manager iff channel status is 'connecting'
    if (statusCode == Channel::Status::CHANNEL_STATUS_CONNECTING)
    {
        ORBEngine::GetSharedInstance().GetApplicationManager()->OnChannelChanged(onetId, transId,
            servId);
    }

    // prepare event properties and request event dispatching
    json properties;
    properties.emplace("onetId", onetId);
    properties.emplace("transId", transId);
    properties.emplace("servId", servId);
    properties.emplace("statusCode", statusCode);
    if (statusCode >= Channel::ErrorState::CHANNEL_ERROR_STATE_NOT_SUPPORTED)
    {
        properties.emplace("permanentError", permanentError);
    }

    ORBEngine::GetSharedInstance().GetEventListener()->OnJavaScriptEventDispatchRequested(
        "ChannelStatusChanged", properties.dump(), "", true);
}

/**
 * Dispatch the ParentalRatingChange bridge event to the current page's JavaScript context.
 *
 * @param blocked Indicates if the current service is blocked by the parental control system
 */
void ORBPlatformEventHandlerImpl::OnParentalRatingChanged(bool blocked)
{
    ORB_LOG("blocked=%s", blocked ? "yes" : "no");

    // prepare event properties and request event dispatching
    json properties;
    properties["blocked"] = blocked;

    ORBEngine::GetSharedInstance().GetEventListener()->OnJavaScriptEventDispatchRequested(
        "ParentalRatingChange", properties.dump(), "", true);
}

/**
 * Dispatch the ParentalRatingError bridge event to the current page's JavaScript context.
 *
 * @param contentId   Content ID to which the parental rating error applies
 * @param ratings     The parental rating value of the currently playing content
 * @param drmSystemId DRM System ID of the DRM system that generated the event
 */
void ORBPlatformEventHandlerImpl::OnParentalRatingError(std::string contentId,
    std::vector<ParentalRating> ratings, std::string drmSystemId)
{
    ORB_LOG_NO_ARGS();

    // prepare event properties and request event dispatching
    json properties = "{}"_json;
    json json_ratings;

    for (ParentalRating rating : ratings)
    {
        json_ratings.push_back(JsonUtil::ParentalRatingToJsonObject(rating));
    }

    properties.emplace("contentID", contentId);
    properties.emplace("ratings", json_ratings);
    properties.emplace("DRMSystemID", drmSystemId);

    ORBEngine::GetSharedInstance().GetEventListener()->OnJavaScriptEventDispatchRequested(
        "ParentalRatingError", properties.dump(), "", true);
}

/**
 * Dispatch the SelectedComponentChanged bridge event to the current page's JavaScript context.
 *
 * @param componentType The component type (0: video, 1: audio, 2: subtitle)
 */
void ORBPlatformEventHandlerImpl::OnSelectedComponentChanged(int componentType)
{
    ORB_LOG("componentType=%d", componentType);

    // prepare event properties and request event dispatching
    json properties;
    properties["componentType"] = componentType;

    ORBEngine::GetSharedInstance().GetEventListener()->OnJavaScriptEventDispatchRequested(
        "SelectedComponentChanged", properties.dump(), "", true);
}

/**
 * Dispatch the ComponentChanged bridge event to the current page's JavaScript context.
 *
 * @param componentType The component type (0: video, 1: audio, 2: subtitle)
 */
void ORBPlatformEventHandlerImpl::OnComponentChanged(int componentType)
{
    ORB_LOG("componentType=%d", componentType);

    // prepare event properties and request event dispatching
    json properties;

    if (componentType >= COMPONENT_TYPE_VIDEO && componentType <= COMPONENT_TYPE_SUBTITLE)
    {
        properties["componentType"] = componentType;
    }

    ORBEngine::GetSharedInstance().GetEventListener()->OnJavaScriptEventDispatchRequested(
        "ComponentChanged", properties.dump(), "", true);
}

/**
 * Dispatch the ProgrammesChanged bridge event to the current page's JavaScript context.
 */
void ORBPlatformEventHandlerImpl::OnProgrammesChanged()
{
    ORB_LOG_NO_ARGS();

    // prepare event properties and request event dispatching
    json properties = "{}"_json;

    ORBEngine::GetSharedInstance().GetEventListener()->OnJavaScriptEventDispatchRequested(
        "ProgrammesChanged", properties.dump(), "", true);
}

/**
 * Dispatch the LowMemory bridge event to the current page's JavaScript context.
 */
void ORBPlatformEventHandlerImpl::OnLowMemory()
{
    ORB_LOG_NO_ARGS();

    // prepare event properties and request event dispatching
    json properties = "{}"_json;

    ORBEngine::GetSharedInstance().GetEventListener()->OnJavaScriptEventDispatchRequested(
        "LowMemory", properties.dump(), "", false);
}

/**
 * Dispatch the accesstodistinctiveidentifier bridge event to the current page's JavaScript context.
 *
 * @param origin        The origin of the requesting application
 * @param accessAllowed True if access allowed, false otherwise
 */
void ORBPlatformEventHandlerImpl::OnAccessToDistinctiveIdentifierDecided(std::string origin, bool
    accessAllowed)
{
    ORB_LOG("origin=%s accessAllowed=%s", origin.c_str(), accessAllowed ? "yes" : "no");

    json properties;
    properties["allowAccess"] = accessAllowed;

    ORBEngine::GetSharedInstance().GetEventListener()->OnJavaScriptEventDispatchRequested(
        "accesstodistinctiveidentifier", properties.dump(), origin, false);
}

/**
 * Dispatch the TransitionedToBroadcastRelated bridge event to the current page's JavaScript context.
 */
void ORBPlatformEventHandlerImpl::OnAppTransitionedToBroadcastRelated()
{
    ORB_LOG_NO_ARGS();

    // prepare event properties and request event dispatching
    json properties = "{}"_json;

    ORBEngine::GetSharedInstance().GetEventListener()->OnJavaScriptEventDispatchRequested(
        "TransitionedToBroadcastRelated", properties.dump(), "", false);
}

/**
 * Dispatch the StreamEvent bridge event to the current page's JavaScript context.
 *
 * @param id     The stream event id
 * @param name   The stream event name
 * @param data   The stream event data encoded in Hexadecimal
 * @param text   The stream event data encoded in UTF-8
 * @param status The stream event status
 */
void ORBPlatformEventHandlerImpl::OnStreamEvent(int id, std::string name, std::string data,
    std::string text, std::string status)
{
    ORB_LOG("id=%d name=%s data=%s text=%s status=%s",
        id, name.c_str(), data.c_str(), text.c_str(), status.c_str()
        );

    // prepare event properties and request event dispatching
    json properties;
    properties["id"] = id;
    properties["name"] = name;
    properties["data"] = data;
    properties["text"] = text;
    properties["status"] = status;

    std::string eventProperties = "";
    try
    {
        eventProperties = properties.dump();
        ORBEngine::GetSharedInstance().GetEventListener()->OnJavaScriptEventDispatchRequested(
            "StreamEvent", eventProperties, "", true);
    }
    catch (json::type_error& e)
    {
        ORB_ERROR(e.what(), e.id);
    }
}

/**
 * Notify all subscribers that the specified DVB URL load has finished.
 *
 * @param requestId         The request identifier
 * @param fileContent       The file content
 * @param fileContentLength The file content length in number of bytes
 */
void ORBPlatformEventHandlerImpl::OnDvbUrlLoaded(int requestId, std::vector<uint8_t> fileContent,
    unsigned int fileContentLength)
{
    ORB_LOG("requestId=%d fileContentLength=%u", requestId, fileContentLength);
    ORBEngine::GetSharedInstance().GetEventListener()->OnDvbUrlLoaded(requestId, fileContent,
        fileContentLength);
}

/**
 * Notify all subscribers that the specified DVB URL load has finished. The content is not passed
 *
 * @param requestId         The request identifier
 * @param fileContentLength The file content length in number of bytes
 */
void ORBPlatformEventHandlerImpl::OnDvbUrlLoadedNoData(int requestId, unsigned int
    fileContentLength)
{
    ORB_LOG("requestId=%d fileContentLength=%u", requestId, fileContentLength);
    ORBEngine::GetSharedInstance().GetEventListener()->OnDvbUrlLoadedNoData(requestId,
        fileContentLength);
}

/**
 * Notify the browser that the specified input key was generated.
 *
 * @param keyCode   The JavaScript key code
 * @param keyAction The key action (0 = keyup , 1 = keydown)
 */
bool ORBPlatformEventHandlerImpl::OnInputKeyGenerated(int keyCode, KeyAction keyAction)
{
    ORB_LOG("keyCode=%d, action=%d", keyCode, keyAction);

    // Apply platform-specific keycode mapping if neccessary
    keyCode = ORBEngine::GetSharedInstance().GetORBPlatform()->Platform_MapKeyCode(keyCode);

    bool consumed = false;
    uint16_t currentAppId = ORBEngine::GetSharedInstance().GetCurrentAppId();

    // check if there is any application currently running
    if (currentAppId == UINT16_MAX)
    {
        ORB_LOG("No app is currently running");
        return false;
    }

    if (ORBEngine::GetSharedInstance().GetApplicationManager()->InKeySet(currentAppId, keyCode))
    {
        consumed = true;

        if (keyAction == KEY_ACTION_UP)
        {
            ORBEngine::GetSharedInstance().GetEventListener()->OnInputKeyGenerated(keyCode, 0);
        }
        else if (keyAction == KEY_ACTION_DOWN)
        {
            ORBEngine::GetSharedInstance().GetEventListener()->OnInputKeyGenerated(keyCode, 1);
        }
    }

    return consumed;
}

/**
 * Notify the browser about DRM licensing errors during playback of DRM protected A/V content.
 *
 * @param errorState      Details the type of error
 * @param contentId       Unique identifier of the protected content
 * @param drmSystemId     ID of the DRM system
 * @param rightsIssuerUrl Indicates the value of the rightsIssuerURL that can be used to
 *                        non-silently obtain the rights for the content item
 */
void ORBPlatformEventHandlerImpl::OnDrmRightsError(
    DrmRightsError errorState,
    std::string contentId,
    std::string drmSystemId,
    std::string rightsIssuerUrl
    )
{
    ORB_LOG("errorState=%u contentId=%s drmSystemId=%s rightsIssuerUrl=%s",
        errorState, contentId.c_str(), drmSystemId.c_str(), rightsIssuerUrl.c_str());

    json properties = "{}"_json;

    properties.emplace("errorState", errorState);
    properties.emplace("contentID", contentId);
    properties.emplace("DRMSystemID", drmSystemId);
    properties.emplace("rightsIssuerURL", rightsIssuerUrl);

    ORBEngine::GetSharedInstance().GetEventListener()->OnJavaScriptEventDispatchRequested(
        "DRMRightsError", properties.dump(), "", false);
}

/**
 * Notify the browser about a change in the status of a DRM system.
 *
 * @param drmSystem          ID of the DRM system
 * @param drmSystemIds       List of the DRM System IDs handled by the DRM System
 * @param status             Status of the indicated DRM system
 * @param protectionGateways Space-separated list of zero or more CSP Gateway types that are
 *                           capable of supporting the DRM system
 * @param supportedFormats   Space separated list of zero or more supported file and/or
 *                           container formats by the DRM system
 */
void ORBPlatformEventHandlerImpl::OnDrmSystemStatusChanged(
    std::string drmSystem,
    std::vector<std::string> drmSystemIds,
    DrmSystemStatus::State status,
    std::string protectionGateways,
    std::string supportedFormats
    )
{
    ORB_LOG("drmSystem=%s status=%u protectionGateways=%s supportedFormats=%s",
        drmSystem.c_str(), status, protectionGateways.c_str(), supportedFormats.c_str());

    json properties = "{}"_json;
    json json_drmSystemIds;

    for (std::string drmSystemId : drmSystemIds)
    {
        json_drmSystemIds.push_back(drmSystemId);
    }

    properties.emplace("DRMSystem", drmSystem);
    properties.emplace("DRMSystemIDs", json_drmSystemIds);
    properties.emplace("status", status);
    properties.emplace("protectionGateways", protectionGateways);
    properties.emplace("supportedFormats", supportedFormats);

    ORBEngine::GetSharedInstance().GetEventListener()->OnJavaScriptEventDispatchRequested(
        "DRMSystemStatusChange", properties.dump(), "", false);
}

/**
 * Notify the browser that the underlying DRM system has a result message as a consequence
 * of a call to Drm_SendDrmMessage.
 *
 * @param messageId  Identifies the original message which has led to this resulting message
 * @param result     DRM system specific result message
 * @param resultCode Result code
 */
void ORBPlatformEventHandlerImpl::OnSendDrmMessageResult(
    std::string messageId,
    std::string result,
    SendDrmMessageResultCode resultCode
    )
{
    ORB_LOG("messageId=%s result=%s resultCode=%u", messageId.c_str(), result.c_str(), resultCode);

    json properties = "{}"_json;

    properties.emplace("msgID", messageId);
    properties.emplace("resultMsg", result);
    properties.emplace("resultCode", resultCode);

    ORBEngine::GetSharedInstance().GetEventListener()->OnJavaScriptEventDispatchRequested(
        "DRMMessageResult", properties.dump(), "", false);
}

/**
 * Notify the browser that the underlying DRM system has a message to report.
 *
 * @param message     DRM system specific message
 * @param drmSystemId ID of the DRM System
 */
void ORBPlatformEventHandlerImpl::OnDrmSystemMessage(std::string message, std::string drmSystemId)
{
    ORB_LOG("message=%s drmSystemId=%s", message.c_str(), drmSystemId.c_str());

    json properties = "{}"_json;

    properties.emplace("msg", message);
    properties.emplace("DRMSystemID", drmSystemId);

    ORBEngine::GetSharedInstance().GetEventListener()->OnJavaScriptEventDispatchRequested(
        "DRMSystemMessage", properties.dump(), "", false);
}
} // namespace orb
