/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

#include "ORBEvents.h"
#include "ORBPlatform.h"
#include "ORB.h"

using namespace WPEFramework::Plugin;

/**
 * Notify the application manager that the broadcast playback has stopped.
 */
void Event_OnBroadcastStopped()
{
    fprintf(stderr, "Event_OnBroadcastStopped\n");
    ORB::instance(nullptr)->GetApplicationManager()->OnBroadcastStopped();
}

/**
 * Notify the application manager that an AIT section was received.
 *
 * @param aitPid             The AID PID
 * @param serviceId          The corresponding service id
 * @param aitSectionData     The AIT section data
 * @param aitSectionDataSize The AIT section data size in number of bytes
 */
void Event_OnAitSectionReceived(unsigned short aitPid, unsigned short serviceId, unsigned
    char *aitSectionData, unsigned int aitSectionDataSize)
{
    fprintf(stderr, "Event_OnAitSectionReceived aitPid=0x%x serviceId=%hu aitSectionDataSize=%u\n",
        aitPid, serviceId, aitSectionDataSize);

    bool isConnectedToInternet = ORB::instance(
        nullptr)->GetORBPlatform()->Network_IsConnectedToInternet();
    ORB::instance(nullptr)->GetApplicationManager()->OnNetworkAvailabilityChanged(
        isConnectedToInternet);

    ORB::instance(nullptr)->GetApplicationManager()->ProcessAitSection(aitPid, serviceId,
        aitSectionData, aitSectionDataSize);
}

/**
 * Notify the application manager that the current channel's status has changed.
 * Also dispatch the ChannelStatusChanged bridge event to the current page's JavaScript context.
 *
 * @param onetId         The original network id
 * @param transId        The transport stream id
 * @param servId         The service id
 * @param statusCode     The channel status code
 * @param permanentError Permanent error indicator
 */
void Event_OnChannelStatusChanged(int onetId, int transId, int servId, int statusCode, bool
    permanentError)
{
    fprintf(stderr,
        "Event_OnChannelStatusChanged onetId=%d transId=%d servId=%d statusCode=%d permanentError=%s\n",
        onetId, transId, servId, statusCode, permanentError ? "yes" : "no");

    // notify the application manager iff channel status is 'connecting'
    if (statusCode == CHANNEL_STATUS_CONNECTING)
    {
        ORB::instance(nullptr)->GetApplicationManager()->OnChannelChanged(onetId, transId, servId);
    }

    // prepare event properties and request event dispatching
    JsonObject properties;
    properties.Set("onetId", onetId);
    properties.Set("transId", transId);
    properties.Set("servId", servId);
    properties.Set("statusCode", statusCode);
    if (statusCode >= CHANNEL_STATUS_NOT_SUPPORTED)
    {
        properties.Set("permanentError", permanentError);
    }

    ORB::instance(nullptr)->NotifyJavaScriptEventDispatchRequested("ChannelStatusChanged",
        properties, true, "");
}

/**
 * Dispatch the ServiceListChanged bridge event to the current page's JavaScript context.
 */
void Event_OnServiceListChanged()
{
    fprintf(stderr, "Event_OnServiceListChanged\n");
    // TODO Implement me
}

/**
 * Dispatch the ParentalRatingChange bridge event to the current page's JavaScript context.
 *
 * @param blocked Indicates if the current service is blocked by the parental control system
 */
void Event_OnParentalRatingChanged(bool blocked)
{
    fprintf(stderr, "Event_OnParentalRatingChanged blocked=%s\n", blocked ? "yes" : "no");

    // prepare event properties and request event dispatching
    JsonObject properties;
    properties["blocked"] = blocked;

    ORB::instance(nullptr)->NotifyJavaScriptEventDispatchRequested("ParentalRatingChange",
        properties, true, "");
}

/**
 * Dispatch the ParentalRatingError bridge event to the current page's JavaScript context.
 */
void Event_OnParentalRatingError()
{
    fprintf(stderr, "Event_OnParentalRatingError\n");

    // prepare event properties and request event dispatching
    JsonObject properties;
    properties.FromString("{}");

    ORB::instance(nullptr)->NotifyJavaScriptEventDispatchRequested("ParentalRatingError",
        properties, true, "");
}

/**
 * Dispatch the SelectedComponentChanged bridge event to the current page's JavaScript context.
 *
 * @param componentType The component type (0: video, 1: audio, 2: subtitle)
 */
void Event_OnSelectedComponentChanged(int componentType)
{
    fprintf(stderr, "Event_OnSelectedComponentChanged componentType=%d\n", componentType);

    // prepare event properties and request event dispatching
    JsonObject properties;
    properties["componentType"] = componentType;

    ORB::instance(nullptr)->NotifyJavaScriptEventDispatchRequested("SelectedComponentChanged",
        properties, true, "");
}

/**
 * Dispatch the ComponentChanged bridge event to the current page's JavaScript context.
 *
 * @param componentType The component type (0: video, 1: audio, 2: subtitle)
 */
void Event_OnComponentChanged(int componentType)
{
    fprintf(stderr, "Event_OnComponentChanged componentType=%d\n", componentType);

    // prepare event properties and request event dispatching
    JsonObject properties;
    properties["componentType"] = componentType;

    ORB::instance(nullptr)->NotifyJavaScriptEventDispatchRequested("ComponentChanged", properties,
        true, "");
}

/**
 * Dispatch the ProgrammesChanged bridge event to the current page's JavaScript context.
 */
void Event_OnProgrammesChanged()
{
    fprintf(stderr, "Event_OnProgrammesChanged\n");

    // prepare event properties and request event dispatching
    JsonObject properties;
    properties.FromString("{}");

    ORB::instance(nullptr)->NotifyJavaScriptEventDispatchRequested("ProgrammesChanged", properties,
        true, "");
}

/**
 * Dispatch the LowMemory bridge event to the current page's JavaScript context.
 */
void Event_OnLowMemory()
{
    fprintf(stderr, "Event_OnLowMemoryEvent\n");

    // prepare event properties and request event dispatching
    JsonObject properties;
    properties.FromString("{}");

    ORB::instance(nullptr)->NotifyJavaScriptEventDispatchRequested("LowMemory", properties, false,
        "");
}

/**
 * Dispatch the accesstodistinctiveidentifier bridge event to the current page's JavaScript context.
 *
 * @param origin        The origin of the requesting application
 * @param accessAllowed True if access allowed, false otherwise
 */
void Event_OnAccessToDistinctiveIdentifierDecided(std::string origin, bool accessAllowed)
{
    fprintf(stderr, "Event_OnAccessToDistinctiveIdentifierDecided origin=%s accessAllowed=%s\n",
        origin.c_str(),
        accessAllowed ? "yes" : "no"
        );

    JsonObject properties;
    properties["allowAccess"] = accessAllowed;

    ORB::instance(nullptr)->NotifyJavaScriptEventDispatchRequested("accesstodistinctiveidentifier",
        properties, false, origin);
}

/**
 * Dispatch the TransitionedToBroadcastRelated bridge event to the current page's JavaScript context.
 */
void Event_OnAppTransitionedToBroadcastRelated()
{
    fprintf(stderr, "OnAppTransitionedToBroadcastRelated\n");

    // prepare event properties and request event dispatching
    JsonObject properties;
    properties.FromString("{}");

    ORB::instance(nullptr)->NotifyJavaScriptEventDispatchRequested("TransitionedToBroadcastRelated",
        properties, false, "");
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
void Event_OnStreamEvent(int id, std::string name, std::string data, std::string text, std::string
    status)
{
    fprintf(stderr, "Event_OnStreamEvent id=%d name=%s data=%s text=%s status=%s\n",
        id, name.c_str(), data.c_str(), text.c_str(), status.c_str()
        );

    // prepare event properties and request event dispatching
    JsonObject properties;
    properties["id"] = id;
    properties["name"] = name;
    properties["data"] = data;
    properties["text"] = text;
    properties["status"] = status;

    ORB::instance(nullptr)->NotifyJavaScriptEventDispatchRequested("StreamEvent", properties, true,
        "");
}

/**
 * Notify all subscribers that the specified DVB URL load has finished.
 *
 * @param requestId         The request identifier
 * @param fileContentLength The file content length in number of bytes
 */
void Event_OnDvbUrlLoaded(int requestId, unsigned int fileContentLength)
{
    fprintf(stderr, "Event_OnDvbUrlLoaded requestId=%d fileContentLength=%u\n", requestId,
        fileContentLength);
    ORB::instance(nullptr)->NotifyDvbUrlLoaded(requestId, fileContentLength);
}
