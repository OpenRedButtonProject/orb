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

#include "Module.h"
#include <interfaces/json/JsonData_ORB.h>
#include <interfaces/IORB.h>
#include <memory>

#include "ORBComRpcServer.h"

namespace WPEFramework {
namespace Plugin {
using namespace JsonData::ORB;

/**
 * @brief orb::ORB
 *
 * Implementation of the ORB plugin
 */
class ORB
    : public PluginHost::IPlugin
    , public PluginHost::JSONRPC {
/**
 * @brief ORB::Notification
 *
 * Used to receive activation/deactivation events.
 */
    class Notification : public RPC::IRemoteConnection::INotification
    {
private:
        Notification() = delete;
        Notification(const Notification &) = delete;
        Notification &operator=(const Notification &) = delete;

public:

        explicit Notification(ORB *parent) : _parent(*parent)
        {
            ASSERT(parent != nullptr);
        }

        virtual ~Notification() override
        {
        }

public:
        void Activated(RPC::IRemoteConnection *) override
        {
        }

        void Deactivated(RPC::IRemoteConnection *connection) override
        {
            _parent.Deactivated(connection);
        }

        BEGIN_INTERFACE_MAP(Notification)
        INTERFACE_ENTRY(RPC::IRemoteConnection::INotification)
        END_INTERFACE_MAP
private:
        ORB &_parent;
    }; // class Notification

public:

    ORB();
    ~ORB() override;

    // Dont allow copy/move constructors
    ORB(const ORB &) = delete;
    ORB &operator=(const ORB &) = delete;

    /**
     * Singleton.
     */
    static ORB* instance(ORB *orb = nullptr)
    {
        static ORB *orb_instance;
        if (orb != nullptr)
        {
            orb_instance = orb;
        }
        return orb_instance;
    }

    BEGIN_INTERFACE_MAP(ORB)
    INTERFACE_ENTRY(PluginHost::IPlugin)
    INTERFACE_ENTRY(PluginHost::IDispatcher)
    INTERFACE_AGGREGATE(Exchange::IORB, _orb)
    END_INTERFACE_MAP

public:

    // IPlugin methods
    virtual const string Initialize(PluginHost::IShell *service) override;
    virtual void Deinitialize(PluginHost::IShell *service) override;
    virtual string Information() const override;

private:

    void Deactivated(RPC::IRemoteConnection *connection);

    // JsonRpc
    void RegisterAll();
    void UnregisterAll();

    // JsonRpc methods
    uint32_t SendKeyEvent(const SendKeyEventParamsData& params, Core::JSON::Boolean& response);
    uint32_t SetPreferredUILanguage(Core::JSON::String preferredUiLanguage);
    uint32_t LaunchApplication(Core::JSON::String url, Core::JSON::Boolean& response);

private:

    // member variables
    PluginHost::IShell *_service;
    Exchange::IORB *_orb;
    Core::Sink<Notification> _notification;
    uint32_t _connectionId;

    // If set in the config, we should host our own COM-RPC server
    ORBComRpcServer *_rpcServer;
    Core::ProxyType<RPC::InvokeServer> _rpcEngine;
}; // class ORB
} // namespace Plugin
} // namespace WPEFramework
