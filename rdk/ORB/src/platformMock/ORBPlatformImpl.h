/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

#pragma once

#include "ORBPlatform.h"

/**
 * Mock implementation of the ORB platform APIs
 */
class ORBPlatformImpl : public ORBPlatform {

public:

  ORBPlatformImpl();
  virtual ~ORBPlatformImpl();

  // Platform api
  virtual void Platform_Initialise() override;
  virtual void Platform_Finalise() override;

  // Application api
  virtual bool Application_Load(std::string url) override;
  virtual bool Application_SetVisible(bool visible) override;
  virtual bool Application_SendKeyEvent(int keyCode) override;
  
  // Network api
  virtual bool Network_IsConnectedToInternet() override;
  
  // Broadcast api
  virtual void Broadcast_SetVideoRectangle(int x, int y, int width, int height) override;
  virtual std::shared_ptr<Channel> Broadcast_GetCurrentChannel() override;
  virtual std::vector<Channel> Broadcast_GetChannelList() override;
  virtual bool Broadcast_SetChannel(std::shared_ptr<Channel> channel, bool trickplay, std::string contentAccessDescriptorURL, bool quiet, int *channelChangeError) override;
  virtual std::vector<Programme> Broadcast_GetProgrammes(std::string ccid) override;
  virtual std::vector<Component> Broadcast_GetComponents(std::string ccid, int componentType) override;
  virtual void Broadcast_SelectComponent(int componentType, int pid) override;
  virtual void Broadcast_UnselectComponent(int componentType) override;
  virtual void Broadcast_SetPresentationSuspended(bool presentationSuspended) override;
  virtual void Broadcast_Stop() override;
  virtual void Broadcast_Reset() override;

  // Configuration api
  virtual std::shared_ptr<LocalSystem> Configuration_GetLocalSystem() override;
  virtual std::string Configuration_GetPreferredAudioLanguage() override;
  virtual std::string Configuration_GetPreferredSubtitleLanguage() override;
  virtual std::string Configuration_GetPreferredUILanguage() override;
  virtual std::string Configuration_GetCountryId() override;
  virtual bool Configuration_GetSubtitlesEnabled() override;
  virtual bool Configuration_GetAudioDescriptionEnabled() override;
  virtual std::vector<int> Configuration_GetDttNetworkIds() override;
  virtual std::string Configuration_GetDeviceId() override;
  virtual bool Configuration_RequestAccessToDistinctiveIdentifier(std::string origin) override;
  
  // Dsmcc api
  virtual void Dsmcc_RequestFile(std::string url, int requestId) override;
  virtual bool Dsmcc_SubscribeToStreamEventByName(std::string url, std::string name, int listenId) override;
  virtual bool Dsmcc_SubscribeStreamEventId(std::string name, int componentTag, int eventId, int listenId) override;
  virtual void Dsmcc_UnsubscribeFromStreamEvents(int listenId) override;

  // Manager api
  virtual std::string Manager_GetKeyIcon(int keyCode) override;

  // ParentalControl api
  virtual int ParentalControl_GetAge() override;
  virtual std::string ParentalControl_GetRegion() override;
  virtual std::string ParentalControl_GetRegion3() override;
  virtual std::map<std::string, std::vector<ParentalRating>> ParentalControl_GetRatingSchemes() override;
  virtual std::shared_ptr<ParentalRating> ParentalControl_GetThreshold(std::string scheme) override;
  virtual bool ParentalControl_IsRatingBlocked(std::string scheme, std::string region, int value) override;

  // Programme api
  virtual std::vector<std::string> Programme_GetSiDescriptors(std::string ccid, std::string programmeId, int descriptorTag, int descriptorTagExtension, int privateDataSpecifier) override;

private:
  std::string toLower(const std::string& data);
};
