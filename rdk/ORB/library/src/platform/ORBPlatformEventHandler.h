/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */
#pragma once

#include <string>
#include <vector>

namespace orb
{
/**
 * Interface of the ORB platform event handler. The ORB platform implementation is expected to
 * properly call the methods of this interface as to notify the HbbTV application manager embedded
 * in ORB core, and/or the JavaScript layer (i.e. the HbbTV app) of platform-specific events.
 */
class ORBPlatformEventHandler
{
public:

   /**
    * Destructor.
    */
   virtual ~ORBPlatformEventHandler()
   {
   }

public:

   /**
    * Notify the application manager that the broadcast playback has stopped.
    */
   virtual void OnBroadcastStopped() = 0;

   /**
    * Notify the application manager that an AIT section was received.
    *
    * @param aitPid             The AID PID
    * @param serviceId          The corresponding service id
    * @param aitSectionData     The AIT section data
    * @param aitSectionDataSize The AIT section data size in number of bytes
    */
   virtual void OnAitSectionReceived(unsigned short aitPid, unsigned short serviceId, unsigned char *aitSectionData, unsigned int aitSectionDataSize) = 0;

   /**
    * Notify the application manager that the current channel's status has changed.
    * Also dispatch the ChannelStatusChanged bridge event to the current page's JavaScript context.
    *
    * The channel status (statusCode) must be set to one of the predefined values in Channel.h.
    *
    * @param onetId         The original network id
    * @param transId        The transport stream id
    * @param servId         The service id
    * @param statusCode     The channel status code (value from Channel::Status or Channel::ErrorState)
    * @param permanentError Permanent error indicator
    */
   virtual void OnChannelStatusChanged(int onetId, int transId, int servId, int statusCode, bool permanentError) = 0;

   /**
    * Dispatch the ParentalRatingChange bridge event to the current page's JavaScript context.
    *
    * @param blocked Indicates if the current service is blocked by the parental control system
    */
   virtual void OnParentalRatingChanged(bool blocked) = 0;

   /**
    * Dispatch the ParentalRatingError bridge event to the current page's JavaScript context.
    */
   virtual void OnParentalRatingError() = 0;

   /**
    * Dispatch the SelectedComponentChanged bridge event to the current page's JavaScript context.
    *
    * @param componentType The component type (0: video, 1: audio, 2: subtitle)
    */
   virtual void OnSelectedComponentChanged(int componentType) = 0;

   /**
    * Dispatch the ComponentChanged bridge event to the current page's JavaScript context.
    *
    * @param componentType The component type (0: video, 1: audio, 2: subtitle)
    */
   virtual void OnComponentChanged(int componentType) = 0;

   /**
    * Dispatch the ProgrammesChanged bridge event to the current page's JavaScript context.
    */
   virtual void OnProgrammesChanged() = 0;

   /**
    * Dispatch the LowMemory bridge event to the current page's JavaScript context.
    */
   virtual void OnLowMemory() = 0;

   /**
    * Dispatch the accesstodistinctiveidentifier bridge event to the current page's JavaScript context.
    *
    * @param origin        The origin of the requesting application
    * @param accessAllowed True if access allowed, false otherwise
    */
   virtual void OnAccessToDistinctiveIdentifierDecided(std::string origin, bool accessAllowed) = 0;

   /**
    * Dispatch the TransitionedToBroadcastRelated bridge event to the current page's JavaScript context.
    */
   virtual void OnAppTransitionedToBroadcastRelated() = 0;

   /**
    * Dispatch the StreamEvent bridge event to the current page's JavaScript context.
    *
    * @param id     The stream event id
    * @param name   The stream event name
    * @param data   The stream event data encoded in Hexadecimal
    * @param text   The stream event data encoded in UTF-8
    * @param status The stream event status
    */
   virtual void OnStreamEvent(int id, std::string name, std::string data, std::string text, std::string status) = 0;

   /**
    * Notify all subscribers that the specified DVB URL load has finished.
    *
    * @param requestId         The request identifier
    * @param fileContent       The file content
    * @param fileContentLength The file content length in number of bytes
    */
   virtual void OnDvbUrlLoaded(int requestId, unsigned short int *fileContent, unsigned int fileContentLength) = 0;

   /**
    * Notify the browser that the specified input key was generated.
    *
    * @param keyCode The JavaScript key code
    */
   virtual void OnInputKeyGenerated(int keyCode) = 0;
}; // class ORBPlatformEventHandler
} // namespace orb
