/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

#ifndef __IORB_H
#define __IORB_H

#include "Module.h"

namespace WPEFramework {
namespace Exchange {

    struct EXTERNAL IORB : virtual public Core::IUnknown {
        enum { ID = ID_ORB };

        /* @event */
        struct EXTERNAL INotification : virtual public Core::IUnknown {
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
                const uint8_t* fileContent /* @length:fileContentLength */, 
                unsigned int fileContentLength
            ) = 0;

            // @brief Event that is fired when a key is pressed
            // @param keyCode: The keyCode that was generated
            virtual void EventInputKeyGenerated(int keyCode) = 0;

            virtual ~INotification() {}
        };

        virtual ~IORB() {}
        
        // clients registration for events
        virtual void Register(INotification* sink) = 0;
        virtual void Unregister(INotification* sink) = 0;
        
        // methods 
        virtual void LoadPlatform() = 0;
        virtual void UnLoadPlatform() = 0;
        virtual std::string ExecuteBridgeRequest(std::string request) = 0;
        virtual std::string CreateToken(std::string uri) = 0;
        virtual void NotifyApplicationLoadFailed(std::string url, std::string errorDescription) = 0;
        virtual void NotifyApplicationPageChanged(std::string url) = 0;
        virtual bool SendKeyEvent(int keyCode) = 0;
        virtual void LoadDvbUrl(std::string url, int requestId) = 0;

        // methods to trigger notifications
        virtual void JavaScriptEventDispatchRequest(
            std::string name,
            std::string properties,
            bool broadcastRelated,
            std::string targetOrigin
        ) = 0;

        virtual void DvbUrlLoaded(
            int requestId,
            const uint8_t* fileContent /* @length:fileContentLength */, 
            unsigned int fileContentLength
        ) = 0;

        virtual void EventInputKeyGenerated(int keyCode) = 0;

    };

} // Exchange
} // WPEFramework

#endif //__IORB_H