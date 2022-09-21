/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

#include "ORBPlatformEventHandlerImpl.h"
#include "ORBPlatform.h"
#include "ORBEngine.h"
#include "Logging.h"
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
void ORBPlatformEventHandlerImpl::OnAitSectionReceived(unsigned short aitPid, unsigned short serviceId, unsigned char *aitSectionData, unsigned int aitSectionDataSize)
{
   ORB_LOG("aitPid=0x%x serviceId=%hu aitSectionDataSize=%u", aitPid, serviceId, aitSectionDataSize);

   bool isConnectedToInternet = ORBEngine::GetSharedInstance().GetORBPlatform()->Network_IsConnectedToInternet();
   ORBEngine::GetSharedInstance().GetApplicationManager()->OnNetworkAvailabilityChanged(isConnectedToInternet);

   ORBEngine::GetSharedInstance().GetApplicationManager()->ProcessAitSection(aitPid, serviceId, aitSectionData, aitSectionDataSize);
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
void ORBPlatformEventHandlerImpl::OnChannelStatusChanged(int onetId, int transId, int servId, int statusCode, bool permanentError)
{
   ORB_LOG("onetId=%d transId=%d servId=%d statusCode=%d permanentError=%s",
      onetId, transId, servId, statusCode, permanentError ? "yes" : "no");

   // notify the application manager iff channel status is 'connecting'
   if (statusCode == Channel::Status::CHANNEL_STATUS_CONNECTING)
   {
      ORBEngine::GetSharedInstance().GetApplicationManager()->OnChannelChanged(onetId, transId, servId);
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
 */
void ORBPlatformEventHandlerImpl::OnParentalRatingError()
{
   ORB_LOG_NO_ARGS();

   // prepare event properties and request event dispatching
   json properties = "{}"_json;
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
   properties["componentType"] = componentType;

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
void ORBPlatformEventHandlerImpl::OnAccessToDistinctiveIdentifierDecided(std::string origin, bool accessAllowed)
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
void ORBPlatformEventHandlerImpl::OnStreamEvent(int id, std::string name, std::string data, std::string text, std::string status)
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

   ORBEngine::GetSharedInstance().GetEventListener()->OnJavaScriptEventDispatchRequested(
      "StreamEvent", properties.dump(), "", true);
}

/**
 * Notify all subscribers that the specified DVB URL load has finished.
 *
 * @param requestId         The request identifier
 * @param fileContent       The file content
 * @param fileContentLength The file content length in number of bytes
 */
void ORBPlatformEventHandlerImpl::OnDvbUrlLoaded(int requestId, unsigned short int *fileContent, unsigned int fileContentLength)
{
   ORB_LOG("requestId=%d fileContentLength=%u", requestId, fileContentLength);
   ORBEngine::GetSharedInstance().GetEventListener()->OnDvbUrlLoaded(requestId, fileContent, fileContentLength);
}

/**
 * Notify the browser that the specified input key was generated.
 *
 * @param keyCode The JavaScript key code
 */
void ORBPlatformEventHandlerImpl::OnInputKeyGenerated(int keyCode)
{
   ORB_LOG("keyCode=%d", keyCode);

   uint16_t currentAppId = ORBEngine::GetSharedInstance().GetCurrentAppId();

   // check if there is any application currently running
   if (currentAppId == UINT16_MAX)
   {
      ORB_LOG("No app is currently running");
      return;
   }

   uint16_t mask = ORBEngine::GetSharedInstance().GetApplicationManager()->GetKeySetMask(currentAppId);
   uint16_t keyEventCode = 0;

   keyEventCode = ORBEngine::GetSharedInstance().GetORBPlatform()->Platform_ResolveKeyEvent(keyCode);

   if (mask & keyEventCode)
   {
      ORBEngine::GetSharedInstance().GetEventListener()->OnInputKeyGenerated(keyCode);
   }
}
} // namespace orb