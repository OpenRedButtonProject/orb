/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2020 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include "Module.h"
#include <interfaces/IORB.h>
#include "tracing/Logging.h"
#include "ORBEngine.h"
#include "ORBEventListenerImpl.h"

namespace WPEFramework {
namespace Plugin {

    class ORBImplementation : public Exchange::IORB {

    public:
        ORBImplementation();
        ~ORBImplementation() override;
        
        /**
        * Singleton.
        * It is used to receive an instance of the ORBImplementation, to 
        * have access on dispatch event methods
        */
        static ORBImplementation* instance(ORBImplementation *orb = nullptr)
        {
            static ORBImplementation *implementation_instance;
            if (orb != nullptr)
            {
                fprintf(stderr,"[ORB] Setting the singleton\n");
                implementation_instance = orb;
            }
            return implementation_instance;
        }

        // We do not allow this plugin to be copied !!
        ORBImplementation(const ORBImplementation&) = delete;
        ORBImplementation& operator=(const ORBImplementation&) = delete;

        BEGIN_INTERFACE_MAP(ORBImplementation)
        INTERFACE_ENTRY(Exchange::IORB)
        END_INTERFACE_MAP
    
    public:  
        // interface methods
        void Register(INotification *sink) override;
        void Unregister(INotification* sink) override;
        void LoadPlatform() override;
        void UnLoadPlatform() override;

        std::string ExecuteBridgeRequest(std::string request) override;
        std::string CreateToken(std::string uri) override;
        void NotifyApplicationLoadFailed(std::string url, std::string errorDescription) override;
        void NotifyApplicationPageChanged(std::string url) override;
        bool SendKeyEvent(int keyCode) override;
        void LoadDvbUrl(std::string url, int requestId) override;

        // methods to be called when events need to fire
        void JavaScriptEventDispatchRequest(
            std::string name,
            std::string properties,
            bool broadcastRelated,
            std::string targetOrigin
        ) override;

        void DvbUrlLoaded(
            int requestId,
            const uint8_t* fileContent, 
            const uint16_t fileContentLength
        ) override {};

        void EventInputKeyGenerated(int keyCode) override {};

    private:
        mutable Core::CriticalSection _adminLock;
        std::list<Exchange::IORB::INotification*> _notificationClients;

        std::shared_ptr<ORBEventListenerImpl> _orbEventListener;
        std::mutex _notificationMutex;
    };



} // namespace Plugin
} // namespace WPEFramework
