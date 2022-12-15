/**
 * ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 *
 * Licensed under the ORB License that can be found in the LICENSE file at
 * the top level of this repository.
 */

#pragma once

#include "Module.h"
#include <interfaces/IORB.h>

#include "ORBGenericClient.h"

using namespace WPEFramework;

namespace orb
{
class ORBComRpcClient : public ORBGenericClient
{
    // We want do run our own custom code when the plugin raises a notification
    // Implement the INotification class to do what we want
    class NotificationHandler : public Exchange::IORB::INotification
    {
public:
        explicit NotificationHandler(ORBComRpcClient *parent) : _parent(*parent)
        {
            ASSERT(parent != nullptr);
        }

        virtual void JavaScriptEventDispatchRequest(
            std::string name,
            std::string properties,
            bool broadcastRelated,
            std::string targetOrigin
            ) override;

        virtual void DvbUrlLoaded(
            int requestId,
            const uint8_t *fileContent,
            unsigned int fileContentLength
            ) override;

        virtual void DvbUrlLoadedNoData(int requestId, unsigned int fileContentLength) override;

        virtual void EventInputKeyGenerated(int keyCode, uint8_t keyAction) override;

        // Must define an interface map since we are implementing an interface on the exchange
        // so Thunder knows what type we are
        BEGIN_INTERFACE_MAP(NotificationHandler)
        INTERFACE_ENTRY(Exchange::IORB::INotification)
        END_INTERFACE_MAP

private:
        ORBComRpcClient &_parent;
    };

public:
    // provide callbacks
    ORBComRpcClient(
        OnJavaScriptEventDispatchRequested_cb onJavaScriptEventDispatchRequested_cb,
        OnDvbUrlLoaded_cb onDvbUrlLoaded_cb,
        OnDvbUrlLoadedNoData_cb onDvbUrlLoadedNoData_cb,
        OnInputKeyGenerated_cb onInputKeyGenerated_cb
        );
    virtual ~ORBComRpcClient();

public:

    // ORBBrowser API
    std::string ExecuteBridgeRequest(std::string jsonRequest) override;
    std::string CreateToken(std::string uri) override;
    void LoadDvbUrl(std::string url, int requestId) override;
    void NotifyApplicationLoadFailed(std::string url, std::string errorDescription) override;
    void NotifyApplicationPageChanged(std::string url) override;
    std::string GetUserAgentString() override;
    std::string GetCurrentAppUrl() override;

    bool IsValid();

public:
    // Events subscription
    void SubscribeToJavaScriptEventDispatchRequestedEvent() override;
    void SubscribeToDvbUrlLoadedEvent() override;
    void SubscribeToDvbUrlLoadedNoDataEvent() override;
    void SubscribeToInputKeyGeneratedEvent() override;

    void UnsubscribeFromJavaScriptEventDispatchRequestedEvent() override;
    void UnsubscribeFromDvbUrlLoadedEvent() override;
    void UnsubscribeFromDvbUrlLoadedNoDataEvent() override;
    void UnsubscribeFromInputKeyGeneratedEvent() override;

private:
    Core::NodeId GetConnectionEndpoint();

private:
    Core::NodeId m_remoteConnection;

    // An engine that can serialize/deserialize the COMRPC messages. Can be configured
    // to tune performance:
    // 1 = Number of threads allocated to this connection
    // 0 = Stack size per thread
    // 4 = Message slots. 4 which means that if 4 messages have
    // been queued, the submission of the 5th element will be a
    // blocking call until there is a free slot again
    Core::ProxyType<RPC::InvokeServerType<1, 0, 4> > m_engine;

    Core::ProxyType<RPC::CommunicatorClient> m_client;

    // The implementation of the ISamplePlugin interface - this could be an in-process, out-of-process
    // or distributed plugin, but as a client we don't need to worry about that
    Exchange::IORB *_orb;
    PluginHost::IShell *m_controller;

    // used to store if valid after connection
    bool m_valid;

    // Instance of our notification handler
    Core::Sink<NotificationHandler> m_notification;

    // keep subscribed events to filter
    std::map<std::string, bool> m_subscribedEvents;
}; // class ORBComRpcClient
}  // namespace orb