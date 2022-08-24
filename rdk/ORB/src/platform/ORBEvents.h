/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

#ifndef ORB_EVENTS_H
#define ORB_EVENTS_H

#include <string>
#include <vector>

/**
 * Notify the application manager that the broadcast playback has stopped.
 */
void Event_OnBroadcastStopped();

/**
  * Notify the application manager that an AIT section was received.
  *
  * @param aitPid             The AID PID        
  * @param serviceId          The corresponding service id
  * @param aitSectionData     The AIT section data
  * @param aitSectionDataSize The AIT section data size in number of bytes
  */
void Event_OnAitSectionReceived(unsigned short aitPid, unsigned short serviceId, unsigned char *aitSectionData, unsigned int aitSectionDataSize);

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
void Event_OnChannelStatusChanged(int onetId, int transId, int servId, int statusCode, bool permanentError);

/**
 * Dispatch the ServiceListChanged bridge event to the current page's JavaScript context.
 */
void Event_OnServiceListChanged();

/**
 * Dispatch the ParentalRatingChange bridge event to the current page's JavaScript context.
 *
 * @param blocked Indicates if the current service is blocked by the parental control system
 */
void Event_OnParentalRatingChanged(bool blocked);

/**
 * Dispatch the ParentalRatingError bridge event to the current page's JavaScript context.
 */
void Event_OnParentalRatingError();

/**
 * Dispatch the SelectedComponentChanged bridge event to the current page's JavaScript context.
 *
 * @param componentType The component type (0: video, 1: audio, 2: subtitle)
 */
void Event_OnSelectedComponentChanged(int componentType);

/**
 * Dispatch the ComponentChanged bridge event to the current page's JavaScript context.
 *
 * @param componentType The component type (0: video, 1: audio, 2: subtitle)
 */
void Event_OnComponentChanged(int componentType);

/**
 * Dispatch the ProgrammesChanged bridge event to the current page's JavaScript context.
 */
void Event_OnProgrammesChanged();

/**
 * Dispatch the LowMemory bridge event to the current page's JavaScript context.
 */
void Event_OnLowMemory();

/**
 * Dispatch the accesstodistinctiveidentifier bridge event to the current page's JavaScript context.
 *
 * @param origin        The origin of the requesting application
 * @param accessAllowed True if access allowed, false otherwise
 */
void Event_OnAccessToDistinctiveIdentifierDecided(std::string origin, bool accessAllowed);

/**
 * Dispatch the TransitionedToBroadcastRelated bridge event to the current page's JavaScript context.
 */
void Event_OnAppTransitionedToBroadcastRelated();

/**
 * Dispatch the StreamEvent bridge event to the current page's JavaScript context.
 *
 * @param id     The stream event id
 * @param name   The stream event name
 * @param data   The stream event data encoded in Hexadecimal
 * @param text   The stream event data encoded in UTF-8
 * @param status The stream event status
 */
void Event_OnStreamEvent(int id, std::string name, std::string data, std::string text, std::string status);

/**
 * Notify all subscribers that the specified DVB URL load has finished.
 *
 * @param requestId         The request identifier
 * @param fileContentLength The file content length in number of bytes
 */
void Event_OnDvbUrlLoaded(int requestId, unsigned int fileContentLength);

#endif // ORB_EVENTS_H
