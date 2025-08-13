// create class PlatformMock

#ifndef PLATFORM_ANDROID_H
#define PLATFORM_ANDROID_H

#include "IPlatform.h"
#include "OrbConstants.h"

namespace orb
{
class AndroidPlatform : public IPlatform
{
public:
    AndroidPlatform(ApplicationType apptype);
    ~AndroidPlatform();

    // Configuration API
    std::shared_ptr<Capabilities> Configuration_GetCapabilities() override;
    std::vector<AudioProfile> Configuration_GetAudioProfiles() override;
    std::vector<VideoProfile> Configuration_GetVideoProfiles() override;

    // Broadcast API
    void Broadcast_SetVideoRectangle(int x, int y, int width, int height) override;
    std::shared_ptr<Channel> Broadcast_GetCurrentChannel() override;
    std::vector<Channel> Broadcast_GetChannelList() override;
    virtual int Broadcast_SetChannelToCcid(std::string ccid, bool trickplay, std::string
        contentAccessDescriptorURL, int quiet) override;
    virtual int Broadcast_SetChannelToNull() override;
    virtual int Broadcast_SetChannelToTriplet(int idType, int onid, int tsid, int sid, int sourceID,
        std::string ipBroadcastID, bool trickplay, std::string contentAccessDescriptorURL, int
        quiet) override;
    virtual int Broadcast_SetChannelToDsd(std::string dsd, int sid, bool trickplay, std::string
        contentAccessDescriptorURL, int quiet) override;
    std::vector<Programme> Broadcast_GetProgrammes(std::string ccid) override;
    std::vector<Component> Broadcast_GetComponents(std::string ccid, int componentType) override;
    std::shared_ptr<Component> Broadcast_GetPrivateAudioComponent(std::string ccid) override;
    std::shared_ptr<Component> Broadcast_GetPrivateVideoComponent(std::string ccid) override;
    void Broadcast_OverrideComponentSelection(int componentType, std::string id) override;
    void Broadcast_RestoreComponentSelection(int componentType) override;
    void Broadcast_SetPresentationSuspended(bool presentationSuspended) override;
    void Broadcast_Stop() override;
    void Broadcast_Reset() override;

private:
    Channel GenerateChannel(int onid, int tsid, int sid, std::string name);

private:
    ApplicationType mAppType;
    std::shared_ptr<Channel> mCurrentChannel;
    std::vector<Channel> mChannelList;

};

} // namespace orb

#endif // PLATFORM_ANDROID_H