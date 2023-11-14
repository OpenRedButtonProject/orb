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
#include <interfaces/IORB.h>

using namespace WPEFramework;

/**
 * A custom COM-RPC server implementation for a private connection to our plugin
 * instead of going via Thunder communicator
 */
class ORBComRpcServer : public RPC::Communicator
{
public:

    ORBComRpcServer() = delete;
    ORBComRpcServer(const ORBComRpcServer&) = delete;
    ORBComRpcServer& operator=(const ORBComRpcServer&) = delete;

    ORBComRpcServer(const Core::NodeId& socket,
        Exchange::IORB *parentInterface,
        PluginHost::IShell *shell,
        const string& proxyStubPath,
        const Core::ProxyType<RPC::InvokeServer>& engine);

    ~ORBComRpcServer();

private:

    // If a client wants to acquire an interface, provide it to them
    void* Aquire(const string& className, const uint32_t interfaceId, const uint32_t
        versionId) override;

private:

    Exchange::IORB *_orb;
    PluginHost::IShell *_shell;
}; // class ORBComRpcServer