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

#include <string>
#include <vector>
#include "ParentalRating.h"
#include "DrmSystemStatus.h"

namespace orb
{
/**
 * Enumerate the available input key actions.
 */
enum KeyAction
{
    KEY_ACTION_UP   = 0x00,
    KEY_ACTION_DOWN = 0x01
};

/**
 * Enumerate the types of DRM errors
 */
enum DrmRightsError
{
    DRM_NO_LICENSE      = 0x00,// (decimal 0) no license, consumption of the content is blocked
    DRM_INVALID_LICENSE = 0x01, // (decimal 1) invalid license, consumption of the content is blocked
    DRM_VALID_LICENSE   = 0x02 // (decimal 2) valid license, consumption of the content is unblocked
};

enum SendDrmMessageResultCode
{
    SEND_DRM_MESSAGE_RESULT_SUCCESSFUL             = 0x00,// (decimal 0) Successful
    SEND_DRM_MESSAGE_RESULT_UNKNOWN_ERROR          = 0x01,// (decimal 1) Unknown error
    SEND_DRM_MESSAGE_RESULT_CANNOT_PROCESS_REQUEST = 0x02, // (decimal 2) Cannot process request
    SEND_DRM_MESSAGE_RESULT_UNKNOWN_MIME_TYPE      = 0x03,// (decimal 3) Unknown MIME type
    SEND_DRM_MESSAGE_RESULT_USER_CONSENT_NEEDED    = 0x04,// (decimal 4) User consent needed
    SEND_DRM_MESSAGE_RESULT_UNKNOWN_DRM_SYSTEM     = 0x05,// (decimal 5) Unknown DRM system
    SEND_DRM_MESSAGE_RESULT_WRONG_FORMAT           = 0x06 // (decimal 6) Wrong format
};

/**
 * Interface of the ORB platform event handler. The ORB platform implementation is expected to
 * properly call the methods of this interface as to notify the HbbTV application manager embedded
 * in ORB core, and/or the JavaScript layer (i.e. the HbbTV app) of platform-specific events.
 */
class ORBPlatformEventHandler
{
public:

    /**
     * Destructor.
     */
    virtual ~ORBPlatformEventHandler()
    {
    }

public:

    /**
     * Notify the application manager that the broadcast playback has stopped.
     */
    virtual void OnBroadcastStopped() = 0;

    /**
     * Notify the application manager that an AIT section was received.
     *
     * @param aitPid             The AID PID
     * @param serviceId          The corresponding service id
     * @param aitSectionData     The AIT section data
     * @param aitSectionDataSize The AIT section data size in number of bytes
     */
    virtual void OnAitSectionReceived(unsigned short aitPid, unsigned short serviceId, unsigned
        char *aitSectionData, unsigned int aitSectionDataSize) = 0;

    /**
     * Notify the application manager that the current channel's status has changed.
     * Also dispatch the ChannelStatusChanged bridge event to the current page's JavaScript context.
     *
     * The channel status (statusCode) must be set to one of the predefined values in Channel.h.
     *
     * @param onetId         The original network id
     * @param transId        The transport stream id
     * @param servId         The service id
     * @param statusCode     The channel status code (value from Channel::Status or Channel::ErrorState)
     * @param permanentError Permanent error indicator
     */
    virtual void OnChannelStatusChanged(int onetId, int transId, int servId, int statusCode, bool
        permanentError) = 0;

    /**
     * Dispatch the ParentalRatingChange bridge event to the current page's JavaScript context.
     *
     * @param blocked Indicates if the current service is blocked by the parental control system
     */
    virtual void OnParentalRatingChanged(bool blocked) = 0;

    /**
     * Dispatch the ParentalRatingError bridge event to the current page's JavaScript context.
     *
     * @param contentId   Content ID to which the parental rating error applies
     * @param ratings     The parental rating value of the currently playing content
     * @param drmSystemId DRM System ID of the DRM system that generated the event
     */
    virtual void OnParentalRatingError(std::string contentId, std::vector<ParentalRating> ratings,
        std::string drmSystemId) = 0;

    /**
     * Dispatch the SelectedComponentChanged bridge event to the current page's JavaScript context.
     *
     * @param componentType The component type (0: video, 1: audio, 2: subtitle)
     */
    virtual void OnSelectedComponentChanged(int componentType) = 0;

    /**
     * Dispatch the ComponentChanged bridge event to the current page's JavaScript context.
     *
     * @param componentType The component type (0: video, 1: audio, 2: subtitle)
     */
    virtual void OnComponentChanged(int componentType) = 0;

    /**
     * Dispatch the ProgrammesChanged bridge event to the current page's JavaScript context.
     */
    virtual void OnProgrammesChanged() = 0;

    /**
     * Dispatch the LowMemory bridge event to the current page's JavaScript context.
     */
    virtual void OnLowMemory() = 0;

    /**
     * Dispatch the accesstodistinctiveidentifier bridge event to the current page's JavaScript context.
     *
     * @param origin        The origin of the requesting application
     * @param accessAllowed True if access allowed, false otherwise
     */
    virtual void OnAccessToDistinctiveIdentifierDecided(std::string origin, bool accessAllowed) = 0;

    /**
     * Dispatch the TransitionedToBroadcastRelated bridge event to the current page's JavaScript context.
     */
    virtual void OnAppTransitionedToBroadcastRelated() = 0;

    /**
     * Dispatch the StreamEvent bridge event to the current page's JavaScript context.
     *
     * @param id     The stream event id
     * @param name   The stream event name
     * @param data   The stream event data encoded in Hexadecimal
     * @param text   The stream event data encoded in UTF-8
     * @param status The stream event status
     */
    virtual void OnStreamEvent(int id, std::string name, std::string data, std::string text,
        std::string status) = 0;

    /**
     * Notify all subscribers that the specified DVB URL load has finished.
     *
     * @param requestId         The request identifier
     * @param fileContent       The file content
     * @param fileContentLength The file content length in number of bytes
     */
    virtual void OnDvbUrlLoaded(int requestId, std::vector<uint8_t> fileContent, unsigned int
        fileContentLength) = 0;

    /**
     * Notify all subscribers that the specified DVB URL load has finished.
     *
     * @param requestId         The request identifier
     * @param fileContentLength The file content length in number of bytes
     */
    virtual void OnDvbUrlLoadedNoData(int requestId, unsigned int fileContentLength) = 0;

    /**
     * Notify the browser that the specified input key was generated.
     *
     * @param keyCode   The JavaScript key code
     * @param keyAction The JavaScript key action (0 = keyup , 1 = keydown)
     */
    virtual bool OnInputKeyGenerated(int keyCode, KeyAction keyAction) = 0;

    /**
     * Notify the browser about DRM licensing errors during playback of DRM protected A/V content.
     *
     * @param errorState      Details the type of error
     * @param contentId       Unique identifier of the protected content
     * @param drmSystemId     ID of the DRM system
     * @param rightsIssuerUrl Indicates the value of the rightsIssuerURL that can be used to
     *                        non-silently obtain the rights for the content item
     */
    virtual void OnDrmRightsError(
        DrmRightsError errorState,
        std::string contentId,
        std::string drmSystemId,
        std::string rightsIssuerUrl
        ) = 0;

    /**
     * Notify the browser about a change in the status of a DRM system.
     *
     * @param drmSystem          ID of the DRM system
     * @param drmSystemIds       List of the DRM System IDs handled by the DRM System
     * @param status             Status of the indicated DRM system
     * @param protectionGateways Space-separated list of zero or more CSP Gateway types that are
     *                           capable of supporting the DRM system
     * @param supportedFormats   Space separated list of zero or more supported file and/or
     *                           container formats by the DRM system
     */
    virtual void OnDrmSystemStatusChanged(
        std::string drmSystem,
        std::vector<std::string> drmSystemIds,
        DrmSystemStatus::State status,
        std::string protectionGateways,
        std::string supportedFormats
        ) = 0;

    /**
     * Notify the browser that the underlying DRM system has a result message as a consequence
     * of a call to Drm_SendDrmMessage.
     *
     * @param messageId  Identifies the original message which has led to this resulting message
     * @param result     DRM system specific result message
     * @param resultCode Result code
     */
    virtual void OnSendDrmMessageResult(
        std::string messageId,
        std::string result,
        SendDrmMessageResultCode resultCode
        ) = 0;

    /**
     * Notify the browser that the underlying DRM system has a message to report.
     *
     * @param message     DRM system specific message
     * @param drmSystemId ID of the DRM System
     */
    virtual void OnDrmSystemMessage(std::string message, std::string drmSystemId) = 0;
}; // class ORBPlatformEventHandler
} // namespace orb
