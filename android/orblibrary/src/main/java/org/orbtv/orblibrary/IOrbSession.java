package org.orbtv.orblibrary;

import android.view.View;

import org.orbtv.orbpolyfill.BridgeTypes;

import java.nio.ByteBuffer;
import java.util.List;

public interface IOrbSession {
   /**
    * Get the View of the TV browser session. This should be added to the content view of the
    * application.
    *
    * @return The View of the TV browser session.
    */
   View getView();

   /**
    * Get the URL of the inter-device sync service
    *
    * @return The URL of the inter-device sync service.
    */
   String getInterDevSyncUrl();

   /**
    * Get the base URL of the app2app local service.
    *
    * @return The base URL of the app2app remote service.
    */
   String getApp2AppLocalBaseUrl();

   /**
    * Get the base URL of the app2app remote service.
    *
    * @return The base URL of the app2app remote service.
    */
   String getApp2AppRemoteBaseUrl();

   /**
    * Launches a "Broadcast-INDEPENDENT" application, the url could be an XML-AIT file.
    *
    * @param url URL where application is to be found
    *
    * @return true if the application might be launched, false otherwise
    */
   boolean launchApplication(String url);

   /**
    * Requests the HbbTV engine to process the specified AIT. The HbbTV engine expects the relevant
    * AITs only (the first one after HBBTV_Start and when the version/PID changes). If more than one
    * stream is signalled in the PMT for a service with an application_signalling_descriptor, then
    * the application_signalling_descriptor for the stream containing the AIT for the HbbTV
    * application shall include the HbbTV application_type (0x0010).
    *
    * @param aitPid PID of the AIT
    * @param serviceId Service ID the AIT refers to
    * @param data The buffer containing the AIT row data
    */
   void processAitSection(int aitPid, int serviceId, byte[] data);

   /**
    * TODO(comment)
    *
    * @param xmlAit
    */
   void processXmlAit(String xmlAit);

   /**
    * Called when the service list has changed.
    */
   void onServiceListChanged();

   /**
    * Called when the parental rating of the currently playing service has changed.
    *
    * @param blocked TRUE if the current service is blocked by the parental control system.
    */
   void onParentalRatingChanged(boolean blocked);

   /**
    * Called when there is a parental rating error.
    */
   void onParentalRatingError(String contentID, List<BridgeTypes.ParentalRating> ratings,
      String DRMSystemID);

   /**
    * Called when there is a change in the set of components being presented.
    *
    * @param componentType Type of component whose presentation has changed.
    */
   void onSelectedComponentChanged(int componentType);

   /**
    * Called when there is a change in the set of components being presented.
    *
    * @param componentType Type of component whose presentation has changed.
    */
   void onComponentChanged(int componentType);

   /**
    * Called when there is a change in status of the service identified by the DVB triplet.
    *
    * @param onetId Original Network ID
    * @param transId Transport Stream ID
    * @param servId Service ID
    * @param statusCode
    * @param permanentError
    */
   void onChannelStatusChanged(int onetId, int transId, int servId, int statusCode,
      boolean permanentError);

   /**
    * Called when the present/following events have changed on the current service.
    */
   void onProgrammesChanged();

   /**
    * Called when the video aspect ratio has changed.
    *
    * @param aspectRatioCode Code as defined by TvBrowserTypes.ASPECT_RATIO_*
    */
   void onVideoAspectRatioChanged(int aspectRatioCode);

   /**
    * TODO(comment)
    */
   void onLowMemoryEvent();

   /**
    * TODO(comment)
    */
   void onTimelineUnavailableEvent(int filterId);

   /**
    * TODO(comment)
    */
   void onTimelineAvailableEvent(int filterId, long currentTime, long timescale, double speed);

   /**
    * TODO(comment)
    *
    * @param connected
    */
   void onNetworkStatusEvent(boolean connected);

   /**
    * Called when the user has decided whether the application at origin should be allowed access to
    * a distinctive identifier.
    *
    * @param origin The origin of the requesting application
    * @param accessAllowed true if access allowed, otherwise false
    */
   void onAccessToDistinctiveIdentifierDecided(String origin, boolean accessAllowed);

   /**
    * TODO(comment)
    *
    * @param search
    * @param status
    * @param programmes
    * @param offset
    * @param totalSize
    */
   void onMetadataSearchCompleted(int search, int status, List<BridgeTypes.Programme> programmes, int offset, int totalSize);

   /**
    * Notify about DRM licensing errors during playback of DRM protected A/V content.
    *
    * @param errorState details the type of error:
    *           - 0: no license, consumption of the content is blocked.
    *           - 1: invalid license, consumption of the content is blocked.
    *           - 2: valid license, consumption of the content is unblocked.
    * @param contentID unique identifier of the protected content
    * @param DRMSystemID ID of the DRM System
    * @param rightsIssuerURL indicates the value of the rightsIssuerURL that can
    *        be used to non-silently obtain the rights for the content item
    */
   void onDRMRightsError(int errorState, String contentID, String DRMSystemID,
      String rightsIssuerURL);

   /**
    * Called when the status of a DRM system changes.
    *
    * @param drmSystem ID of the DRM System
    * @param drmSystemIds List of the DRM System IDs handled by the DRM System
    * @param status status of the indicated DRM system. Possible states:
    *    - 0 READY, fully initialised and ready
    *    - 1 UNKNOWN, no longer available
    *    - 2 INITIALISING, initialising and not ready to communicate
    *    - 3 ERROR, in error state
    * @param protectionGateways space separated list of zero or more CSP Gateway
    *        types that are capable of supporting the DRM system
    * @param supportedFormats space separated list of zero or more supported
    *        file and/or container formats by the DRM system
    */
   void onDRMSystemStatusChange(String drmSystem, List<String> drmSystemIds, int status,
      String protectionGateways, String supportedFormats);

   /**
    * Called when the underlying DRM system has a result message as a consequence
    * of a call to sendDRMMessage.
    *
    * @param msgID identifies the original message which has led to this resulting message
    * @param resultMsg DRM system specific result message
    * @param resultCode result code. Valid values include:
    *    - 0 Successful
    *    - 1 Unknown error
    *    - 2 Cannot process request
    *    - 3 Unknown MIME type
    *    - 4 User consent needed
    *    - 5 Unknown DRM system
    *    - 6 Wrong format
    */
   void onDRMMessageResult(String msgID, String resultMsg, int resultCode);

   /**
    * Called when the underlying DRM system has a message to report.
    *
    * @param msg DRM system specific message
    * @param DRMSystemID ID of the DRM System
    */
   void onDRMSystemMessage(String msg, String DRMSystemID);

   /**
    * Called by IDsmcc on receiving content
    *
    * @param requestId ID of request
    * @param buffer ByteBuffer with content for DSMCC file
    */
   void onDsmccReceiveContent(int requestId, ByteBuffer buffer);

   /**
    * Called by IDsmcc on receiving Stream Event
    *
    * @param listenId ID of listener
    * @param name Name of Stream event
    * @param data Data asssociated with stream event
    */
   void onDsmccReceiveStreamEvent(int listenId, String name, String data, String text, String status);

   /**
    * TODO(library) What makes sense here?
    */
   void close();
}
