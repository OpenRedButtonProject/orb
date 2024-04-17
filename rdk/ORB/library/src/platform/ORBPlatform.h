/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ORB_PLATFORM_H
#define ORB_PLATFORM_H

#include "ORBPlatformEventHandler.h"
#include "Capabilities.h"
#include "Channel.h"
#include "Component.h"
#include "LocalSystem.h"
#include "Programme.h"
#include "DisplayInfo.h"

#include <map>
#include <memory>
#include <vector>

using namespace orb;

/**
 * Interface of the ORB platform to be implemented by the ORB integrator.
 * ORB core interacts with the ORB platform implementation exclusively via this interface.
 */
class ORBPlatform
{
public:

    /**
     * Virtual destructor.
     */
    virtual ~ORBPlatform()
    {
    }

    /**
     * Perform any platform-specific initialisation tasks.
     *
     * @param platformEventHandler Pointer to the platform event handler
     */
    virtual void Platform_Initialise(
        std::shared_ptr<ORBPlatformEventHandler> platformEventHandler) = 0;

    /**
     * Perform any platform-specific finalisation tasks.
     */
    virtual void Platform_Finalise() = 0;

    /**
     * Map the given, potentially platform-specific key code into the proper, HbbTV-compliant value.
     *
     * @param keyCode The key code to be mapped
     *
     * @return The mapped value
     */
    virtual unsigned int Platform_MapKeyCode(unsigned int keyCode) = 0;

    /**
     * Let the ORB platform know of the current HbbTV app's keyset mask.
     *
     * @param keySetMask The keyset mask
     */
    virtual void Platform_SetCurrentKeySetMask(uint16_t keySetMask) = 0;

    /**
     * Check if the specified key code corresponds to the EXIT (or similar) button in the
     * RCU of the underlying platform.
     *
     * @param keyCode The key code to be checked
     *
     * @return True if the specified key code corresponds to the EXIT button, false otherwise
     */
    virtual bool Platform_IsExitButton(unsigned int keyCode) = 0;


    /******************************************************************************
    ** Application API
    *****************************************************************************/


    /**
     * Load the specified HbbTV application.
     *
     * @param url The HbbTV application URL
     *
     * @return true in success, false otherwise
     */
    virtual bool Application_Load(std::string url) = 0;

    /**
     * Set the visibility of the current HbbTV application (if any).
     *
     * @param visible Set to true to show the application, or false to hide the application
     *
     * @return true in success, false otherwise
     */
    virtual bool Application_SetVisible(bool visible) = 0;



    /******************************************************************************
    ** Network API
    *****************************************************************************/



    /**
     * Check if the device is currently connected to the Internet.
     *
     * @return true if connected, false otherwise
     */
    virtual bool Network_IsConnectedToInternet() = 0;


    /**
     * Resolves netrwork error by passing the response status text received.
     * 
     * @param responseText 
     * @return std::string the dash DVBError code
     */
    virtual std::string Network_ResolveNetworkError(std::string responseText) = 0;
    /******************************************************************************
    ** Broadcast API
    *****************************************************************************/



    /**
     * Set the broadcasted video playback window.
     *
     * @param x      The x-position of the window
     * @param y      The y-position of the window
     * @param width  The window width
     * @param height The window height
     */
    virtual void Broadcast_SetVideoRectangle(int x, int y, int width, int height) = 0;

    /**
     * Get the currently tuned broadcast channel.
     * If there is no currently tuned channel, then the returned Channel entity
     * shall have an empty ccid.
     *
     * @return The current channel
     */
    virtual std::shared_ptr<Channel> Broadcast_GetCurrentChannel() = 0;

    /**
     * Get the scanned channel list.
     *
     * @return A vector with the scanned channels
     */
    virtual std::vector<Channel> Broadcast_GetChannelList() = 0;

    /**
     * Select the broadcast channel (e.g. tune) with the given CCID.
     *
     * Security: FOR_RUNNING_APP_ONLY.
     *
     * @param ccid                         The CCID of the channel to set.
     * @param trickplay                    True if the application has optionally hinted trickplay resources are
     *                                     required; or false otherwise. Does not affect the success of this operation.
     * @param contentAccessDescriptorURL   Optionally, additional information for DRM-protected IPTV
     *                                     broadcasts; or an empty string otherwise.
     * @param quiet                        Type of channel change: 0 for normal; 1 for normal, no UI; 2 for quiet (HbbTV
     *                                     A.2.4.3.2).
     *
     * @return A CHANNEL_STATUS_* code (on success, the code has a value less than 0).
     */
    virtual int Broadcast_SetChannelToCcid(
        std::string ccid,
        bool trickplay,
        std::string contentAccessDescriptorURL,
        int quiet
        ) = 0;

    /**
     * Select a logically null broadcast channel (e.g. tune off).
     *
     * When a logically null broadcast channel is selected, the Application Manager must transition
     * the running application to broadcast-independent or kill it, depending on the signalling.
     *
     * Security: FOR_RUNNING_APP_ONLY.
     *
     * @return A CHANNEL_STATUS_* code (on success, the code has a value less than 0).
     */
    virtual int Broadcast_SetChannelToNull() = 0;

    /**
     * Select the given broadcast channel (e.g. tune) with the given triplet and information.
     *
     * Security: FOR_RUNNING_APP_ONLY.
     *
     * @param idType                       The type of the channel to set (ID_* code).
     * @param onid                         The original network ID of the channel to set.
     * @param tsid                         The transport stream ID of the channel to set.
     * @param sid                          The service ID of the channel to set.
     * @param sourceID                     Optionally, the ATSC source_ID of the channel to set; or -1 otherwise.
     * @param ipBroadcastID                Optionally, the DVB textual service ID of the (IP broadcast) channel
     *                                     to set; or an empty string otherwise.
     * @param trickplay                    True if the application has optionally hinted trickplay resources are
     *                                     required; or false otherwise. Does not affect the success of this operation.
     * @param contentAccessDescriptorURL   Optionally, additional information for DRM-protected IPTV
     *                                     broadcasts; or an empty string otherwise.
     * @param quiet                        Type of channel change: 0 for normal, 1 for normal no UI, 2 for quiet (HbbTV
     *                                     A.2.4.3.2).
     *
     * @return A CHANNEL_STATUS_* code (on success, the code has a value less than 0).
     */
    virtual int Broadcast_SetChannelToTriplet(
        int idType,
        int onid,
        int tsid,
        int sid,
        int sourceID,
        std::string ipBroadcastID,
        bool trickplay,
        std::string contentAccessDescriptorURL,
        int quiet
        ) = 0;

    /**
     * Select the broadcast channel with the given DSD. 8 Security: FOR_RUNNING_APP_ONLY.
     *
     * @param dsd                          The DSD of the channel to set.
     * @param sid                          The service ID of the channel to set.
     * @param trickplay                    True if the application has optionally hinted trickplay resources are
     *                                     required; or false otherwise. Does not affect the success of this operation.
     * @param contentAccessDescriptorURL   Optionally, additional information for DRM-protected IPTV
     *                                     broadcasts; or an empty string otherwise.
     * @param quiet                        Type of channel change: 0 for normal, 1 for normal no UI, 2 for quiet (HbbTV
     *                                     A.2.4.3.2).
     *
     * @return A CHANNEL_STATUS_* code (on success, the code has a value less than 0).
     */
    virtual int Broadcast_SetChannelToDsd(
        std::string dsd,
        int sid,
        bool trickplay,
        std::string contentAccessDescriptorURL,
        int quiet
        ) = 0;

    /**
     * Get the programmes of the channel identified by the given ccid.
     *
     * @param ccid The channel ccid
     *
     * @return A vector with the channel programmes
     */
    virtual std::vector<Programme> Broadcast_GetProgrammes(std::string ccid) = 0;

    /**
     * Get the components of the channel identified by the given ccid.
     *
     * @param ccid          The channel ccid
     * @param componentType Component filter (-1: any, 0: video, 1: audio, 2: subtitle)
     *
     * @return A vector with the matching channel components
     */
    virtual std::vector<Component> Broadcast_GetComponents(std::string ccid, int componentType) = 0;

    /**
     * Get a private audio component in the selected channel.
     *
     * Security: FOR_BROADCAST_APP_ONLY
     *
     * @param componentTag The component_tag of the component
     *
     * @return A pointer to the private component with the specified component_tag in the PMT of the
     * currently selected broadcast channel; or nullptr if unavailable or the component is not
     * private (i.e. the stream type is audio, video or subtitle).
     *
     * Mandatory properties of the returned Component are: id, pid and encrypted.
     * The id property shall be usable with the Broadcast_OverrideComponentSelection method to
     * select the component as an audio track. Other Component properties are not required.
     */
    virtual std::shared_ptr<Component> Broadcast_GetPrivateAudioComponent(std::string
        componentTag) = 0;

    /**
     * Get a private video component in the selected channel.
     *
     * Security: FOR_BROADCAST_APP_ONLY
     *
     * @param componentTag The component_tag of the component
     *
     * @return A pointer to the private component with the specified component_tag in the PMT of the
     * currently selected broadcast channel; or nullptr if unavailable or the component is not
     * private (i.e. the stream type is audio, video or subtitle).
     *
     * Mandatory properties of the reutrned Component are: id, pid and encrypted.
     * The id property shall be usable with the Broadcast_OverrideComponentSelection method to
     * select the component as an video track. Other Component properties are not required.
     */
    virtual std::shared_ptr<Component> Broadcast_GetPrivateVideoComponent(std::string
        componentTag) = 0;

    /**
     * Override the default component selection of the terminal for the specified type.
     *
     * If id is empty, no component shall be selected for presentation (presentation is explicitly
     * disabled). Otherwise, the specified component shall be selected for presentation.
     *
     * If playback has already started, the presented component shall be updated.
     *
     * Default component selection shall be restored (revert back to the control of the terminal)
     * when: (1) the application terminates, (2) the channel is changed, (3) presentation has not
     * been explicitly disabled and the user selects another track in the terminal UI, or (4) the
     * Broadcast_RestoreComponentSelection method is called.
     *
     * Security: FOR_BROADCAST_APP_ONLY
     *
     * @param componentType  The component type (0: video, 1: audio, 2: subtitle)
     * @param id             A platform-defined component id or an empty string to disable presentation
     */
    virtual void Broadcast_OverrideComponentSelection(int componentType, std::string id) = 0;

    /**
     * Restore the default component selection of the terminal for the specified type.
     *
     * If playback has already started, the presented component shall be updated.
     *
     * Security: FOR_BROADCAST_APP_ONLY
     *
     * @param componentType The component type (0: video, 1: audio, 2: subtitle)
     */
    virtual void Broadcast_RestoreComponentSelection(int componentType) = 0;

    /**
     * Suspend/resume the presentation of the current broadcast playback.
     *
     * @param presentationSuspended Set to true to suspend, otherwise set to false to resume
     */
    virtual void Broadcast_SetPresentationSuspended(bool presentationSuspended) = 0;

    /**
     * Stop the current broadcast playback and call the OnBroadcastStopped event.
     */
    virtual void Broadcast_Stop() = 0;

    /**
     * Reset the current broadcast playback.
     */
    virtual void Broadcast_Reset() = 0;



    /******************************************************************************
    ** Configuration API
    *****************************************************************************/



    /**
     * Get the current capabilities of the terminal.
     *
     * @return Pointer to the Capabilities object
     */
    virtual std::shared_ptr<Capabilities> Configuration_GetCapabilities() = 0;

    /**
     * Get a list of audio profiles supported by the terminal, as defined by HBBTV 10.2.4.7 for
     * the audio_profile element.
     *
     * @return A vector of audio profiles supported by the terminal
     */
    virtual std::vector<AudioProfile> Configuration_GetAudioProfiles() = 0;

    /**
     * Get a list of video profiles supported by the terminal, as defined by HBBTV 10.2.4.7 for
     * the video_profile element.
     *
     * @return A vector of video profiles supported by the terminal
     */
    virtual std::vector<VideoProfile> Configuration_GetVideoProfiles() = 0;

    /**
     * If the terminal supports UHD, get a list that describes the highest quality video format the
     * terminal supports, as defined by HBBTV 10.2.4.7 for the video_display_format element;
     * otherwise get an empty list.
     *
     * Note: If the terminal changes its display format based on the content being played, multiple
     * elements may be included in the list when multiple frame rate families are usable or the
     * highest resolution does not support each highest quality parameter.
     *
     * @return A vector that describes the highest quality video format
     */
    virtual std::vector<VideoDisplayFormat> Configuration_GetVideoDisplayFormats() = 0;

    /**
     * Get the current number of additional media streams containing SD video accompanied by audio
     * that can be decoded and presented by an A/V control object or HTML5 media element.
     *
     * @return The current number of additional media streams. If the value is non-zero, then a call
     *         to play an A/V control object, HTML5 media element or video/broadcast object shall not fail
     *         due to lack of resources for SD media.
     */
    virtual int Configuration_GetExtraSDVideoDecodes() = 0;

    /**
     * Get the current number of additional media streams containing HD video accompanied by audio
     * that can be decoded and presented by an A/V control object or HTML5 media element.
     *
     * @return The current number of additional media streams. If the value is non-zero, then a call
     *         to play an A/V control object, HTML5 media element or video/broadcast object shall not fail
     *         due to lack of resources for HD media.
     */
    virtual int Configuration_GetExtraHDVideoDecodes() = 0;

    /**
     * Get the current number of additional media streams containing UHD video accompanied by audio
     * that can be decoded and presented by an A/V control object or HTML5 media element.
     *
     * @return The current number of additional media streams. If the value is non-zero, then a call
     *         to play an A/V control object, HTML5 media element or video/broadcast object shall not fail
     *         due to lack of resources for UHD media.
     */
    virtual int Configuration_GetExtraUHDVideoDecodes() = 0;

    /**
     * Get local system information.
     *
     * @return Pointer to the LocalSystem object
     */
    virtual std::shared_ptr<LocalSystem> Configuration_GetLocalSystem() = 0;

    /**
     * Get the preferred audio language.
     *
     * @return A comma-separated set of languages to be used for audio playback,
     *         in order of preference.Each language shall be indicated by its
     *         ISO 639-2 language code as defined in [ISO639-2].
     */
    virtual std::string Configuration_GetPreferredAudioLanguage() = 0;

    /**
     * Get the preferred subtitle language.
     *
     * @return A comma-separated set of languages to be used for subtitle playback,
     *         in order of preference. Each language shall be indicated by its
     *         ISO 639-2 language code as defined in [ISO639-2] or as a wildcard
     *         specifier "***".
     */
    virtual std::string Configuration_GetPreferredSubtitleLanguage() = 0;

    /**
     * Get the preferred UI language.
     *
     * @return Comma separated string of languages (ISO 639-2 codes), in order of preference.
     */
    virtual std::string Configuration_GetPreferredUILanguage() = 0;

    /**
     * Get the id of the country in which the receiver is deployed.
     *
     * @return An ISO-3166 three character country code identifying the country in
     *         which the receiver is deployed.
     */
    virtual std::string Configuration_GetCountryId() = 0;

    /**
     * Get the flag indicating whether the subtitles are enabled or not.
     *
     * @return true if subtitles are enabled, otherwise false
     */
    virtual bool Configuration_GetSubtitlesEnabled() = 0;

    /**
     * Get the flag indicating whether the audio description is enabled or not.
     *
     * @return true if the audio description is enabled, otherwise false
     */
    virtual bool Configuration_GetAudioDescriptionEnabled() = 0;

    /**
     * Get the device identifier.
     *
     * @return The device identifier
     */
    virtual std::string Configuration_GetDeviceId() = 0;

    /**
     * Called when the application at origin requests access to the distinctive identifier.
     *
     * @param origin   The origin of the application
     * @param appNames The map of <lang,name> entries of the application
     *
     * @return true if access already granted, false otherwise
     */
    virtual bool Configuration_RequestAccessToDistinctiveIdentifier(std::string origin,
        std::map<std::string, std::string> appNames) = 0;

    /**
     * Get the User-Agent string to be used by the browser.
     *
     * @return The User-Agent string
     */
    virtual std::string Configuration_GetUserAgentString() = 0;

#ifdef BBC_API_ENABLE
    /**
     * Get a report of the device's primary display capabilities in accordance with the BBC TV
     * Platform Certification specs.
     *
     * @return A pointer to the corresponding DisplayInfo object
     */
    virtual std::shared_ptr<DisplayInfo> Configuration_GetPrimaryDisplay() = 0;
#endif

    /******************************************************************************
    ** DSM-CC API
    *****************************************************************************/



    /**
     * Request the specified DVB file from the DSM-CC implementation.
     *
     * @param url       The URL of the requested DVB file
     * @param requestId The unique request identifier
     */
    virtual void Dsmcc_RequestFile(std::string url, int requestId) = 0;

    /**
     * Request notifications from the DSM-CC implementation when a named stream event occurs.
     *
     * @param url      The stream URL
     * @param name     The stream event name
     * @param listenId The reference id of the subscriber
     *
     * @return true in success, false otherwise
     */
    virtual bool Dsmcc_SubscribeToStreamEventByName(std::string url, std::string name, int
        listenId) = 0;

    /**
     * Request notifications from the DSM-CC implementation whenever the named event with the given id occurs.
     *
     * @param name         The stream event name
     * @param componentTag The stream component tag
     * @param eventId      The stream event id
     * @param listenId     The reference id of the subscriber
     *
     * @return true in success, false otherwise
     */
    virtual bool Dsmcc_SubscribeStreamEventId(std::string name, int componentTag, int eventId, int
        listenId) = 0;

    /**
     * Unsubscribe from all previously establishe stream event subscriptions with the DSM-CC implementation.
     *
     * @param listenId The reference id of the subscriber
     */
    virtual void Dsmcc_UnsubscribeFromStreamEvents(int listenId) = 0;


    /**
     * Request the carousel id of current service.
     *
     * @param componentTag the component tag provided via dvburl
     *
     * @return the carousel id
     */
    virtual uint32_t Dsmcc_RequestCarouselId(uint32_t componentTag) = 0;

    /******************************************************************************
    ** Manager API
    ******************************************************************************/



    /**
     * Get the location of the icon file that corresponds to the given input key code.
     *
     * @param keyCode The input key code
     *
     * @return The location of the icon file or an empty string if there is no such file
     */
    virtual std::string Manager_GetKeyIcon(int keyCode) = 0;



    /******************************************************************************
    ** ParentalControl API
    *****************************************************************************/



    /**
     * Return the current age set for parental control. 0 will be returned if parental control is
     * disabled or no age is set.
     *
     * @return The currently set parental control age
     */
    virtual int ParentalControl_GetAge() = 0;

    /**
     * Return the region set for parental control.
     *
     * @return The region country using the 2-character code as specified in ISO 3166
     */
    virtual std::string ParentalControl_GetRegion() = 0;

    /**
     * Return the region set for parental control.
     *
     * @return The region country using the 3-character code as specified in ISO 3166
     */
    virtual std::string ParentalControl_GetRegion3() = 0;

    /**
     * Get the rating schemes supported by the system.
     *
     * @return The rating schemes
     */
    virtual std::map<std::string,
                     std::vector<ParentalRating> > ParentalControl_GetRatingSchemes() = 0;

    /**
     * Get the parental control threshold for the given parental rating scheme.
     *
     * @param scheme The parental rating scheme
     *
     * @return A ParentalRating object representing the parental control threshold
     */
    virtual std::shared_ptr<ParentalRating> ParentalControl_GetThreshold(std::string scheme) = 0;

    /**
     * Retrieve the blocked property for the provided parental rating.
     *
     * @param scheme The parental rating scheme
     * @param region The parental rating 2-character region
     * @param value  The parental rating control age value
     *
     * @return The blocked property
     */
    virtual bool ParentalControl_IsRatingBlocked(std::string scheme, std::string region, int
        value) = 0;



    /******************************************************************************
    ** Programme API
    *****************************************************************************/



    /**
     * Retrieve raw SI descriptor data with the defined descriptor tag id, and optionally the
     * extended descriptor tag id, for an event on a service.
     *
     * @param ccid                   CCID for the required channel
     * @param programmeId            Event ID for the required programme
     * @param descriptorTag          Descriptor tag ID of data to be returned
     * @param descriptorTagExtension Optional extended descriptor tag ID of data to be returned, or -1
     * @param privateDataSpecifier   Optional private data specifier of data to be returned, or 0
     *
     * @return The buffer containing the data. If there are multiple descriptors with the same
     *         tag id then they will all be returned.
     */
    virtual std::vector<std::string> Programme_GetSiDescriptors(
        std::string ccid,
        std::string programmeId,
        int descriptorTag,
        int descriptorTagExtension,
        int privateDataSpecifier
        ) = 0;



    /******************************************************************************
    ** Drm API
    *****************************************************************************/



    /**
     * Get the list of supported DRM System IDs currently available. Once called,
     * the caller can track the availability changes by listening to OnDrmSystemStatusChanged
     * events.
     *
     * @return A vector containing the supported DRM systems and their statuses
     */
    virtual std::vector<DrmSystemStatus> Drm_GetSupportedDrmSystemIds() = 0;

    /**
     * Send message to the specified DRM system.
     *
     * @param messageId   Unique identifier of the message
     * @param messageType Message type as defined by the DRM system
     * @param message     Message to be provided to the DRM system
     * @param drmSystemId ID of the DRM system
     * @param blocked     Whether the function needs to block until the reply is received
     *
     * @return Result message when block is true, ignored otherwise
     */
    virtual std::string Drm_SendDrmMessage(
        std::string messageId,
        std::string messageType,
        std::string message,
        std::string drmSystemId,
        bool blocked
        ) = 0;

    /**
     * Check the availability of a valid license for playing a protected content item.
     *
     * @param drmPrivateData DRM proprietary private data
     * @param drmSystemId    DRM system ID
     *
     * @return true if the content can be played, false otherwise
     */
    virtual bool Drm_CanPlayContent(std::string drmPrivateData, std::string drmSystemId) = 0;

    /**
     * Check the availability of a valid license for recording a protected content item.
     *
     * @param drmPrivateData DRM proprietary private data
     * @param drmSystemId    DRM system ID
     *
     * @return true if the content can be recorded, false otherwise
     */
    virtual bool Drm_CanRecordContent(std::string drmPrivateData, std::string drmSystemId) = 0;

    /**
     * Set the DRM system that the terminal shall use for playing protected broadband content.
     *
     * @param drmSystemId ID of the DRM system
     *
     * @return true if the call was successful, false otherwise
     */
    virtual bool Drm_SetActiveDrm(std::string drmSystemId) = 0;
}; // class ORBPlatform

/**
 * Platform implementations must implement a Create method of this type.
 * The Create method is called by the ORB core while loading the ORB platform implementation.
 */
typedef ORBPlatform *CreatePlatformInstance_t ();

/**
 * Platform implementations must implement a Destroy method of this type.
 * The Destroy method is called by the ORB core while unloading the ORB platform implementation.
 */
typedef void DestroyPlatformInstance_t (void *);


#endif // ORB_PLATFORM_H
