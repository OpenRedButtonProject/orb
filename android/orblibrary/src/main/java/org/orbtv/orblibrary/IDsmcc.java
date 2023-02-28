/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

package org.orbtv.orblibrary;

public interface IDsmcc {
   /**
    * Called by TvBrowser library at initialisation
    *
    * @param callback Callback to return DSMCC files and stream events
    */
   void setDsmccCallback(IDsmccCallback callback);

   /**
    * Request file from DSM-CC
    *
    * @param url DVB Url of requested file
    * @param requestId ID of request (returned to DsmccClient.receiveContent)
    */
   boolean requestDvbContent(String url, int requestId);

   /**
    * Release resources for DSM-CC file request
    *
    * @param requestId ID of request
    */
   void closeDvbContent(int requestId);

   /**
    * Subscribe to DSM-CC Stream Event with URL and event name
    *
    * @param url DVB Url of event object
    * @param name Name of stream event
    * @param listenId ID of subscriber
    */
   boolean subscribeStreamEventName(String url, String name, int listenId);

   /**
    * Subscribe to DSM-CC Stream Event with component tag and event ID
    *
    * @param name Name of stream event
    * @param componentTag Component tag for stream event
    * @param eventId Event Id of stream event
    * @param listenId ID of subscriber
    */
   boolean subscribeStreamEventId(String name, int componentTag, int eventId, int listenId);

   /**
    * Subscribe to DSM-CC Stream Event with component tag and event ID
    *
    * @param listenId ID of subscriber
    */
   void unsubscribeStreamEvent(int listenId);
}
