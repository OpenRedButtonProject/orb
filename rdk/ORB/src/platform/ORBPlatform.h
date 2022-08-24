/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

#ifndef ORB_PLATFORM_H
#define ORB_PLATFORM_H

#include "Channel.h"
#include "Component.h"
#include "LocalSystem.h"
#include "Programme.h"
#include "Query.h"

#include <map>
#include <memory>
#include <vector>

using namespace orb;

class ORBPlatform {

public:

/**
 * Perform any platform-specific initialisation tasks.
 */
virtual void Platform_Initialise() = 0;

/**
 * Perform any platform-specific finalisation tasks.
 */
virtual void Platform_Finalise() = 0;



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

/**
 * Send the specified input key event to the current HbbTV application (if any).
 *
 * @param keyCode The input key code
 *
 * @return true in success, false otherwise
 */
virtual bool Application_SendKeyEvent(int keyCode) = 0;



/******************************************************************************
 ** Network API
 *****************************************************************************/



/**
 * Check if the device is currently connected to the Internet.
 *
 * @return true if connected, false otherwise
 */
virtual bool Network_IsConnectedToInternet() = 0;



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
 * Tune to the specified broadcast channel.
 * 
 * @param channel                    The requested channel or nullptr
 * @param trickplay                  <currently not supported>
 * @param contentAccessDescriptorURL <currently not supported>
 * @param quiet                      <currently not supported>
 * @param channelChangeError         Channel change error code, set only in case of failure
 *                                   (See OIPF DAE spec section 7.13.1.2 onChannelChangeError table) 
 * 
 * @return true in success, otherwise false
 */
virtual bool Broadcast_SetChannel(
  std::shared_ptr<Channel> channel, 
  bool trickplay, 
  std::string contentAccessDescriptorURL, 
  bool quiet,
  int *channelChangeError
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
 * Select the specified component of the currently tuned broadcast channel.
 * 
 * @param componentType The component type (0: video, 1: audio, 2: subtitle)
 * @param pid           The component's pid used as identifier
 */
virtual void Broadcast_SelectComponent(int componentType, int pid) = 0;

/**
 * Unselect any currently selected component of the given type for the 
 * currently tuned broadcast channel.
 * 
 * @param componentType The componentType (0: video, 1: audio, 2: subtitle)
 */
virtual void Broadcast_UnselectComponent(int componentType) = 0;

/**
 * Suspend/resume the presentation of the current broadcast playback.
 * 
 * @param presentationSuspended Set to true to suspend, otherwise set to false to resume
 */
virtual void Broadcast_SetPresentationSuspended(bool presentationSuspended) = 0;

/**
 * Stop the current broadcast playback and call the Event_OnBroadcastStopped event.
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
 * @return A comma-separated set of languages to be used for the user interface 
 *         of a service, in order of preference. Each language shall be indicated 
 *         by its ISO 639-2 language code as defined in [ISO639-2].
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
 * Get the DTT network ids.
 * 
 * @return Vector containing the DTT network ids.
 */
virtual std::vector<int> Configuration_GetDttNetworkIds() = 0;

/**
 * Get the device identifier.
 * 
 * @return The device identifier
 */
virtual std::string Configuration_GetDeviceId() = 0;

/**
 * Called when the application at origin requests access to the distinctive identifier.
 * 
 * @param origin The origin of the application
 * 
 * @return true if access already granted, false otherwise
 */
virtual bool Configuration_RequestAccessToDistinctiveIdentifier(std::string origin) = 0;



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
virtual bool Dsmcc_SubscribeToStreamEventByName(std::string url, std::string name, int listenId) = 0;

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
virtual bool Dsmcc_SubscribeStreamEventId(std::string name, int componentTag, int eventId, int listenId) = 0;

/**
 * Unsubscribe from all previously establishe stream event subscriptions with the DSM-CC implementation.
 *
 * @param listenId The reference id of the subscriber
 */
virtual void Dsmcc_UnsubscribeFromStreamEvents(int listenId) = 0;



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
virtual std::map<std::string, std::vector<ParentalRating>> ParentalControl_GetRatingSchemes() = 0;

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
virtual bool ParentalControl_IsRatingBlocked(std::string scheme, std::string region, int value) = 0;



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

}; // class ORBPlatform


typedef ORBPlatform *CreatePlatformInstance_t();
typedef void DestroyPlatformInstance_t(void*);


#endif // ORB_PLATFORM_H
