/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

#include "ORBPlatformMockImpl.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>
#include <cctype>
#include <algorithm>

using namespace orb;

#define SEARCH_STATUS_COMPLETED 0
#define SEARCH_STATUS_ABORTED 3
#define SEARCH_STATUS_NO_RESOURCE 4

#define SIMPLE_FILE_NAME strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__

#define ORB_LOG(msg, ...) do \
   { \
      fprintf(stderr, "ORBPlatformMockImpl [%s]::[%s]::[%d] ", SIMPLE_FILE_NAME, __FUNCTION__, __LINE__); \
      fprintf(stderr, msg, ##__VA_ARGS__); \
      fprintf(stderr, "\n"); \
   } \
   while (0)


#define ORB_LOG_NO_ARGS() do \
   { \
      fprintf(stderr, "ORBPlatformMockImpl [%s]::[%s]::[%d]\n", SIMPLE_FILE_NAME, __FUNCTION__, __LINE__); \
   } \
   while (0)

// Video rectangle
int s_VideoRectangleX;
int s_VideoRectangleY;
int s_VideoRectangleW;
int s_VideoRectangleH;

std::shared_ptr<Channel> s_CurrentChannel;

// Currently selected components
int s_SelectedComponent_Pid_Video;
int s_SelectedComponent_Pid_Audio;
int s_SelectedComponent_Pid_Subtitle;

bool s_BroadcastPresentationSuspended;
bool s_DsmccStarted;

ORBPlatformMockImpl::ORBPlatformMockImpl()
   : m_dvb(std::make_shared<DVB>())
{
   ORB_LOG_NO_ARGS();
}

ORBPlatformMockImpl::~ORBPlatformMockImpl()
{
   ORB_LOG_NO_ARGS();
}

/**
 * Perform any platform-specific initialisation tasks.
 */
void ORBPlatformMockImpl::Platform_Initialise(std::shared_ptr<ORBPlatformEventHandler> platformEventHandler)
{
   ORB_LOG_NO_ARGS();

   m_platformEventHandler = platformEventHandler;
   m_dvb->Initialise();

   s_VideoRectangleX = 0;
   s_VideoRectangleY = 0;
   s_VideoRectangleW = 0;
   s_VideoRectangleH = 0;

   s_CurrentChannel = std::make_shared<Channel>();

   s_SelectedComponent_Pid_Video = 0;
   s_SelectedComponent_Pid_Audio = 0;
   s_SelectedComponent_Pid_Subtitle = 0;

   s_BroadcastPresentationSuspended = false;
   s_DsmccStarted = false;
}

/**
 * Perform any platform-specific finalisation tasks.
 */
void ORBPlatformMockImpl::Platform_Finalise()
{
   ORB_LOG_NO_ARGS();
   m_dvb->Finalise();
}

/**
 * Map the given, potentially platform-specific key code into the proper, HbbTV-compliant value.
 *
 * @param keyCode The key code to be mapped
 *
 * @return The mapped value
 */
unsigned int ORBPlatformMockImpl::Platform_MapKeyCode(unsigned int keyCode)
{
   ORB_LOG("keyCode=%u", keyCode);
   return 0;
}

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
bool ORBPlatformMockImpl::Application_Load(std::string url)
{
   ORB_LOG("url=%s", url.c_str());
   return true;
}

/**
 * Set the visibility of the current HbbTV application (if any).
 *
 * @param visible Set to true to show the application, or false to hide the application
 *
 * @return true in success, false otherwise
 */
bool ORBPlatformMockImpl::Application_SetVisible(bool visible)
{
   ORB_LOG("visible=%s", visible ? "yes" : "no");
   return true;
}

/******************************************************************************
** Network API
*****************************************************************************/



/**
 * Check if the device is currently connected to the Internet.
 *
 * @return true if connected, false otherwise
 */
bool ORBPlatformMockImpl::Network_IsConnectedToInternet()
{
   ORB_LOG_NO_ARGS();
   return true;
}

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
void ORBPlatformMockImpl::Broadcast_SetVideoRectangle(int x, int y, int width, int height)
{
   ORB_LOG("x=%d y=%d w=%d h=%d", x, y, width, height);
   s_VideoRectangleX = x;
   s_VideoRectangleY = y;
   s_VideoRectangleW = width;
   s_VideoRectangleH = height;
}

/**
 * Get the currently tuned broadcast channel.
 * If there is no currently tuned channel, then the returned Channel entity
 * shall have an empty ccid.
 *
 * @return The current channel
 */
std::shared_ptr<Channel> ORBPlatformMockImpl::Broadcast_GetCurrentChannel()
{
   ORB_LOG_NO_ARGS();
   return s_CurrentChannel;
}

/**
 * Get the scanned channel list.
 *
 * @return A vector with the scanned channels
 */
std::vector<Channel> ORBPlatformMockImpl::Broadcast_GetChannelList()
{
   ORB_LOG_NO_ARGS();
   return m_dvb->GetChannels();
}

/**
 * Tune to the specified broadcast channel using ccid.
 *
 * @param ccid                       The ccid of the channel
 * @param trickplay                  <currently not supported>
 * @param contentAccessDescriptorURL <currently not supported>
 * @param quiet                      <currently not supported>
 * @param errorState                 Channel error state code, set only in case of failure
 *
 * @return true in success, otherwise false
 */
bool ORBPlatformMockImpl::Broadcast_SetChannelToCcid(
   std::string ccid,
   bool trickplay,
   std::string contentAccessDescriptorURL,
   int quiet,
   Channel::ErrorState *errorState
   )
{
   // TODO Implement me
   return false;
}

/**
 * Tune off channel.
 *
 * @param trickplay                  <currently not supported>
 * @param contentAccessDescriptorURL <currently not supported>
 * @param quiet                      <currently not supported>
 * @param errorState                 Channel error state code, set only in case of failure
 *
 * @return true in success, otherwise false
 */
bool ORBPlatformMockImpl::Broadcast_SetChannelToNull(
   bool trickplay,
   std::string contentAccessDescriptorURL,
   int quiet,
   Channel::ErrorState *errorState
   )
{
   // TODO Implement me
   return false;
}

/**
 * Tune to the specified broadcast channel using dvb triplet.
 *
 * @param idType                     The channel type
 * @param onid                       Channel onid
 * @param tsid                       Channel tsid
 * @param sid                        Channel sid
 * @param sourceID                   Channel source id
 * @param ipBroadcastID              Channel ip broadcast id
 * @param trickplay                  <currently not supported>
 * @param contentAccessDescriptorURL <currently not supported>
 * @param quiet                      <currently not supported>
 * @param errorState                 Channel error state code, set only in case of failure
 *
 * @return true in success, otherwise false
 */
bool ORBPlatformMockImpl::Broadcast_SetChannelToTriplet(
   int idType,
   int onid,
   int tsid,
   int sid,
   int sourceID,
   std::string ipBroadcastID,
   bool trickplay,
   std::string contentAccessDescriptorURL,
   int quiet,
   Channel::ErrorState *errorState
   )
{
   // TODO Implement me
   return false;
}

/**
 * Tune to the specified broadcast channel using delivery system descriptor.
 *
 * @param dsd                        The channel delivery system descriptor
 * @param sid                        Channel sid
 * @param trickplay                  <currently not supported>
 * @param contentAccessDescriptorURL <currently not supported>
 * @param quiet                      <currently not supported>
 * @param errorState                 Channel error state code, set only in case of failure
 *
 * @return true in success, otherwise false
 */
bool ORBPlatformMockImpl::Broadcast_SetChannelToDsd(
   std::string dsd,
   int sid,
   bool trickplay,
   std::string contentAccessDescriptorURL,
   int quiet,
   Channel::ErrorState *errorState
   )
{
   // TODO Implement me
   return false;
}

/**
 * Get the programmes of the channel identified by the given ccid.
 *
 * @param ccid The channel ccid
 *
 * @return A vector with the channel programmes
 */
std::vector<Programme> ORBPlatformMockImpl::Broadcast_GetProgrammes(std::string ccid)
{
   ORB_LOG("ccid=%s", ccid.c_str());
   return m_dvb->GetProgrammes(ccid);
}

/**
 * Get the components of the channel identified by the given ccid.
 *
 * @param ccid          The channel ccid
 * @param componentType Component filter (-1: any, 0: video, 1: audio, 2: subtitle)
 *
 * @return A vector with the matching channel components
 */
std::vector<Component> ORBPlatformMockImpl::Broadcast_GetComponents(std::string ccid, int componentType)
{
   ORB_LOG("ccid=%s componentType=%d", ccid.c_str(), componentType);
   std::vector<Component> components;
   return components;
}

/**
 * Override the default component selection of the terminal for the specified type.
 *
 * The component in the stream that has the specified PID, CTAG (if specified), and language (if
 * specified) shall be selected. If pidOrSuspended equals 0, no component for the specified type
 * shall be selected for presentation.
 *
 * Default component selection shall be restored for the specified type when
 * restoreDefaultComponentSelection is called, the channel is changed, the application
 * terminates, or the user selects a different track of the same type in the terminal UI.
 *
 * Security: FOR_BROADCAST_APP_ONLY
 *
 * @param componentType  The component type (0: video, 1: audio, 2: subtitle)
 * @param pidOrSuspended The component PID or 0 to suspend presentation
 * @param ctag           The component tag or 0 if not specified
 * @param language       The component language or an empty string if not specified
 */
void ORBPlatformMockImpl::Broadcast_OverrideDefaultComponentSelection(
   int componentType,
   int pidOrSuspended,
   int ctag,
   std::string language
   )
{
   ORB_LOG("componentType=%d pidOrSuspended=%d", componentType, pidOrSuspended);
   switch (componentType)
   {
      case 0:
         s_SelectedComponent_Pid_Video = pidOrSuspended;
         break;
      case 1:
         s_SelectedComponent_Pid_Audio = pidOrSuspended;
         break;
      case 2:
         s_SelectedComponent_Pid_Subtitle = pidOrSuspended;
         break;
      default:
         break;
   }
   m_platformEventHandler->OnComponentChanged(componentType);
   m_platformEventHandler->OnSelectedComponentChanged(componentType);
}

/**
 * Restore the default component selection of the terminal for the specified type.
 *
 * If playback has already started, the presented component shall be updated.
 *
 * Security: FOR_BROADCAST_APP_ONLY
 *
 * @param componentType The component type (0: video, 1: audio, 2: subtitle)
 */
void ORBPlatformMockImpl::Broadcast_RestoreDefaultComponentSelection(int componentType)
{
   ORB_LOG("componentType=%d", componentType);
   switch (componentType)
   {
      case 0:
         s_SelectedComponent_Pid_Video = 0;
         break;
      case 1:
         s_SelectedComponent_Pid_Audio = 0;
         break;
      case 2:
         s_SelectedComponent_Pid_Subtitle = 0;
         break;
      default:
         break;
   }
   m_platformEventHandler->OnComponentChanged(componentType);
   m_platformEventHandler->OnSelectedComponentChanged(componentType);
}

/**
 * Suspend/resume the presentation of the current broadcast playback.
 *
 * @param presentationSuspended Set to true to suspend, otherwise set to false to resume
 */
void ORBPlatformMockImpl::Broadcast_SetPresentationSuspended(bool presentationSuspended)
{
   ORB_LOG("presentationSuspended=%s", presentationSuspended ? "yes" : "no");
   s_BroadcastPresentationSuspended = presentationSuspended;
}

/**
 * Stop the current broadcast playback.
 */
void ORBPlatformMockImpl::Broadcast_Stop()
{
   ORB_LOG_NO_ARGS();
   m_platformEventHandler->OnBroadcastStopped();
}

/**
 * Reset the current broadcast playback to its original settings.
 */
void ORBPlatformMockImpl::Broadcast_Reset()
{
   ORB_LOG_NO_ARGS();
   s_VideoRectangleX = 0;
   s_VideoRectangleY = 0;
   s_VideoRectangleW = 1280;
   s_VideoRectangleH = 720;

   Broadcast_SetPresentationSuspended(false);
}

/******************************************************************************
** Configuration API
*****************************************************************************/



/**
 * Get local system information.
 *
 * @return Pointer to the LocalSystem object
 */
std::shared_ptr<LocalSystem> ORBPlatformMockImpl::Configuration_GetLocalSystem()
{
   ORB_LOG_NO_ARGS();

   std::shared_ptr<LocalSystem> localSystem = std::make_shared<LocalSystem>(
      "OBS", "Mock", "1.0", "1.0"
      );
   return localSystem;
}

/**
 * Get the preferred audio language.
 *
 * @return A comma-separated set of languages to be used for audio playback,
 *         in order of preference.Each language shall be indicated by its
 *         ISO 639-2 language code as defined in [ISO639-2].
 */
std::string ORBPlatformMockImpl::Configuration_GetPreferredAudioLanguage()
{
   ORB_LOG_NO_ARGS();
   return "eng,spa,gre";
}

/**
 * Get the preferred subtitle language.
 *
 * @return A comma-separated set of languages to be used for subtitle playback,
 *         in order of preference. Each language shall be indicated by its
 *         ISO 639-2 language code as defined in [ISO639-2] or as a wildcard
 *         specifier "***".
 */
std::string ORBPlatformMockImpl::Configuration_GetPreferredSubtitleLanguage()
{
   ORB_LOG_NO_ARGS();
   return "eng,spa,gre";
}

/**
 * Get the id of the country in which the receiver is deployed.
 *
 * @return An ISO-3166 three character country code identifying the country in
 *         which the receiver is deployed.
 */
std::string ORBPlatformMockImpl::Configuration_GetCountryId()
{
   ORB_LOG_NO_ARGS();
   return "GBR";
}

/**
 * Get the flag indicating whether the subtitles are enabled or not.
 *
 * @return true if subtitles are enabled, otherwise false
 */
bool ORBPlatformMockImpl::Configuration_GetSubtitlesEnabled()
{
   ORB_LOG_NO_ARGS();
   return true;
}

/**
 * Get the flag indicating whether the audio description is enabled or not.
 *
 * @return true if the audio description is enabled, otherwise false
 */
bool ORBPlatformMockImpl::Configuration_GetAudioDescriptionEnabled()
{
   ORB_LOG_NO_ARGS();
   return false;
}

/**
 * Get the device identifier.
 *
 * @return The device identifier
 */
std::string ORBPlatformMockImpl::Configuration_GetDeviceId()
{
   ORB_LOG_NO_ARGS();
   return "aDevice";
}

/**
 * Called when the application at origin requests access to the distinctive identifier.
 *
 * @param origin The origin of the application
 *
 * @return true if access already granted, false otherwise
 */
bool ORBPlatformMockImpl::Configuration_RequestAccessToDistinctiveIdentifier(std::string origin)
{
   ORB_LOG("origin=%s", origin.c_str());
   return true;
}

/******************************************************************************
** DSM-CC API
*****************************************************************************/



/**
 * Request the specified DVB file from the DSM-CC implementation.
 *
 * @param url       The URL of the requested DVB file
 * @param requestId The unique request identifier
 */
void ORBPlatformMockImpl::Dsmcc_RequestFile(std::string url, int requestId)
{
   ORB_LOG("url=%s requestId=%d", url.c_str(), requestId);

   std::string content = "<html><body style=\"background-color: #333333; color: #aaaaaa;\"><h1>DVB</h1></body></html>";
   m_platformEventHandler->OnDvbUrlLoaded(requestId, (unsigned char *)content.c_str(), content.length());
}

/**
 * Request notifications from the DSM-CC implementation when a named stream event occurs.
 *
 * @param url      The stream URL
 * @param name     The stream event name
 * @param listenId The reference id of the subscriber
 *
 * @return true in success, false otherwise
 */
bool ORBPlatformMockImpl::Dsmcc_SubscribeToStreamEventByName(std::string url, std::string name, int listenId)
{
   ORB_LOG("url=%s name=%s listenId=%d", url.c_str(), name.c_str(), listenId);
   return true;
}

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
bool ORBPlatformMockImpl::Dsmcc_SubscribeStreamEventId(std::string name, int componentTag, int eventId, int listenId)
{
   ORB_LOG("name=%s componentTag=%d eventId=%d listenId=%d", name.c_str(), componentTag, eventId, listenId);
   return true;
}

/**
 * Unsubscribe from all previously establishe stream event subscriptions with the DSM-CC implementation.
 *
 * @param listenId The reference id of the subscriber
 */
void ORBPlatformMockImpl::Dsmcc_UnsubscribeFromStreamEvents(int listenId)
{
   ORB_LOG("listenId=%d", listenId);
}

/******************************************************************************
** Manager API
*****************************************************************************/



/**
 * Get the location of the icon file that corresponds to the given input key code.
 *
 * @param keyCode The input key code
 *
 * @return The location of the icon file or an empty string if there is no such file
 */
std::string ORBPlatformMockImpl::Manager_GetKeyIcon(int keyCode)
{
   ORB_LOG("keyCode=%d", keyCode);
   return "";
}

/******************************************************************************
** ParentalControl API
*****************************************************************************/



/**
 * Return the current age set for parental control. 0 will be returned if parental control is
 * disabled or no age is set.
 *
 * @return The currently set parental control age
 */
int ORBPlatformMockImpl::ParentalControl_GetAge()
{
   return 18;
}

/**
 * Return the region set for parental control.
 *
 * @return The region country using the 3-character code as specified in ISO 3166
 */
std::string ORBPlatformMockImpl::ParentalControl_GetRegion()
{
   return "GB";
}

/**
 * Return the region set for parental control.
 *
 * @return The region country using the 3-character code as specified in ISO 3166
 */
std::string ORBPlatformMockImpl::ParentalControl_GetRegion3()
{
   return "GBR";
}

/**
 * Get the rating schemes supported by the system.
 *
 * @return The rating schemes
 */
std::map<std::string, std::vector<ParentalRating> > ORBPlatformMockImpl::ParentalControl_GetRatingSchemes()
{
   std::map<std::string, std::vector<ParentalRating> > schemes;
   std::vector<ParentalRating> ratings;
   for (int i = 4; i < 18; i++)
   {
      ParentalRating rating(std::to_string(i), "dvb-si", "gbr", i, 0);
      ratings.push_back(rating);
   }
   schemes["dvb-si"] = ratings;
   return schemes;
}

/**
 * Get the parental control threshold for the given parental rating scheme.
 *
 * @param scheme The parental rating scheme
 *
 * @return A ParentalRating object representing the parental control threshold
 */
std::shared_ptr<ParentalRating> ORBPlatformMockImpl::ParentalControl_GetThreshold(std::string scheme)
{
   std::shared_ptr<ParentalRating> threshold = std::make_shared<ParentalRating>(
      "18", "dvb-si", "gb", 18, 0
      );
   return threshold;
}

/**
 * Retrieve the blocked property for the provided parental rating.
 *
 * @param scheme The parental rating scheme
 * @param region The parental rating 2-character region
 * @param value  The parental rating control age value
 *
 * @return The blocked property
 */
bool ORBPlatformMockImpl::ParentalControl_IsRatingBlocked(std::string scheme, std::string region, int value)
{
   bool blocked = true;

   std::string thresholdRegion = ToLower(ParentalControl_GetRegion());
   int thresholdAge = ParentalControl_GetAge();

   if (scheme == "dvb-si")
   {
      if (thresholdRegion == ToLower(region) && thresholdAge > value + 3)
      {
         blocked = false;
      }
   }

   return blocked;
}

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
std::vector<std::string> ORBPlatformMockImpl::Programme_GetSiDescriptors(
   std::string ccid,
   std::string programmeId,
   int descriptorTag,
   int descriptorTagExtension,
   int privateDataSpecifier
   )
{
   ORB_LOG("ccid=%s programmeId=%s descriptorTag=%d descriptorTagExtension=%d privateDataSpecifier=%d",
      ccid.c_str(), programmeId.c_str(), descriptorTag, descriptorTagExtension, privateDataSpecifier);
   std::vector<std::string> siDescriptors;
   return siDescriptors;
}

/**
 * @brief Convert the provided string to lowercase
 *
 * @param[in] data  string to convert to lowercase
 *
 * @return  The lowercase version of the provided string
 */
std::string ORBPlatformMockImpl::ToLower(const std::string &data)
{
   std::string tmp = data;
   std::transform(tmp.begin(), tmp.end(), tmp.begin(), [](unsigned char c) {
      return std::tolower(c);
   });
   return tmp;
}

extern "C"
ORBPlatform* Create()
{
   return new ORBPlatformMockImpl();
}

extern "C"
void Destroy(ORBPlatform *platform)
{
   delete reinterpret_cast<ORBPlatformMockImpl *>(platform);
}
