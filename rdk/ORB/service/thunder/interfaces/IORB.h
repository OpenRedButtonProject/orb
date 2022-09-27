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

        struct INotification : virtual public Core::IUnknown {
            enum { ID = ID_ORB_NOTIFICATION};
            
            // events
            virtual void JavaScriptEventDispatchRequest(
                std::string name,
                std::string properties,
                bool broadcastRelated,
                std::string targetOrigin
            );

            virtual void DvbUrlLoaded(
                int requestId,
                uint8_t* fileContent,
                uint32_t fileContentLength
            );

            virtual void EventInputKeyGenerated(int keyCode);

            virtual ~INotification() {}
        };

        virtual ~IORB() {}
        
        // clients registration
        virtual void Register(INotification* sink) = 0;
        virtual void Unregister(INotification* sink) = 0;
        
        // methods 
        virtual std::string ExecuteBridgeRequest(std::string request);
        virtual std::string CreateToken(std::string uri);
        virtual void LoadDvbUrl(std::string url, int requestId);
        virtual bool SendKeyEvent(int keyCode);
        virtual void NotifyApplicationPageChanged(std::string url);
        virtual void NotifyApplicationLoadFailed(std::string url, std::string errorDescription);

    };

} // Exchange
} // WPEFramework

#endif //__IORB_H