#pragma once

#include "ORBPlatformEventHandler.h"

namespace orb
{
/**
 * Implementation of the ORB platform event handler.
 */
class ORBPlatformEventHandlerImpl : public ORBPlatformEventHandler
{
public:

   /**
    * Constructor.
    */
   ORBPlatformEventHandlerImpl();

   /**
    * Destructor.
    */
   ~ORBPlatformEventHandlerImpl();

public:

   /**
    * Notify the application manager that the broadcast playback has stopped.
    */
   virtual void OnBroadcastStopped() override final;

   /**
    * Notify the application manager that an AIT section was received.
    *
    * @param aitPid             The AID PID
    * @param serviceId          The corresponding service id
    * @param aitSectionData     The AIT section data
    * @param aitSectionDataSize The AIT section data size in number of bytes
    */
   virtual void OnAitSectionReceived(unsigned short aitPid, unsigned short serviceId, unsigned char *aitSectionData, unsigned int aitSectionDataSize) override final;

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
   virtual void OnChannelStatusChanged(int onetId, int transId, int servId, int statusCode, bool permanentError) override final;

   /**
    * Dispatch the ParentalRatingChange bridge event to the current page's JavaScript context.
    *
    * @param blocked Indicates if the current service is blocked by the parental control system
    */
   virtual void OnParentalRatingChanged(bool blocked) override final;

   /**
    * Dispatch the ParentalRatingError bridge event to the current page's JavaScript context.
    *
    * @param contentId   Content ID to which the parental rating error applies
    * @param ratings     The parental rating value of the currently playing content
    * @param drmSystemId DRM System ID of the DRM system that generated the event
    */
   virtual void OnParentalRatingError(std::string contentId, std::vector<ParentalRating> ratings, std::string drmSystemId) override final;

   /**
    * Dispatch the SelectedComponentChanged bridge event to the current page's JavaScript context.
    *
    * @param componentType The component type (0: video, 1: audio, 2: subtitle)
    */
   virtual void OnSelectedComponentChanged(int componentType) override final;

   /**
    * Dispatch the ComponentChanged bridge event to the current page's JavaScript context.
    *
    * @param componentType The component type (0: video, 1: audio, 2: subtitle)
    */
   virtual void OnComponentChanged(int componentType) override final;

   /**
    * Dispatch the ProgrammesChanged bridge event to the current page's JavaScript context.
    */
   virtual void OnProgrammesChanged() override final;

   /**
    * Dispatch the LowMemory bridge event to the current page's JavaScript context.
    */
   virtual void OnLowMemory() override final;

   /**
    * Dispatch the accesstodistinctiveidentifier bridge event to the current page's JavaScript context.
    *
    * @param origin        The origin of the requesting application
    * @param accessAllowed True if access allowed, false otherwise
    */
   virtual void OnAccessToDistinctiveIdentifierDecided(std::string origin, bool accessAllowed) override final;

   /**
    * Dispatch the TransitionedToBroadcastRelated bridge event to the current page's JavaScript context.
    */
   virtual void OnAppTransitionedToBroadcastRelated() override final;

   /**
    * Dispatch the StreamEvent bridge event to the current page's JavaScript context.
    *
    * @param id     The stream event id
    * @param name   The stream event name
    * @param data   The stream event data encoded in Hexadecimal
    * @param text   The stream event data encoded in UTF-8
    * @param status The stream event status
    */
   virtual void OnStreamEvent(int id, std::string name, std::string data, std::string text, std::string status) override final;

   /**
    * Notify all subscribers that the specified DVB URL load has finished.
    *
    * @param requestId         The request identifier
    * @param fileContent       The file content
    * @param fileContentLength The file content length in number of bytes
    */
   virtual void OnDvbUrlLoaded(int requestId, std::vector<uint8_t> fileContent, unsigned int fileContentLength) override final;

   /**
    * Notify all subscribers that the specified DVB URL load has finished. The content is not passed
    *
    * @param requestId         The request identifier
    * @param fileContentLength The file content length in number of bytes
    */
   virtual void OnDvbUrlLoadedNoData(int requestId, unsigned int fileContentLength) override final;

   /**
    * Notify the browser that the specified input key was generated.
    *
    * @param keyCode   The JavaScript key code
    * @param keyAction The key action (0 = keyup , 1 = keydown)
    */
   virtual bool OnInputKeyGenerated(int keyCode, KeyAction keyAction) override final;

   /**
    * Notify the browser about DRM licensing errors during playback of DRM protected A/V content.
    *
    * @param errorState      Details the type of error
    * @param contentId       Unique identifier of the protected content
    * @param drmSystemId     ID of the DRM system
    * @param rightsIssuerUrl Indicates the value of the rightsIssuerURL that can be used to
    *                        non-silently obtain the rights for the content item
    */
   virtual void OnDrmRightsError(
      DrmRightsError errorState,
      std::string contentId,
      std::string drmSystemId,
      std::string rightsIssuerUrl
      ) override final;

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
   virtual void OnDrmSystemStatusChanged(
      std::string drmSystem,
      std::vector<std::string> drmSystemIds,
      DrmSystemStatus::State status,
      std::string protectionGateways,
      std::string supportedFormats
      ) override final;

   /**
    * Notify the browser that the underlying DRM system has a result message as a consequence
    * of a call to Drm_SendDrmMessage.
    *
    * @param messageId  Identifies the original message which has led to this resulting message
    * @param result     DRM system specific result message
    * @param resultCode Result code
    */
   virtual void OnSendDrmMessageResult(
      std::string messageId,
      std::string result,
      SendDrmMessageResultCode resultCode
      ) override final;

   /**
    * Notify the browser that the underlying DRM system has a message to report.
    *
    * @param message     DRM system specific message
    * @param drmSystemId ID of the DRM System
    */
   virtual void OnDrmSystemMessage(std::string message, std::string drmSystemId) override final;
}; // class ORBPlatformEventHandlerImpl
} // namespace orb
