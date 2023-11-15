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

#include "ORB.h"
#include "ORBConfiguration.h"

namespace WPEFramework {
namespace Plugin {
SERVICE_REGISTRATION(ORB, 1, 0);

/**
 * Constructor
 */
ORB::ORB()
    : _service(nullptr)
    , _orb(nullptr)
    , _connectionId(0)
    , _notification(this)
{
    SYSLOG(Logging::Startup, (_T("ORB service instance constructed")));
}

/**
 * Destructor.
 */
ORB::~ORB()
{
    SYSLOG(Logging::Shutdown, (_T("ORB service instance destructed")));
}

/**
 * @brief ORB::Initialize
 *
 * Initialise the ORB plugin.
 *
 * @param service
 *
 * @return An empty string in success, or else an error message
 */
const string ORB::Initialize(PluginHost::IShell *service)
{
    string message;
    bool orbEngineStarted;

    ASSERT(_service == nullptr);
    ASSERT(_orb == nullptr);

    SYSLOG(Logging::Startup, (_T("ORB Initialisation started in process %d"),
                              Core::ProcessInfo().Id()));

    // Register Connection::Notification
    _service = service;
    _service->Register(&_notification);
    instance(this);

    _orb = service->Root<Exchange::IORB>(_connectionId, 2000, _T("ORBImplementation"));

    // Check if ORB plugin initialisation failed
    if (_orb != nullptr)
    {
        orbEngineStarted = _orb->LoadPlatform();
        if (orbEngineStarted)
        {
            RegisterAll();

            ORBConfiguration config;
            config.FromString(_service->ConfigLine());

            // start the comrpc server, in case it is set on config
            if (config.PrivateComRpcServer.Value() == true)
            {
                _rpcEngine = Core::ProxyType<RPC::InvokeServer>::Create(
                    &Core::IWorkerPool::Instance());
                _rpcServer = new ORBComRpcServer(Core::NodeId("/tmp/ORB"), _orb, service,
                    _service->ProxyStubPath(), _rpcEngine);

                if (_rpcServer->IsListening())
                {
                    SYSLOG(Logging::Startup, (_T("Successfully started COM-RPC server")));
                }
                else
                {
                    delete _rpcServer;
                    _rpcServer = nullptr;
                    _rpcEngine.Release();
                    SYSLOG(Logging::Error, (_T("Failed to start COM-RPC server")));

                    // return string for WPEFramework to print as error
                    message = "Failed to start COM-RPC server";
                }
            }
        }
    }
    else
    {
        SYSLOG(Logging::Error, (_T("ORB plugin could not be initialised")));
        _service->Unregister(&_notification);
        _service = nullptr;

        message = _T("ORB plugin could not be initialised");
    }

    // Reached successful initialisation
    SYSLOG(Logging::Startup, (_T("ORB Initialisation finished")));
    return message;
}

/**
 * @brief ORB::Deinitialize
 *
 * @param service
 */
void ORB::Deinitialize(PluginHost::IShell *service)
{
    ASSERT(_service == service);

    SYSLOG(Logging::Shutdown, (_T("ORB Deinitialisation started")));

    // Destroy our COM-RPC server if we started one
    if (_rpcServer != nullptr)
    {
        // release rpcserver and engine
        delete _rpcServer;
        _rpcServer = nullptr;
        _rpcEngine.Release();
    }

    if (_orb != nullptr)
    {
        _service->Unregister(&_notification);
        _orb->UnLoadPlatform();

        UnregisterAll();
        _orb->Release();
    }

    // Set everything back to default
    _connectionId = 0;
    _service = nullptr;
    _orb = nullptr;

    SYSLOG(Logging::Shutdown, (_T("ORB Deinitialisation finished")));
}

/**
 * @brief ORB::Information
 *
 * @return
 */
string ORB::Information() const
{
    return string();
}

/**
 * @brief ORB::Deactivated
 *
 * @param connection
 */
void ORB::Deactivated(RPC::IRemoteConnection *connection)
{
    SYSLOG(Logging::Notification, (_T("ORB Deactivation started")));
    if (connection->Id() == _connectionId)
    {
        ASSERT(_service != nullptr);
        Core::IWorkerPool::Instance().Submit(
            PluginHost::IShell::Job::Create(
                _service,
                PluginHost::IShell::DEACTIVATED,
                PluginHost::IShell::FAILURE
                )
            );
    }
    SYSLOG(Logging::Notification, (_T("ORB Deactivation finished")));
}
} // namespace Plugin
} // namespace WPEFramework
