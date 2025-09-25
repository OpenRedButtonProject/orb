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
 *
 * Mock ApplicationSessionCallback for testing
 */

#ifndef MOCK_APPLICATION_SESSION_CALLBACK_H
#define MOCK_APPLICATION_SESSION_CALLBACK_H

#include "third_party/orb/orblibrary/moderator/app_mgr/application_session_callback.h"
#include "third_party/orb/orblibrary/moderator/app_mgr/utils.h"

namespace orb
{

class MockApplicationSessionCallback : public ApplicationSessionCallback
{
public:
    MockApplicationSessionCallback() = default;
    virtual ~MockApplicationSessionCallback() = default;

    // ApplicationSessionCallback interface
    void LoadApplication(const int appId, const char *entryUrl) override
    {
        // Mock implementation - do nothing
    }

    void LoadApplication(const int appId, const char *entryUrl, int size, const std::vector<uint16_t> graphics) override
    {
        // Mock implementation - do nothing
    }

    void ShowApplication(const int appId) override
    {
        // Mock implementation - do nothing
    }

    void HideApplication(const int appId) override
    {
        // Mock implementation - do nothing
    }

    void StopBroadcast() override
    {
        // Mock implementation - do nothing
    }

    void ResetBroadcastPresentation() override
    {
        // Mock implementation - do nothing
    }

    void DispatchApplicationLoadErrorEvent() override
    {
        // Mock implementation - do nothing
    }

    void DispatchTransitionedToBroadcastRelatedEvent(const int appId) override
    {
        // Mock implementation - do nothing
    }

    std::string GetXmlAitContents(const std::string &url) override
    {
        // Mock implementation - return empty string
        return "";
    }

    int GetParentalControlAge() override
    {
        return 18; // Mock age
    }

    std::string GetParentalControlRegion() override
    {
        return "GB"; // Mock region
    }

    std::string GetParentalControlRegion3() override
    {
        return "GBR"; // Mock region3
    }

    void DispatchApplicationSchemeUpdatedEvent(const int appId, const std::string &scheme) override
    {
        // Mock implementation - do nothing
    }

    void DispatchOperatorApplicationStateChange(const int appId, const std::string &oldState, const std::string &newState) override
    {
        // Mock implementation - do nothing
    }

    void DispatchOperatorApplicationStateChangeCompleted(const int appId, const std::string &oldState, const std::string &newState) override
    {
        // Mock implementation - do nothing
    }

    void DispatchOperatorApplicationContextChange(const int appId, const std::string &startupLocation, const std::string &launchLocation) override
    {
        // Mock implementation - do nothing
    }

    void DispatchOpAppUpdate(const int appId, const std::string &updateEvent) override
    {
        // Mock implementation - do nothing
    }

    bool isInstanceInCurrentService(const Utils::S_DVB_TRIPLET &triplet) override
    {
        // Mock implementation - return false
        return false;
    }
};

} // namespace orb

#endif // MOCK_APPLICATION_SESSION_CALLBACK_H
