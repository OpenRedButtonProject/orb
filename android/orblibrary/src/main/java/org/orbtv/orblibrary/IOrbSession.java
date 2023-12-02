package org.orbtv.orblibrary;

import android.util.Log;
import android.view.KeyEvent;
import android.view.View;

import org.json.JSONObject;
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
     * @param aitPid    PID of the AIT
     * @param serviceId Service ID the AIT refers to
     * @param data      The buffer containing the AIT row data
     */
    void processAitSection(int aitPid, int serviceId, byte[] data);

    /**
     * TODO(comment)
     *
     * @param xmlAit
     */
    void processXmlAit(String xmlAit);

    /**
     * Returns whether a Teletext application is signalled in the current AIT.
     *
     * @return True if a Teletext application is signalled, false otherwise.
     */
    boolean isTeletextApplicationSignalled();

    /**
     * Launch the Teletext application signalled in the current AIT (e.g., when the user presses the
     * TEXT key).
     *
     * @return True if the application is launched, false otherwise.
     */
    boolean launchTeletextApplication();

    /**
     * Called to Tell the browser to dispatch an key press event.
     *
     * @param event The KeyEvent, with an action and a KeyCode, to be handled
     * @return true if the event is handled
     */
    boolean dispatchKeyEvent(KeyEvent event);

    /**
     * Called to Tell the browser to dispatch an text input.
     *
     * @param text The content of the text input
     */
    void dispatchTextInput(String text);

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
     * @param onetId         Original Network ID
     * @param transId        Transport Stream ID
     * @param servId         Service ID
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
     * @param origin        The origin of the requesting application
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
     * @param errorState      details the type of error:
     *                        - 0: no license, consumption of the content is blocked.
     *                        - 1: invalid license, consumption of the content is blocked.
     *                        - 2: valid license, consumption of the content is unblocked.
     * @param contentID       unique identifier of the protected content
     * @param DRMSystemID     ID of the DRM System
     * @param rightsIssuerURL indicates the value of the rightsIssuerURL that can
     *                        be used to non-silently obtain the rights for the content item
     */
    void onDRMRightsError(int errorState, String contentID, String DRMSystemID,
                          String rightsIssuerURL);

    /**
     * Called when the status of a DRM system changes.
     *
     * @param drmSystem          ID of the DRM System
     * @param drmSystemIds       List of the DRM System IDs handled by the DRM System
     * @param status             status of the indicated DRM system. Possible states:
     *                           - 0 READY, fully initialised and ready
     *                           - 1 UNKNOWN, no longer available
     *                           - 2 INITIALISING, initialising and not ready to communicate
     *                           - 3 ERROR, in error state
     * @param protectionGateways space separated list of zero or more CSP Gateway
     *                           types that are capable of supporting the DRM system
     * @param supportedFormats   space separated list of zero or more supported
     *                           file and/or container formats by the DRM system
     */
    void onDRMSystemStatusChange(String drmSystem, List<String> drmSystemIds, int status,
                                 String protectionGateways, String supportedFormats);

    /**
     * Called when the underlying DRM system has a result message as a consequence
     * of a call to sendDRMMessage.
     *
     * @param msgID      identifies the original message which has led to this resulting message
     * @param resultMsg  DRM system specific result message
     * @param resultCode result code. Valid values include:
     *                   - 0 Successful
     *                   - 1 Unknown error
     *                   - 2 Cannot process request
     *                   - 3 Unknown MIME type
     *                   - 4 User consent needed
     *                   - 5 Unknown DRM system
     *                   - 6 Wrong format
     */
    void onDRMMessageResult(String msgID, String resultMsg, int resultCode);

    /**
     * Called when the underlying DRM system has a message to report.
     *
     * @param msg         DRM system specific message
     * @param DRMSystemID ID of the DRM System
     */
    void onDRMSystemMessage(String msg, String DRMSystemID);

    /**
     * Called by IDsmcc on receiving content
     *
     * @param requestId ID of request
     * @param buffer    ByteBuffer with content for DSMCC file
     */
    void onDsmccReceiveContent(int requestId, ByteBuffer buffer);

    /**
     * Called by IDsmcc on receiving Stream Event
     *
     * @param listenId ID of listener
     * @param name     Name of Stream event
     * @param data     Data asssociated with stream event
     */
    void onDsmccReceiveStreamEvent(int listenId, String name, String data, String text, String status);

    /**
     * TODO(library) What makes sense here?
     */
    void close();

    /**
     * Called to send a response message for a result of overriding dialogue enhancement
     *
     * @param connection              The request and response should have the same value
     * @param id                      The request and response should have the same value
     * @param dialogueEnhancementGain The applied gain value in dB of the dialogue enhancement
     */
    void onRespondDialogueEnhancementOverride(int connection, String id,
                                              int dialogueEnhancementGain);

    /**
     * Called to send a response message for a result of trigger response to user action
     *
     * @param connection The request and response should have the same value
     * @param id         The request and response should have the same value
     * @param actioned   The result of user action mechanism
     *                   - true: successfully triggered
     *                   - false: unsuccessfully triggered
     */
    void onRespondTriggerResponseToUserAction(int connection, String id, boolean actioned);

    /**
     * Called to send a response message for the support information of a feature
     *
     * @param connection The request and response should have the same value
     * @param id         The request and response should have the same value
     * @param feature    The index of a particular accessibility feature
     * @param result     The result code of the support for the accessibility feature
     */
    void onRespondFeatureSupportInfo(int connection, String id, int feature, String result);

    /**
     * Called to send a response message for suppressing the support of a feature
     *
     * @param connection The request and response should have the same value
     * @param id         The request and response should have the same value
     * @param feature    The index of a particular accessibility feature
     * @param value      The result code for suppressing
     */
    void onRespondFeatureSuppress(int connection, String id, int feature, String value);

    /**
     * Called to send an error message
     *
     * @param connection The request and response should have the same value
     * @param id         The request and response should have the same value
     * @param code       The error code
     * @param message    The error message
     */
    void onRespondError(int connection, String id, int code, String message);

    /**
     * Called to send an error message with some data
     *
     * @param connection The request and response should have the same value
     * @param id         The request and response should have the same value
     * @param code       The error code
     * @param message    The error message
     * @param data       The error data
     */
    void onRespondError(int connection, String id, int code, String message, String data);

    /**
     * Called to send a message with the user settings of subtitles
     *
     * @param connection        The request and response should have the same value
     * @param id                The request and response should have the same value
     *                          - Not empty: a message of user settings query
     *                          - Empty: a message of notification
     * @param enabled           Enabled subtitles
     * @param size              The font size
     * @param fontFamily        The description of the font family
     * @param textColour        The text colour in RGB24 format
     * @param textOpacity       The test opacity with the percentage from 0 to 100
     * @param edgeType          The description of edge type
     * @param edgeColour        The edge colour in RGB24 format
     * @param backgroundColour  The background colour in RGB24 format
     * @param backgroundOpacity The background opacity with the percentage from 0 to 100
     * @param windowColour      The window colour in RGB24 format
     * @param windowOpacity     The window opacity with the percentage from 0 to 100
     * @param language          The description of language in ISO639-2 3-character code
     */
    void onQuerySubtitles(int connection, String id, boolean enabled,
                          int size, String fontFamily, String textColour, int textOpacity,
                          String edgeType, String edgeColour,
                          String backgroundColour, int backgroundOpacity,
                          String windowColour, int windowOpacity, String language);

    /**
     * Called to send a message with the settings of dialogue enhancement
     *
     * @param connection     The request and response should have the same value
     * @param id             The request and response should have the same value
     *                       - Not empty: a message of user settings query
     *                       - Empty: a message of notification
     * @param gainPreference The dialogue enhancement gain preference in dB
     * @param gain           The currently-active gain value in dB
     * @param limitMin       The current allowed minimum gain value in dB
     * @param limitMax       The current allowed maximum gain value in dB
     */
    void onQueryDialogueEnhancement(int connection, String id, int gainPreference, int gain,
                                    int limitMin, int limitMax);

    /**
     * Called to send a message with the settings of a user Interface Magnification feature
     *
     * @param connection The request and response should have the same value
     * @param id         The request and response should have the same value
     *                   - Not empty: a message of user settings query
     *                   - Empty: a message of notification
     * @param enabled    Enabled a screen magnification UI setting
     * @param magType    The description of the type of magnification scheme currently set
     */
    void onQueryUIMagnifier(int connection, String id, boolean enabled, String magType);

    /**
     * Called to send a message with the settings of a high contrast UI feature
     *
     * @param connection The request and response should have the same value
     * @param id         The request and response should have the same value
     *                   - Not empty: a message of user settings query
     *                   - Empty: a message of notification
     * @param enabled    Enabled a high contrast UI
     * @param hcType     The description of the type of high contrast scheme currently set
     */
    void onQueryHighContrastUI(int connection, String id, boolean enabled, String hcType);

    /**
     * Called to send a message with the settings of a screen reader feature
     *
     * @param connection The request and response should have the same value
     * @param id         The request and response should have the same value
     *                   - Not empty: a message of user settings query
     *                   - Empty: a message of notification
     * @param enabled    Enabled a screen reader preference
     * @param speed      A percentage scaling factor of the default speech speed, 100% considered normal speed
     * @param voice      The description of the voice
     * @param language   The description of language in ISO639-2 3-character code
     */
    void onQueryScreenReader(int connection, String id,
                             boolean enabled, int speed, String voice, String language);

    /**
     * Called to send a message with the settings of a "response to a user action" feature
     *
     * @param connection The request and response should have the same value
     * @param id         The request and response should have the same value
     *                   - Not empty: a message of user settings query
     *                   - Empty: a message of notification
     * @param enabled    Enabled a "response to a user action" preference
     * @param type       The description of the mechanism the terminal uses to feedback to the user that the user action has occurred.
     */
    void onQueryResponseToUserAction(int connection, String id, boolean enabled, String type);

    /**
     * Called to send a message with the settings of an audio description feature
     *
     * @param connection           The request and response should have the same value
     * @param id                   The request and response should have the same value
     *                             - Not empty: a message of user settings query
     *                             - Empty: a message of notification
     * @param enabled              Enabled audio description
     * @param gainPreference       The audio description gain preference set by the user in dB.
     * @param panAzimuthPreference The degree of the azimuth pan preference set by the user
     */
    void onQueryAudioDescription(int connection, String id, boolean enabled,
                                 int gainPreference, int panAzimuthPreference);

    /**
     * Called to send a message with the settings of an in-vision signing feature
     *
     * @param connection The request and response should have the same value
     * @param id         The request and response should have the same value
     *                   - Not empty: a message of user settings query
     *                   - Empty: a message of notification
     * @param enabled    Enabled an in-vision signing preference
     */
    void onQueryInVisionSigning(int connection, String id, boolean enabled);

    /**
     * Called to send an intent for a request to operate the media playback
     *
     * @param cmd The index of a basic intent of media playback
     *            - 0: pause
     *            - 1: play
     *            - 2: fast-forward
     *            - 3: fast-reverse
     *            - 4: stop
     */
    void onSendIntentMediaBasics(int cmd);

    /**
     * Called to send an intent for a request to seek a time position relative to the start or end of the media content
     *
     * @param anchor The value indicates an anchor point of the content
     *               - "start": the start or end of the content
     *               - "end": the start or end of the content
     * @param offset The number value for the time position, a positive or negative number of seconds
     */
    void onSendIntentMediaSeekContent(String anchor, int offset);

    /**
     * Called to send an intent for a request to seek a time position relative to the current time of the media content
     *
     * @param offset The number value for the current time position, a positive or negative number of seconds
     */
    void onSendIntentMediaSeekRelative(int offset);

    /**
     * Called to send an intent for a request to seek a time position relative to the live edge of the media content
     *
     * @param offset The number value for the time position at or before the live edge, zero or negative number of seconds
     */
    void onSendIntentMediaSeekLive(int offset);

    /**
     * Called to send an intent for a request to seek a time position relating to absolute wall clock time
     *
     * @param dayTime The value conveys the wall clock time, in internet date-time format
     */
    void onSendIntentMediaSeekWallclock(String dayTime);

    /**
     * Called to send an intent to request a search of content available
     *
     * @param query The string value is the search term specified by the user.
     */
    void onSendIntentSearch(String query);

    /**
     * Called to send an intent to request a display (but not playback) of a specific identified piece of content
     *
     * @param mediaId The value for a URI uniquely identifying a piece of content
     */
    void onSendIntentDisplay(String mediaId);

    /**
     * Called to send an intent to request immediate playback of a specific identified piece of content
     *
     * @param mediaId The value for a URI uniquely identifying a piece of content
     *                === With meanings as seek-content ===
     * @param anchor  The value indicates an anchor point of the content
     *                - "start": the start or end of the content
     *                - "end": the start or end of the content
     * @param offset  The number value for the time position, a positive or negative number of seconds
     *                === With meaning as seek-live ===
     * @param offset  The number value for the time position at or before the live edge, zero or negative number of seconds
     */
    void onSendIntentPlayback(String mediaId, String anchor, int offset);

    /**
     * Request for the Description of the current media playback on the application
     */
    void onRequestMediaDescription();

}
