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

#pragma once

#include "ORBPlatform.h"
#include "DVB.h"

/**
 * Mock implementation of the ORB platform APIs
 */
class ORBPlatformMockImpl : public ORBPlatform {
public:

    ORBPlatformMockImpl();
    virtual ~ORBPlatformMockImpl();

    // Platform api
    virtual void Platform_Initialise(
        std::shared_ptr<ORBPlatformEventHandler> platformEventHandler) override;
    virtual void Platform_Finalise() override;
    virtual unsigned int Platform_MapKeyCode(unsigned int keyCode) override;
    virtual void Platform_SetCurrentKeySetMask(uint16_t keySetMask) override;
    virtual bool Platform_IsExitButton(unsigned int keyCode) override;

    // Application api
    virtual bool Application_Load(std::string url) override;
    virtual bool Application_SetVisible(bool visible) override;

    // Network api
    virtual bool Network_IsConnectedToInternet() override;

    virtual std::string Network_ResolveNetworkError(std::string responseText) override;

    // Broadcast api
    virtual void Broadcast_SetVideoRectangle(int x, int y, int width, int height) override;
    virtual std::shared_ptr<Channel> Broadcast_GetCurrentChannel() override;
    virtual std::vector<Channel> Broadcast_GetChannelList() override;
    virtual int Broadcast_SetChannelToCcid(std::string ccid, bool trickplay, std::string
        contentAccessDescriptorURL, int quiet) override;
    virtual int Broadcast_SetChannelToNull() override;
    virtual int Broadcast_SetChannelToTriplet(int idType, int onid, int tsid, int sid, int sourceID,
        std::string ipBroadcastID, bool trickplay, std::string contentAccessDescriptorURL, int
        quiet) override;
    virtual int Broadcast_SetChannelToDsd(std::string dsd, int sid, bool trickplay, std::string
        contentAccessDescriptorURL, int quiet) override;
    virtual std::vector<Programme> Broadcast_GetProgrammes(std::string ccid) override;
    virtual std::vector<Component> Broadcast_GetComponents(std::string ccid, int
        componentType) override;
    virtual std::shared_ptr<Component> Broadcast_GetPrivateAudioComponent(std::string
        componentTag) override;
    virtual std::shared_ptr<Component> Broadcast_GetPrivateVideoComponent(std::string
        componentTag) override;
    virtual void Broadcast_OverrideComponentSelection(int componentType, std::string id) override;
    virtual void Broadcast_RestoreComponentSelection(int componentType) override;
    virtual void Broadcast_SetPresentationSuspended(bool presentationSuspended) override;
    virtual void Broadcast_Stop() override;
    virtual void Broadcast_Reset() override;

    // Configuration api
    virtual std::shared_ptr<Capabilities> Configuration_GetCapabilities() override;
    virtual std::vector<AudioProfile> Configuration_GetAudioProfiles() override;
    virtual std::vector<VideoProfile> Configuration_GetVideoProfiles() override;
    virtual std::vector<VideoDisplayFormat> Configuration_GetVideoDisplayFormats() override;
    virtual int Configuration_GetExtraSDVideoDecodes() override;
    virtual int Configuration_GetExtraHDVideoDecodes() override;
    virtual int Configuration_GetExtraUHDVideoDecodes() override;
    virtual std::shared_ptr<LocalSystem> Configuration_GetLocalSystem() override;
    virtual std::string Configuration_GetPreferredAudioLanguage() override;
    virtual std::string Configuration_GetPreferredSubtitleLanguage() override;
    virtual std::string Configuration_GetPreferredUILanguage() override;
    virtual std::string Configuration_GetCountryId() override;
    virtual bool Configuration_GetSubtitlesEnabled() override;
    virtual bool Configuration_GetAudioDescriptionEnabled() override;
    virtual std::string Configuration_GetDeviceId() override;
    virtual bool Configuration_RequestAccessToDistinctiveIdentifier(std::string origin,
        std::map<std::string, std::string> appNames) override;
    virtual std::string Configuration_GetUserAgentString() override;
#ifdef BBC_API_ENABLE
    virtual std::shared_ptr<DisplayInfo> Configuration_GetPrimaryDisplay() override;
#endif

    // Dsmcc api
    virtual void Dsmcc_RequestFile(std::string url, int requestId) override;
    virtual bool Dsmcc_SubscribeToStreamEventByName(std::string url, std::string name, int
        listenId) override;
    virtual bool Dsmcc_SubscribeStreamEventId(std::string name, int componentTag, int eventId, int
        listenId) override;
    virtual void Dsmcc_UnsubscribeFromStreamEvents(int listenId) override;
    virtual uint32_t Dsmcc_RequestCarouselId(uint32_t componentTag) override;

    // Manager api
    virtual std::string Manager_GetKeyIcon(int keyCode) override;

    // ParentalControl api
    virtual int ParentalControl_GetAge() override;
    virtual std::string ParentalControl_GetRegion() override;
    virtual std::string ParentalControl_GetRegion3() override;
    virtual std::map<std::string,
                     std::vector<ParentalRating> > ParentalControl_GetRatingSchemes() override;
    virtual std::shared_ptr<ParentalRating> ParentalControl_GetThreshold(std::string
        scheme) override;
    virtual bool ParentalControl_IsRatingBlocked(std::string scheme, std::string region, int
        value) override;

    // Programme api
    virtual std::vector<std::string> Programme_GetSiDescriptors(std::string ccid, std::string
        programmeId, int descriptorTag, int descriptorTagExtension, int
        privateDataSpecifier) override;

    // Drm api
    virtual std::vector<DrmSystemStatus> Drm_GetSupportedDrmSystemIds() override;
    virtual std::string Drm_SendDrmMessage(std::string messageId, std::string messageType,
        std::string message, std::string drmSystemId, bool blocked) override;
    virtual bool Drm_CanPlayContent(std::string drmPrivateData, std::string drmSystemId) override;
    virtual bool Drm_CanRecordContent(std::string drmPrivateData, std::string drmSystemId) override;
    virtual bool Drm_SetActiveDrm(std::string drmSystemId) override;

private:

    std::string ToLower(const std::string& data);

    // member variables
    std::shared_ptr<ORBPlatformEventHandler> m_platformEventHandler;
    std::shared_ptr<DVB> m_dvb;
};
