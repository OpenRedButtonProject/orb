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

#include <memory>
#include <string>
#include "ORBBrowserApi.h"

typedef void (*OnJavaScriptEventDispatchRequested_cb)(std::string name, std::string properties);
typedef void (*OnDvbUrlLoaded_cb)(int requestId, unsigned char *content, unsigned int
    contentLength);
typedef void (*OnDvbUrlLoadedNoData_cb)(int requestId, unsigned int contentLength);
typedef void (*OnInputKeyGenerated_cb)(int keyCode, unsigned char keyAction);
typedef void (*OnExitButtonPressed_cb)();

namespace orb
{
/**
 * Interface of the ORB client.
 */
class ORBGenericClient : public ORBBrowserApi
{
public:

    ORBGenericClient(
        OnJavaScriptEventDispatchRequested_cb onJavaScriptEventDispatchRequested_cb,
        OnDvbUrlLoaded_cb onDvbUrlLoaded_cb,
        OnDvbUrlLoadedNoData_cb onDvbUrlLoadedNoData_cb,
        OnInputKeyGenerated_cb onInputKeyGenerated_cb,
        OnExitButtonPressed_cb onExitButtonPressed_cb
        )
    {
        m_onJavaScriptEventDispatchRequested = onJavaScriptEventDispatchRequested_cb;
        m_onDvbUrlLoaded = onDvbUrlLoaded_cb;
        m_onDvbUrlLoadedNoData = onDvbUrlLoadedNoData_cb;
        m_onInputKeyGenerated = onInputKeyGenerated_cb;
        m_onExitButtonPressed = onExitButtonPressed_cb;
    }

    virtual ~ORBGenericClient()
    {
    }

public:

    // ORBBrowserApi

    virtual std::string ExecuteBridgeRequest(std::string jsonRequest) = 0;
    virtual std::string CreateToken(std::string uri) = 0;
    virtual void LoadDvbUrl(std::string url, int requestId) = 0;
    virtual void NotifyApplicationLoadFailed(std::string url, std::string errorDescription) = 0;
    virtual void NotifyApplicationPageChanged(std::string url) = 0;
    virtual std::string GetUserAgentString() = 0;
    virtual std::string GetCurrentAppUrl() = 0;

public:
    // Events subscription
    virtual void SubscribeToJavaScriptEventDispatchRequestedEvent() = 0;
    virtual void SubscribeToDvbUrlLoadedEvent() = 0;
    virtual void SubscribeToDvbUrlLoadedNoDataEvent() = 0;
    virtual void SubscribeToInputKeyGeneratedEvent() = 0;
    virtual void SubscribeToExitButtonPressedEvent() = 0;

    virtual void UnsubscribeFromJavaScriptEventDispatchRequestedEvent() = 0;
    virtual void UnsubscribeFromDvbUrlLoadedEvent() = 0;
    virtual void UnsubscribeFromDvbUrlLoadedNoDataEvent() = 0;
    virtual void UnsubscribeFromInputKeyGeneratedEvent() = 0;
    virtual void UnsubscribeFromExitButtonPressedEvent() = 0;

protected:
    // callbacks
    OnJavaScriptEventDispatchRequested_cb m_onJavaScriptEventDispatchRequested;
    OnDvbUrlLoaded_cb m_onDvbUrlLoaded;
    OnDvbUrlLoadedNoData_cb m_onDvbUrlLoadedNoData;
    OnInputKeyGenerated_cb m_onInputKeyGenerated;
    OnExitButtonPressed_cb m_onExitButtonPressed;
}; // class ORBGenericClient


/**
 * Create a new ORB client instance.
 *
 * @param onJavaScriptEventDispatchRequested_cb The OnJavaScriptEventDispatchRequested callback
 * @param onDvbUrlLoaded_cb                     The OnDvbUrlLoaded callback
 * @param onDvbUrlLoadedNoData_cb               The OnDvbUrlLoadedNoData callback
 * @param onInputKeyGenerated_cb                The OnInputKeyGenerated callback
 * @param onExitButtonPressed_cb                The OnExitButtonPressed callback
 *
 * @return Pointer to the new ORB client instance
 */
std::shared_ptr<ORBGenericClient> CreateORBClient(
    OnJavaScriptEventDispatchRequested_cb onJavaScriptEventDispatchRequested_cb,
    OnDvbUrlLoaded_cb onDvbUrlLoaded_cb,
    OnDvbUrlLoadedNoData_cb onDvbUrlLoadedNoData_cb,
    OnInputKeyGenerated_cb onInputKeyGenerated_cb,
    OnExitButtonPressed_cb OnExitButtonPressed_cb
    );
} // namespace orb
