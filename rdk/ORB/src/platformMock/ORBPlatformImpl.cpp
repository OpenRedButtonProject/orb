/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

#include "ORBPlatformImpl.h"
#include "ORBEvents.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>

using namespace orb;

#define SEARCH_STATUS_COMPLETED 0
#define SEARCH_STATUS_ABORTED 3
#define SEARCH_STATUS_NO_RESOURCE 4

#define SIMPLE_FILE_NAME strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__

#define ORB_LOG(msg, ...) do\
{\
  fprintf(stderr, "ORBPlatformImpl [%s]::[%s]::[%d] ", SIMPLE_FILE_NAME, __FUNCTION__, __LINE__);\
  fprintf(stderr, msg, ##__VA_ARGS__);\
  fprintf(stderr, "\n");\
}\
while (0)


#define ORB_LOG_NO_ARGS() do\
{\
  fprintf(stderr, "ORBPlatformImpl [%s]::[%s]::[%d]\n", SIMPLE_FILE_NAME, __FUNCTION__, __LINE__);\
}\
while (0)

// Hard-coded channel list
std::vector<std::string> s_ChannelList = {
  "{\"valid\":true,\"ccid\":\"ccid:800\",\"name\":\"ARDTest-CookieD\",\"dsd\":\"0\",\"ipBroadcastId\":\"0\",\"channelType\":0,\"idType\":12,\"majorChannel\":0,\"terminalChannel\":800,\"nid\":1,\"onid\":1,\"tsid\":65283,\"sid\":28703,\"hidden\":false,\"sourceId\":0}",
  "{\"valid\":true,\"ccid\":\"ccid:801\",\"name\":\"ARDTest-CookieNo\",\"dsd\":\"0\",\"ipBroadcastId\":\"0\",\"channelType\":0,\"idType\":12,\"majorChannel\":0,\"terminalChannel\":801,\"nid\":1,\"onid\":1,\"tsid\":65283,\"sid\":28705,\"hidden\":false,\"sourceId\":0}",
  "{\"valid\":true,\"ccid\":\"ccid:802\",\"name\":\"ARDTest-CookieW\",\"dsd\":\"0\",\"ipBroadcastId\":\"0\",\"channelType\":0,\"idType\":12,\"majorChannel\":0,\"terminalChannel\":802,\"nid\":1,\"onid\":1,\"tsid\":65283,\"sid\":28702,\"hidden\":false,\"sourceId\":0}",
  "{\"valid\":true,\"ccid\":\"ccid:803\",\"name\":\"ARDTest-DsmStart\",\"dsd\":\"0\",\"ipBroadcastId\":\"0\",\"channelType\":0,\"idType\":12,\"majorChannel\":0,\"terminalChannel\":803,\"nid\":1,\"onid\":1,\"tsid\":65283,\"sid\":28704,\"hidden\":false,\"sourceId\":0}",
  "{\"valid\":true,\"ccid\":\"ccid:804\",\"name\":\"ARDTest-LStorage1\",\"dsd\":\"0\",\"ipBroadcastId\":\"0\",\"channelType\":0,\"idType\":12,\"majorChannel\":0,\"terminalChannel\":804,\"nid\":1,\"onid\":1,\"tsid\":65283,\"sid\":28700,\"hidden\":false,\"sourceId\":0}",
  "{\"valid\":true,\"ccid\":\"ccid:805\",\"name\":\"ARDTest-LStorage2\",\"dsd\":\"0\",\"ipBroadcastId\":\"0\",\"channelType\":0,\"idType\":12,\"majorChannel\":0,\"terminalChannel\":805,\"nid\":1,\"onid\":1,\"tsid\":65283,\"sid\":28701,\"hidden\":false,\"sourceId\":0}",
  "{\"valid\":true,\"ccid\":\"ccid:806\",\"name\":\"HbbTV-AIT-update\",\"dsd\":\"0\",\"ipBroadcastId\":\"0\",\"channelType\":0,\"idType\":12,\"majorChannel\":0,\"terminalChannel\":806,\"nid\":1,\"onid\":1,\"tsid\":65283,\"sid\":28180,\"hidden\":false,\"sourceId\":0}",
  "{\"valid\":true,\"ccid\":\"ccid:807\",\"name\":\"HbbTV-ARD\",\"dsd\":\"0\",\"ipBroadcastId\":\"0\",\"channelType\":0,\"idType\":12,\"majorChannel\":0,\"terminalChannel\":807,\"nid\":1,\"onid\":1,\"tsid\":65283,\"sid\":28181,\"hidden\":false,\"sourceId\":0}",
  "{\"valid\":true,\"ccid\":\"ccid:808\",\"name\":\"HbbTV-ARDmediathek\",\"dsd\":\"0\",\"ipBroadcastId\":\"0\",\"channelType\":0,\"idType\":12,\"majorChannel\":0,\"terminalChannel\":808,\"nid\":1,\"onid\":1,\"tsid\":65283,\"sid\":5023,\"hidden\":false,\"sourceId\":0}",
  "{\"valid\":true,\"ccid\":\"ccid:809\",\"name\":\"HbbTV-Dashtest\",\"dsd\":\"0\",\"ipBroadcastId\":\"0\",\"channelType\":0,\"idType\":12,\"majorChannel\":0,\"terminalChannel\":809,\"nid\":1,\"onid\":1,\"tsid\":65283,\"sid\":28182,\"hidden\":false,\"sourceId\":0}",
  "{\"valid\":true,\"ccid\":\"ccid:810\",\"name\":\"HbbTV-HTTPS\",\"dsd\":\"0\",\"ipBroadcastId\":\"0\",\"channelType\":0,\"idType\":12,\"majorChannel\":0,\"terminalChannel\":810,\"nid\":1,\"onid\":1,\"tsid\":65283,\"sid\":28183,\"hidden\":false,\"sourceId\":0}",
  "{\"valid\":true,\"ccid\":\"ccid:811\",\"name\":\"HbbTV-KiKA\",\"dsd\":\"0\",\"ipBroadcastId\":\"0\",\"channelType\":0,\"idType\":12,\"majorChannel\":0,\"terminalChannel\":811,\"nid\":1,\"onid\":1,\"tsid\":65283,\"sid\":2817,\"hidden\":false,\"sourceId\":0}",
  "{\"valid\":true,\"ccid\":\"ccid:812\",\"name\":\"HbbTV-Large-AIT\",\"dsd\":\"0\",\"ipBroadcastId\":\"0\",\"channelType\":0,\"idType\":12,\"majorChannel\":0,\"terminalChannel\":812,\"nid\":1,\"onid\":1,\"tsid\":65283,\"sid\":5021,\"hidden\":false,\"sourceId\":0}",
  "{\"valid\":true,\"ccid\":\"ccid:813\",\"name\":\"HbbTV-MHP\",\"dsd\":\"0\",\"ipBroadcastId\":\"0\",\"channelType\":0,\"idType\":12,\"majorChannel\":0,\"terminalChannel\":813,\"nid\":1,\"onid\":1,\"tsid\":65283,\"sid\":28184,\"hidden\":false,\"sourceId\":0}",
  "{\"valid\":true,\"ccid\":\"ccid:814\",\"name\":\"HbbTV-MultiPID\",\"dsd\":\"0\",\"ipBroadcastId\":\"0\",\"channelType\":0,\"idType\":12,\"majorChannel\":0,\"terminalChannel\":814,\"nid\":1,\"onid\":1,\"tsid\":65283,\"sid\":28185,\"hidden\":false,\"sourceId\":0}",
  "{\"valid\":true,\"ccid\":\"ccid:815\",\"name\":\"HbbTV-Subtitles\",\"dsd\":\"0\",\"ipBroadcastId\":\"0\",\"channelType\":0,\"idType\":12,\"majorChannel\":0,\"terminalChannel\":815,\"nid\":1,\"onid\":1,\"tsid\":65283,\"sid\":28189,\"hidden\":false,\"sourceId\":0}",
  "{\"valid\":true,\"ccid\":\"ccid:816\",\"name\":\"HbbTV-Testsuite1\",\"dsd\":\"0\",\"ipBroadcastId\":\"0\",\"channelType\":0,\"idType\":12,\"majorChannel\":0,\"terminalChannel\":816,\"nid\":1,\"onid\":1,\"tsid\":65283,\"sid\":28186,\"hidden\":false,\"sourceId\":0}",
  "{\"valid\":true,\"ccid\":\"ccid:817\",\"name\":\"HbbTV-Testsuite2\",\"dsd\":\"0\",\"ipBroadcastId\":\"0\",\"channelType\":0,\"idType\":12,\"majorChannel\":0,\"terminalChannel\":817,\"nid\":1,\"onid\":1,\"tsid\":65283,\"sid\":28187,\"hidden\":false,\"sourceId\":0}",
  "{\"valid\":true,\"ccid\":\"ccid:818\",\"name\":\"HbbTV-VoD-KDG\",\"dsd\":\"0\",\"ipBroadcastId\":\"0\",\"channelType\":0,\"idType\":12,\"majorChannel\":0,\"terminalChannel\":818,\"nid\":1,\"onid\":1,\"tsid\":65283,\"sid\":28188,\"hidden\":false,\"sourceId\":0}"
};

// Hard-coded programmes for channel with ccid:816
std::vector<std::string> s_Programmes = {
  "{\"programmeID\":\"1\",\"programmeIDType\":1,\"name\":\"Event 1, umlaut ä\",\"description\":\"subtitle\",\"longDescription\":\"\",\"startTime\":1627483530,\"duration\":300,\"channelID\":\"ccid:816\",\"parentalRatings\":[]}",
  "{\"programmeID\":\"2\",\"programmeIDType\":1,\"name\":\"Event 2, umlaut ö\",\"description\":\"subtitle\",\"longDescription\":\"\",\"startTime\":1627483830,\"duration\":300,\"channelID\":\"ccid:816\",\"parentalRatings\":[]}",
  "{\"programmeID\":\"100\",\"programmeIDType\":1,\"name\":\"Event 3, umlaut ä\",\"description\":\"subtitle\",\"longDescription\":\"\",\"startTime\":1627484430,\"duration\":3600,\"channelID\":\"ccid:816\",\"parentalRatings\":[]}",
  "{\"programmeID\":\"99\",\"programmeIDType\":1,\"name\":\"Event -1, umlaut ä\",\"description\":\"subtitle\",\"longDescription\":\"\",\"startTime\":1627487130,\"duration\":3600,\"channelID\":\"ccid:816\",\"parentalRatings\":[]}",
  "{\"programmeID\":\"101\",\"programmeIDType\":1,\"name\":\"Event 4, umlaut ä\",\"description\":\"subtitle\",\"longDescription\":\"\",\"startTime\":1627488030,\"duration\":3600,\"channelID\":\"ccid:816\",\"parentalRatings\":[]}",
  "{\"programmeID\":\"98\",\"programmeIDType\":1,\"name\":\"Event -2, umlaut ä\",\"description\":\"subtitle\",\"longDescription\":\"\",\"startTime\":1627490730,\"duration\":3600,\"channelID\":\"ccid:816\",\"parentalRatings\":[]}",
  "{\"programmeID\":\"102\",\"programmeIDType\":1,\"name\":\"Event 5, umlaut ä\",\"description\":\"subtitle\",\"longDescription\":\"\",\"startTime\":1627491630,\"duration\":3600,\"channelID\":\"ccid:816\",\"parentalRatings\":[]}",
  "{\"programmeID\":\"103\",\"programmeIDType\":1,\"name\":\"Event 6, umlaut ä\",\"description\":\"subtitle\",\"longDescription\":\"\",\"startTime\":1627495230,\"duration\":3600,\"channelID\":\"ccid:816\",\"parentalRatings\":[]}",
  "{\"programmeID\":\"104\",\"programmeIDType\":1,\"name\":\"Event 7, umlaut ä\",\"description\":\"subtitle\",\"longDescription\":\"\",\"startTime\":1627498830,\"duration\":3600,\"channelID\":\"ccid:816\",\"parentalRatings\":[]}",
  "{\"programmeID\":\"105\",\"programmeIDType\":1,\"name\":\"Event 8, umlaut ä\",\"description\":\"subtitle\",\"longDescription\":\"\",\"startTime\":1627502430,\"duration\":3600,\"channelID\":\"ccid:816\",\"parentalRatings\":[]}",
  "{\"programmeID\":\"106\",\"programmeIDType\":1,\"name\":\"Event 9, umlaut ä\",\"description\":\"subtitle\",\"longDescription\":\"\",\"startTime\":1627506030,\"duration\":3600,\"channelID\":\"ccid:816\",\"parentalRatings\":[]}",
  "{\"programmeID\":\"107\",\"programmeIDType\":1,\"name\":\"Event 10, umlaut ä\",\"description\":\"subtitle\",\"longDescription\":\"\",\"startTime\":1627509630,\"duration\":3600,\"channelID\":\"ccid:816\",\"parentalRatings\":[]}",
  "{\"programmeID\":\"108\",\"programmeIDType\":1,\"name\":\"Event 11, umlaut ä\",\"description\":\"subtitle\",\"longDescription\":\"\",\"startTime\":1627513230,\"duration\":3600,\"channelID\":\"ccid:816\",\"parentalRatings\":[]}",
  "{\"programmeID\":\"109\",\"programmeIDType\":1,\"name\":\"Event 12, umlaut ä\",\"description\":\"subtitle\",\"longDescription\":\"\",\"startTime\":1627516830,\"duration\":3600,\"channelID\":\"ccid:816\",\"parentalRatings\":[]}",
  "{\"programmeID\":\"110\",\"programmeIDType\":1,\"name\":\"Event 13, umlaut ä\",\"description\":\"subtitle\",\"longDescription\":\"\",\"startTime\":1627520430,\"duration\":3600,\"channelID\":\"ccid:816\",\"parentalRatings\":[]}",
  "{\"programmeID\":\"111\",\"programmeIDType\":1,\"name\":\"Event 14, umlaut ä\",\"description\":\"subtitle\",\"longDescription\":\"\",\"startTime\":1627524030,\"duration\":3600,\"channelID\":\"ccid:816\",\"parentalRatings\":[]}",
  "{\"programmeID\":\"112\",\"programmeIDType\":1,\"name\":\"Event 15, umlaut ä\",\"description\":\"subtitle\",\"longDescription\":\"\",\"startTime\":1627527630,\"duration\":3600,\"channelID\":\"ccid:816\",\"parentalRatings\":[]}",
  "{\"programmeID\":\"113\",\"programmeIDType\":1,\"name\":\"Event 16, umlaut ä\",\"description\":\"subtitle\",\"longDescription\":\"\",\"startTime\":1627531230,\"duration\":3600,\"channelID\":\"ccid:816\",\"parentalRatings\":[]}",
  "{\"programmeID\":\"114\",\"programmeIDType\":1,\"name\":\"Event 17, umlaut ä\",\"description\":\"subtitle\",\"longDescription\":\"\",\"startTime\":1627534830,\"duration\":3600,\"channelID\":\"ccid:816\",\"parentalRatings\":[]}",
  "{\"programmeID\":\"115\",\"programmeIDType\":1,\"name\":\"Event 18, umlaut ä\",\"description\":\"subtitle\",\"longDescription\":\"\",\"startTime\":1627538430,\"duration\":3600,\"channelID\":\"ccid:816\",\"parentalRatings\":[]}",
  "{\"programmeID\":\"116\",\"programmeIDType\":1,\"name\":\"Event 19, umlaut ä\",\"description\":\"subtitle\",\"longDescription\":\"\",\"startTime\":1627542030,\"duration\":3600,\"channelID\":\"ccid:816\",\"parentalRatings\":[]}",
  "{\"programmeID\":\"117\",\"programmeIDType\":1,\"name\":\"Event 20, umlaut ä\",\"description\":\"subtitle\",\"longDescription\":\"\",\"startTime\":1627545630,\"duration\":3600,\"channelID\":\"ccid:816\",\"parentalRatings\":[]}",
  "{\"programmeID\":\"118\",\"programmeIDType\":1,\"name\":\"Event 21, umlaut ä\",\"description\":\"subtitle\",\"longDescription\":\"\",\"startTime\":1627549230,\"duration\":3600,\"channelID\":\"ccid:816\",\"parentalRatings\":[]}",
  "{\"programmeID\":\"119\",\"programmeIDType\":1,\"name\":\"Event 22, umlaut ä\",\"description\":\"subtitle\",\"longDescription\":\"\",\"startTime\":1627552830,\"duration\":3600,\"channelID\":\"ccid:816\",\"parentalRatings\":[]}",
  "{\"programmeID\":\"120\",\"programmeIDType\":1,\"name\":\"Event 23, umlaut ä\",\"description\":\"subtitle\",\"longDescription\":\"\",\"startTime\":1627556430,\"duration\":3600,\"channelID\":\"ccid:816\",\"parentalRatings\":[]}",
  "{\"programmeID\":\"121\",\"programmeIDType\":1,\"name\":\"Event 24, umlaut ä\",\"description\":\"subtitle\",\"longDescription\":\"\",\"startTime\":1627560030,\"duration\":3600,\"channelID\":\"ccid:816\",\"parentalRatings\":[]}"
};

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

ORBPlatformImpl::ORBPlatformImpl()
{
  ORB_LOG_NO_ARGS();
}

ORBPlatformImpl::~ORBPlatformImpl()
{
  ORB_LOG_NO_ARGS();
}

/**
 * Perform any platform-specific initialisation tasks.
 */
void ORBPlatformImpl::Platform_Initialise()
{
  ORB_LOG_NO_ARGS();
  
  s_VideoRectangleX = 0;
  s_VideoRectangleY = 0;
  s_VideoRectangleW = 0;
  s_VideoRectangleH = 0;

  s_CurrentChannel = std::make_shared<Channel>(
    false, "", "", "", "", 0, 0, 0, 0, 0, 0, 0, 0, false, 0
  );

  s_SelectedComponent_Pid_Video = 0;
  s_SelectedComponent_Pid_Audio = 0;
  s_SelectedComponent_Pid_Subtitle = 0;

  s_BroadcastPresentationSuspended = false;
  s_DsmccStarted = false;
}

/**
 * Perform any platform-specific finalisation tasks.
 */
void ORBPlatformImpl::Platform_Finalise()
{
  ORB_LOG_NO_ARGS();
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
bool ORBPlatformImpl::Application_Load(std::string url)
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
bool ORBPlatformImpl::Application_SetVisible(bool visible)
{
  ORB_LOG("visible=%s", visible ? "yes" : "no");
  return true;
}

/**
 * Send the specified input key event to the current HbbTV application (if any).
 *
 * @param keyCode The input key code
 *
 * @return true in success, false otherwise
 */
bool ORBPlatformImpl::Application_SendKeyEvent(int keyCode)
{
  ORB_LOG("keyCode=%d", keyCode);
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
bool ORBPlatformImpl::Network_IsConnectedToInternet()
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
void ORBPlatformImpl::Broadcast_SetVideoRectangle(int x, int y, int width, int height)
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
std::shared_ptr<Channel> ORBPlatformImpl::Broadcast_GetCurrentChannel()
{
  ORB_LOG_NO_ARGS();
  return s_CurrentChannel;
}

/**
 * Get the scanned channel list.
 * 
 * @return A vector with the scanned channels
 */
std::vector<Channel> ORBPlatformImpl::Broadcast_GetChannelList()
{
  ORB_LOG_NO_ARGS();
  std::vector<Channel> channelList;
  
  for (auto jsonChannelAsString : s_ChannelList) {
    std::shared_ptr<Channel> channel = Channel::FromJsonString(jsonChannelAsString);
    channelList.push_back(*(channel.get()));
  }

  return channelList;
}

/**
 * Tune to the specified broadcast channel.
 * 
 * @param channel                    The requested channel
 * @param trickplay                  <currently not supported>
 * @param contentAccessDescriptorURL <currently not supported>
 * @param quiet                      <currently not supported>
 * @param channelChangeError         Channel change error code, set only in case of failure
 *                                   (See OIPF DAE spec section 7.13.1.2 onChannelChangeError table) 
 * 
 * @return true in success, otherwise false
 */
bool ORBPlatformImpl::Broadcast_SetChannel(
  std::shared_ptr<Channel> channel, 
  bool trickplay, 
  std::string contentAccessDescriptorURL, 
  bool quiet,
  int *channelChangeError
)
{
  ORB_LOG("channel.ccid=%s trickplay=%s contentAccessDescriptorURL=%s quiet=%s",
    channel->GetCcid().c_str(),
    trickplay ? "yes" : "no",
    contentAccessDescriptorURL.c_str(),
    quiet ? "yes" : "no"
  );
  s_CurrentChannel = channel;
  return true;
}

/**
 * Get the programmes of the channel identified by the given ccid.
 * 
 * @param ccid The channel ccid
 * 
 * @return A vector with the channel programmes
 */
std::vector<Programme> ORBPlatformImpl::Broadcast_GetProgrammes(std::string ccid)
{
  ORB_LOG("ccid=%s", ccid.c_str());
  std::vector<Programme> programmes;

  if (ccid == "ccid:816") {
    for (auto jsonProgrammeAsString : s_Programmes) {
      std::shared_ptr<Programme> programme = Programme::FromJsonString(jsonProgrammeAsString);
      programmes.push_back(*(programme.get()));
    }
  }

  return programmes;
}

/**
 * Get the components of the channel identified by the given ccid.
 * 
 * @param ccid          The channel ccid
 * @param componentType Component filter (-1: any, 0: video, 1: audio, 2: subtitle)
 * 
 * @return A vector with the matching channel components
 */ 
std::vector<Component> ORBPlatformImpl::Broadcast_GetComponents(std::string ccid, int componentType)
{
  ORB_LOG("ccid=%s componentType=%d", ccid.c_str(), componentType);
  std::vector<Component> components;
  return components;
}

/**
 * Select the specified component of the currently tuned broadcast channel.
 * 
 * @param componentType The component type (0: video, 1: audio, 2: subtitle)
 * @param pid           The component's pid used as identifier
 */
void ORBPlatformImpl::Broadcast_SelectComponent(int componentType, int pid)
{
  ORB_LOG("componentType=%d pid=%d", componentType, pid);
  switch (componentType)
  {
    case 0:
      s_SelectedComponent_Pid_Video = pid;
      break;
    case 1:
      s_SelectedComponent_Pid_Audio = pid;
      break;
    case 2:
      s_SelectedComponent_Pid_Subtitle = pid;
      break;
    default:
      break;
  }
  Event_OnComponentChanged(componentType);
  Event_OnSelectedComponentChanged(componentType);
}

/**
 * Unselect any currently selected component of the given type for the 
 * currently tuned broadcast channel.
 * 
 * @param componentType The componentType (0: video, 1: audio, 2: subtitle)
 */
void ORBPlatformImpl::Broadcast_UnselectComponent(int componentType)
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
  Event_OnComponentChanged(componentType);
  Event_OnSelectedComponentChanged(componentType);
}

/**
 * Suspend/resume the presentation of the current broadcast playback.
 * 
 * @param presentationSuspended Set to true to suspend, otherwise set to false to resume
 */
void ORBPlatformImpl::Broadcast_SetPresentationSuspended(bool presentationSuspended)
{
  ORB_LOG("presentationSuspended=%s", presentationSuspended ? "yes" : "no");
  s_BroadcastPresentationSuspended = presentationSuspended;
}

/**
 * Stop the current broadcast playback.
 */
void ORBPlatformImpl::Broadcast_Stop()
{
  ORB_LOG_NO_ARGS();
  Event_OnBroadcastStopped();
}

/**
 * Reset the current broadcast playback to its original settings.
 */
void ORBPlatformImpl::Broadcast_Reset()
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
std::shared_ptr<LocalSystem> ORBPlatformImpl::Configuration_GetLocalSystem()
{
  ORB_LOG_NO_ARGS();

  std::shared_ptr<LocalSystem> localSystem = std::make_shared<LocalSystem>(
    true, "OBS", "Mock", "1.0", "1.0"
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
std::string ORBPlatformImpl::Configuration_GetPreferredAudioLanguage()
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
std::string ORBPlatformImpl::Configuration_GetPreferredSubtitleLanguage()
{
  ORB_LOG_NO_ARGS();
  return "eng,spa,gre";
}

/**
 * Get the preferred UI language.
 * 
 * @return A comma-separated set of languages to be used for the user interface 
 *         of a service, in order of preference. Each language shall be indicated 
 *         by its ISO 639-2 language code as defined in [ISO639-2].
 */
std::string ORBPlatformImpl::Configuration_GetPreferredUILanguage()
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
std::string ORBPlatformImpl::Configuration_GetCountryId()
{
  ORB_LOG_NO_ARGS();
  return "GBR";
}

/**
 * Get the flag indicating whether the subtitles are enabled or not.
 * 
 * @return true if subtitles are enabled, otherwise false
 */
bool ORBPlatformImpl::Configuration_GetSubtitlesEnabled()
{
  ORB_LOG_NO_ARGS();
  return true;
}

/**
 * Get the flag indicating whether the audio description is enabled or not.
 * 
 * @return true if the audio description is enabled, otherwise false
 */
bool ORBPlatformImpl::Configuration_GetAudioDescriptionEnabled()
{
  ORB_LOG_NO_ARGS();
  return false;
}

/**
 * Get the DTT network ids.
 * 
 * @return Vector containing the DTT network ids.
 */
std::vector<int> ORBPlatformImpl::Configuration_GetDttNetworkIds()
{
  ORB_LOG_NO_ARGS();
  std::vector<int> dttNetworkIds;
  return dttNetworkIds;
}

/**
 * Get the device identifier.
 * 
 * @return The device identifier
 */
std::string ORBPlatformImpl::Configuration_GetDeviceId()
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
bool ORBPlatformImpl::Configuration_RequestAccessToDistinctiveIdentifier(std::string origin)
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
void ORBPlatformImpl::Dsmcc_RequestFile(std::string url, int requestId)
{
  ORB_LOG("url=%s requestId=%d", url.c_str(), requestId);
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
bool ORBPlatformImpl::Dsmcc_SubscribeToStreamEventByName(std::string url, std::string name, int listenId)
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
bool ORBPlatformImpl::Dsmcc_SubscribeStreamEventId(std::string name, int componentTag, int eventId, int listenId)
{
  ORB_LOG("name=%s componentTag=%d eventId=%d listenId=%d", name.c_str(), componentTag, eventId, listenId);
  return true;
}

/**
 * Unsubscribe from all previously establishe stream event subscriptions with the DSM-CC implementation.
 *
 * @param listenId The reference id of the subscriber
 */
void ORBPlatformImpl::Dsmcc_UnsubscribeFromStreamEvents(int listenId)
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
std::string ORBPlatformImpl::Manager_GetKeyIcon(int keyCode)
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
int ORBPlatformImpl::ParentalControl_GetAge()
{
  return 18;
}



/**
 * Return the region set for parental control.
 *
 * @return The region country using the 3-character code as specified in ISO 3166
 */
std::string ORBPlatformImpl::ParentalControl_GetRegion()
{
  return "GB";
}



/**
 * Return the region set for parental control.
 *
 * @return The region country using the 3-character code as specified in ISO 3166
 */
std::string ORBPlatformImpl::ParentalControl_GetRegion3()
{
  return "GBR";
}

/**
 * Get the rating schemes supported by the system.
 *
 * @return The rating schemes
 */
std::map<std::string, std::vector<ParentalRating>> ORBPlatformImpl::ParentalControl_GetRatingSchemes()
{
  std::map<std::string, std::vector<ParentalRating>> schemes;
  std::vector<ParentalRating> ratings;
  for (int i = 4; i < 18; i++) {
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
std::shared_ptr<ParentalRating> ORBPlatformImpl::ParentalControl_GetThreshold(std::string scheme)
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
bool ORBPlatformImpl::ParentalControl_IsRatingBlocked(std::string scheme, std::string region, int value)
{
  bool blocked = true;

  std::string thresholdRegion = std::tolower(ParentalControl_GetRegion());
  int thresholdAge = ParentalControl_GetAge();

  if (scheme == "dvb-si") {
    if (thresholdRegion == std::tolower(region) && thresholdAge > value + 3) {
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
std::vector<std::string> ORBPlatformImpl::Programme_GetSiDescriptors(
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

extern "C"
ORBPlatform *Create()
{
  return new ORBPlatformImpl();
}

extern "C"
void Destroy(ORBPlatform *platform)
{
  delete reinterpret_cast<ORBPlatformImpl*>(platform);
}
