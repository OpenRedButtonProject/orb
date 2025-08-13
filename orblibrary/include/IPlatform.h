#ifndef PLATFORM_H
#define PLATFORM_H

#include <memory>
#include <cstdint>
#include <string>

#include "Capabilities.h"
#include "Channel.h"
#include "Programme.h"
#include "Component.h"

namespace orb
{

class IPlatform {
public:

    /**
     * Destructor.
     */
    virtual ~IPlatform() {}

    /******************************************************************************
    ** Configuration API
    *****************************************************************************/

    /**
     * Get the capabilities of the platform.
     *
     * @return The capabilities of the platform
     */
    virtual std::shared_ptr<Capabilities> Configuration_GetCapabilities() = 0;

    /**
     * Get the audio profiles of the platform.
     *
     * @return The audio profiles of the platform
     */
    virtual std::vector<AudioProfile> Configuration_GetAudioProfiles() = 0;

    /**
     * Get the video profiles of the platform.
     *
     * @return The video profiles of the platform
     */
    virtual std::vector<VideoProfile> Configuration_GetVideoProfiles() = 0;

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

};

}
#endif // PLATFORM_H