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
        private:
            ORBImplementation *_orb;

    public:
        // We do not allow this plugin to be copied !!
        ORBImplementation(const ORBImplementation&) = delete;
        ORBImplementation& operator=(const ORBImplementation&) = delete;

        // interface methods
        virtual void Register(INotification* sink) override;
        virtual void Unregister(INotification* sink) override;
        virtual void LoadPlatform() override;

        virtual std::string ExecuteBridgeRequest(std::string request) override;
        virtual std::string CreateToken(std::string uri) override;
        virtual void NotifyApplicationLoadFailed(std::string url, std::string errorDescription) override;
        virtual void NotifyApplicationPageChanged(std::string url) override;
        virtual bool SendKeyEvent(int keyCode) override;
        virtual void LoadDvbUrl(std::string url, int requestId) override;

        BEGIN_INTERFACE_MAP(ORBImplementation)
        INTERFACE_ENTRY(Exchange::IORB)
        END_INTERFACE_MAP

    private:
        mutable Core::CriticalSection _adminLock;
        std::list<Exchange::IORB::INotification*> _notificationClients;

        std::shared_ptr<ORBEventListenerImpl> _orbEventListener;
    public:
    
        ORBImplementation();
        virtual ~ORBImplementation();
    };



} // namespace Plugin
} // namespace WPEFramework
