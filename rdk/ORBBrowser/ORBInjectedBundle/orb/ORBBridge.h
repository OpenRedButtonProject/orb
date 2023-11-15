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
#include <WPE/WebKit.h>
#include <core/core.h>
#include "OrbUtils.h"
#include "ORBGenericClient.h"

namespace orb {
class ORBBridge {
public:

    static ORBBridge& GetSharedInstance()
    {
        static ORBBridge s_ORBBridge;
        return s_ORBBridge;
    }

    ~ORBBridge();

    void DispatchEvent(std::string type, JsonObject properties, bool broadcastRelated, std::string
        targetOrigin);
    void SetJavaScriptContext(JSContextRef jsContextRef);
    void GenerateKey(int keyCode);

    void AddDsmccCaller(int requestId, void *caller);
    void AddDsmccCallback(int requestId, OnDvbUrlLoadedNoData callback);

    void* GetDsmccCaller(int requestId);
    OnDvbUrlLoadedNoData GetDsmccCallback(int requestId);

    void RemoveDsmccCaller(int requestId);
    void RemoveDsmccCallback(int requestId);

    std::shared_ptr<ORBGenericClient> GetORBClient()
    {
        return m_orbClient;
    }

private:

    ORBBridge();
    ORBBridge(ORBBridge const&) = delete;
    void operator=(ORBBridge const&) = delete;

    // member variables
    JSContextRef m_javaScriptContext;
    std::map<int, void *> m_dsmccCallers;
    std::map<int, OnDvbUrlLoadedNoData> m_dsmccCallbacks;
    std::shared_ptr<ORBGenericClient> m_orbClient;
}; // class ORBBridge
} // namespace orb
