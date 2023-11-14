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

#ifndef __IORB_H
#define __IORB_H

#include "Module.h"

namespace WPEFramework {
namespace Exchange {
struct EXTERNAL IORB : virtual public Core::IUnknown
{
    enum { ID = ID_ORB };

    /* @event */
    struct EXTERNAL INotification : virtual public Core::IUnknown
    {
        enum {ID = ID_ORB_NOTIFICATION};

        // events

        // @brief
        // @param name: The javascript event name (e.g. ChannelStatusChange)
        // @param properties: Properties of the event (e.g. onid)
        // @param broadcastRelated: True if event is related to broadcast
        // @param targetOrigin: The origin
        virtual void JavaScriptEventDispatchRequest(
            std::string name,
            std::string properties,
            bool broadcastRelated,
            std::string targetOrigin
            ) = 0;

        // @brief Event that signifies the successful load of the dvb url
        // @param requestId: The id for the dvb url request
        // @param fileContent: The content of the actual file
        // @param fileContentLength: The length of the file
        virtual void DvbUrlLoaded(
            int requestId,
            const uint8_t *fileContent /* @length:fileContentLength */,
            unsigned int fileContentLength
            ) = 0;

        // @brief Event that signifies the successful load of the dvb url
        // @param requestId: The id for the dvb url request
        // @param fileContentLength: The length of the file
        virtual void DvbUrlLoadedNoData(int requestId, unsigned int fileContentLength) = 0;

        // @brief Event that is fired when a key is pressed
        // @param keyCode: The keyCode that was generated
        // @param keyAction: The keyAction (0 = keyup , 1 = keydown)
        virtual void EventInputKeyGenerated(int keyCode, uint8_t keyAction) = 0;

        // @brief Event that is fired when the EXIT (or similar) button is pressed by the user
        virtual void ExitButtonPressed() = 0;

        virtual ~INotification()
        {
        }
    };

    virtual ~IORB()
    {
    }

    // clients registration for events
    virtual void Register(INotification *sink) = 0;
    virtual void Unregister(INotification *sink) = 0;

    // methods
    virtual bool LoadPlatform() = 0;
    virtual void UnLoadPlatform() = 0;
    virtual std::string ExecuteBridgeRequest(std::string request) = 0;
    virtual std::string CreateToken(std::string uri) = 0;
    virtual void NotifyApplicationLoadFailed(std::string url, std::string errorDescription) = 0;
    virtual void NotifyApplicationPageChanged(std::string url) = 0;
    virtual bool SendKeyEvent(int keyCode, uint8_t keyAction) = 0;
    virtual void LoadDvbUrl(std::string url, int requestId) = 0;
    virtual void SetPreferredUILanguage(std::string preferredUiLanguage) = 0;
    virtual std::string GetUserAgentString() = 0;
    virtual std::string GetCurrentAppUrl() = 0;
    virtual bool LaunchApplication(std::string url) = 0;

    // methods to trigger notifications
    virtual void JavaScriptEventDispatchRequest(
        std::string name,
        std::string properties,
        bool broadcastRelated,
        std::string targetOrigin
        ) = 0;

    virtual void DvbUrlLoaded(
        int requestId,
        const uint8_t *fileContent /* @length:fileContentLength */,
        unsigned int fileContentLength
        ) = 0;

    virtual void DvbUrlLoadedNoData(int requestId, unsigned int fileContentLength) = 0;

    virtual void EventInputKeyGenerated(int keyCode, uint8_t keyAction) = 0;

    virtual void ExitButtonPressed() = 0;
};
} // Exchange
} // WPEFramework

#endif //__IORB_H