/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

#pragma once

#include "Module.h"
#include <interfaces/IORB.h>

using namespace WPEFramework;

/**
 * A custom COM-RPC server implementation for a private connection to our plugin
 * instead of going via Thunder communicator
*/
class ORBComRpcServer : public RPC::Communicator {
public:
    ORBComRpcServer() = delete;
    ORBComRpcServer(const ORBComRpcServer&) = delete;
    ORBComRpcServer& operator=(const ORBComRpcServer&) = delete;

    ORBComRpcServer(const Core::NodeId& socket,
        Exchange::IORB* parentInterface,
        PluginHost::IShell* shell,
        const string& proxyStubPath,
        const Core::ProxyType<RPC::InvokeServer>& engine);
    ~ORBComRpcServer();

private:
    // If a client wants to acquire an interface, provide it to them
    void* Aquire(const string& className, const uint32_t interfaceId, const uint32_t versionId) override;

private:
    Exchange::IORB* _orb;
    PluginHost::IShell* _shell;
};